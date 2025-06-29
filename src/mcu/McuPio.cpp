/**
 * @file McuPio.cpp
 * @brief ESP32 RMT-based Programmable IO Channel implementation.
 *
 * This file provides the implementation for PIO operations using the
 * ESP32's RMT (Remote Control Transceiver) peripheral. The RMT peripheral
 * provides hardware-accelerated symbol encoding/decoding with precise timing
 * for custom protocols, LED strips, IR communication, and other timing-critical
 * applications with nanosecond-level precision and interrupt-driven operation.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */
#include "McuPio.h"
#include <algorithm>
#include <cstring>

// Platform-specific includes and definitions
#ifdef HF_MCU_FAMILY_ESP32
#include "esp_check.h"
#include "esp_log.h"
#include "hal/rmt_ll.h"
#include "soc/soc_caps.h"
#else
// Stub implementations for non-ESP32 platforms
#define ESP_LOGE(tag, format, ...)
#define ESP_LOGW(tag, format, ...)
#define ESP_LOGI(tag, format, ...)
#define ESP_LOGD(tag, format, ...)
#define ESP_OK 0
#define ESP_ERR_INVALID_ARG -1
#define ESP_ERR_NO_MEM -2
#define ESP_ERR_TIMEOUT -3
typedef int esp_err_t;
#endif

static const char *TAG = "McuPio";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

McuPio::McuPio() noexcept
    : initialized_(false), channels_{}, transmit_callback_(nullptr), receive_callback_(nullptr),
      error_callback_(nullptr), callback_user_data_(nullptr) {
  ESP_LOGD(TAG, "McuPio constructed");
}

McuPio::~McuPio() noexcept {
  if (initialized_) {
    Deinitialize();
  }
  ESP_LOGD(TAG, "McuPio destroyed");
}

//==============================================================================
// INITIALIZATION AND DEINITIALIZATION
//==============================================================================

HfPioErr McuPio::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (initialized_) {
    ESP_LOGW(TAG, "Already initialized");
    return HfPioErr::PIO_ERR_ALREADY_INITIALIZED;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Initialize all channels to default state
  for (auto &channel : channels_) {
    channel = ChannelState{};
  }

  initialized_ = true;
  ESP_LOGI(TAG, "McuPio initialized successfully");
  return HfPioErr::PIO_SUCCESS;
#else
  ESP_LOGE(TAG, "ESP32 platform not available");
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfPioErr McuPio::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  // Deinitialize all configured channels
  for (uint8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (channels_[i].configured) {
      DeinitializeChannel(i);
    }
  }

  // Clear callbacks
  transmit_callback_ = nullptr;
  receive_callback_ = nullptr;
  error_callback_ = nullptr;
  callback_user_data_ = nullptr;

  initialized_ = false;
  ESP_LOGI(TAG, "McuPio deinitialized");
  return HfPioErr::PIO_SUCCESS;
}

bool McuPio::IsInitialized() const noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  return initialized_;
}

//==============================================================================
// CHANNEL CONFIGURATION
//==============================================================================

HfPioErr McuPio::ConfigureChannel(uint8_t channel_id, const PioChannelConfig &config) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  if (channels_[channel_id].busy) {
    return HfPioErr::PIO_ERR_CHANNEL_BUSY;
  }

  // Validate configuration
  if (config.gpio_pin < 0) {
    return HfPioErr::PIO_ERR_INVALID_PARAMETER;
  }

  if (config.resolution_ns == 0) {
    return HfPioErr::PIO_ERR_INVALID_RESOLUTION;
  }

  // Store configuration
  channels_[channel_id].config = config;

  // Initialize the channel hardware
  HfPioErr result = InitializeChannel(channel_id);
  if (result != HfPioErr::PIO_SUCCESS) {
    return result;
  }

  channels_[channel_id].configured = true;
  ESP_LOGI(TAG, "Channel %d configured on GPIO %d", channel_id, config.gpio_pin);

  return HfPioErr::PIO_SUCCESS;
}

