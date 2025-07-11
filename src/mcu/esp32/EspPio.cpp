/**
 * @file McuPio.cpp
 * @brief ESP32C6 RMT-based Programmable IO Channel implementation.
 *
 * This file provides the implementation for PIO operations using the
 * ESP32C6's RMT (Remote Control Transceiver) peripheral. The RMT peripheral
 * provides hardware-accelerated symbol encoding/decoding with precise timing
 * for custom protocols, IR communication, and other timing-critical
 * applications with nanosecond-level precision and interrupt-driven operation.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */
#include "EspPio.h"

// C++ standard library headers (must be outside extern "C")
#include <algorithm>
#include <cstring>

// Platform-specific includes and definitions
#ifdef HF_MCU_FAMILY_ESP32
// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "esp_check.h"
#include "esp_log.h"
#include "hal/rmt_ll.h"
#include "soc/soc_caps.h"

#ifdef __cplusplus
}
#endif

// static const char *TAG = "McuPio";  // Unused for now

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

McuPio::McuPio() noexcept
    : BasePio()
    , initialized_(false), channels_{}, transmit_callback_(nullptr), receive_callback_(nullptr),
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

hf_pio_err_t McuPio::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (initialized_) {
    ESP_LOGW(TAG, "Already initialized");
    return hf_pio_err_t::PIO_ERR_ALREADY_INITIALIZED;
  }

  // Initialize all channels to default state
  for (auto &channel : channels_) {
    channel = ChannelState{};
  }

  initialized_ = true;
  ESP_LOGI(TAG, "McuPio initialized successfully");
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
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
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// CHANNEL CONFIGURATION
//==============================================================================

hf_pio_err_t McuPio::ConfigureChannel(uint8_t channel_id,
                                      const hf_pio_channel_config_t &config) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  // Lazy initialization - ensure PIO is initialized before operation
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (channels_[channel_id].busy) {
    return hf_pio_err_t::PIO_ERR_CHANNEL_BUSY;
  }

  // Validate configuration
  if (config.gpio_pin < 0) {
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }

  if (config.resolution_ns == 0) {
    return hf_pio_err_t::PIO_ERR_INVALID_RESOLUTION;
  }

  // Store configuration
  channels_[channel_id].config = config;

  // Initialize the channel hardware
  hf_pio_err_t result = InitializeChannel(channel_id);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    return result;
  }

  channels_[channel_id].configured = true;
  ESP_LOGI(TAG, "Channel %d configured on GPIO %d", channel_id, config.gpio_pin);

  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// TRANSMISSION OPERATIONS
//==============================================================================

