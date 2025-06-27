/**
 * @file McuCan.cpp
 * @brief Implementation of MCU-integrated CAN controller.
 *
 * This file provides the implementation for CAN bus communication using the
 * microcontroller's built-in CAN peripheral. For ESP32, it wraps TWAI
 * (Two-Wire Automotive Interface). All platform-specific types and
 * implementations are isolated through McuTypes.h.
 */
#include "McuCan.h"
#include <algorithm>
#include <mutex>

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

McuCan::McuCan(const CanBusConfig &config) noexcept : config_(config), receive_callback_(nullptr) {}

McuCan::~McuCan() noexcept {
  Deinitialize();
}

//==============================================================================
// INITIALIZATION AND DEINITIALIZATION
//==============================================================================

bool McuCan::Initialize() noexcept {
  if (initialized_) {
    return true;
  }

  if (!PlatformInitialize()) {
    return false;
  }

  initialized_ = true;
  return true;
}

bool McuCan::Deinitialize() noexcept {
  if (!initialized_) {
    return true;
  }

  PlatformDeinitialize();
  initialized_ = false;
  receive_callback_ = nullptr;
  return true;
}

bool McuCan::Start() noexcept {
  if (!initialized_) {
    return false;
  }

  return PlatformStart();
}

bool McuCan::Stop() noexcept {
  if (!initialized_) {
    return false;
  }

  return PlatformStop();
}

//==============================================================================
// MESSAGE TRANSMISSION AND RECEPTION
//==============================================================================

bool McuCan::SendMessage(const CanMessage &message, uint32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return false;
  }

  return PlatformSendMessage(message, timeout_ms);
}

bool McuCan::ReceiveMessage(CanMessage &message, uint32_t timeout_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return false;
  }

  return PlatformReceiveMessage(message, timeout_ms);
}

//==============================================================================
// CALLBACK MANAGEMENT
//==============================================================================

bool McuCan::SetReceiveCallback(CanReceiveCallback callback) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return false;
  }

  receive_callback_ = callback;
  return true;
}

void McuCan::ClearReceiveCallback() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  receive_callback_ = nullptr;
}

//==============================================================================
// STATUS AND DIAGNOSTICS
//==============================================================================

bool McuCan::GetStatus(CanBusStatus &status) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return false;
  }

  return PlatformGetStatus(status);
}

bool McuCan::Reset() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return false;
  }

  return PlatformReset();
}

bool McuCan::IsTransmitQueueFull() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return true;
  }

  return PlatformIsTransmitQueueFull();
}

bool McuCan::IsReceiveQueueEmpty() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return true;
  }

  return PlatformIsReceiveQueueEmpty();
}

uint32_t McuCan::GetTransmitErrorCount() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return 0;
  }

  return PlatformGetTransmitErrorCount();
}

uint32_t McuCan::GetReceiveErrorCount() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return 0;
  }

  return PlatformGetReceiveErrorCount();
}

//==============================================================================
// FILTER MANAGEMENT
//==============================================================================

bool McuCan::SetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return false;
  }

  return PlatformSetAcceptanceFilter(id, mask, extended);
}

bool McuCan::ClearAcceptanceFilter() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return false;
  }

  return PlatformClearAcceptanceFilter();
}

//==============================================================================
// PLATFORM-SPECIFIC IMPLEMENTATION METHODS
//==============================================================================

bool McuCan::PlatformInitialize() noexcept {
  // Configure TWAI driver (ESP32's CAN implementation)
  hf_can_general_config_t general_config = {
      .mode = config_.silent_mode
                  ? HF_CAN_MODE_LISTEN_ONLY
                  : (config_.loopback_mode ? HF_CAN_MODE_LOOPBACK : HF_CAN_MODE_NORMAL),
      .tx_io = config_.tx_pin,
      .rx_io = config_.rx_pin,
      .clkout_io = HF_CAN_IO_UNUSED,
      .bus_off_io = HF_CAN_IO_UNUSED,
      .tx_queue_len = static_cast<uint32_t>(config_.tx_queue_size),
      .rx_queue_len = static_cast<uint32_t>(config_.rx_queue_size),
      .alerts_enabled = HF_CAN_ALERT_ALL,
      .clkout_divider = 0};

  // Configure timing parameters based on baud rate
  hf_can_timing_config_t timing_config;
  if (!GetTimingConfig(config_.baudrate, &timing_config)) {
    return false;
  }

  // Configure filter (accept all by default)
  hf_can_filter_config_t filter_config = {
      .acceptance_code = 0, .acceptance_mask = 0xFFFFFFFF, .single_filter = true};

  // Install TWAI driver
  if (HF_CAN_DRIVER_INSTALL(&general_config, &timing_config, &filter_config) != HF_CAN_OK) {
    return false;
  }

  return true;
}