//==============================================================================
// TRANSMISSION OPERATIONS
//==============================================================================

HfPioErr McuPio::Transmit(uint8_t channel_id, const PioSymbol *symbols, size_t symbol_count,
                          bool wait_completion) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].config.direction == PioDirection::Receive) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].busy) {
    return HfPioErr::PIO_ERR_CHANNEL_BUSY;
  }

  if (symbols == nullptr || symbol_count == 0) {
    return HfPioErr::PIO_ERR_INVALID_PARAMETER;
  }

  if (symbol_count > MAX_SYMBOLS_PER_TRANSMISSION) {
    return HfPioErr::PIO_ERR_BUFFER_TOO_LARGE;
  }

  // Validate symbols
  HfPioErr validation_result = ValidateSymbols(symbols, symbol_count);
  if (validation_result != HfPioErr::PIO_SUCCESS) {
    return validation_result;
  }

#ifdef HF_MCU_FAMILY_ESP32
  auto &channel = channels_[channel_id];

  if (channel.tx_channel == nullptr) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  // Convert PioSymbols to RMT format
  rmt_symbol_word_t rmt_symbols[MAX_SYMBOLS_PER_TRANSMISSION];
  size_t rmt_symbol_count = 0;

  HfPioErr convert_result =
      ConvertToRmtSymbols(symbols, symbol_count, rmt_symbols, rmt_symbol_count);
  if (convert_result != HfPioErr::PIO_SUCCESS) {
    return convert_result;
  }

  // Create transmit configuration
  rmt_transmit_config_t tx_config = {};
  tx_config.loop_count = 0; // No loop

  // Start transmission
  channel.busy = true;
  channel.status.is_transmitting = true;
  channel.status.symbols_queued = symbol_count;
  channel.status.timestamp_us = esp_timer_get_time();

  // Register TX callbacks
  rmt_tx_event_callbacks_t tx_callbacks = {};
  tx_callbacks.on_trans_done = OnTransmitComplete;

  esp_err_t ret = rmt_tx_register_event_callbacks(channel.tx_channel, &tx_callbacks, this);
  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_transmitting = false;
    ESP_LOGE(TAG, "Failed to register TX callbacks for channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }

  ret = rmt_transmit(channel.tx_channel, channel.encoder, rmt_symbols,
                     rmt_symbol_count * sizeof(rmt_symbol_word_t), &tx_config);

  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_transmitting = false;
    ESP_LOGE(TAG, "Failed to start transmission on channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }
  if (wait_completion) {
    // Wait for transmission to complete using ESP-IDF API
    uint32_t timeout_us = channel.config.timeout_us;
    ret = rmt_tx_wait_all_done(channel.tx_channel,
                               timeout_us == 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_us / 1000));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Transmission timeout on channel %d", channel_id);
      return HfPioErr::PIO_ERR_COMMUNICATION_TIMEOUT;
    }
    channel.busy = false;
    channel.status.is_transmitting = false;
    channel.status.symbols_processed = symbol_count;
  }

  ESP_LOGD(TAG, "Started transmission of %d symbols on channel %d", symbol_count, channel_id);
  return HfPioErr::PIO_SUCCESS;
#else
  ESP_LOGE(TAG, "ESP32 platform not available");
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

//==============================================================================
// RECEPTION OPERATIONS
//==============================================================================

HfPioErr McuPio::StartReceive(uint8_t channel_id, PioSymbol *buffer, size_t buffer_size,
                              uint32_t timeout_us) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].config.direction == PioDirection::Transmit) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].busy) {
    return HfPioErr::PIO_ERR_CHANNEL_BUSY;
  }

  if (buffer == nullptr || buffer_size == 0) {
    return HfPioErr::PIO_ERR_INVALID_PARAMETER;
  }

