/**
 * @file EspSpi.h
 * @brief Advanced MCU-integrated SPI controller implementation with ESP32C6/ESP-IDF v5.5+ features.
 *
 * This header provides a comprehensive SPI implementation that utilizes all the advanced
 * features available in ESP-IDF v5.5+ for ESP32C6, including DMA acceleration, octal/quad modes,
 * advanced timing control, multi-device management, power optimization, and comprehensive
 * error handling. The implementation supports both master and slave modes with extensive
 * configuration options for high-performance and low-power applications.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is the unified SPI implementation for MCUs with integrated SPI controllers,
 *       featuring both basic and advanced ESP32C6-specific capabilities.
 */

#pragma once

#include "RtosMutex.h"
#include "BaseSpi.h"
#include "McuTypes.h"
#include <functional>
#include <memory>
#include <vector>

#ifdef ESP_PLATFORM
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "driver/spi_common.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "hal/spi_types.h"
#endif

// Type aliases to centralized types in McuTypes.h (following naming convention)
using hf_spi_bus_config_alias_t = hf_spi_bus_config_t;
using hf_spi_device_config_alias_t = hf_spi_device_interface_config_t;
using hf_spi_transaction_alias_t = hf_spi_transaction_t;
using hf_spi_device_handle_alias_t = hf_spi_device_handle_t;
using hf_spi_host_device_alias_t = hf_spi_host_device_t;

//--------------------------------------
//  Advanced SPI Configuration
//--------------------------------------

// Type aliases for centralized types from McuTypes.h (following naming convention)
using hf_spi_transfer_mode_alias_t = hf_spi_transfer_mode_t;
using hf_spi_clock_source_alias_t = hf_spi_clock_source_t;

/**
 * @brief Advanced SPI configuration for ESP32C6/ESP-IDF v5.5+.
 */
struct hf_spi_advanced_config_t {
  // Basic configuration
  hf_spi_bus_config_alias_t base_config;         ///< Base SPI bus configuration
  hf_spi_host_device_alias_t host_device;       ///< SPI host device (SPI2 for ESP32C6)
  hf_spi_device_config_alias_t device_config;   ///< Device-specific configuration
  
  // Advanced ESP32C6 features
  hf_spi_transfer_mode_alias_t transfer_mode;  ///< Transfer mode (single/dual/quad/octal)
  hf_spi_clock_source_alias_t clock_source;    ///< Clock source selection
  bool dma_enabled;                 ///< Enable DMA acceleration
  uint32_t dma_channel;             ///< DMA channel selection (auto if 0)
  uint32_t max_transfer_size;       ///< Maximum transfer size in bytes
  
  // Performance and timing
  bool use_iomux;                   ///< Use IOMUX for better performance
  uint8_t input_delay_ns;           ///< Input delay compensation
  uint8_t cs_setup_time;            ///< CS setup time (clock cycles)
  uint8_t cs_hold_time;             ///< CS hold time (clock cycles)
  
  // Power management
  bool auto_suspend_enabled;        ///< Auto-suspend when idle
  uint32_t suspend_delay_ms;        ///< Delay before auto-suspend
  bool clock_gating_enabled;        ///< Enable clock gating for power saving
  
  // Queue and buffering
  uint8_t transaction_queue_size;   ///< Transaction queue depth
  bool polling_mode;                ///< Use polling instead of interrupts
  uint32_t timeout_ms;              ///< Default operation timeout
  
  // Diagnostics and monitoring
  bool statistics_enabled;          ///< Enable operation statistics
  bool error_recovery_enabled;      ///< Enable automatic error recovery
  
  // Default constructor
  hf_spi_advanced_config_t()
      : host_device(hf_spi_host_device_alias_t::HF_SPI2_HOST), transfer_mode(hf_spi_transfer_mode_alias_t::HF_SPI_TRANSFER_MODE_SINGLE),
        clock_source(hf_spi_clock_source_alias_t::HF_SPI_CLK_SRC_DEFAULT), dma_enabled(true), dma_channel(0),
        max_transfer_size(4092), use_iomux(true), input_delay_ns(0), cs_setup_time(0), cs_hold_time(0),
        auto_suspend_enabled(false), suspend_delay_ms(5000), clock_gating_enabled(false),
        transaction_queue_size(7), polling_mode(false), timeout_ms(1000),
        statistics_enabled(false), error_recovery_enabled(true) {}
};