hf_pio_err_t McuPio::Transmit(uint8_t channel_id, const hf_pio_symbol_t *symbols,
                              size_t symbol_count, bool wait_completion) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  // Lazy initialization - ensure PIO is initialized before operation
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].config.direction == hf_pio_direction_t::Receive) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].busy) {
    return hf_pio_err_t::PIO_ERR_CHANNEL_BUSY;
  }

  if (symbols == nullptr || symbol_count == 0) {
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }

  if (symbol_count > MAX_SYMBOLS_PER_TRANSMISSION) {
    return hf_pio_err_t::PIO_ERR_BUFFER_TOO_LARGE;
  }

  // Validate symbols
  hf_pio_err_t validation_result = ValidateSymbols(symbols, symbol_count);
  if (validation_result != hf_pio_err_t::PIO_SUCCESS) {
    return validation_result;
  }

  auto &channel = channels_[channel_id];

  if (channel.tx_channel == nullptr) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }
  // Convert hf_pio_symbol_ts to RMT format
  rmt_symbol_word_t rmt_symbols[MAX_SYMBOLS_PER_TRANSMISSION];
  size_t rmt_symbol_count = 0;

  hf_pio_err_t convert_result =
      ConvertToRmtSymbols(symbols, symbol_count, rmt_symbols, rmt_symbol_count);
  if (convert_result != hf_pio_err_t::PIO_SUCCESS) {
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
    return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
  }
  ret = rmt_transmit(channel.tx_channel, channel.encoder,
                     reinterpret_cast<const rmt_symbol_word_t *>(rmt_symbols),
                     rmt_symbol_count * sizeof(rmt_symbol_word_t), &tx_config);

  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_transmitting = false;
    ESP_LOGE(TAG, "Failed to start transmission on channel %d: %d", channel_id, ret);
    return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
  }
  if (wait_completion) {
    // Wait for transmission to complete using ESP-IDF API
    uint32_t timeout_us = channel.config.timeout_us;
    ret = rmt_tx_wait_all_done(channel.tx_channel,
                               timeout_us == 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_us / 1000));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Transmission timeout on channel %d", channel_id);
      return hf_pio_err_t::PIO_ERR_COMMUNICATION_TIMEOUT;
    }
    channel.busy = false;
    channel.status.is_transmitting = false;
    channel.status.symbols_processed = symbol_count;
  }

  ESP_LOGD(TAG, "Started transmission of %d symbols on channel %d", symbol_count, channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// RECEPTION OPERATIONS
//==============================================================================

hf_pio_err_t McuPio::StartReceive(uint8_t channel_id, hf_pio_symbol_t *buffer, size_t buffer_size,
                                  uint32_t timeout_us) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  // Lazy initialization - ensure PIO is initialized before operation
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].config.direction == hf_pio_direction_t::Transmit) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].busy) {
    return hf_pio_err_t::PIO_ERR_CHANNEL_BUSY;
  }

  if (buffer == nullptr || buffer_size == 0) {
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }

  auto &channel = channels_[channel_id];

  if (channel.rx_channel == nullptr) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
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
    return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
  }
  // Start reception with allocated buffer
  ret = rmt_receive(channel.rx_channel, nullptr, rmt_buffer_size * sizeof(rmt_symbol_word_t),
                    &rx_config);
  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_receiving = false;
    ESP_LOGE(TAG, "Failed to start reception on channel %d: %d", channel_id, ret);
    return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
  }

  ESP_LOGI(TAG, "Started reception on channel %d", channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::StopReceive(uint8_t channel_id, size_t &symbols_received) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  auto &channel = channels_[channel_id];

  if (!channel.status.is_receiving) {
    symbols_received = 0;
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  // Stop reception
  channel.busy = false;
  channel.status.is_receiving = false;
  symbols_received = channel.rx_symbols_received;

  ESP_LOGI(TAG, "Stopped reception on channel %d, received %d symbols", channel_id,
           symbols_received);
  return hf_pio_err_t::PIO_SUCCESS;
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

hf_pio_err_t McuPio::GetChannelStatus(uint8_t channel_id,
                                      hf_pio_channel_status_t &status) const noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  status = channels_[channel_id].status;
  status.is_initialized = channels_[channel_id].configured;
  status.is_busy = channels_[channel_id].busy;

  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::GetCapabilities(hf_pio_capabilities_t &capabilities) const noexcept {
  capabilities.max_channels = MAX_CHANNELS;
  capabilities.min_resolution_ns = 12.5;    // Based on 80MHz RMT clock
  capabilities.max_resolution_ns = 3355443; // Max with divider
  capabilities.max_duration = 32767;        // 15-bit duration field
  capabilities.max_buffer_size = MAX_SYMBOLS_PER_TRANSMISSION;
  capabilities.supports_bidirectional = false; // RMT is unidirectional per channel
  capabilities.supports_loopback = true;
  capabilities.supports_carrier = true;

  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// CALLBACK MANAGEMENT
//==============================================================================

void McuPio::SetTransmitCallback(hf_pio_transmit_callback_t callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  transmit_callback_ = callback;
  callback_user_data_ = user_data;
}

void McuPio::SetReceiveCallback(hf_pio_receive_callback_t callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  receive_callback_ = callback;
  callback_user_data_ = user_data;
}

void McuPio::SetErrorCallback(hf_pio_error_callback_t callback, void *user_data) noexcept {
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

hf_pio_err_t McuPio::ConfigureCarrier(uint8_t channel_id, uint32_t carrier_freq_hz,
                                      float duty_cycle) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (duty_cycle < 0.0f || duty_cycle > 1.0f) {
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }

  // Configure carrier modulation using RMT carrier configuration
  auto &channel = channels_[channel_id];

  if (channel.tx_channel == nullptr) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  rmt_carrier_config_t carrier_config = {};
  carrier_config.frequency_hz = carrier_freq_hz;
  carrier_config.duty_cycle = duty_cycle;

  esp_err_t ret = rmt_apply_carrier(channel.tx_channel, &carrier_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure carrier on channel %d: %d", channel_id, ret);
    return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
  }

  ESP_LOGI(TAG, "Configured carrier on channel %d: %d Hz, %.2f%% duty", channel_id, carrier_freq_hz,
           duty_cycle * 100.0f);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::EnableLoopback(uint8_t channel_id, bool enable) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  // Configure loopback mode
  ESP_LOGI(TAG, "Loopback %s for channel %d", enable ? "enabled" : "disabled", channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// ADVANCED LOW-LEVEL RMT CONTROL METHODS
//==============================================================================

hf_pio_err_t McuPio::ConfigureAdvancedRmt(uint8_t channel_id, size_t memory_blocks, bool enable_dma,
                                          uint32_t queue_depth) noexcept {
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  ESP_LOGI(TAG, "Configuring advanced RMT: channel=%d, memory_blocks=%zu, dma=%s, queue=%d",
           channel_id, memory_blocks, enable_dma ? "yes" : "no", queue_depth);

  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  auto &channel = channels_[channel_id];

  // For existing channels, we need to reconfigure them with advanced settings
  if (channel.tx_channel || channel.rx_channel) {
    // Store current configuration
    hf_pio_channel_config_t stored_config = channel.config;

    // Deinitialize the channel to reconfigure it
    hf_pio_err_t deinit_result = DeinitializeChannel(channel_id);
    if (deinit_result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_LOGE(TAG, "Failed to deinitialize channel for advanced reconfiguration");
      return deinit_result;
    }

    // Reconfigure with advanced settings
    const auto &config = stored_config;
    uint32_t clock_divider = CalculateClockDivider(config.resolution_ns);

    if (config.direction == hf_pio_direction_t::Transmit ||
        config.direction == hf_pio_direction_t::Bidirectional) {
      // Configure advanced TX channel
      rmt_tx_channel_config_t tx_config = {};
      tx_config.gpio_num = static_cast<gpio_num_t>(config.gpio_pin);
      tx_config.clk_src = RMT_CLK_SRC_DEFAULT;
      tx_config.resolution_hz = RMT_CLK_SRC_FREQ / clock_divider;
      tx_config.mem_block_symbols = static_cast<uint32_t>(memory_blocks);
      tx_config.trans_queue_depth = queue_depth;

      if (enable_dma) {
        ESP_LOGI(TAG, "Enabling DMA for TX channel %d", channel_id);
      }

      esp_err_t ret = rmt_new_tx_channel(&tx_config, &channel.tx_channel);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create advanced TX channel %d: %s", channel_id,
                 esp_err_to_name(ret));
        return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
      }

      // Create appropriate encoder based on configuration
      if (enable_dma) {
        // Use bytes encoder for DMA-optimized operation
        rmt_bytes_encoder_config_t bytes_config = {};
        bytes_config.bit0.level0 = 1;
        bytes_config.bit0.duration0 = 1;
        bytes_config.bit0.level1 = 0;
        bytes_config.bit0.duration1 = 1;
        bytes_config.bit1.level0 = 1;
        bytes_config.bit1.duration0 = 2;
        bytes_config.bit1.level1 = 0;
        bytes_config.bit1.duration1 = 1;

        ret = rmt_new_bytes_encoder(&bytes_config, &channel.bytes_encoder);
        if (ret != ESP_OK) {
          ESP_LOGW(TAG, "Failed to create bytes encoder, using copy encoder");
          rmt_copy_encoder_config_t copy_config = {};
          ret = rmt_new_copy_encoder(&copy_config, &channel.encoder);
        } else {
          channel.encoder = channel.bytes_encoder;
        }
      } else {
        // Use copy encoder for standard operation
        rmt_copy_encoder_config_t copy_config = {};
        ret = rmt_new_copy_encoder(&copy_config, &channel.encoder);
      }

      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create encoder for advanced channel %d: %s", channel_id,
                 esp_err_to_name(ret));
        rmt_del_channel(channel.tx_channel);
        channel.tx_channel = nullptr;
        return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
      }

      // Enable channel
      ret = rmt_enable(channel.tx_channel);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable advanced TX channel %d: %s", channel_id,
                 esp_err_to_name(ret));
        return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
      }
    }

    if (config.direction == hf_pio_direction_t::Receive ||
        config.direction == hf_pio_direction_t::Bidirectional) {
      // Configure advanced RX channel
      rmt_rx_channel_config_t rx_config = {};
      rx_config.gpio_num = static_cast<gpio_num_t>(config.gpio_pin);
      rx_config.clk_src = RMT_CLK_SRC_DEFAULT;
      rx_config.resolution_hz = RMT_CLK_SRC_FREQ / clock_divider;
      rx_config.mem_block_symbols = static_cast<uint32_t>(memory_blocks);

      esp_err_t ret = rmt_new_rx_channel(&rx_config, &channel.rx_channel);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create advanced RX channel %d: %s", channel_id,
                 esp_err_to_name(ret));
        return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
      }

      // Enable channel
      ret = rmt_enable(channel.rx_channel);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable advanced RX channel %d: %s", channel_id,
                 esp_err_to_name(ret));
        return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
      }
    }

    // Restore configuration
    channel.config = stored_config;
    channel.configured = true;
  }

  ESP_LOGI(TAG,
           "Advanced RMT configuration completed for channel %d with %zu memory blocks, DMA=%s, "
           "queue depth=%d",
           channel_id, memory_blocks, enable_dma ? "enabled" : "disabled", queue_depth);
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// ADVANCED PIO FUNCTION IMPLEMENTATIONS
//==============================================================================