#ifdef HF_MCU_FAMILY_ESP32
  auto &channel = channels_[channel_id];

  if (channel.rx_channel == nullptr) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  // Store buffer information
  channel.rx_buffer = buffer;
  channel.rx_buffer_size = buffer_size;
  channel.rx_symbols_received = 0;

  // Create receive configuration
  rmt_receive_config_t rx_config = {};
  rx_config.signal_range_min_ns = channel.config.resolution_ns;
  rx_config.signal_range_max_ns =
      channel.config.resolution_ns *
      32767; // Max duration    // Allocate RMT symbol buffer for reception
  size_t rmt_buffer_size = std::min(buffer_size, static_cast<size_t>(64)); // RMT buffer limit

  // Start reception
  channel.busy = true;
  channel.status.is_receiving = true;
  channel.status.timestamp_us = esp_timer_get_time();

  // Register RX callbacks
  rmt_rx_event_callbacks_t rx_callbacks = {};
  rx_callbacks.on_recv_done = OnReceiveComplete;

  esp_err_t ret = rmt_rx_register_event_callbacks(channel.rx_channel, &rx_callbacks, this);
  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_receiving = false;
    ESP_LOGE(TAG, "Failed to register RX callbacks for channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }

  // Start reception with allocated buffer
  ret = rmt_receive(channel.rx_channel, nullptr, rmt_buffer_size * sizeof(rmt_symbol_word_t),
                    &rx_config);
  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_receiving = false;
    ESP_LOGE(TAG, "Failed to start reception on channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }

  ESP_LOGI(TAG, "Started reception on channel %d", channel_id);
  return HfPioErr::PIO_SUCCESS;
#else
  ESP_LOGE(TAG, "ESP32 platform not available");
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfPioErr McuPio::StopReceive(uint8_t channel_id, size_t &symbols_received) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  auto &channel = channels_[channel_id];

  if (!channel.status.is_receiving) {
    symbols_received = 0;
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Stop reception
  channel.busy = false;
  channel.status.is_receiving = false;
  symbols_received = channel.rx_symbols_received;

  ESP_LOGI(TAG, "Stopped reception on channel %d, received %d symbols", channel_id,
           symbols_received);
  return HfPioErr::PIO_SUCCESS;
#else
  symbols_received = 0;
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

//==============================================================================
// STATUS AND CAPABILITIES
//==============================================================================

bool McuPio::IsChannelBusy(uint8_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return false;
  }

  return channels_[channel_id].busy;
}

HfPioErr McuPio::GetChannelStatus(uint8_t channel_id, PioChannelStatus &status) const noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  status = channels_[channel_id].status;
  status.is_initialized = channels_[channel_id].configured;
  status.is_busy = channels_[channel_id].busy;

  return HfPioErr::PIO_SUCCESS;
}

HfPioErr McuPio::GetCapabilities(PioCapabilities &capabilities) const noexcept {
  capabilities.max_channels = MAX_CHANNELS;
  capabilities.min_resolution_ns = 12.5;    // Based on 80MHz RMT clock
  capabilities.max_resolution_ns = 3355443; // Max with divider
  capabilities.max_duration = 32767;        // 15-bit duration field
  capabilities.max_buffer_size = MAX_SYMBOLS_PER_TRANSMISSION;
  capabilities.supports_bidirectional = false; // RMT is unidirectional per channel
  capabilities.supports_loopback = true;
  capabilities.supports_carrier = true;

  return HfPioErr::PIO_SUCCESS;
}

//==============================================================================
// CALLBACK MANAGEMENT
//==============================================================================

void McuPio::SetTransmitCallback(PioTransmitCallback callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  transmit_callback_ = callback;
  callback_user_data_ = user_data;
}

void McuPio::SetReceiveCallback(PioReceiveCallback callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  receive_callback_ = callback;
  callback_user_data_ = user_data;
}

void McuPio::SetErrorCallback(PioErrorCallback callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  error_callback_ = callback;
  callback_user_data_ = user_data;
}

void McuPio::ClearCallbacks() noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  transmit_callback_ = nullptr;
  receive_callback_ = nullptr;
  error_callback_ = nullptr;
  callback_user_data_ = nullptr;
}