/**
 * @brief SPI operation statistics for performance monitoring.
 */
struct hf_spi_statistics_t {
  uint64_t total_transactions;      ///< Total transactions performed
  uint64_t successful_transactions; ///< Successful transactions
  uint64_t failed_transactions;     ///< Failed transactions
  uint64_t timeout_transactions;    ///< Timed-out transactions
  uint64_t bytes_transmitted;       ///< Total bytes transmitted
  uint64_t bytes_received;          ///< Total bytes received
  uint64_t average_transfer_time_us;///< Average transfer time (microseconds)
  uint64_t max_transfer_time_us;    ///< Maximum transfer time
  uint64_t min_transfer_time_us;    ///< Minimum transfer time
  uint32_t dma_transfers;           ///< DMA-accelerated transfers
  uint32_t polling_transfers;       ///< Polling-mode transfers
  
  hf_spi_statistics_t()
      : total_transactions(0), successful_transactions(0), failed_transactions(0),
        timeout_transactions(0), bytes_transmitted(0), bytes_received(0),
        average_transfer_time_us(0), max_transfer_time_us(0), min_transfer_time_us(UINT64_MAX),
        dma_transfers(0), polling_transfers(0) {}
};

/**
 * @brief SPI transfer descriptor for batch operations.
 */
struct hf_spi_transfer_descriptor_t {
  const uint8_t *tx_data;      ///< Transmit data (nullptr for read-only)
  uint8_t *rx_data;            ///< Receive data (nullptr for write-only)
  uint16_t length;             ///< Transfer length in bytes
  uint32_t timeout_ms;         ///< Transfer timeout (0 = use default)
  bool manage_cs;              ///< Manage CS for this transfer
  uint32_t flags;              ///< Transfer-specific flags
  
  hf_spi_transfer_descriptor_t(const uint8_t *tx = nullptr, uint8_t *rx = nullptr,
                       uint16_t len = 0, uint32_t timeout = 0, bool cs = true)
      : tx_data(tx), rx_data(rx), length(len), timeout_ms(timeout),
        manage_cs(cs), flags(0) {}
};

// Callback function types (following naming convention)
using hf_spi_async_callback_t = std::function<void(hf_spi_err_t result, size_t bytesTransferred, void *userData)>;
using hf_spi_event_callback_t = std::function<void(int eventType, void *eventData, void *userData)>;

// Type alias for centralized event type (following naming convention)
using hf_spi_event_type_alias_t = hf_spi_event_type_t;

/**
 * @class EspSpi
 * @brief Advanced SPI bus implementation for microcontrollers with integrated SPI peripherals.
 *
 * This class provides comprehensive SPI communication using the microcontroller's built-in
 * SPI peripheral with support for both basic and advanced features. On ESP32C6, it utilizes
 * the latest ESP-IDF v5.5+ SPI master driver features including DMA acceleration, octal/quad
 * modes, advanced timing control, and power management.
 *
 * Features:
 * - High-performance SPI communication using MCU's integrated controller
 * - Support for all SPI modes (0-3) with configurable timing
 * - Advanced ESP32C6/ESP-IDF v5.5+ features:
 *   - DMA acceleration for high-throughput transfers
 *   - Octal/Quad SPI modes for increased bandwidth
 *   - Advanced timing control and signal conditioning
 *   - Multiple clock sources for power optimization
 *   - Automatic power management and clock gating
 *   - Comprehensive error handling and recovery
 *   - Performance monitoring and statistics
 *   - Asynchronous operation support
 * - Multiple device management with individual configurations
 * - Batch transfer operations for complex protocols
 * - Register-based communication utilities
 * - Thread-safe operation with mutex protection
 * - Lazy initialization support
 *
 * @note This implementation is thread-safe when used with multiple threads.
 * @note Advanced features require ESP-IDF v5.5+ for full functionality.
 */
class EspSpi : public BaseSpi {
public:
  /**
   * @brief Constructor with basic configuration.
   * @param config SPI bus configuration parameters
   */
  explicit EspSpi(const hf_spi_bus_config_alias_t &config) noexcept;

  /**
   * @brief Constructor with advanced configuration.
   * @param config Advanced SPI configuration parameters
   */
  explicit EspSpi(const hf_spi_advanced_config_t &config) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup.
   */
  ~EspSpi() noexcept override;
  //==============================================//
  // OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
  //==============================================//