bool McuCan::PlatformDeinitialize() noexcept {
  // Uninstall TWAI driver
  HF_CAN_DRIVER_UNINSTALL();
  return true;
}

bool McuCan::PlatformStart() noexcept {
  return HF_CAN_START() == HF_CAN_OK;
}

bool McuCan::PlatformStop() noexcept {
  return HF_CAN_STOP() == HF_CAN_OK;
}

bool McuCan::PlatformSendMessage(const CanMessage &message, uint32_t timeout_ms) noexcept {
  // Convert CanMessage to platform-specific hf_can_message_t
  hf_can_message_t platform_msg;
  platform_msg.id = message.id;
  platform_msg.is_extended = message.extended_id;
  platform_msg.is_rtr = message.remote_frame;
  platform_msg.dlc = message.data_length > 8 ? 8 : message.data_length; // Classic CAN limit

  // Copy data (limit to 8 bytes for classic CAN)
  for (uint8_t i = 0; i < platform_msg.dlc && i < sizeof(platform_msg.data); ++i) {
    platform_msg.data[i] = message.data[i];
  }

  hf_can_err_t result = HF_CAN_TRANSMIT(&platform_msg, HF_TICKS_FROM_MS(timeout_ms));
  return result == HF_CAN_OK;
}

bool McuCan::PlatformReceiveMessage(CanMessage &message, uint32_t timeout_ms) noexcept {
  hf_can_message_t platform_msg;
  hf_can_err_t result = HF_CAN_RECEIVE(&platform_msg, HF_TICKS_FROM_MS(timeout_ms));

  if (result == HF_CAN_OK) {
    // Convert platform message to CanMessage
    message.id = platform_msg.id;
    message.extended_id = platform_msg.is_extended;
    message.remote_frame = platform_msg.is_rtr;
    message.format = CanFrameFormat::Classic; // ESP32 TWAI only supports classic CAN
    message.dlc = platform_msg.dlc;
    message.data_length = platform_msg.dlc;
    message.error_state_indicator = false;
    message.bit_rate_switch = false;

    // Copy data
    for (uint8_t i = 0; i < platform_msg.dlc && i < sizeof(message.data); ++i) {
      message.data[i] = platform_msg.data[i];
    }

    // Clear remaining data
    for (uint8_t i = platform_msg.dlc; i < sizeof(message.data); ++i) {
      message.data[i] = 0;
    }

    return true;
  }

  return false;
}

bool McuCan::PlatformGetStatus(CanBusStatus &status) noexcept {
  hf_can_status_info_t status_info;

  if (HF_CAN_GET_STATUS_INFO(&status_info) != HF_CAN_OK) {
    return false;
  }

  status.tx_error_count = status_info.tx_error_counter;
  status.rx_error_count = status_info.rx_error_counter;
  status.tx_failed_count = status_info.tx_failed_count;
  status.rx_missed_count = status_info.rx_missed_count;
  status.bus_off = (status_info.state == HF_CAN_STATE_BUS_OFF);
  status.error_warning = (status_info.tx_error_counter > 96 || status_info.rx_error_counter > 96);
  status.error_passive = (status_info.tx_error_counter > 127 || status_info.rx_error_counter > 127);

  return true;
}

bool McuCan::PlatformReset() noexcept {
  // Stop the driver
  if (HF_CAN_STOP() != HF_CAN_OK) {
    return false;
  }

  // Start the driver again
  return HF_CAN_START() == HF_CAN_OK;
}

bool McuCan::PlatformSetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended) noexcept {
  // For ESP32 TWAI, filters need to be set during initialization
  // This would require stopping and restarting the driver
  // For now, return false to indicate filter changes are not supported at runtime
  (void)id;
  (void)mask;
  (void)extended;
  return false;
}

bool McuCan::PlatformClearAcceptanceFilter() noexcept {
  // Same limitation as above
  return false;
}

bool McuCan::PlatformIsTransmitQueueFull() const noexcept {
  hf_can_status_info_t status_info;

  if (HF_CAN_GET_STATUS_INFO(&status_info) != HF_CAN_OK) {
    return true;
  }

  return status_info.tx_queue_len >= config_.tx_queue_size;
}

bool McuCan::PlatformIsReceiveQueueEmpty() const noexcept {
  hf_can_status_info_t status_info;

  if (HF_CAN_GET_STATUS_INFO(&status_info) != HF_CAN_OK) {
    return true;
  }

  return status_info.rx_queue_len == 0;
}

uint32_t McuCan::PlatformGetTransmitErrorCount() const noexcept {
  hf_can_status_info_t status_info;

  if (HF_CAN_GET_STATUS_INFO(&status_info) != HF_CAN_OK) {
    return 0;
  }

  return status_info.tx_error_counter;
}