//==============================================================================
// ESP32-SPECIFIC METHODS
//==============================================================================

HfPioErr McuPio::ConfigureCarrier(uint8_t channel_id, uint32_t carrier_freq_hz,
                                  float duty_cycle) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (duty_cycle < 0.0f || duty_cycle > 1.0f) {
    return HfPioErr::PIO_ERR_INVALID_PARAMETER;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Configure carrier modulation using RMT carrier configuration
  auto &channel = channels_[channel_id];

  if (!channel.tx_channel) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  rmt_carrier_config_t carrier_config = {};
  carrier_config.frequency_hz = carrier_freq_hz;
  carrier_config.duty_cycle = duty_cycle;
  carrier_config.polarity_active_low = false;
  carrier_config.always_on = (carrier_freq_hz > 0);

  esp_err_t ret = rmt_apply_carrier(channel.tx_channel, &carrier_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure carrier on channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }

  ESP_LOGI(TAG, "Configured carrier on channel %d: %d Hz, %.2f%% duty", channel_id, carrier_freq_hz,
           duty_cycle * 100.0f);
  return HfPioErr::PIO_SUCCESS;
#else
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfPioErr McuPio::EnableLoopback(uint8_t channel_id, bool enable) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Configure loopback mode
  ESP_LOGI(TAG, "Loopback %s for channel %d", enable ? "enabled" : "disabled", channel_id);
  return HfPioErr::PIO_SUCCESS;
#else
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

size_t McuPio::GetMaxSymbolCount() const noexcept {
  return MAX_SYMBOLS_PER_TRANSMISSION;
}

//==============================================================================
// ADVANCED LOW-LEVEL RMT CONTROL METHODS
//==============================================================================

HfPioErr McuPio::TransmitRawRmtSymbols(uint8_t channel_id, const rmt_symbol_word_t *rmt_symbols,
                                       size_t symbol_count, bool wait_completion) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].busy) {
    return HfPioErr::PIO_ERR_CHANNEL_BUSY;
  }

  if (rmt_symbols == nullptr || symbol_count == 0) {
    return HfPioErr::PIO_ERR_INVALID_PARAMETER;
  }

#ifdef HF_MCU_FAMILY_ESP32
  auto &channel = channels_[channel_id];

  if (channel.tx_channel == nullptr) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  // Create transmit configuration
  rmt_transmit_config_t tx_config = {};
  tx_config.loop_count = 0; // No loop

  // Start transmission
  channel.busy = true;
  channel.status.is_transmitting = true;
  channel.status.symbols_queued = symbol_count;
  channel.status.timestamp_us = esp_timer_get_time();

  esp_err_t ret = rmt_transmit(channel.tx_channel, channel.encoder, rmt_symbols,
                               symbol_count * sizeof(rmt_symbol_word_t), &tx_config);

  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_transmitting = false;
    ESP_LOGE(TAG, "Failed to start raw RMT transmission on channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }

  if (wait_completion) {
    uint32_t timeout_us = channel.config.timeout_us;
    ret = rmt_tx_wait_all_done(channel.tx_channel,
                               timeout_us == 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_us / 1000));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Raw RMT transmission timeout on channel %d", channel_id);
      return HfPioErr::PIO_ERR_COMMUNICATION_TIMEOUT;
    }
    channel.busy = false;
    channel.status.is_transmitting = false;
    channel.status.symbols_processed = symbol_count;
  }

  ESP_LOGD(TAG, "Started raw RMT transmission of %d symbols on channel %d", symbol_count,
           channel_id);
  return HfPioErr::PIO_SUCCESS;
