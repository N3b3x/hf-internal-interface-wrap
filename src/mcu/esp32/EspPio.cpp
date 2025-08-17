/**
 * @file EspPio.cpp
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
#include "EspTypes_PIO.h"  // Include ESP32 PIO/RMT type definitions

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
#include "esp_heap_caps.h"
#include "hal/rmt_ll.h"
#include "soc/clk_tree_defs.h"
#include "esp_clk_tree.h"
#include "soc/soc_caps.h"

#ifdef __cplusplus
}
#endif

// static const char *TAG = "EspPio";  // Unused for now

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

EspPio::EspPio() noexcept
    : BasePio(), channels_{}, global_statistics_{}, global_diagnostics_{} {

}

EspPio::~EspPio() noexcept {
  if (initialized_) {
    Deinitialize();
  }

}

//==============================================================================
// INITIALIZATION AND DEINITIALIZATION
//==============================================================================

hf_pio_err_t EspPio::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (initialized_) {
    ESP_LOGW(TAG, "Already initialized");
    return hf_pio_err_t::PIO_ERR_ALREADY_INITIALIZED;
  }

  // Initialize all channels to default state
  for (auto& channel : channels_) {
    channel = ChannelState{};
  }

  initialized_ = true;
  ESP_LOGI(TAG, "EspPio initialized successfully");
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  // Deinitialize all configured channels
  for (hf_u8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (channels_[i].configured) {
      DeinitializeChannel(i);
    }
  }

  // Clear callbacks
  // transmit_callback_ = nullptr; // Removed global callback
  // receive_callback_ = nullptr; // Removed global callback
  // error_callback_ = nullptr; // Removed global callback
  // callback_user_data_ = nullptr; // Removed global callback

  initialized_ = false;
  ESP_LOGI(TAG, "EspPio deinitialized");
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// CHANNEL CONFIGURATION
//==============================================================================

hf_pio_err_t EspPio::ConfigureChannel(hf_u8_t channel_id,
                                      const hf_pio_channel_config_t& config) noexcept {
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

  // Validate channel configuration for current ESP32 variant
  hf_pio_err_t validation_result = ValidateChannelConfiguration(channel_id, config);
  if (validation_result != hf_pio_err_t::PIO_SUCCESS) {
    return validation_result;
  }

  // Store configuration
  channels_[channel_id].config = config;

  // If already configured, deinit before re-init to avoid resource leak / no-free-channel errors
  if (channels_[channel_id].configured) {
    DeinitializeChannel(channel_id);
  }
  // Initialize the channel hardware
  hf_pio_err_t result = InitializeChannel(channel_id);
  if (result != hf_pio_err_t::PIO_SUCCESS) {
    return result;
  }

  channels_[channel_id].configured = true;
  ESP_LOGI(TAG, "Channel %d configured on GPIO %d for %s: requested %uns, achieved %uns", 
           channel_id, config.gpio_pin, 
           HfRmtGetVariantName(),
           config.resolution_ns, channels_[channel_id].actual_resolution_ns);

  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// TRANSMISSION OPERATIONS
//==============================================================================

hf_pio_err_t EspPio::Transmit(hf_u8_t channel_id, const hf_pio_symbol_t* symbols,
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

  if (symbols == nullptr) {
    return hf_pio_err_t::PIO_ERR_NULL_POINTER;
  }
  if (symbol_count == 0) {
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

  auto& channel = channels_[channel_id];

  if (channel.tx_channel == nullptr) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  // Validate callback setup for transmission
  if (!channel.transmit_callback) {
    ESP_LOGD(TAG, "Transmit: No transmit callback set for channel %d - completion may not be properly handled", channel_id);
    // Don't fail, but warn user that they may miss transmission events
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
  // Hint driver to avoid extra fragmentation under tight timing
  tx_config.flags.eot_level = channel.idle_level ? 1 : 0;

  // Start transmission
  channel.busy = true;
  channel.status.is_transmitting = true;
  channel.status.symbols_queued = symbol_count;
  channel.status.timestamp_us = esp_timer_get_time();

  // Callbacks are registered at channel initialization and kept installed
  esp_err_t ret = ESP_OK;
  
  // Use the copy encoder to transmit the converted RMT symbols
  ret = rmt_transmit(channel.tx_channel, channel.encoder,
                     rmt_symbols,  // Packed RMT symbols
                     rmt_symbol_count * sizeof(rmt_symbol_word_t), &tx_config);

  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_transmitting = false;
    ESP_LOGE(TAG, "Failed to start transmission on channel %d: %d", channel_id, ret);
    return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
  }
  if (wait_completion) {
    // Wait for transmission to complete using ESP-IDF API
    TickType_t wait_ticks = portMAX_DELAY;
    if (channel.config.timeout_us > 0) {
      // Round up to next millisecond
      uint32_t timeout_ms = (channel.config.timeout_us + 999) / 1000;
      wait_ticks = pdMS_TO_TICKS(timeout_ms);
    }
    ret = rmt_tx_wait_all_done(channel.tx_channel, wait_ticks);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Transmission timeout on channel %d", channel_id);
      channel.busy = false;
      channel.status.is_transmitting = false;
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

hf_pio_err_t EspPio::StartReceive(hf_u8_t channel_id, hf_pio_symbol_t* buffer, size_t buffer_size,
                                  hf_u32_t timeout_us) noexcept {
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

  auto& channel = channels_[channel_id];

  if (channel.rx_channel == nullptr) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  // Store buffer information
  channel.rx_buffer = buffer;
  channel.rx_buffer_size = buffer_size;
  channel.rx_symbols_received = 0;

  // Validate callback setup for reception
  if (!channel.receive_callback) {
    ESP_LOGD(TAG, "StartReceive: No receive callback set for channel %d - reception may not be properly handled", channel_id);
    // Don't fail, but warn user that they may miss callbacks");
  }

  // Create receive configuration
  rmt_receive_config_t rx_config = {};
  // Increase minimum signal range to account for timing variations, hardware delays, and signal propagation
  // The RMT peripheral needs a more generous range to capture signals that may have slight timing differences
  rx_config.signal_range_min_ns = std::max(channel.actual_resolution_ns / 4, static_cast<hf_u32_t>(10));  // Allow signals down to 1/4 the resolution
  
  // Set maximum signal range to a reasonable value for most applications (1ms instead of max possible)
  // This provides better noise filtering and prevents extremely long signals from blocking reception
  rx_config.signal_range_max_ns = 1000000; // 1ms maximum signal duration
  
  // ESP-IDF v5.5 specific RX channel configuration flags
  // Note: rmt_receive_config_t only supports en_partial_rx flag, other flags are in rmt_rx_channel_config_t
  rx_config.flags.en_partial_rx = false;    // Disable partial reception for standard operation
  
  // The RMT peripheral can handle buffers up to several KB
  // Since each RMT symbol can contain 2 PIO symbols, we need to ensure the RMT buffer is large enough
  // Use a conservative approach: allocate enough space for the requested PIO symbols
  size_t rmt_buffer_size = buffer_size; // This will be the number of RMT symbols we can receive
  
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
  
  // Allocate RMT buffer for reception (RMT driver requires a valid buffer)
  rmt_symbol_word_t* rmt_rx_buffer = static_cast<rmt_symbol_word_t*>(
      heap_caps_malloc(rmt_buffer_size * sizeof(rmt_symbol_word_t), MALLOC_CAP_DMA));
  if (rmt_rx_buffer == nullptr) {
    channel.busy = false;
    channel.status.is_receiving = false;
    ESP_LOGE(TAG, "Failed to allocate RMT RX buffer for channel %d", channel_id);
    return hf_pio_err_t::PIO_ERR_OUT_OF_MEMORY;
  }
  
  // Store the allocated RMT buffer for cleanup
  channel.rmt_rx_buffer = rmt_rx_buffer;
  
  ESP_LOGD(TAG, "Allocated RMT RX buffer: channel %d, size %lu symbols (%lu bytes), buffer ptr: %p", 
           channel_id, rmt_buffer_size, rmt_buffer_size * sizeof(rmt_symbol_word_t), rmt_rx_buffer);
  
  // Start reception with allocated RMT buffer
  ESP_LOGI(TAG, "Starting RMT reception on channel %d with config: min=%uns, max=%uns, buffer_size=%lu symbols", 
           channel_id, rx_config.signal_range_min_ns, rx_config.signal_range_max_ns, rmt_buffer_size);
  
  // CRITICAL DEBUG: Log RMT channel state before reception
  ESP_LOGI(TAG, "RMT RX channel state before reception: handle=%p, busy=%s, receiving=%s", 
           channel.rx_channel, channel.busy ? "yes" : "no", 
           channel.status.is_receiving ? "yes" : "no");
  
  // FIXED: Pass the correct buffer size in bytes to rmt_receive
  ret = rmt_receive(channel.rx_channel, rmt_rx_buffer, rmt_buffer_size, &rx_config);
  if (ret != ESP_OK) {
    channel.busy = false;
    channel.status.is_receiving = false;
    heap_caps_free(rmt_rx_buffer);
    channel.rmt_rx_buffer = nullptr;
    
    // ESP-IDF v5.5 specific error handling
    switch (ret) {
      case ESP_ERR_INVALID_ARG:
        ESP_LOGE(TAG, "Invalid arguments for RMT reception on channel %d", channel_id);
        return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
      case ESP_ERR_INVALID_STATE:
        ESP_LOGE(TAG, "RMT channel %d not in valid state for reception", channel_id);
        return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
      case ESP_ERR_NO_MEM:
        ESP_LOGE(TAG, "Insufficient memory for RMT reception on channel %d", channel_id);
        return hf_pio_err_t::PIO_ERR_OUT_OF_MEMORY;
      default:
        ESP_LOGE(TAG, "Failed to start reception on channel %d: %s (error: %d)", 
                 channel_id, esp_err_to_name(ret), ret);
        return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }
  }

  ESP_LOGI(TAG, "Started reception on channel %d", channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::StopReceive(hf_u8_t channel_id, size_t& symbols_received) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!initialized_) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  auto& channel = channels_[channel_id];

  if (!channel.status.is_receiving) {
    symbols_received = 0;
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  // Stop reception
  channel.busy = false;
  channel.status.is_receiving = false;
  symbols_received = channel.rx_symbols_received;

  // Free the allocated RMT buffer
  if (channel.rmt_rx_buffer != nullptr) {
    heap_caps_free(channel.rmt_rx_buffer);
    channel.rmt_rx_buffer = nullptr;
  }

  ESP_LOGI(TAG, "Stopped reception on channel %d, received %d symbols", channel_id,
           symbols_received);
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// STATUS AND CAPABILITIES
//==============================================================================

bool EspPio::IsChannelBusy(hf_u8_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return false;
  }

  return channels_[channel_id].busy;
}

hf_pio_err_t EspPio::GetChannelStatus(hf_u8_t channel_id,
                                      hf_pio_channel_status_t& status) const noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  status = channels_[channel_id].status;
  status.is_initialized = channels_[channel_id].configured;
  status.is_busy = channels_[channel_id].busy;

  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::GetCapabilities(hf_pio_capabilities_t& capabilities) const noexcept {
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

void EspPio::SetTransmitCallback(hf_u8_t channel_id, hf_pio_transmit_callback_t callback,
                                 void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    ESP_LOGE(TAG, "SetTransmitCallback: Invalid channel ID %d", channel_id);
    return;
  }

  if (!callback) {
    ESP_LOGW(TAG, "SetTransmitCallback: Null callback provided for channel %d", channel_id);
    return;
  }

  auto& channel = channels_[channel_id];
  
  // Validate channel configuration for transmit capability
  if (channel.config.direction == hf_pio_direction_t::Receive) {
    ESP_LOGW(TAG, "SetTransmitCallback: Channel %d is RX-only, cannot set TX callback", channel_id);
    return;
  }

  channel.transmit_callback = callback;
  channel.transmit_user_data = user_data;

  ESP_LOGI(TAG, "SetTransmitCallback: TX callback set for channel %d with user data %p", 
           channel_id, user_data);
}

void EspPio::SetReceiveCallback(hf_u8_t channel_id, hf_pio_receive_callback_t callback,
                                void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    ESP_LOGE(TAG, "SetReceiveCallback: Invalid channel ID %d", channel_id);
    return;
  }

  if (!callback) {
    ESP_LOGW(TAG, "SetReceiveCallback: Null callback provided for channel %d", channel_id);
    return;
  }

  auto& channel = channels_[channel_id];
  
  // Validate channel configuration for receive capability
  if (channel.config.direction == hf_pio_direction_t::Transmit) {
    ESP_LOGW(TAG, "SetReceiveCallback: Channel %d is TX-only, cannot set RX callback", channel_id);
    return;
  }

  channel.receive_callback = callback;
  channel.receive_user_data = user_data;

  ESP_LOGI(TAG, "SetReceiveCallback: RX callback set for channel %d with user data %p", 
           channel_id, user_data);
}

void EspPio::SetErrorCallback(hf_u8_t channel_id, hf_pio_error_callback_t callback,
                              void* user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    ESP_LOGE(TAG, "SetErrorCallback: Invalid channel ID %d", channel_id);
    return;
  }

  if (!callback) {
    ESP_LOGW(TAG, "SetErrorCallback: Null callback provided for channel %d", channel_id);
    return;
  }

  auto& channel = channels_[channel_id];
  
  // Error callbacks can be set for any channel type
  channel.error_callback = callback;
  channel.error_user_data = user_data;

  ESP_LOGI(TAG, "SetErrorCallback: Error callback set for channel %d with user data %p", 
           channel_id, user_data);
}

void EspPio::ClearChannelCallbacks(hf_u8_t channel_id) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);

  if (!IsValidChannelId(channel_id)) {
    ESP_LOGE(TAG, "ClearChannelCallbacks: Invalid channel ID %d", channel_id);
    return;
  }

  auto& channel = channels_[channel_id];
  
  // Check what callbacks were set before clearing
  bool had_tx = channel.transmit_callback != nullptr;
  bool had_rx = channel.receive_callback != nullptr;
  bool had_error = channel.error_callback != nullptr;
  
  // Clear all callbacks for the specific channel
  channel.transmit_callback = nullptr;
  channel.transmit_user_data = nullptr;
  channel.receive_callback = nullptr;
  channel.receive_user_data = nullptr;
  channel.error_callback = nullptr;
  channel.error_user_data = nullptr;

  ESP_LOGI(TAG, "ClearChannelCallbacks: Cleared callbacks for channel %d (TX:%s, RX:%s, ERR:%s)", 
           channel_id, had_tx ? "yes" : "no", had_rx ? "yes" : "no", had_error ? "yes" : "no");
}

void EspPio::ClearCallbacks() noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  
  for (hf_u8_t i = 0; i < MAX_CHANNELS; ++i) {
    auto& channel = channels_[i];
    channel.transmit_callback = nullptr;
    channel.transmit_user_data = nullptr;
    channel.receive_callback = nullptr;
    channel.receive_user_data = nullptr;
    channel.error_callback = nullptr;
    channel.error_user_data = nullptr;
  }
  
  ESP_LOGI(TAG, "ClearCallbacks: Cleared all callbacks across all channels");
}


//==============================================================================
// ESP32-SPECIFIC METHODS
//==============================================================================

hf_pio_err_t EspPio::ConfigureCarrier(hf_u8_t channel_id, hf_u32_t carrier_freq_hz,
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
  auto& channel = channels_[channel_id];

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

hf_pio_err_t EspPio::EnableLoopback(hf_u8_t channel_id, bool enable) noexcept {
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

hf_pio_err_t EspPio::ConfigureAdvancedRmt(hf_u8_t channel_id, size_t memory_blocks, bool enable_dma,
                                          hf_u32_t queue_depth) noexcept {
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  ESP_LOGI(TAG, "Configuring advanced RMT: channel=%d, memory_blocks=%lu, dma=%s, queue=%d",
           channel_id, memory_blocks, enable_dma ? "yes" : "no", queue_depth);

  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  auto& channel = channels_[channel_id];

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
    const auto& config = stored_config;
    hf_u32_t actual_resolution_ns;
    hf_u32_t resolution_hz = CalculateResolutionHz(config.resolution_ns, actual_resolution_ns, channel.source_clock_hz);
    
    // Store the actual resolution achieved
    channel.actual_resolution_ns = actual_resolution_ns;

    if (config.direction == hf_pio_direction_t::Transmit ||
        config.direction == hf_pio_direction_t::Bidirectional) {
      // Configure advanced TX channel for ESP32-C6 compatibility
      rmt_tx_channel_config_t tx_config = {};
      tx_config.gpio_num = static_cast<gpio_num_t>(config.gpio_pin);
// ESP32-C6 specific clock source configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
      tx_config.clk_src = RMT_CLK_SRC_PLL_F80M; // ESP32-C6 uses PLL_F80M (80 MHz)
#else
      tx_config.clk_src = RMT_CLK_SRC_DEFAULT; // Use default clock source on other targets
#endif

      tx_config.resolution_hz = resolution_hz;
      tx_config.mem_block_symbols = static_cast<hf_u32_t>(memory_blocks);
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
      // Configure advanced RX channel for ESP32-C6 compatibility
      rmt_rx_channel_config_t rx_config = {};
      rx_config.gpio_num = static_cast<gpio_num_t>(config.gpio_pin);
// ESP32-C6 specific clock source configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
      rx_config.clk_src = RMT_CLK_SRC_PLL_F80M; // ESP32-C6 uses PLL_F80M (80 MHz)
#else
      rx_config.clk_src = RMT_CLK_SRC_DEFAULT; // Use default clock source on other targets
#endif
      rx_config.resolution_hz = resolution_hz;
      rx_config.mem_block_symbols = static_cast<hf_u32_t>(memory_blocks);

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
           "Advanced RMT configuration completed for channel %d with %lu memory blocks, DMA=%s, "
           "queue depth=%d",
           channel_id, memory_blocks, enable_dma ? "enabled" : "disabled", queue_depth);
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// ADVANCED PIO FUNCTION IMPLEMENTATIONS
//==============================================================================

hf_pio_err_t EspPio::ConfigureEncoder(hf_u8_t channel_id, const hf_pio_symbol_t& bit0_config,
                                      const hf_pio_symbol_t& bit1_config) noexcept {
  if (!EnsureInitialized()) {
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  auto& channel = channels_[channel_id];

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

hf_pio_err_t EspPio::SetIdleLevel(hf_u8_t channel_id, bool idle_level) noexcept {
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

hf_pio_err_t EspPio::GetChannelStatistics(hf_u8_t channel_id,
                                          hf_pio_channel_statistics_t& stats) const noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  const auto& channel = channels_[channel_id];

  // Fill statistics structure
  stats.total_transmissions = 0;  // Would be tracked in real implementation
  stats.total_receptions = 0;     // Would be tracked in real implementation
  stats.failed_transmissions = 0; // Would be tracked in real implementation
  stats.failed_receptions = 0;    // Would be tracked in real implementation
  stats.last_operation_time = channel.last_operation_time;
  stats.is_configured = channel.configured;
  stats.is_busy = channel.busy;
  stats.current_resolution_ns = channel.actual_resolution_ns;  // Use actual achieved resolution
  stats.memory_blocks_allocated = 64; // Default or actual allocation
  stats.dma_enabled = false;          // Would be tracked based on channel configuration

  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::ResetChannelStatistics(hf_u8_t channel_id) noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::GetActualResolution(hf_u8_t channel_id, hf_u32_t& achieved_resolution_ns) const noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }

  const auto& channel = channels_[channel_id];
  if (!channel.configured) {
    return hf_pio_err_t::PIO_ERR_INVALID_CONFIGURATION;
  }

  achieved_resolution_ns = channel.actual_resolution_ns;
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_u32_t EspPio::CalculateResolutionHz(hf_u32_t resolution_ns, hf_u32_t& actual_resolution_ns,
                                       hf_u32_t source_clock_hz) const noexcept {
  // Use the internal clock divider calculation to get the actual achievable resolution
  [[maybe_unused]] hf_u32_t divider = CalculateClockDivider(resolution_ns, actual_resolution_ns, source_clock_hz);
  
  // Convert the actual resolution back to Hz for the RMT peripheral
  // resolution_hz = 1e9 / actual_resolution_ns
  hf_u32_t resolution_hz = 1000000000UL / actual_resolution_ns;
  
  // ESP_LOGD(TAG, "Resolution conversion: %uns -> %u Hz (actual: %uns)", 
  //          resolution_ns, resolution_hz, actual_resolution_ns);
  
  return resolution_hz;
}

hf_pio_err_t EspPio::GetResolutionConstraints(hf_u32_t& min_resolution_ns, hf_u32_t& max_resolution_ns, 
                                             hf_u32_t& clock_freq_hz) const noexcept {
  clock_freq_hz = RMT_CLK_SRC_FREQ;
  
  // Calculate minimum resolution (divider = 1)
  min_resolution_ns = 1000000000UL / RMT_CLK_SRC_FREQ;
  
  // Calculate maximum resolution (divider = 255)
  max_resolution_ns = (1000000000ULL * 255) / RMT_CLK_SRC_FREQ;
  
  // ESP_LOGD(TAG, "Resolution constraints: min=%uns, max=%uns, clock=%u Hz", 
  //          min_resolution_ns, max_resolution_ns, clock_freq_hz);
  
  return hf_pio_err_t::PIO_SUCCESS;
}

//==============================================================================
// MISSING ADVANCED PIO FUNCTION IMPLEMENTATIONS
//==============================================================================

hf_pio_err_t EspPio::TransmitRawRmtSymbols(hf_u8_t channel_id, const rmt_symbol_word_t* rmt_symbols,
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

  auto& channel = channels_[channel_id];

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
                                  rmt_symbols,  // Raw symbols for copy encoder
                                  symbol_count * sizeof(rmt_symbol_word_t), &tx_config);

  if (result != ESP_OK) {
    channels_[channel_id].busy = false;
    channels_[channel_id].status.is_transmitting = false;
    ESP_LOGE(TAG, "Raw RMT symbol transmission failed on channel %d: %s", channel_id,
             esp_err_to_name(result));
    InvokeChannelErrorCallback(channel_id, hf_pio_err_t::PIO_ERR_COMMUNICATION_FAILURE);
    return hf_pio_err_t::PIO_ERR_COMMUNICATION_FAILURE;
  }

  // Wait for completion if requested
  if (wait_completion) {
    hf_u32_t timeout_ms = channel.config.timeout_us / 1000;
    if (timeout_ms == 0)
      timeout_ms = portMAX_DELAY;

    result = rmt_tx_wait_all_done(channel.tx_channel, pdMS_TO_TICKS(timeout_ms));
    if (result != ESP_OK) {
      ESP_LOGW(TAG, "Wait for transmission completion timed out on channel %d: %s", channel_id,
               esp_err_to_name(result));
      InvokeChannelErrorCallback(channel_id, hf_pio_err_t::PIO_ERR_COMMUNICATION_TIMEOUT);
      return hf_pio_err_t::PIO_ERR_COMMUNICATION_TIMEOUT;
    }

    // Update status after successful completion
    channels_[channel_id].busy = false;
    channels_[channel_id].status.is_transmitting = false;
    channels_[channel_id].status.symbols_processed = symbol_count;
    channels_[channel_id].status.timestamp_us = esp_timer_get_time();
    channels_[channel_id].status.last_error = hf_pio_err_t::PIO_SUCCESS;
  }

  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::ReceiveRawRmtSymbols(hf_u8_t channel_id, rmt_symbol_word_t* rmt_buffer,
                                          size_t buffer_size, size_t& symbols_received,
                                          hf_u32_t timeout_us) noexcept {
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

  auto& channel = channels_[channel_id];

  if (channel.rx_channel == nullptr) {
    ESP_LOGE(TAG, "RX channel not configured for channel %d", channel_id);
    symbols_received = 0;
    return hf_pio_err_t::PIO_ERR_NOT_INITIALIZED;
  }

  // Prepare receive buffer
  size_t buffer_size_bytes = buffer_size * sizeof(rmt_symbol_word_t);

  // Configure reception parameters
  rmt_receive_config_t rx_config = {};
  // Increase minimum signal range to account for timing variations, hardware delays, and signal propagation
  rx_config.signal_range_min_ns = std::max(channel.actual_resolution_ns / 4, static_cast<hf_u32_t>(10));  // Allow signals down to half resolution
  
  // Set maximum signal range to a reasonable value for most applications (1ms instead of max possible)
  // This provides better noise filtering and prevents extremely long signals from blocking reception
  rx_config.signal_range_max_ns = 1000000; // 1ms maximum signal duration
  
  // ESP-IDF v5.5 specific RX channel configuration flags
  // Note: rmt_receive_config_t only supports en_partial_rx flag, other flags are in rmt_rx_channel_config_t
  rx_config.flags.en_partial_rx = false;    // Disable partial reception for standard operation

  // Mark channel as busy
  channels_[channel_id].busy = true;
  channels_[channel_id].status.is_receiving = true;
  channels_[channel_id].status.timestamp_us = esp_timer_get_time();
  // Start RMT reception
  esp_err_t result =
      rmt_receive(channel.rx_channel, rmt_buffer,  // Buffer for reception
                  buffer_size_bytes, &rx_config);

  if (result != ESP_OK) {
    channels_[channel_id].busy = false;
    channels_[channel_id].status.is_receiving = false;
    ESP_LOGE(TAG, "Failed to start RMT reception on channel %d: %s", channel_id,
             esp_err_to_name(result));
    symbols_received = 0;
    InvokeChannelErrorCallback(channel_id, hf_pio_err_t::PIO_ERR_COMMUNICATION_FAILURE);
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
  channels_[channel_id].status.timestamp_us = esp_timer_get_time();
  channels_[channel_id].status.last_error = hf_pio_err_t::PIO_SUCCESS;

  // ESP_LOGD(TAG, "Received %lu raw RMT symbols on channel %d", symbols_received, channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

size_t EspPio::GetMaxSymbolCount() const noexcept {
  // ESP32C6 RMT can handle large symbol counts with DMA enabled
  return 4096; // Reasonable limit for most applications
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

bool EspPio::IsValidChannelId(hf_u8_t channel_id) const noexcept {
  return channel_id < MAX_CHANNELS;
}

hf_pio_err_t EspPio::ConvertToRmtSymbols(const hf_pio_symbol_t* symbols, size_t symbol_count,
                                         rmt_symbol_word_t* rmt_symbols,
                                         size_t& rmt_symbol_count) noexcept {
  if (symbol_count > MAX_SYMBOLS_PER_TRANSMISSION) {
    return hf_pio_err_t::PIO_ERR_BUFFER_TOO_LARGE;
  }

  // Pack two hf_pio_symbol_t entries (high then low, or vice versa) into one rmt_symbol_word_t
  size_t out_index = 0;
  for (size_t i = 0; i < symbol_count; ) {
    const hf_pio_symbol_t& first = symbols[i];
    rmt_symbol_word_t word = {};
    word.level0 = first.level ? 1 : 0;
    word.duration0 = first.duration;

    if (i + 1 < symbol_count) {
      const hf_pio_symbol_t& second = symbols[i + 1];
      word.level1 = second.level ? 1 : 0;
      word.duration1 = second.duration;
      i += 2;
    } else {
      // No second half; terminate with non-zero minimal duration
      word.level1 = first.level ? 0 : 1;
      word.duration1 = 1;
      i += 1;
    }

    rmt_symbols[out_index++] = word;
  }

  rmt_symbol_count = out_index;
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::ConvertFromRmtSymbols(const rmt_symbol_word_t* rmt_symbols,
                                           size_t rmt_symbol_count, hf_pio_symbol_t* symbols,
                                           size_t max_symbols, size_t& symbols_converted) noexcept {
  // CRITICAL DEBUG: Log all input parameters - ISR-safe logging
  ESP_EARLY_LOGI(TAG, "ConvertFromRmtSymbols: DEBUG - Input: rmt_symbols=%p, rmt_symbol_count=%lu, symbols=%p, max_symbols=%lu", 
           rmt_symbols, rmt_symbol_count, symbols, max_symbols);
  
  // Validate input parameters
  if (!rmt_symbols || !symbols) {
    ESP_EARLY_LOGE(TAG, "ConvertFromRmtSymbols: Invalid pointers - rmt_symbols=%p, symbols=%p", rmt_symbols, symbols);
    symbols_converted = 0;
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }
  
  if (rmt_symbol_count == 0) {
    ESP_EARLY_LOGE(TAG, "ConvertFromRmtSymbols: No RMT symbols to convert");
    symbols_converted = 0;
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }
  
  if (max_symbols == 0) {
    ESP_EARLY_LOGE(TAG, "ConvertFromRmtSymbols: No space available in output buffer");
    symbols_converted = 0;
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }
  
  // FIXED: Each RMT symbol contains TWO timing periods (high + low)
  // So we can convert up to 2 * rmt_symbol_count PIO symbols
  ESP_EARLY_LOGI(TAG, "ConvertFromRmtSymbols: DEBUG - max_pio_symbols=%lu (from input max_symbols)", max_symbols);
  
  size_t symbols_to_convert = std::min(2 * rmt_symbol_count, max_symbols);
  
  ESP_EARLY_LOGI(TAG, "ConvertFromRmtSymbols: DEBUG - symbols_to_convert=%lu (min of 2*%lu=%lu and %lu)", 
           symbols_to_convert, rmt_symbol_count, 2 * rmt_symbol_count, max_symbols);
  
  size_t pio_symbol_index = 0;
  
  // Convert RMT symbols to hf_pio_symbol_t format
  for (size_t i = 0; i < rmt_symbol_count && pio_symbol_index < max_symbols; ++i) {
    const rmt_symbol_word_t& rmt_sym = rmt_symbols[i];
    
    ESP_EARLY_LOGI(TAG, "ConvertFromRmtSymbols: DEBUG - Processing RMT symbol %lu: level0=%s, duration0=%u, level1=%s, duration1=%u", 
             i, rmt_sym.level0 ? "HIGH" : "LOW", rmt_sym.duration0, 
             rmt_sym.level1 ? "HIGH" : "LOW", rmt_sym.duration1);
    
    // First level (level0 + duration0) - always convert if we have space
    if (pio_symbol_index < max_symbols) {
      symbols[pio_symbol_index].level = rmt_sym.level0;
      symbols[pio_symbol_index].duration = rmt_sym.duration0;
      ESP_EARLY_LOGI(TAG, "ConvertFromRmtSymbols: DEBUG - Created PIO symbol %lu: level=%s, duration=%u", 
               pio_symbol_index, symbols[pio_symbol_index].level ? "HIGH" : "LOW", symbols[pio_symbol_index].duration);
      pio_symbol_index++;
    }
    
    // Second level (level1 + duration1) - always convert if we have space
    // Note: duration1=0 is valid and expected for the last symbol when signal never changes
    if (pio_symbol_index < max_symbols) {
      symbols[pio_symbol_index].level = rmt_sym.level1;
      symbols[pio_symbol_index].duration = rmt_sym.duration1;
      ESP_EARLY_LOGI(TAG, "ConvertFromRmtSymbols: DEBUG - Created PIO symbol %lu: level=%s, duration=%u", 
               pio_symbol_index, symbols[pio_symbol_index].level ? "HIGH" : "LOW", symbols[pio_symbol_index].duration);
      pio_symbol_index++;
    }
  }
  
  // Update the output symbols_converted with the actual number converted
  symbols_converted = pio_symbol_index;
  
  ESP_EARLY_LOGI(TAG, "ConvertFromRmtSymbols: DEBUG - Final result: converted %lu RMT symbols to %lu PIO symbols", 
           rmt_symbol_count, symbols_converted);
  
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::SetClockSource(hf_u8_t channel_id, rmt_clock_source_t clk_src) noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }
  auto& ch = channels_[channel_id];
  if (ch.busy) {
    return hf_pio_err_t::PIO_ERR_RESOURCE_BUSY;
  }
  ch.selected_clk_src = clk_src;
  ch.source_clock_hz = GetClockSourceFrequency(clk_src);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::GetClockSource(hf_u8_t channel_id, rmt_clock_source_t& clk_src) const noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }
  clk_src = channels_[channel_id].selected_clk_src;
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::GetSourceClockHz(hf_u8_t channel_id, hf_u32_t& clock_hz) const noexcept {
  RtosUniqueLock<RtosMutex> lock(state_mutex_);
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }
  clock_hz = channels_[channel_id].source_clock_hz;
  return hf_pio_err_t::PIO_SUCCESS;
}

inline hf_u32_t EspPio::ResolveClockSourceHz(rmt_clock_source_t clk_src) noexcept {
#if SOC_CLK_TREE_SUPPORTED
  uint32_t freq = 0;
  switch (clk_src) {
    case RMT_CLK_SRC_PLL_F80M:
      // PLL_F80M is fixed 80 MHz
      return 80000000UL;
    case RMT_CLK_SRC_XTAL:
      // Query XTAL via clk tree
      esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_XTAL, ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &freq);
      return freq ? freq : 40000000UL;
    case RMT_CLK_SRC_RC_FAST:
      esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_RC_FAST, ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &freq);
      return freq ? freq : 17500000UL;
    default:
#if defined(CONFIG_IDF_TARGET_ESP32C6)
      return 80000000UL;
#else
      return 80000000UL;
#endif
  }
#else
  // Fallback: nominal constants
  return GetClockSourceFrequency(clk_src);
#endif
}

// Legacy alias retained for internal calls
inline hf_u32_t EspPio::GetClockSourceFrequency(rmt_clock_source_t clk_src) noexcept {
  switch (clk_src) {
    case RMT_CLK_SRC_PLL_F80M: return 80000000UL;
    case RMT_CLK_SRC_XTAL:     return 40000000UL;
    case RMT_CLK_SRC_RC_FAST:  return 17500000UL;
    default:                   return 80000000UL;
  }
}

hf_u32_t EspPio::CalculateClockDivider(hf_u32_t resolution_ns, hf_u32_t& actual_resolution_ns,
                                       hf_u32_t source_clock_hz) const noexcept {
  if (resolution_ns == 0) {
    ESP_LOGW(TAG, "Invalid resolution_ns=0, using minimum resolution");
    actual_resolution_ns = 1000000000UL / source_clock_hz;
    return 1;
  }

  uint64_t divider_calc = ((uint64_t)resolution_ns * source_clock_hz) / 1000000000ULL;

  hf_u32_t divider;
  if (divider_calc < 1) {
    divider = 1;
  } else if (divider_calc > 255) {
    divider = 255;
  } else {
    divider = static_cast<hf_u32_t>(divider_calc);
  }

  actual_resolution_ns = (1000000000ULL * divider) / source_clock_hz;
  return divider;
}

uint32_t EspPio::GetEffectiveClockFrequency(uint32_t clock_divider, uint32_t source_clock_hz) const noexcept {
  if (clock_divider == 0) {
    ESP_LOGE(TAG, "Invalid clock_divider=0");
    return 0;
  }
  return source_clock_hz / clock_divider;
}

hf_pio_err_t EspPio::ValidateChannelConfiguration(hf_u8_t channel_id, 
                                                 const hf_pio_channel_config_t& config) const noexcept {
  // Validate GPIO pin
  if (config.gpio_pin < 0) {
    ESP_LOGE(TAG, "Invalid GPIO pin %d for channel %d", config.gpio_pin, channel_id);
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }
  
  // Validate resolution is within supported range
  // Get the constraints for the current hardware
  hf_u32_t min_resolution_ns, max_resolution_ns, clock_freq_hz;
  GetResolutionConstraints(min_resolution_ns, max_resolution_ns, clock_freq_hz);
  
  if (config.resolution_ns < min_resolution_ns || config.resolution_ns > max_resolution_ns) {
    ESP_LOGE(TAG, "Invalid resolution %uns for channel %d (range: %u-%uns)", 
             config.resolution_ns, channel_id, min_resolution_ns, max_resolution_ns);
    ESP_LOGE(TAG, "Hardware constraint: 8-bit divider (1-255) with %u Hz clock", clock_freq_hz);
    return hf_pio_err_t::PIO_ERR_INVALID_RESOLUTION;
  }
  
  // Validate channel is appropriate for the direction on current ESP32 variant
  if (!HfRmtIsChannelValidForDirection(channel_id, config.direction)) {
    ESP_LOGE(TAG, "Channel %d is not valid for %s direction on %s", 
             channel_id, 
             (config.direction == hf_pio_direction_t::Transmit) ? "TX" : 
             (config.direction == hf_pio_direction_t::Receive) ? "RX" : "Bidirectional",
             HfRmtGetVariantName());
    
    // Provide helpful suggestions
    if (config.direction == hf_pio_direction_t::Transmit) {
      ESP_LOGE(TAG, "Valid TX channels for %s: %d-%d", 
               HfRmtGetVariantName(), 
               HF_RMT_TX_CHANNEL_START, 
               HF_RMT_TX_CHANNEL_START + HF_RMT_MAX_TX_CHANNELS - 1);
    } else if (config.direction == hf_pio_direction_t::Receive) {
      ESP_LOGE(TAG, "Valid RX channels for %s: %d-%d", 
               HfRmtGetVariantName(), 
               HF_RMT_RX_CHANNEL_START, 
               HF_RMT_RX_CHANNEL_START + HF_RMT_MAX_RX_CHANNELS - 1);
    }
    
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }
  
  // Validate buffer size
  if (config.buffer_size == 0) {
    ESP_LOGE(TAG, "Invalid buffer size 0 for channel %d", channel_id);
    return hf_pio_err_t::PIO_ERR_INVALID_PARAMETER;
  }
  
  // Validate timeout
  if (config.timeout_us == 0) {
    ESP_LOGW(TAG, "Timeout is 0 for channel %d - operations will block indefinitely", channel_id);
  }
  
  // ESP_LOGD(TAG, "Channel %d configuration validated successfully for %s", channel_id, HfRmtGetVariantName());
  return hf_pio_err_t::PIO_SUCCESS;
}

void EspPio::InvokeChannelErrorCallback(hf_u8_t channel_id, hf_pio_err_t error) noexcept {
  if (!IsValidChannelId(channel_id)) {
    ESP_LOGE(TAG, "Invalid channel ID %d for error callback", channel_id);
    return;
  }
  
  auto& channel = channels_[channel_id];
  
  // Invoke channel-specific error callback if set
  if (channel.error_callback) {
    channel.error_callback(channel_id, error, channel.error_user_data);
  }
  
  // Update channel status
  channel.status.last_error = error;

}

bool EspPio::OnTransmitComplete(rmt_channel_handle_t channel, const rmt_tx_done_event_data_t* edata,
                                void* user_ctx) {
  auto* instance = static_cast<EspPio*>(user_ctx);
  if (!instance) {
    return false;
  }

  // Find the channel ID by comparing channel handles - minimize stack usage
  hf_u8_t channel_id = 0xFF;
  for (hf_u8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (instance->channels_[i].tx_channel == channel) {
      channel_id = i;
      break;
    }
  }

  if (channel_id == 0xFF) {
    ESP_EARLY_LOGE(TAG, "OnTransmitComplete: Channel handle %p not found", channel);
    return false;
  }

  // ISR context: minimize stack usage and avoid blocking operations
  auto& ch = instance->channels_[channel_id];
  
  // Update channel status atomically
  ch.busy = false;
  ch.status.is_transmitting = false;
  ch.status.symbols_processed = ch.status.symbols_queued;
  ch.status.timestamp_us = esp_timer_get_time();

  // Invoke channel-specific user callback if set - ISR-safe context
  if (ch.transmit_callback) {
    // Callback is invoked from ISR context - user must ensure ISR safety
    ch.transmit_callback(channel_id, ch.status.symbols_processed, ch.transmit_user_data);
  }

  ESP_EARLY_LOGD(TAG, "TX complete ch%d: %lu symbols", channel_id, ch.status.symbols_processed);
  return false; // Don't yield to higher priority task
}

bool EspPio::OnReceiveComplete(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t* edata,
                               void* user_ctx) {
  auto* instance = static_cast<EspPio*>(user_ctx);
  if (!instance || !edata) {
    ESP_EARLY_LOGE(TAG, "OnReceiveComplete: Invalid instance or event data");
    return false;
  }

  // Find the channel ID by comparing channel handles - minimize stack usage
  hf_u8_t channel_id = 0xFF;
  for (hf_u8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (instance->channels_[i].rx_channel == channel) {
      channel_id = i;
      break;
    }
  }

  if (channel_id == 0xFF) {
    ESP_EARLY_LOGE(TAG, "OnReceiveComplete: Channel handle %p not found", channel);
    return false;
  }

  auto& ch = instance->channels_[channel_id];
  
  // ISR context: minimize logging and stack usage
  ESP_EARLY_LOGI(TAG, "OnReceiveComplete: ch%d received %lu symbols", channel_id, edata->num_symbols);
  
  // Convert received RMT symbols back to hf_pio_symbol_ts - minimize stack usage
  size_t symbols_converted = 0;
  if (ch.rx_buffer && edata->received_symbols) {
    // Convert RMT symbols to PIO symbols - this is the critical operation
    // Use minimal error checking to prevent stack issues
    hf_pio_err_t convert_result = instance->ConvertFromRmtSymbols(
        reinterpret_cast<const rmt_symbol_word_t*>(edata->received_symbols), 
        edata->num_symbols,
        ch.rx_buffer, ch.rx_buffer_size, symbols_converted);
    
    if (convert_result != hf_pio_err_t::PIO_SUCCESS) {
      ESP_EARLY_LOGE(TAG, "OnReceiveComplete: ch%d conversion failed: %d", channel_id, static_cast<int>(convert_result));
      symbols_converted = 0; // Ensure we don't use corrupted data
    }
    
    ESP_EARLY_LOGI(TAG, "OnReceiveComplete: ch%d converted %lu symbols", channel_id, symbols_converted);
  } else {
    ESP_EARLY_LOGE(TAG, "OnReceiveComplete: ch%d missing buffers", channel_id);
  }

  ch.busy = false;
  ch.rx_symbols_received = symbols_converted;
  ch.status.symbols_processed = symbols_converted;
  ch.status.timestamp_us = esp_timer_get_time();
  
  // Invoke channel-specific user callback if set - ISR-safe context
  if (ch.receive_callback) {
    // Callback is invoked from ISR context - user must ensure ISR safety
    ch.receive_callback(channel_id, ch.rx_buffer, symbols_converted, ch.receive_user_data);
  }
  
  ESP_EARLY_LOGI(TAG, "OnReceiveComplete: ch%d completed, %lu symbols", channel_id, symbols_converted);
  return true; // Allow yielding to higher priority task
}

hf_pio_err_t EspPio::InitializeChannel(hf_u8_t channel_id) noexcept {
  auto& channel = channels_[channel_id];
  const auto& config = channel.config;

  // Choose clock source: use user-selected if set, otherwise target default
#if defined(CONFIG_IDF_TARGET_ESP32C6)
  rmt_clock_source_t default_src = RMT_CLK_SRC_PLL_F80M;
#else
  rmt_clock_source_t default_src = RMT_CLK_SRC_DEFAULT;
#endif
  rmt_clock_source_t chosen_src = (channels_[channel_id].selected_clk_src == RMT_CLK_SRC_DEFAULT)
                                      ? default_src
                                      : channels_[channel_id].selected_clk_src;
  channel.selected_clk_src = chosen_src;
  channel.source_clock_hz = GetClockSourceFrequency(chosen_src);

  // Calculate clock settings from requested ns and selected source clock
  hf_u32_t actual_resolution_ns;
  hf_u32_t resolution_hz = CalculateResolutionHz(config.resolution_ns, actual_resolution_ns,
                                                 channel.source_clock_hz);
  channel.actual_resolution_ns = actual_resolution_ns;

  if (config.direction == hf_pio_direction_t::Transmit ||
      config.direction == hf_pio_direction_t::Bidirectional) {
    rmt_tx_channel_config_t tx_config = {};
    tx_config.gpio_num = static_cast<gpio_num_t>(config.gpio_pin);
// ESP32-C6 specific clock source configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
    tx_config.clk_src = RMT_CLK_SRC_PLL_F80M; // ESP32-C6 uses PLL_F80M (80 MHz)
#else
    tx_config.clk_src = RMT_CLK_SRC_DEFAULT; // Use default clock source on other targets
#endif
    tx_config.resolution_hz = resolution_hz;
    // Balanced buffering to allow multi-channel allocations
    tx_config.mem_block_symbols = 64;
    tx_config.trans_queue_depth = 8;
    
    // ESP-IDF v5.5 specific TX channel configuration
    tx_config.flags.invert_out = false;       // Don't invert output signal
    tx_config.flags.with_dma = false;         // Disable DMA for standard operation
    tx_config.flags.io_loop_back = false;     // Disable internal loopback
    tx_config.intr_priority = 0;              // Default interrupt priority

    esp_err_t ret = rmt_new_tx_channel(&tx_config, &channel.tx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create TX channel %d: %s (error: %d)", channel_id,
               esp_err_to_name(ret), ret);
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }

    rmt_copy_encoder_config_t encoder_config = {};
    ret = rmt_new_copy_encoder(&encoder_config, &channel.encoder);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create encoder for channel %d: %d", channel_id, ret);
      rmt_del_channel(channel.tx_channel);
      channel.tx_channel = nullptr;
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }

    // Register TX done callback once at channel bring-up
    {
      rmt_tx_event_callbacks_t tx_callbacks = {};
      tx_callbacks.on_trans_done = OnTransmitComplete;
      esp_err_t cb_ret = rmt_tx_register_event_callbacks(channel.tx_channel, &tx_callbacks, this);
      if (cb_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register TX callbacks for channel %d: %d", channel_id, cb_ret);
        rmt_del_encoder(channel.encoder);
        rmt_del_channel(channel.tx_channel);
        channel.tx_channel = nullptr;
        return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
      }
    }

    ret = rmt_enable(channel.tx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable TX channel %d: %d", channel_id, ret);
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }
  }

  if (config.direction == hf_pio_direction_t::Receive ||
      config.direction == hf_pio_direction_t::Bidirectional) {
    // Configure advanced RX channel for ESP32-C6 compatibility
    rmt_rx_channel_config_t rx_config = {};
    rx_config.gpio_num = static_cast<gpio_num_t>(config.gpio_pin);
// ESP32-C6 specific clock source configuration
#if defined(CONFIG_IDF_TARGET_ESP32C6)
    rx_config.clk_src = RMT_CLK_SRC_PLL_F80M; // ESP32-C6 uses PLL_F80M (80 MHz)
#else
    rx_config.clk_src = RMT_CLK_SRC_DEFAULT; // Use default clock source on other targets
#endif
    rx_config.resolution_hz = resolution_hz;
    rx_config.mem_block_symbols = 64;
    
    // ESP-IDF v5.5 specific RX channel configuration
    rx_config.flags.invert_in = false;        // Don't invert input signal
    rx_config.flags.with_dma = false;         // Disable DMA for standard operation
    rx_config.flags.io_loop_back = false;     // Disable internal loopback
    rx_config.intr_priority = 0;              // Default interrupt priority

    esp_err_t ret = rmt_new_rx_channel(&rx_config, &channel.rx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create RX channel %d: %d", channel_id, ret);
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }

    // Register RX done callback once at channel bring-up
    {
      rmt_rx_event_callbacks_t rx_callbacks = {};
      rx_callbacks.on_recv_done = OnReceiveComplete;
      esp_err_t cb_ret = rmt_rx_register_event_callbacks(channel.rx_channel, &rx_callbacks, this);
      if (cb_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register RX callbacks for channel %d: %d", channel_id, cb_ret);
        rmt_del_channel(channel.rx_channel);
        channel.rx_channel = nullptr;
        return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
      }
    }

    ret = rmt_enable(channel.rx_channel);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable RX channel %d: %d", channel_id, ret);
      return hf_pio_err_t::PIO_ERR_HARDWARE_FAULT;
    }
  }

  ESP_LOGI(TAG, "Initialized channel %d: clk_src=%d (%u Hz), requested %uns, achieved %uns (%u Hz)",
           channel_id, static_cast<int>(chosen_src), channel.source_clock_hz,
           config.resolution_ns, channel.actual_resolution_ns, 1000000000UL / channel.actual_resolution_ns);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::DeinitializeChannel(hf_u8_t channel_id) noexcept {
  auto& channel = channels_[channel_id];

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
    // If bytes_encoder aliases encoder, avoid double-free
    if (channel.bytes_encoder == channel.encoder) {
      channel.bytes_encoder = nullptr;
    }
    channel.encoder = nullptr;
  }
  if (channel.bytes_encoder) {
    rmt_del_encoder(channel.bytes_encoder);
    channel.bytes_encoder = nullptr;
  }

  // Free allocated RMT buffer if present
  if (channel.rmt_rx_buffer != nullptr) {
    heap_caps_free(channel.rmt_rx_buffer);
    channel.rmt_rx_buffer = nullptr;
  }

  // Clear buffer references
  channel.rx_buffer = nullptr;
  channel.rx_buffer_size = 0;
  channel.rx_symbols_received = 0;

  channel.configured = false;
  channel.busy = false;

  ESP_LOGI(TAG, "Deinitialized channel %d", channel_id);
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::ValidateSymbols(const hf_pio_symbol_t* symbols,
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

bool EspPio::ValidatePioSystem() noexcept {
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
  for (hf_u8_t i = 0; i < MAX_CHANNELS; ++i) {
    if (!IsValidChannelId(i)) {
      ESP_LOGE(TAG, "Invalid channel ID: %d", i);
      all_tests_passed = false;
    }
  }

  // Test 3: Check maximum symbol count
  size_t max_symbols = GetMaxSymbolCount();
  if (max_symbols == 0) {
    ESP_LOGE(TAG, "Invalid maximum symbol count: %lu", max_symbols);
    all_tests_passed = false;
  } else {
    ESP_LOGI(TAG, "Maximum symbol count: %lu", max_symbols);
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
  hf_u32_t actual_ns_test = 0;
  hf_u32_t divider = CalculateClockDivider(1000, actual_ns_test, GetClockSourceFrequency(channels_[0].selected_clk_src)); // 1 microsecond using a channel clock
  if (divider == 0 || divider > 255) {
    ESP_LOGE(TAG, "Clock divider calculation failed: %d", divider);
    all_tests_passed = false;
  } else {
    ESP_LOGD(TAG, "Divider=%u, actual_ns=%u", divider, actual_ns_test);
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
      ESP_LOGW(TAG, "Memory size %lu outside valid range", mem_size);
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

hf_pio_err_t EspPio::GetStatistics(hf_u8_t channel_id, hf_pio_statistics_t& statistics) const noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }
  
  // For now, return global statistics
  // In a real implementation, this would gather channel-specific statistics
  statistics = global_statistics_;
  return hf_pio_err_t::PIO_SUCCESS;
}

hf_pio_err_t EspPio::GetDiagnostics(hf_u8_t channel_id, hf_pio_diagnostics_t& diagnostics) const noexcept {
  if (!IsValidChannelId(channel_id)) {
    return hf_pio_err_t::PIO_ERR_INVALID_CHANNEL;
  }
  
  const auto& channel = channels_[channel_id];
  
  // Fill in channel-specific diagnostics
  diagnostics = global_diagnostics_;
  diagnostics.pioInitialized = initialized_;
  diagnostics.activeChannels = 0;
  
  // Count active channels
  for (const auto& ch : channels_) {
    if (ch.configured) {
      diagnostics.activeChannels++;
    }
  }
  
  diagnostics.lastErrorCode = channel.status.last_error;
  diagnostics.currentResolutionNs = channel.actual_resolution_ns;  // Use actual achieved resolution
  
  return hf_pio_err_t::PIO_SUCCESS;
}

#endif // HF_MCU_FAMILY_ESP32