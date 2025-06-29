/**
 * @file McuCan.h
 * @brief MCU-integrated CAN controller implementation.
 *
 * This header provides a CAN bus implementation for microcontrollers with
 * built-in CAN peripherals. On ESP32, this wraps TWAI (Two-Wire Automotive Interface),
 * on STM32 it would wrap CAN peripheral, etc. The implementation supports standard
 * and extended CAN frames, filtering, error handling, and interrupt-driven operation.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is the primary CAN implementation for the ESP32C6 and similar MCUs
 *       that have integrated CAN controllers with external transceivers.
 */

#pragma once

#include "../utils/RtosMutex.h"
#include "BaseCan.h"
#include "McuTypes.h"

/**
 * @class McuCan
 * @brief CAN bus implementation for microcontrollers with integrated CAN peripherals.
 *
 * This class provides CAN communication using the microcontroller's built-in
 * CAN peripheral. On ESP32, it uses the TWAI (Two-Wire Automotive Interface)
 * controller. The implementation handles platform-specific details while
 * providing the unified BaseCan API.
 *
 * Features:
 * - High-performance CAN communication using MCU's integrated controller
 * - Interrupt-driven reception with callback support
 * - Configurable TX/RX queue sizes
 * - Comprehensive error handling and status reporting
 * - Support for both standard and extended CAN frames
 * - Acceptance filtering for selective message reception
 * - Lazy initialization support
 *
 * @note This implementation requires an external CAN transceiver chip for ESP32.
 */
class McuCan : public BaseCan {
public:
  /**
   * @brief Constructor with configuration.
   * @param config CAN bus configuration parameters
   */
  explicit McuCan(const CanBusConfig &config) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup.
   */
  ~McuCan() noexcept override;
  // Implement BaseCan interface
  bool Initialize() noexcept override;
  bool Deinitialize() noexcept override;
  bool SendMessage(const CanMessage &message, uint32_t timeout_ms = 1000) noexcept override;
  bool ReceiveMessage(CanMessage &message, uint32_t timeout_ms = 0) noexcept override;

  bool SetReceiveCallback(CanReceiveCallback callback) noexcept override;
  void ClearReceiveCallback() noexcept override;

  bool GetStatus(CanBusStatus &status) noexcept override;
  bool Reset() noexcept override;

  bool SetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended = false) noexcept override;
  bool ClearAcceptanceFilter() noexcept override;

  /**
   * @brief Start the CAN controller (after initialization).
   * @return true if started successfully, false otherwise
   */
  bool Start() noexcept;

  /**
   * @brief Stop the CAN controller (but keep initialized).
   * @return true if stopped successfully, false otherwise
   */
  bool Stop() noexcept;

  /**
   * @brief Get the current configuration.
   * @return Reference to the configuration structure
   */
  const CanBusConfig &GetConfig() const noexcept {
    return config_;
  }

  /**
   * @brief Check if the transmit queue is full.
   * @return true if queue is full, false otherwise
   */
  bool IsTransmitQueueFull() const noexcept;

  /**
   * @brief Check if the receive queue is empty.
   * @return true if queue is empty, false otherwise
   */
  bool IsReceiveQueueEmpty() const noexcept;

  /**
   * @brief Get the current transmit error count.
   * @return Number of transmit errors
   */
  uint32_t GetTransmitErrorCount() const noexcept;

  /**
   * @brief Get the current receive error count.
   * @return Number of receive errors
   */
  uint32_t GetReceiveErrorCount() const noexcept;

  // CAN-FD Support methods (override base class defaults)
  bool SupportsCanFD() const noexcept override;
  bool SetCanFDMode(bool enable, uint32_t data_baudrate = 2000000,
                    bool enable_brs = true) noexcept override;
  bool ConfigureCanFDTiming(uint16_t nominal_prescaler, uint8_t nominal_tseg1,
                            uint8_t nominal_tseg2, uint16_t data_prescaler, uint8_t data_tseg1,
                            uint8_t data_tseg2, uint8_t sjw = 1) noexcept override;
  bool SetTransmitterDelayCompensation(uint8_t tdc_offset, uint8_t tdc_filter) noexcept override;
  bool GetCanFDCapabilities(uint8_t &max_data_bytes, uint32_t &max_nominal_baudrate,
                            uint32_t &max_data_baudrate, bool &supports_brs,
                            bool &supports_esi) noexcept override;

private:
  CanBusConfig config_;                 ///< CAN bus configuration
  CanReceiveCallback receive_callback_; ///< User receive callback
  mutable RtosMutex mutex_;             ///< Thread safety mutex

  // Platform-specific implementation methods
  bool PlatformInitialize() noexcept;
  bool PlatformDeinitialize() noexcept;
  bool PlatformStart() noexcept;
  bool PlatformStop() noexcept;
  bool PlatformSendMessage(const CanMessage &message, uint32_t timeout_ms) noexcept;
  bool PlatformReceiveMessage(CanMessage &message, uint32_t timeout_ms) noexcept;
  bool PlatformGetStatus(CanBusStatus &status) noexcept;
  bool PlatformReset() noexcept;
  bool PlatformSetAcceptanceFilter(uint32_t id, uint32_t mask, bool extended) noexcept;
  bool PlatformClearAcceptanceFilter() noexcept;

  // Platform-specific helper methods
  bool GetTimingConfig(uint32_t baud_rate, void *timing_config) noexcept;
  bool PlatformIsTransmitQueueFull() const noexcept;
  bool PlatformIsReceiveQueueEmpty() const noexcept;
  uint32_t PlatformGetTransmitErrorCount() const noexcept;
  uint32_t PlatformGetReceiveErrorCount() const noexcept;

  // Static callback handler for platform interrupts
  static void StaticReceiveHandler(void *arg);
  void HandleReceiveInterrupt();
};