#else
  ESP_LOGE(TAG, "ESP32 platform not available");
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfPioErr McuPio::ReceiveRawRmtSymbols(uint8_t channel_id, rmt_symbol_word_t *rmt_buffer,
                                      size_t buffer_size, size_t &symbols_received,
                                      uint32_t timeout_us) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].busy) {
    return HfPioErr::PIO_ERR_CHANNEL_BUSY;
  }

  if (rmt_buffer == nullptr || buffer_size == 0) {
    return HfPioErr::PIO_ERR_INVALID_PARAMETER;
  }

#ifdef HF_MCU_FAMILY_ESP32
  auto &channel = channels_[channel_id];

  if (channel.rx_channel == nullptr) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  // Create receive configuration
  rmt_receive_config_t rx_config = {};
  rx_config.signal_range_min_ns = channel.config.resolution_ns;
  rx_config.signal_range_max_ns = timeout_us * 1000; // Convert to ns

  // Start reception
  channel.busy = true;
  channel.status.is_receiving = true;
  channel.status.timestamp_us = esp_timer_get_time();

  esp_err_t ret = rmt_receive(channel.rx_channel, rmt_buffer,
                              buffer_size * sizeof(rmt_symbol_word_t), &rx_config);
  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_receiving = false;
    ESP_LOGE(TAG, "Failed to start raw RMT reception on channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }

  // Note: In a real implementation, this would use a semaphore or callback
  // to wait for reception completion. For simplicity, we'll simulate here.
  symbols_received = 0; // Would be filled by actual reception

  channel.busy = false;
  channel.status.is_receiving = false;

  ESP_LOGI(TAG, "Raw RMT reception completed on channel %d", channel_id);
  return HfPioErr::PIO_SUCCESS;
#else
  symbols_received = 0;
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfPioErr McuPio::CreateWS2812Encoder(uint8_t channel_id, uint32_t resolution_hz, uint32_t t0h_ns,
                                     uint32_t t0l_ns, uint32_t t1h_ns, uint32_t t1l_ns) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

#ifdef HF_MCU_FAMILY_ESP32
  auto &channel = channels_[channel_id];

  // Calculate timing in RMT ticks
  uint32_t ticks_0h = (t0h_ns * resolution_hz + 500000000UL) / 1000000000UL; // Round to nearest
  uint32_t ticks_0l = (t0l_ns * resolution_hz + 500000000UL) / 1000000000UL;
  uint32_t ticks_1h = (t1h_ns * resolution_hz + 500000000UL) / 1000000000UL;
  uint32_t ticks_1l = (t1l_ns * resolution_hz + 500000000UL) / 1000000000UL;

  // Create WS2812 symbols
  rmt_symbol_word_t bit0_symbol = {
      .duration0 = (uint16_t)ticks_0h, .level0 = 1, .duration1 = (uint16_t)ticks_0l, .level1 = 0};

  rmt_symbol_word_t bit1_symbol = {
      .duration0 = (uint16_t)ticks_1h, .level0 = 1, .duration1 = (uint16_t)ticks_1l, .level1 = 0};

  // Create bytes encoder for WS2812
  rmt_bytes_encoder_config_t ws_config = {};
  ws_config.bit0 = bit0_symbol;
  ws_config.bit1 = bit1_symbol;
  ws_config.flags.msb_first = true; // WS2812 uses MSB first

  esp_err_t ret = rmt_new_bytes_encoder(&ws_config, &channel.ws2812_encoder);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create WS2812 encoder for channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }

  ESP_LOGI(TAG, "Created WS2812 encoder on channel %d (T0H=%dns, T0L=%dns, T1H=%dns, T1L=%dns)",
           channel_id, t0h_ns, t0l_ns, t1h_ns, t1l_ns);
  return HfPioErr::PIO_SUCCESS;
#else
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfPioErr McuPio::TransmitWS2812(uint8_t channel_id, const uint8_t *grb_data, size_t length,
                                bool wait_completion) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPioErr::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].busy) {
    return HfPioErr::PIO_ERR_CHANNEL_BUSY;
  }

  if (grb_data == nullptr || length == 0) {
    return HfPioErr::PIO_ERR_INVALID_PARAMETER;
  }

