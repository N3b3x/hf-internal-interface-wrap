/**
 * @file McuSpi.h
 * @brief MCU-integrated SPI controller implementation.
 *
 * This header provides an SPI bus implementation for microcontrollers with
 * built-in SPI peripherals. On ESP32, this wraps the SPI driver,
 * on STM32 it would wrap the SPI peripheral, etc. The implementation supports
 * full-duplex communication, configurable clock speeds, multiple chip select
 * management, and DMA-accelerated transfers for high-performance applications.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 *
 * @note This is the primary SPI implementation for MCUs with integrated SPI controllers.
 */

#pragma once

#include "../utils/RtosMutex.h"
#include "BaseSpi.h"
#include "McuTypes.h"

/**
 * @class McuSpi
 * @brief SPI bus implementation for microcontrollers with integrated SPI peripherals.
 *
 * This class provides SPI communication using the microcontroller's built-in
 * SPI peripheral. On ESP32, it uses the SPI driver. The implementation handles
 * platform-specific details while providing the unified BaseSpi API.
 *
 * Features:
 * - High-performance SPI communication using MCU's integrated controller
 * - Support for all SPI modes (0-3)
 * - Configurable clock speeds and bit orders
 * - Full-duplex, write-only, and read-only transfers
 * - Automatic or manual chip select control
 * - DMA support for large transfers (platform dependent)
 * - Comprehensive error handling and status reporting
 * - Lazy initialization support
 * - Thread-safe operation with mutex protection
 *
 * @note This implementation is thread-safe when used with multiple threads.
 */
class McuSpi : public BaseSpi {
public:
  /**
   * @brief Constructor with configuration.
   * @param config SPI bus configuration parameters
   */
  explicit McuSpi(const SpiBusConfig &config) noexcept;

  /**
   * @brief Destructor - ensures proper cleanup.
   */
  ~McuSpi() noexcept override;

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
   * @return HfSpiErr result code
   */
  HfSpiErr Transfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                    uint32_t timeout_ms = 0) noexcept override;

  /**
   * @brief Assert/deassert the chip select signal.
   * @param active True to assert CS, false to deassert
   * @return HfSpiErr result code
   */
  HfSpiErr SetChipSelect(bool active) noexcept override;

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
  HfSpiErr GetLastError() const noexcept {
    return last_error_;
  }

  /**
   * @brief Set a new clock speed (requires reinitialization).
   * @param clock_speed_hz New clock speed in Hz
   * @return true if successful, false otherwise
   */
  bool SetClockSpeed(uint32_t clock_speed_hz) noexcept;

  /**
   * @brief Set a new SPI mode (requires reinitialization).
   * @param mode New SPI mode (0-3)
   * @return true if successful, false otherwise
   */
  bool SetMode(uint8_t mode) noexcept;

  /**
   * @brief Enable or disable DMA for transfers (if supported).
   * @param enable True to enable DMA, false to disable
   * @return true if successful, false otherwise
   */
  bool SetDmaEnabled(bool enable) noexcept;

  /**
   * @brief Get detailed bus status information.
   * @return Platform-specific status information
   */
  uint32_t GetBusStatus() noexcept;

  /**
   * @brief Perform multiple transfers with CS held active.
   * @param transfers Array of transfer descriptors
   * @param num_transfers Number of transfers
   * @return HfSpiErr result code
   */
  struct SpiTransfer {
    const uint8_t *tx_data; ///< Transmit data (can be nullptr)
    uint8_t *rx_data;       ///< Receive data (can be nullptr)
    uint16_t length;        ///< Number of bytes
    uint32_t timeout_ms;    ///< Transfer timeout (0 = use default)
  };

  HfSpiErr TransferSequence(const SpiTransfer *transfers, uint8_t num_transfers) noexcept;

  /**
   * @brief Perform a transfer with custom timing.
   * @param tx_data Transmit data buffer
   * @param rx_data Receive data buffer
   * @param length Number of bytes to transfer
   * @param cs_hold_time_us Time to hold CS active after transfer (microseconds)
   * @param timeout_ms Timeout in milliseconds
   * @return HfSpiErr result code
   */
  HfSpiErr TransferWithTiming(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                              uint32_t cs_hold_time_us, uint32_t timeout_ms = 0) noexcept;

  /**
   * @brief Write data to a specific register address.
   * @param reg_addr Register address
   * @param data Data to write
   * @param length Number of bytes to write
   * @return HfSpiErr result code
   */
  HfSpiErr WriteRegister(uint8_t reg_addr, const uint8_t *data, uint16_t length) noexcept;

  /**
   * @brief Read data from a specific register address.
   * @param reg_addr Register address
   * @param data Buffer to store read data
   * @param length Number of bytes to read
   * @return HfSpiErr result code
   */
  HfSpiErr ReadRegister(uint8_t reg_addr, uint8_t *data, uint16_t length) noexcept;

private:
  //==============================================//
  // PRIVATE METHODS                              //
  //==============================================//

  /**
   * @brief Convert platform-specific error to HfSpiErr.
   * @param platform_error Platform-specific error code
   * @return Corresponding HfSpiErr
   */
  HfSpiErr ConvertPlatformError(int32_t platform_error) noexcept;

  /**
   * @brief Validate SPI mode.
   * @param mode SPI mode to validate
   * @return true if valid, false otherwise
   */
  bool IsValidMode(uint8_t mode) const noexcept {
    return mode <= 3;
  }

  /**
   * @brief Validate clock speed.
   * @param clock_speed_hz Clock speed to validate
   * @return true if valid, false otherwise
   */
  bool IsValidClockSpeed(uint32_t clock_speed_hz) const noexcept {
    // Typical range: 1kHz to 80MHz (platform dependent)
    return (clock_speed_hz >= 1000 && clock_speed_hz <= 80000000);
  }

  /**
   * @brief Get timeout value (use default if timeout_ms is 0).
   * @param timeout_ms Requested timeout
   * @return Actual timeout to use
   */
  uint32_t GetTimeoutMs(uint32_t timeout_ms) const noexcept {
    return (timeout_ms == 0) ? config_.timeout_ms : timeout_ms;
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
   * @brief Internal transfer implementation.
   * @param tx_data Transmit data buffer
   * @param rx_data Receive data buffer
   * @param length Number of bytes to transfer
   * @param timeout_ms Timeout in milliseconds
   * @param manage_cs Whether to manage CS automatically
   * @return HfSpiErr result code
   */
  HfSpiErr InternalTransfer(const uint8_t *tx_data, uint8_t *rx_data, uint16_t length,
                            uint32_t timeout_ms, bool manage_cs) noexcept;

  //==============================================//
  // PRIVATE MEMBERS                              //
  //==============================================//

  mutable RtosMutex mutex_;         ///< Thread safety mutex
  hf_spi_handle_t platform_handle_; ///< Handle for the registered SPI device
  HfSpiErr last_error_;             ///< Last error that occurred
  uint32_t transaction_count_;      ///< Number of transactions performed
  bool cs_active_;                  ///< Current CS state
  bool dma_enabled_;                ///< DMA enable state
  uint16_t max_transfer_size_;      ///< Maximum transfer size in bytes
};