uint32_t McuCan::PlatformGetReceiveErrorCount() const noexcept {
  hf_can_status_info_t status_info;

  if (HF_CAN_GET_STATUS_INFO(&status_info) != HF_CAN_OK) {
    return 0;
  }

  return status_info.rx_error_counter;
}

bool McuCan::GetTimingConfig(uint32_t baud_rate, void *timing_config_ptr) noexcept {
  hf_can_timing_config_t *timing_config = static_cast<hf_can_timing_config_t *>(timing_config_ptr);

  // ESP32-C6 CAN timing configuration for different baud rates
  // These values are calculated for 40MHz APB clock

  switch (baud_rate) {
  case 1000000: // 1 Mbps
    timing_config->brp = 4;
    timing_config->tseg_1 = 15;
    timing_config->tseg_2 = 4;
    timing_config->sjw = 3;
    timing_config->triple_sampling = false;
    break;

  case 800000: // 800 kbps
    timing_config->brp = 4;
    timing_config->tseg_1 = 16;
    timing_config->tseg_2 = 8;
    timing_config->sjw = 3;
    timing_config->triple_sampling = false;
    break;

  case 500000: // 500 kbps
    timing_config->brp = 8;
    timing_config->tseg_1 = 15;
    timing_config->tseg_2 = 4;
    timing_config->sjw = 3;
    timing_config->triple_sampling = false;
    break;

  case 250000: // 250 kbps
    timing_config->brp = 16;
    timing_config->tseg_1 = 15;
    timing_config->tseg_2 = 4;
    timing_config->sjw = 3;
    timing_config->triple_sampling = false;
    break;

  case 125000: // 125 kbps
    timing_config->brp = 32;
    timing_config->tseg_1 = 15;
    timing_config->tseg_2 = 4;
    timing_config->sjw = 3;
    timing_config->triple_sampling = false;
    break;

  case 100000: // 100 kbps
    timing_config->brp = 40;
    timing_config->tseg_1 = 15;
    timing_config->tseg_2 = 4;
    timing_config->sjw = 3;
    timing_config->triple_sampling = false;
    break;

  default:
    return false; // Unsupported baud rate
  }

  return true;
}

//==============================================================================
// INTERRUPT HANDLING
//==============================================================================

void McuCan::StaticReceiveHandler(void *arg) {
  McuCan *instance = static_cast<McuCan *>(arg);
  if (instance) {
    instance->HandleReceiveInterrupt();
  }
}

void McuCan::HandleReceiveInterrupt() {
  if (receive_callback_) {
    CanMessage message;
    if (PlatformReceiveMessage(message, 0)) { // Non-blocking receive
      receive_callback_(message);
    }
  }
}

//==============================================================================
// CAN-FD SUPPORT METHODS
//==============================================================================

bool McuCan::SupportsCanFD() const noexcept {
  // ESP32-C6 TWAI does not support CAN-FD currently
  return false;
}

bool McuCan::SetCanFDMode(bool enable, uint32_t data_baudrate, bool enable_brs) noexcept {
  // ESP32-C6 TWAI does not support CAN-FD
  (void)enable;
  (void)data_baudrate;
  (void)enable_brs;
  return false;
}

bool McuCan::ConfigureCanFDTiming(uint16_t nominal_prescaler, uint8_t nominal_tseg1,
                                  uint8_t nominal_tseg2, uint16_t data_prescaler,
                                  uint8_t data_tseg1, uint8_t data_tseg2, uint8_t sjw) noexcept {
  // ESP32-C6 TWAI does not support CAN-FD
  (void)nominal_prescaler;
  (void)nominal_tseg1;
  (void)nominal_tseg2;
  (void)data_prescaler;
  (void)data_tseg1;
  (void)data_tseg2;
  (void)sjw;
  return false;
}

bool McuCan::SetTransmitterDelayCompensation(uint8_t tdc_offset, uint8_t tdc_filter) noexcept {
  // ESP32-C6 TWAI does not support CAN-FD
  (void)tdc_offset;
  (void)tdc_filter;
  return false;
}

bool McuCan::GetCanFDCapabilities(uint8_t &max_data_bytes, uint32_t &max_nominal_baudrate,
                                  uint32_t &max_data_baudrate, bool &supports_brs,
                                  bool &supports_esi) noexcept {
  // ESP32-C6 TWAI does not support CAN-FD, but we can return classic CAN limits
  max_data_bytes = 8;             // Classic CAN limit
  max_nominal_baudrate = 1000000; // 1 Mbps max for ESP32-C6
  max_data_baudrate = 0;          // No data phase for classic CAN
  supports_brs = false;           // No BRS support
  supports_esi = false;           // No ESI support
  return true;                    // Return true to indicate we can provide classic CAN capabilities
}