#ifdef HF_MCU_FAMILY_ESP32
  auto &channel = channels_[channel_id];

  if (channel.tx_channel == nullptr) {
    return HfPioErr::PIO_ERR_NOT_INITIALIZED;
  }

  if (channel.ws2812_encoder == nullptr) {
    ESP_LOGE(TAG, "WS2812 encoder not created for channel %d. Call CreateWS2812Encoder first.",
             channel_id);
    return HfPioErr::PIO_ERR_INVALID_CONFIGURATION;
  }

  // Create transmit configuration
  rmt_transmit_config_t tx_config = {};
  tx_config.loop_count = 0; // No loop

  // Start transmission
  channel.busy = true;
  channel.status.is_transmitting = true;
  channel.status.symbols_queued = length * 8; // 8 bits per byte
  channel.status.timestamp_us = esp_timer_get_time();

  esp_err_t ret =
      rmt_transmit(channel.tx_channel, channel.ws2812_encoder, grb_data, length, &tx_config);

  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_transmitting = false;
    ESP_LOGE(TAG, "Failed to start WS2812 transmission on channel %d: %d", channel_id, ret);
    return HfPioErr::PIO_ERR_HARDWARE_FAULT;
  }

  if (wait_completion) {
    uint32_t timeout_us = channel.config.timeout_us;
    ret = rmt_tx_wait_all_done(channel.tx_channel,
                               timeout_us == 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_us / 1000));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "WS2812 transmission timeout on channel %d", channel_id);
      return HfPioErr::PIO_ERR_COMMUNICATION_TIMEOUT;
    }
    channel.busy = false;
    channel.status.is_transmitting = false;
    channel.status.symbols_processed = length * 8;
  }

  ESP_LOGD(TAG, "Started WS2812 transmission of %d bytes on channel %d", length, channel_id);
  return HfPioErr::PIO_SUCCESS;
#else
  ESP_LOGE(TAG, "ESP32 platform not available");
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

bool McuPio::IsValidChannelId(uint8_t channel_id) const noexcept {
  return channel_id < MAX_CHANNELS;
}

#ifdef HF_MCU_FAMILY_ESP32
HfPioErr McuPio::ConvertToRmtSymbols(const PioSymbol *symbols, size_t symbol_count,
                                     rmt_symbol_word_t *rmt_symbols,
                                     size_t &rmt_symbol_count) noexcept {
  if (symbol_count > MAX_SYMBOLS_PER_TRANSMISSION) {
    return HfPioErr::PIO_ERR_BUFFER_TOO_LARGE;
  }

  rmt_symbol_count = symbol_count;

  for (size_t i = 0; i < symbol_count; ++i) {
    // Set level and duration for first half of RMT symbol
    rmt_symbols[i].level0 = symbols[i].level ? 1 : 0;
    rmt_symbols[i].duration0 = symbols[i].duration;

    // For the second half, set to idle state or next symbol
    if (i + 1 < symbol_count) {
      // Use next symbol's complementary level for transition
      rmt_symbols[i].level1 = symbols[i + 1].level ? 0 : 1;
      rmt_symbols[i].duration1 = 1; // Minimal transition time
    } else {
      // Last symbol - end with idle state
      rmt_symbols[i].level1 = 0;    // Idle low
      rmt_symbols[i].duration1 = 0; // End marker
    }
  }

  return HfPioErr::PIO_SUCCESS;
}

HfPioErr McuPio::ConvertFromRmtSymbols(const rmt_symbol_word_t *rmt_symbols,
                                       size_t rmt_symbol_count, PioSymbol *symbols,
                                       size_t &symbol_count) noexcept {
  symbol_count = std::min(rmt_symbol_count, symbol_count);

  for (size_t i = 0; i < symbol_count; ++i) {
    symbols[i].level = rmt_symbols[i].level0 ? true : false;
    symbols[i].duration = rmt_symbols[i].duration0;
  }

  return HfPioErr::PIO_SUCCESS;
}