  /**
   * @brief Initialize the SPI bus.
   * @return true if successful, false otherwise
   */
  bool Initialize() noexcept override;

  /**
   * @brief Deinitialize the SPI bus.
   * @return true if successful, false otherwise
   */
  bool Deinitialize() noexcept override;

  /**
   * @brief Perform a full-duplex SPI transfer.
   * @param tx_data Transmit data buffer (can be nullptr for read-only)
   * @param rx_data Receive data buffer (can be nullptr for write-only)
   * @param length Number of bytes to transfer
   * @param timeout_ms Timeout in milliseconds (0 = use default)
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t Transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                    uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Assert/deassert the chip select signal.
   * @param active True to assert CS, false to deassert
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t SetChipSelect(bool active) noexcept override;

  //==============================================//
  // ADVANCED SPI OPERATIONS                     //
  //==============================================//

  /**
   * @brief Initialize with advanced configuration.
   * @param config Advanced configuration parameters
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t initializeAdvanced(const hf_spi_advanced_config_t &config) noexcept;

  /**
   * @brief Reconfigure the SPI bus with new settings.
   * @param config New configuration parameters
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t reconfigure(const hf_spi_advanced_config_t &config) noexcept;

  /**
   * @brief Get current SPI configuration.
   * @return Current SPI bus configuration
   */
  const hf_spi_bus_config_alias_t& GetConfig() const noexcept {
    return use_advanced_config_ ? advanced_config_.base_config : config_;
  }

  /**
   * @brief Get current advanced configuration.
   * @return Current advanced SPI configuration
   */
  hf_spi_advanced_config_t getCurrentConfiguration() const noexcept;

  /**
   * @brief Get current transfer mode.
   * @return Current transfer mode
   */
  hf_spi_transfer_mode_alias_t GetTransferMode() const noexcept;

  /**
   * @brief Reset the SPI bus and recover from errors.
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t resetBus() noexcept;

  //==============================================//
  // MULTI-DEVICE MANAGEMENT                     //
  //==============================================//

  /**
   * @brief Add a device to the SPI bus.
   * @param device_config Device configuration
   * @return Device handle or nullptr on failure
   */
  hf_spi_device_handle_alias_t addDevice(const hf_spi_device_config_alias_t &device_config) noexcept;

  /**
   * @brief Remove a device from the SPI bus.
   * @param device_handle Device handle to remove
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t removeDevice(hf_spi_device_handle_alias_t device_handle) noexcept;

  /**
   * @brief Switch to a specific device.
   * @param device_handle Device handle to switch to
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t selectDevice(hf_spi_device_handle_alias_t device_handle) noexcept;

  //==============================================//
  // ADVANCED TRANSFER OPERATIONS                //
  //==============================================//

  /**
   * @brief Perform transfer using quad SPI mode.
   * @param tx_data Transmit data buffer
   * @param rx_data Receive data buffer
   * @param length Number of bytes to transfer
   * @param timeout_ms Timeout in milliseconds
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t transferQuad(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                        uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Perform transfer using octal SPI mode (ESP32C6 specific).
   * @param tx_data Transmit data buffer
   * @param rx_data Receive data buffer
   * @param length Number of bytes to transfer
   * @param timeout_ms Timeout in milliseconds
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t transferOctal(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                         uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Perform DMA-accelerated transfer.
   * @param tx_data Transmit data buffer
   * @param rx_data Receive data buffer
   * @param length Number of bytes to transfer
   * @param timeout_ms Timeout in milliseconds
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t transferDma(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                       uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Perform batch transfers with single CS assertion.
   * @param transfers Array of transfer descriptors
   * @param count Number of transfers
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t transferBatch(const hf_spi_transfer_descriptor_t *transfers, uint8_t count) noexcept;

  //==============================================//
  // ASYNCHRONOUS OPERATIONS                     //
  //==============================================//

  /**
   * @brief Perform asynchronous transfer.
   * @param tx_data Transmit data buffer
   * @param rx_data Receive data buffer
   * @param length Number of bytes to transfer
   * @param callback Completion callback
   * @param userData User data for callback
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t transferAsync(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                         hf_spi_async_callback_t callback, void *userData = nullptr) noexcept;

  /**
   * @brief Cancel pending asynchronous operation.
   * @param operation_id Operation ID to cancel
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t cancelAsyncOperation(uint32_t operation_id) noexcept;

  /**
   * @brief Set event callback for SPI events.
   * @param callback Event callback function
   * @param userData User data for callback
   */
  void setEventCallback(hf_spi_event_callback_t callback, void *userData = nullptr) noexcept;

