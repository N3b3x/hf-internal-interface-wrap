/**
 * @file SfI2cBus.h
 * @brief Thread-safe I2C master driver for ESP32-C6 with software mutex control.
 *
 * This class mirrors the SfSpiBus implementation and relies on a FreeRTOS
 * mutex supplied from the HF-RTOSW-ESPIDF component.
 */
#ifndef SFI2CBUS_H_
#define SFI2CBUS_H_

#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <cstdint>

/**
 * @class SfI2cBus
 * @brief Thread-safe I2C master bus abstraction for ESP32-C6.
 */
class SfI2cBus {
public:
    /**
     * @brief Construct an SfI2cBus instance.
     * @param port I2C port number
     * @param cfg I2C configuration structure
     * @param mutexHandle Mutex used for locking during transfers
     */
    SfI2cBus(i2c_port_t port, const i2c_config_t &cfg, SemaphoreHandle_t mutexHandle) noexcept;

    /**
     * @brief Destructor closes the bus.
     */
    ~SfI2cBus() noexcept;

    /**
     * @brief Open and initialize the I2C port.
     */
    bool Open() noexcept;

    /**
     * @brief Close and de-initialize the I2C port.
     */
    bool Close() noexcept;

    /**
     * @brief Write to a device in a thread-safe manner.
     */
    bool Write(uint8_t addr, const uint8_t *data, uint16_t sizeBytes,
               uint32_t timeoutMsec) noexcept;

    /**
     * @brief Read from a device in a thread-safe manner.
     */
    bool Read(uint8_t addr, uint8_t *data, uint16_t sizeBytes,
              uint32_t timeoutMsec) noexcept;

    /**
     * @brief Combined write then read operation.
     */
    bool WriteRead(uint8_t addr, const uint8_t *txData, uint16_t txSizeBytes,
                   uint8_t *rxData, uint16_t rxSizeBytes,
                   uint32_t timeoutMsec) noexcept;

    /**
     * @brief Lock the bus for exclusive access.
     */
    bool LockBus(uint32_t timeoutMsec = portMAX_DELAY) noexcept;

    /**
     * @brief Unlock the bus.
     */
    bool UnlockBus() noexcept;

    /**
     * @brief Get the clock speed in Hz.
     */
    uint32_t GetClockHz() const noexcept;

    /**
     * @brief Check initialization state.
     */
    bool IsInitialized() const noexcept { return initialized; }

private:
    i2c_port_t i2cPort;      ///< Port number
    i2c_config_t config;     ///< Configuration copy
    SemaphoreHandle_t busMutex; ///< Mutex for thread safety
    bool initialized;        ///< Flag
};

#endif // SFI2CBUS_H_