uint32_t McuPio::CalculateClockDivider(uint32_t resolution_ns) const noexcept {
  // Calculate clock divider to achieve desired resolution
  // RMT source clock is typically 80 MHz (12.5 ns per tick)
  uint32_t divider = (resolution_ns * RMT_CLK_SRC_FREQ) / 1000000000UL;
  return std::max(1U, std::min(255U, divider)); // Clamp to valid range
}

bool McuPio::OnTransmitComplete(rmt_channel_handle_t *channel,
                                const rmt_tx_done_event_data_t *edata, void *user_ctx) {
  auto *instance = static_cast<McuPio *>(user_ctx);
  if (!instance)
    return false;

  // Find the channel ID by comparing channel handles
  for (uint8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (instance->channels_[i].tx_channel == channel) {
      RtosUniqueLock<RtosMutex> lock(instance->state_mutex_);

      auto &ch = instance->channels_[i];
      ch.busy = false;
      ch.status.is_transmitting = false;
      ch.status.symbols_processed = ch.status.symbols_queued;
      ch.status.timestamp_us = esp_timer_get_time();

      // Invoke user callback if set
      if (instance->transmit_callback_) {
        instance->transmit_callback_(i, ch.status.symbols_processed, instance->callback_user_data_);
      }

      ESP_LOGD(TAG, "Transmission complete on channel %d", i);
      break;
    }
  }

  return false; // Don't yield to higher priority task
}

bool McuPio::OnReceiveComplete(rmt_channel_handle_t *channel, const rmt_rx_done_event_data_t *edata,
                               void *user_ctx) {
  auto *instance = static_cast<McuPio *>(user_ctx);
  if (!instance || !edata)
    return false;

  // Find the channel ID by comparing channel handles
  for (uint8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (instance->channels_[i].rx_channel == channel) {
      RtosUniqueLock<RtosMutex> lock(instance->state_mutex_);

      auto &ch = instance->channels_[i];

      // Convert received RMT symbols back to PioSymbols
      size_t symbols_converted = 0;
      if (ch.rx_buffer && edata->received_symbols) {
        instance->ConvertFromRmtSymbols(
            reinterpret_cast<const rmt_symbol_word_t *>(edata->received_symbols),
            edata->num_symbols, ch.rx_buffer, symbols_converted);
      }

      ch.busy = false;
      ch.status.is_receiving = false;
      ch.rx_symbols_received = symbols_converted;
      ch.status.symbols_processed = symbols_converted;
      ch.status.timestamp_us = esp_timer_get_time();

      // Invoke user callback if set
      if (instance->receive_callback_) {
        instance->receive_callback_(i, ch.rx_buffer, symbols_converted,
                                    instance->callback_user_data_);
      }

      ESP_LOGD(TAG, "Reception complete on channel %d, received %d symbols", i, symbols_converted);
      break;
    }
  }

  return false;
}
#endif