  //==============================================//
  // REGISTER-BASED OPERATIONS                   //
  //==============================================//

  /**
   * @brief Write to a device register.
   * @param reg_addr Register address
   * @param value Value to write
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t writeRegister(uint8_t reg_addr, uint8_t value) noexcept;

  /**
   * @brief Read from a device register.
   * @param reg_addr Register address
   * @param value Reference to store read value
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t readRegister(uint8_t reg_addr, uint8_t &value) noexcept;

  /**
   * @brief Write multiple registers sequentially.
   * @param start_reg_addr Starting register address
   * @param data Data to write
   * @param count Number of registers to write
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t writeMultipleRegisters(uint8_t start_reg_addr, const uint8_t *data, uint8_t count) noexcept;

  /**
   * @brief Read multiple registers sequentially.
   * @param start_reg_addr Starting register address
   * @param data Buffer to store read data
   * @param count Number of registers to read
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t readMultipleRegisters(uint8_t start_reg_addr, uint8_t *data, uint8_t count) noexcept;

  //==============================================//
  // POWER MANAGEMENT                            //
  //==============================================//

  /**
   * @brief Enable or disable DMA acceleration.
   * @param enable True to enable DMA, false to disable
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t setDmaEnabled(bool enable) noexcept;

  /**
   * @brief Suspend SPI bus for power saving.
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t suspendBus() noexcept;

  /**
   * @brief Resume SPI bus from suspended state.
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t resumeBus() noexcept;

  /**
   * @brief Set clock source for power optimization.
   * @param clock_source Clock source to use
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t setClockSource(hf_spi_clock_source_alias_t clock_source) noexcept;

  //==============================================//  //==============================================//
  // STATISTICS AND DIAGNOSTICS                  //
  //==============================================//

  /**
   * @brief Get operation statistics.
   * @return Current statistics reference
   */
  const hf_spi_statistics_t& getStatistics() const noexcept;

  /**
   * @brief Reset operation statistics.
   */
  void resetStatistics() noexcept;

  /**
   * @brief Get comprehensive diagnostics information.
   * @return Diagnostics structure with current state
   */
  hf_spi_diagnostics_alias_t getDiagnostics() const noexcept;

  /**
   * @brief Check if SPI bus is healthy.
   * @return true if healthy, false otherwise
   */
  bool isBusHealthy() noexcept;
  //==============================================//
  // ENHANCED METHODS                             //
  //==============================================//

  /**
   * @brief Check if the SPI bus is busy.
   * @return true if busy, false if available
   */
  bool IsBusy() noexcept;

  /**
   * @brief Get the last error that occurred.
   * @return Last error code
   */
  hf_spi_err_t GetLastError() const noexcept {
    return last_error_;
  }

  /**
   * @brief Set a new clock speed (requires device reconfiguration).
   * @param clock_speed_hz New clock speed in Hz
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t SetClockSpeed(uint32_t clock_speed_hz) noexcept;

  /**
   * @brief Set a new SPI mode (requires device reconfiguration).
   * @param mode New SPI mode (0-3)
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t SetMode(uint8_t mode) noexcept;

  /**
   * @brief Get detailed bus status information.
   * @return Platform-specific status information
   */
  uint32_t GetBusStatus() noexcept;

  /**
   * @brief Get maximum supported transfer size.
   * @return Maximum transfer size in bytes
   */
  uint16_t GetMaxTransferSize() const noexcept {
    return max_transfer_size_;
  }

  /**
   * @brief Check if DMA is currently enabled.
   * @return true if DMA enabled, false otherwise
   */
  bool IsDmaEnabled() const noexcept {
    return dma_enabled_;
  }

  /**
   * @brief Get current transfer mode.
   * @return Current transfer mode
   */
  hf_spi_transfer_mode_alias_t GetTransferMode() const noexcept;

private:
  //==============================================//
  // PRIVATE METHODS                              //
  //==============================================//

  /**
   * @brief Convert platform-specific error to hf_spi_err_t.
   * @param platform_error Platform-specific error code
   * @return Corresponding hf_spi_err_t
   */
  hf_spi_err_t ConvertPlatformError(int32_t platform_error) noexcept;