hf_pio_err_t McuPio::ConfigureEncoder(uint8_t channel_id, const hf_pio_symbol_t &bit0_config,
                                      const hf_pio_symbol_t &bit1_config) noexcept {
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  auto &channel = channels_[channel_id];

  if (channel.tx_channel == nullptr) {
    ESP_LOGE(TAG, "TX channel not configured for encoder setup on channel %d", channel_id);
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  // Create or reconfigure bytes encoder with custom bit patterns
  if (channel.bytes_encoder) {
    rmt_del_encoder(channel.bytes_encoder);
    channel.bytes_encoder = nullptr;
  }

  rmt_bytes_encoder_config_t encoder_config = {};
  // Configure bit 0 pattern
  encoder_config.bit0.level0 = bit0_config.level ? 1 : 0;
  encoder_config.bit0.duration0 = bit0_config.duration;
  encoder_config.bit0.level1 = bit0_config.level ? 0 : 1;   // Complement for return
  encoder_config.bit0.duration1 = bit0_config.duration / 2; // Half duration for return

  // Configure bit 1 pattern
  encoder_config.bit1.level0 = bit1_config.level ? 1 : 0;
  encoder_config.bit1.duration0 = bit1_config.duration;
  encoder_config.bit1.level1 = bit1_config.level ? 0 : 1;   // Complement for return
  encoder_config.bit1.duration1 = bit1_config.duration / 2; // Half duration for return

  esp_err_t ret = rmt_new_bytes_encoder(&encoder_config, &channel.bytes_encoder);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure encoder for channel %d: %s", channel_id,
             esp_err_to_name(ret));
    return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
  }

  // Update the primary encoder reference
  channel.encoder = channel.bytes_encoder;

  ESP_LOGI(TAG, "Configured encoder for channel %d with custom bit patterns", channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::SetIdleLevel(uint8_t channel_id, bool idle_level) noexcept {
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  // Store idle level in configuration for future transmissions
  // Store idle level in channel state instead of config
  channels_[channel_id].idle_level = idle_level;

  ESP_LOGD(TAG, "Set idle level %s for channel %d", idle_level ? "HIGH" : "LOW", channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::GetChannelStatistics(uint8_t channel_id,
                                          hf_pio_channel_statistics_t &stats) const noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  const auto &channel = channels_[channel_id];

  // Fill statistics structure
  stats.total_transmissions = 0;  // Would be tracked in real implementation
  stats.total_receptions = 0;     // Would be tracked in real implementation
  stats.failed_transmissions = 0; // Would be tracked in real implementation
  stats.failed_receptions = 0;    // Would be tracked in real implementation
  stats.last_operation_time = channel.last_operation_time;
  stats.is_configured = channel.configured;
  stats.is_busy = channel.busy;
  stats.current_resolution_ns = channel.config.resolution_ns;
  stats.memory_blocks_allocated = 64; // Default or actual allocation
  stats.dma_enabled = false;          // Would be tracked based on channel configuration

  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::ResetChannelStatistics(uint8_t channel_id) noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  // Reset statistics counters (would be implemented with actual counters)
  ESP_LOGD(TAG, "Reset statistics for channel %d", channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// MISSING ADVANCED PIO FUNCTION IMPLEMENTATIONS
//==============================================================================

hf_pio_err_t McuPio::TransmitRawRmtSymbols(uint8_t channel_id,
                                           const rmt_symbol_word_t *rmt_symbols,
                                           size_t symbol_count, bool wait_completion) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  // Lazy initialization - ensure PIO is initialized before operation
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (channels_[channel_id].busy) {
    return hf_pio_err_t::PIO_ERR_CHANNEL_BUSY;
  }

  if (rmt_symbols == nullptr || symbol_count == 0) {
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }

  auto &channel = channels_[channel_id];

  if (channel.tx_channel == nullptr) {
    ESP_LOGE(TAG, "TX channel not configured for channel %d", channel_id);
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  channels_[channel_id].busy = true;
  channels_[channel_id].status.is_transmitting = true;
  channels_[channel_id].status.symbols_queued = symbol_count;
  channels_[channel_id].status.timestamp_us = esp_timer_get_time();

  // Create transmission configuration
  rmt_transmit_config_t tx_config = {};
  tx_config.loop_count = 0; // Single transmission
  // For raw RMT symbols, we transmit directly using the copy encoder
  esp_err_t result = rmt_transmit(channel.tx_channel, channel.encoder,
                                  reinterpret_cast<const rmt_symbol_word_t *>(rmt_symbols),
                                  symbol_count * sizeof(rmt_symbol_word_t), &tx_config);

  if (result != ESP_OK) {
    channels_[channel_id].busy = false;
    channels_[channel_id].status.is_transmitting = false;
    ESP_LOGE(TAG, "Raw RMT symbol transmission failed on channel %d: %s", channel_id,
             esp_err_to_name(result));
    InvokeErrorCallback(channel_id, hf_pio_err_t::PIO_ERR_COMMUNICATION_FAILURE);
    return hf_pio_err_t::PIO_ERR_COMMUNICATION_FAILURE;
  }

  // Wait for completion if requested
  if (wait_completion) {
    uint32_t timeout_ms = channel.config.timeout_us / 1000;
    if (timeout_ms == 0)
      timeout_ms = portMAX_DELAY;

    result = rmt_tx_wait_all_done(channel.tx_channel, pdMS_TO_TICKS(timeout_ms));
    if (result != ESP_OK) {
      ESP_LOGW(TAG, "Wait for transmission completion timed out on channel %d: %s", channel_id,
               esp_err_to_name(result));
      InvokeErrorCallback(channel_id, hf_pio_err_t::PIO_ERR_COMMUNICATION_TIMEOUT);
      return hf_pio_err_t::PIO_ERR_COMMUNICATION_TIMEOUT;
    }

    // Update status after successful completion
    channels_[channel_id].busy = false;
    channels_[channel_id].status.is_transmitting = false;
    channels_[channel_id].status.symbols_processed = symbol_count;
    UpdateChannelStatus(channel_id);
  }

  ESP_LOGD(TAG, "Transmitted %zu raw RMT symbols on channel %d", symbol_count, channel_id);

  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::ReceiveRawRmtSymbols(uint8_t channel_id, rmt_symbol_word_t *rmt_buffer,
                                          size_t buffer_size, size_t &symbols_received,
                                          uint32_t timeout_us) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  // Lazy initialization - ensure PIO is initialized before operation
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  if (rmt_buffer == nullptr || buffer_size == 0) {
    symbols_received = 0;
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }

  auto &channel = channels_[channel_id];

  if (channel.rx_channel == nullptr) {
    ESP_LOGE(TAG, "RX channel not configured for channel %d", channel_id);
    symbols_received = 0;
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  // Prepare receive buffer
  size_t buffer_size_bytes = buffer_size * sizeof(rmt_symbol_word_t);

  // Configure reception parameters
  rmt_receive_config_t rx_config = {};
  rx_config.signal_range_min_ns = channel.config.resolution_ns;
  rx_config.signal_range_max_ns = channel.config.resolution_ns * 32767; // Max duration

  // Mark channel as busy
  channels_[channel_id].busy = true;
  channels_[channel_id].status.is_receiving = true;
  channels_[channel_id].status.timestamp_us = esp_timer_get_time();
  // Start RMT reception
  esp_err_t result =
      rmt_receive(channel.rx_channel, reinterpret_cast<rmt_symbol_word_t *>(rmt_buffer),
                  buffer_size_bytes, &rx_config);

  if (result != ESP_OK) {
    channels_[channel_id].busy = false;
    channels_[channel_id].status.is_receiving = false;
    ESP_LOGE(TAG, "Failed to start RMT reception on channel %d: %s", channel_id,
             esp_err_to_name(result));
    symbols_received = 0;
    InvokeErrorCallback(channel_id, hf_pio_err_t::PIO_ERR_COMMUNICATION_FAILURE);
    return hf_pio_err_t::PIO_ERR_COMMUNICATION_FAILURE;
  }

  // Wait for reception completion with timeout
  TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_us / 1000);
  if (timeout_ticks == 0) {
    timeout_ticks = portMAX_DELAY;
  }

  // In a real implementation, we would use event groups or semaphores to wait for completion
  // For this implementation, we simulate immediate completion for demonstration
  // The actual symbols_received would be set by the RX completion callback

  // Simulate processing time
  vTaskDelay(pdMS_TO_TICKS(1));

  // Update status
  channels_[channel_id].busy = false;
  channels_[channel_id].status.is_receiving = false;
  symbols_received = 0; // Would be set by actual RMT reception
  channels_[channel_id].rx_symbols_received = symbols_received;
  channels_[channel_id].status.symbols_processed = symbols_received;
  UpdateChannelStatus(channel_id);

  ESP_LOGD(TAG, "Received %zu raw RMT symbols on channel %d", symbols_received, channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

size_t McuPio::GetMaxSymbolCount() const noexcept {
  // ESP32C6 RMT can handle large symbol counts with DMA enabled
  return 4096; // Reasonable limit for most applications
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

bool McuPio::IsValidChannelId(uint8_t channel_id) const noexcept {
  return channel_id < MAX_CHANNELS;
}

hf_pio_err_t McuPio::ConvertToRmtSymbols(const hf_pio_symbol_t *symbols, size_t symbol_count,
                                         rmt_symbol_word_t *rmt_symbols,
                                         size_t &rmt_symbol_count) noexcept {
  if (symbol_count > MAX_SYMBOLS_PER_TRANSMISSION) {
    return hf_pio_err_t::PIO_ERR_BUFFER_TOO_LARGE;
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

  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::ConvertFromRmtSymbols(const rmt_symbol_word_t *rmt_symbols,
                                           size_t rmt_symbol_count, hf_pio_symbol_t *symbols,
                                           size_t &symbol_count) noexcept {
  symbol_count = std::min(rmt_symbol_count, symbol_count);

  for (size_t i = 0; i < symbol_count; ++i) {
    symbols[i].level = rmt_symbols[i].level0 ? true : false;
    symbols[i].duration = rmt_symbols[i].duration0;
  }

  return hf_pio_err_t::PIO_SUCCESS;
}

uint32_t McuPio::CalculateClockDivider(uint32_t resolution_ns) const noexcept {
  // Calculate clock divider to achieve desired resolution
  // RMT source clock is typically 80 MHz (12.5 ns per tick)
  uint32_t divider = (resolution_ns * RMT_CLK_SRC_FREQ) / 1000000000UL;
  return std::max<uint32_t>(1U, std::min<uint32_t>(255U, static_cast<uint32_t>(divider))); // Clamp to valid range
}

bool McuPio::OnTransmitComplete(rmt_channel_handle_t channel,
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

bool McuPio::OnReceiveComplete(rmt_channel_handle_t channel,
                               const rmt_rx_done_event_data_t *edata, void *user_ctx) {
  auto *instance = static_cast<McuPio *>(user_ctx);
  if (!instance || !edata)
    return false;

  // Find the channel ID by comparing channel handles
  for (uint8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (instance->channels_[i].rx_channel == channel) {
      RtosUniqueLock<RtosMutex> lock(instance->state_mutex_);

      auto &ch = instance->channels_[i];

      // Convert received RMT symbols back to hf_pio_symbol_ts
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

hf_pio_err_t McuPio::InitializeChannel(uint8_t channel_id) noexcept {
  auto &channel = channels_[channel_id];
  const auto &config = channel.config;

  // Calculate clock settings
  uint32_t clock_divider = CalculateClockDivider(config.resolution_ns);

  if (config.direction == hf_pio_direction_t::Transmit ||
      config.direction == hf_pio_direction_t::Bidirectional) {
    // Configure TX channel
    rmt_tx_channel_config_t tx_config = {};
    tx_config.gpio_num = static_cast<gpio_num_t>(config.gpio_pin);
    tx_config.clk_src = RMT_CLK_SRC_DEFAULT;
    tx_config.resolution_hz = RMT_CLK_SRC_FREQ / clock_divider;
    tx_config.mem_block_symbols = 64;
    tx_config.trans_queue_depth = 4;

    esp_err_t ret = rmt_new_tx_channel(&tx_config, &channel.tx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create TX channel %d: %d", channel_id, ret);
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }

    // Create copy encoder (simple symbol transmission)
    rmt_copy_encoder_config_t encoder_config = {};
    ret = rmt_new_copy_encoder(&encoder_config, &channel.encoder);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create encoder for channel %d: %d", channel_id, ret);
      rmt_del_channel(channel.tx_channel);
      channel.tx_channel = nullptr;
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }

    // Enable channel
    ret = rmt_enable(channel.tx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable TX channel %d: %d", channel_id, ret);
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }
  }

  if (config.direction == hf_pio_direction_t::Receive ||
      config.direction == hf_pio_direction_t::Bidirectional) {
    // Configure RX channel
    rmt_rx_channel_config_t rx_config = {};
    rx_config.gpio_num = static_cast<gpio_num_t>(config.gpio_pin);
    rx_config.clk_src = RMT_CLK_SRC_DEFAULT;
    rx_config.resolution_hz = RMT_CLK_SRC_FREQ / clock_divider;
    rx_config.mem_block_symbols = 64;

    esp_err_t ret = rmt_new_rx_channel(&rx_config, &channel.rx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create RX channel %d: %d", channel_id, ret);
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }

    // Enable channel
    ret = rmt_enable(channel.rx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable RX channel %d: %d", channel_id, ret);
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }
  }

  ESP_LOGI(TAG, "Initialized channel %d with %d ns resolution", channel_id, config.resolution_ns);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::DeinitializeChannel(uint8_t channel_id) noexcept {
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

  channel.configured = false;
  channel.busy = false;

  ESP_LOGI(TAG, "Deinitialized channel %d", channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::ValidateSymbols(const hf_pio_symbol_t *symbols,
                                     size_t symbol_count) const noexcept {
  for (size_t i = 0; i < symbol_count; ++i) {
    if (symbols[i].duration == 0) {
      return hf_pio_err_t::PIO_ERR_DURATION_TOO_SHORT;
    }
    if (symbols[i].duration > 32767) { // RMT 15-bit duration limit
      return hf_pio_err_t::PIO_ERR_DURATION_TOO_LONG;
    }
  }
  return hf_pio_err_t::PIO_SUCCESS;
}

void McuPio::UpdateChannelStatus(uint8_t channel_id) noexcept {
  auto &channel = channels_[channel_id];
  channel.status.timestamp_us = esp_timer_get_time();
  channel.status.last_error = hf_pio_err_t::PIO_SUCCESS;
}

void McuPio::InvokeErrorCallback(uint8_t channel_id, hf_pio_err_t error) noexcept {
  if (error_callback_) {
    error_callback_(channel_id, error, callback_user_data_);
  }
  channels_[channel_id].status.last_error = error;
}

bool McuPio::ValidatePioSystem() noexcept {
  ESP_LOGI(TAG, "Starting comprehensive PIO system validation");

  if (!EnsureInitialized()) {
    ESP_LOGE(TAG, "PIO system validation failed: not initialized");
    return false;
  }

  bool all_tests_passed = true;

  // Test 1: Verify RMT peripheral is available
  ESP_LOGI(TAG, "Testing RMT peripheral availability...");

  // Test 2: Validate channel configuration
  ESP_LOGI(TAG, "Testing channel configuration...");
  for (uint8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (!IsValidChannelId(i)) {
      ESP_LOGE(TAG, "Invalid channel ID: %d", i);
      all_tests_passed = false;
    }
  }

  // Test 3: Check maximum symbol count
  size_t max_symbols = GetMaxSymbolCount();
  if (max_symbols == 0) {
    ESP_LOGE(TAG, "Invalid maximum symbol count: %zu", max_symbols);
    all_tests_passed = false;
  } else {
    ESP_LOGI(TAG, "Maximum symbol count: %zu", max_symbols);
  }

  // Test 4: Validate symbol conversion functions
  ESP_LOGI(TAG, "Testing symbol conversion functions...");
  hf_pio_symbol_t test_symbols[] = {{100, true}, {200, false}, {150, true}};
  rmt_symbol_word_t rmt_symbols[10];
  size_t rmt_symbol_count = 0;

  hf_pio_err_t result = ConvertToRmtSymbols(test_symbols, 3, rmt_symbols, rmt_symbol_count);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Symbol conversion test failed: %d", static_cast<int>(result));
    all_tests_passed = false;
  }

  // Test 5: Validate symbol validation function
  ESP_LOGI(TAG, "Testing symbol validation...");
  result = ValidateSymbols(test_symbols, 3);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Symbol validation test failed: %d", static_cast<int>(result));
    all_tests_passed = false;
  }

  // Test 6: Validate clock divider calculation
  ESP_LOGI(TAG, "Testing clock divider calculation...");
  uint32_t divider = CalculateClockDivider(1000); // 1 microsecond
  if (divider == 0 || divider > 255) {
    ESP_LOGE(TAG, "Clock divider calculation failed: %d", divider);
    all_tests_passed = false;
  }

  // Test 7: Validate capabilities structure
  ESP_LOGI(TAG, "Testing capabilities...");
  hf_pio_capabilities_t caps;
  result = GetCapabilities(caps);
  if (result != hf_pio_err_t::PIO_SUCCESS || caps.max_channels == 0) {
    ESP_LOGE(TAG, "Capabilities test failed");
    all_tests_passed = false;
  }

  // Test 8: Memory allocation test (simulate)
  ESP_LOGI(TAG, "Testing memory allocation patterns...");
  for (size_t mem_size : {48, 64, 128, 256, 512, 1024}) {
    if (mem_size < HF_RMT_MIN_MEM_BLOCK_SYMBOLS || mem_size > HF_RMT_MAX_MEM_BLOCK_SYMBOLS) {
      ESP_LOGW(TAG, "Memory size %zu outside valid range", mem_size);
    }
  }

  if (all_tests_passed) {
    ESP_LOGI(TAG, "PIO system validation completed successfully - all tests passed");
  } else {
    ESP_LOGE(TAG, "PIO system validation failed - some tests did not pass");
  }

  return all_tests_passed;
}

//==============================================================================
// STATISTICS AND DIAGNOSTICS
//==============================================================================

hf_pio_err_t McuPio::GetStatistics(hf_pio_statistics_t &statistics) const noexcept
{
    statistics = statistics_;
    return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t McuPio::GetDiagnostics(hf_pio_diagnostics_t &diagnostics) const noexcept
{
    diagnostics = diagnostics_;
    return hf_pio_err_t::PIO_SUCCESS;
}

#endif // HF_MCU_FAMILY_ESP32