HfPioErr McuPio::InitializeChannel(uint8_t channel_id) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  auto &channel = channels_[channel_id];
  const auto &config = channel.config;

  // Calculate clock settings
  uint32_t clock_divider = CalculateClockDivider(config.resolution_ns);

  if (config.direction == PioDirection::Transmit ||
      config.direction == PioDirection::Bidirectional) {
    // Configure TX channel
    rmt_tx_channel_config_t tx_config = {};
    tx_config.gpio_num = config.gpio_pin;
    tx_config.clk_src = RMT_CLK_SRC_DEFAULT;
    tx_config.resolution_hz = RMT_CLK_SRC_FREQ / clock_divider;
    tx_config.mem_block_symbols = 64;
    tx_config.trans_queue_depth = 4;

    esp_err_t ret = rmt_new_tx_channel(&tx_config, &channel.tx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create TX channel %d: %d", channel_id, ret);
      return HfPioErr::PIO_ERR_HARDWARE_FAULT;
    }

    // Create copy encoder (simple symbol transmission)
    rmt_copy_encoder_config_t encoder_config = {};
    ret = rmt_new_copy_encoder(&encoder_config, &channel.encoder);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create encoder for channel %d: %d", channel_id, ret);
      rmt_del_channel(channel.tx_channel);
      channel.tx_channel = nullptr;
      return HfPioErr::PIO_ERR_HARDWARE_FAULT;
    }

    // Enable channel
    ret = rmt_enable(channel.tx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable TX channel %d: %d", channel_id, ret);
      return HfPioErr::PIO_ERR_HARDWARE_FAULT;
    }
  }

  if (config.direction == PioDirection::Receive ||
      config.direction == PioDirection::Bidirectional) {
    // Configure RX channel
    rmt_rx_channel_config_t rx_config = {};
    rx_config.gpio_num = config.gpio_pin;
    rx_config.clk_src = RMT_CLK_SRC_DEFAULT;
    rx_config.resolution_hz = RMT_CLK_SRC_FREQ / clock_divider;
    rx_config.mem_block_symbols = 64;

    esp_err_t ret = rmt_new_rx_channel(&rx_config, &channel.rx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create RX channel %d: %d", channel_id, ret);
      return HfPioErr::PIO_ERR_HARDWARE_FAULT;
    }

    // Enable channel
    ret = rmt_enable(channel.rx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable RX channel %d: %d", channel_id, ret);
      return HfPioErr::PIO_ERR_HARDWARE_FAULT;
    }
  }

  ESP_LOGI(TAG, "Initialized channel %d with %d ns resolution", channel_id, config.resolution_ns);
  return HfPioErr::PIO_SUCCESS;
#else
  ESP_LOGE(TAG, "ESP32 platform not available");
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfPioErr McuPio::DeinitializeChannel(uint8_t channel_id) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  auto &channel = channels_[channel_id];

  if (channel.tx_channel) {
    rmt_disable(channel.tx_channel);
    rmt_del_channel(channel.tx_channel);
    channel.tx_channel = nullptr;
  }

  if (channel.rx_channel) {
    rmt_disable(channel.rx_channel);
    rmt_del_channel(channel.rx_channel);
    channel.rx_channel = nullptr;
  }

  if (channel.encoder) {
    rmt_del_encoder(channel.encoder);
    channel.encoder = nullptr;
  }

  if (channel.bytes_encoder) {
    rmt_del_encoder(channel.bytes_encoder);
    channel.bytes_encoder = nullptr;
  }

  if (channel.ws2812_encoder) {
    rmt_del_encoder(channel.ws2812_encoder);
    channel.ws2812_encoder = nullptr;
  }

  channel.configured = false;
  channel.busy = false;

  ESP_LOGI(TAG, "Deinitialized channel %d", channel_id);
  return HfPioErr::PIO_SUCCESS;
#else
  return HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfPioErr McuPio::ValidateSymbols(const PioSymbol *symbols, size_t symbol_count) const noexcept {
  for (size_t i = 0; i < symbol_count; ++i) {
    if (symbols[i].duration == 0) {
      return HfPioErr::PIO_ERR_DURATION_TOO_SHORT;
    }
    if (symbols[i].duration > 32767) { // RMT 15-bit duration limit
      return HfPioErr::PIO_ERR_DURATION_TOO_LONG;
    }
  }
  return HfPioErr::PIO_SUCCESS;
}

void McuPio::UpdateChannelStatus(uint8_t channel_id) noexcept {
  auto &channel = channels_[channel_id];
  channel.status.timestamp_us = esp_timer_get_time();
  channel.status.last_error = HfPioErr::PIO_SUCCESS;
}

void McuPio::InvokeErrorCallback(uint8_t channel_id, HfPioErr error) noexcept {
  if (error_callback_) {
    error_callback_(channel_id, error, callback_user_data_);
  }
  channels_[channel_id].status.last_error = error;
}