  /**
   * @brief Validate SPI mode.
   * @param mode SPI mode to validate
   * @return true if valid, false otherwise
   */
  bool IsValidMode(uint8_t mode) const noexcept {
  return HF_SPI_IS_VALID_MODE(mode);
  }

  /**
   * @brief Validate clock speed.
   * @param clock_speed_hz Clock speed to validate
   * @return true if valid, false otherwise
   */
  bool IsValidClockSpeed(uint32_t clock_speed_hz) const noexcept {
    return HF_SPI_IS_VALID_CLOCK_SPEED(clock_speed_hz);
  }

  /**
   * @brief Validate transfer size.
   * @param size Transfer size to validate
   * @return true if valid, false otherwise
   */
  bool IsValidTransferSize(uint16_t size) const noexcept {
    return HF_SPI_IS_VALID_TRANSFER_SIZE(size);
  }

  /**
   * @brief Get timeout value (use default if timeout_ms is 0).
   * @param timeout_ms Requested timeout
   * @return Actual timeout to use
   */
  uint32_t GetTimeoutMs(uint32_t timeout_ms) const noexcept {
    if (timeout_ms == 0) {
      return use_advanced_config_ ? advanced_config_.timeout_ms : DEFAULT_TIMEOUT_MS;
    }
    return timeout_ms;
  }

  /**
   * @brief Perform platform-specific initialization.
   * @return true if successful, false otherwise
   */
  bool PlatformInitialize() noexcept;

  /**
   * @brief Perform platform-specific deinitialization.
   * @return true if successful, false otherwise
   */
  bool PlatformDeinitialize() noexcept;

  /**
   * @brief Internal transfer implementation with advanced features.
   * @param tx_data Transmit data buffer
   * @param rx_data Receive data buffer
   * @param length Number of bytes to transfer
   * @param timeout_ms Timeout in milliseconds
   * @param transfer_mode Transfer mode to use
   * @param manage_cs Whether to manage CS automatically
   * @return hf_spi_err_t result code
   */
  hf_spi_err_t InternalTransfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                            uint32_t timeout_ms, hf_spi_transfer_mode_alias_t transfer_mode, 
                            bool manage_cs) noexcept;
                            
  /**
   * @brief Update operation statistics.
   * @param success Operation success status
   * @param bytesTransferred Number of bytes transferred
   * @param transferTimeUs Transfer time in microseconds
   * @param usedDma Whether DMA was used
   */
  void UpdateStatistics(bool success, size_t bytesTransferred, 
                       uint64_t transferTimeUs, bool usedDma) noexcept;

  /**
   * @brief Handle platform-specific error.
   * @param error Platform error code
   */
  void HandlePlatformError(int32_t error) noexcept;

  //==============================================//
  // PRIVATE MEMBERS                              //
  //==============================================//

  // Platform-specific handles
  hf_spi_device_handle_alias_t platform_handle_;                         ///< Primary device handle
  hf_spi_device_handle_alias_t current_device_;                          ///< Currently selected device
  std::vector<hf_spi_device_handle_alias_t> device_handles_;             ///< All registered devices

  // Configuration storage
  hf_spi_advanced_config_t advanced_config_;                       ///< Advanced configuration
  bool use_advanced_config_;                                ///< Flag indicating advanced config usage

  // State management
  mutable RtosMutex mutex_;            ///< Thread safety mutex
  hf_spi_err_t last_error_;                ///< Last error that occurred
  uint32_t transaction_count_;         ///< Number of transactions performed
  bool cs_active_;                     ///< Current CS state
  bool dma_enabled_;                   ///< DMA enable state
  bool bus_suspended_;                 ///< Bus suspension state
  hf_spi_transfer_mode_alias_t current_transfer_mode_; ///< Current transfer mode
  uint16_t max_transfer_size_;         ///< Maximum transfer size in bytes

  // Asynchronous operation support
  std::vector<uint32_t> async_operations_;  ///< Active async operations
  uint32_t next_operation_id_;              ///< Next operation ID
  hf_spi_event_callback_t event_callback_;         ///< Event callback function
  void *event_user_data_;                   ///< Event callback user data

  // Statistics and diagnostics
  mutable hf_spi_statistics_t statistics_;   ///< Operation statistics
  uint64_t last_transfer_time_;        ///< Last transfer timestamp

  // Platform-specific constants
  static constexpr uint32_t DEFAULT_TIMEOUT_MS = 1000;
  static constexpr uint8_t DEFAULT_QUEUE_SIZE = 7;
};
