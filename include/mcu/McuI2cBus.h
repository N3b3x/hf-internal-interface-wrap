/**
 * @file McuI2cBus.h
 * @brief MCU-integrated I2C controller implementation.
 *
 * This header provides an I2C bus implementation for microcontrollers with
 * built-in I2C peripherals. On ESP32, this wraps the I2C driver,
 * on STM32 it would wrap the I2C peripheral, etc.
 *
 * This is the primary I2C implementation for MCUs with integrated I2C controllers.
 */

#ifndef MCU_I2C_BUS_H
#define MCU_I2C_BUS_H

#include "BaseI2cBus.h"
#include "McuTypes.h"
#include <mutex>

/**
 * @class McuI2cBus
 * @brief I2C bus implementation for microcontrollers with integrated I2C peripherals.
 *
 * This class provides I2C communication using the microcontroller's built-in
 * I2C peripheral. On ESP32, it uses the I2C driver. The implementation handles
 * platform-specific details while providing the unified BaseI2cBus API.
 *
 * Features:
 * - High-performance I2C communication using MCU's integrated controller
 * - Support for standard (100kHz) and fast (400kHz) modes
 * - Configurable timeout and error handling
 * - Master mode operation
 * - Device scanning and presence detection
 * - Register-based communication utilities
 * - Internal pull-up resistor configuration
 * - Lazy initialization support
 * - Thread-safe operation with mutex protection
 *
 * @note This implementation is thread-safe when used with multiple threads.
 */
class McuI2cBus : public BaseI2cBus {
public:
    /**
     * @brief Constructor with configuration.
     * @param config I2C bus configuration parameters
     */
    explicit McuI2cBus(const I2cBusConfig& config) noexcept;

    /**
     * @brief Destructor - ensures proper cleanup.
     */
    ~McuI2cBus() noexcept override;

    //==============================================//
    // OVERRIDDEN PURE VIRTUAL FUNCTIONS            //
    //==============================================//

    /**
     * @brief Initialize the I2C bus.
     * @return true if successful, false otherwise
     */
    bool Initialize() noexcept override;

    /**
     * @brief Deinitialize the I2C bus.
     * @return true if successful, false otherwise
     */
    bool Deinitialize() noexcept override;

    /**
     * @brief Write data to a slave device.
     * @param device_addr 7-bit device address
     * @param data Data buffer to transmit
     * @param length Number of bytes to write
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfI2cErr result code
     */
    HfI2cErr Write(uint8_t device_addr, const uint8_t* data,
                   uint16_t length, uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Read data from a slave device.
     * @param device_addr 7-bit device address
     * @param data Buffer to store received data
     * @param length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfI2cErr result code
     */
    HfI2cErr Read(uint8_t device_addr, uint8_t* data,
                  uint16_t length, uint32_t timeout_ms = 0) noexcept override;

    /**
     * @brief Write then read from a slave device without releasing the bus.
     * @param device_addr 7-bit device address
     * @param tx_data Data buffer to send
     * @param tx_length Number of bytes to send
     * @param rx_data Buffer to store received data
     * @param rx_length Number of bytes to read
     * @param timeout_ms Timeout in milliseconds (0 = use default)
     * @return HfI2cErr result code
     */
    HfI2cErr WriteRead(uint8_t device_addr, const uint8_t* tx_data, uint16_t tx_length,
                       uint8_t* rx_data, uint16_t rx_length, uint32_t timeout_ms = 0) noexcept override;

    //==============================================//
    // ENHANCED METHODS                             //
    //==============================================//

    /**
     * @brief Check if the I2C bus is busy.
     * @return true if busy, false if available
     */
    bool IsBusy() noexcept;

    /**
     * @brief Reset the I2C bus in case of errors.
     * @return true if reset successful, false otherwise
     */
    bool ResetBus() noexcept;

    /**
     * @brief Get the last error that occurred.
     * @return Last error code
     */
    HfI2cErr GetLastError() const noexcept {
        return last_error_;
    }

    /**
     * @brief Set a new clock speed (requires reinitialization).
     * @param clock_speed_hz New clock speed in Hz
     * @return true if successful, false otherwise
     */
    bool SetClockSpeed(uint32_t clock_speed_hz) noexcept;

    /**
     * @brief Enable or disable internal pull-up resistors.
     * @param enable True to enable pull-ups, false to disable
     * @return true if successful, false otherwise
     */
    bool SetPullUps(bool enable) noexcept;

    /**
     * @brief Get detailed bus status information.
     * @return Platform-specific status information
     */
    uint32_t GetBusStatus() noexcept;

    /**
     * @brief Perform a bus recovery sequence.
     * @return true if recovery successful, false otherwise
     */
    bool RecoverBus() noexcept;

private:
    //==============================================//
    // PRIVATE METHODS                              //
    //==============================================//

    /**
     * @brief Convert platform-specific error to HfI2cErr.
     * @param platform_error Platform-specific error code
     * @return Corresponding HfI2cErr
     */
    HfI2cErr ConvertPlatformError(int32_t platform_error) noexcept;

    /**
     * @brief Validate device address.
     * @param device_addr Device address to validate
     * @return true if valid, false otherwise
     */
    bool IsValidDeviceAddress(uint8_t device_addr) const noexcept {
        // Valid 7-bit I2C addresses are 0x08-0x77 (avoiding reserved addresses)
        return (device_addr >= 0x08 && device_addr <= 0x77);
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

    //==============================================//
    // PRIVATE MEMBERS                              //
    //==============================================//

    mutable std::mutex mutex_;           ///< Thread safety mutex
    hf_i2c_handle_t platform_handle_;    ///< Platform-specific I2C handle
    HfI2cErr last_error_;                ///< Last error that occurred
    uint32_t transaction_count_;         ///< Number of transactions performed
    bool bus_locked_;                    ///< Bus lock state for extended operations
};

#endif // MCU_I2C_BUS_H
