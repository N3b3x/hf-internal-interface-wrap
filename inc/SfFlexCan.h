/**
 * @file SfFlexCan.h
 * @brief Thread-safe wrapper around FlexCan using a FreeRTOS mutex.
 */
#ifndef SFFLEXCAN_H_
#define SFFLEXCAN_H_

#include "FlexCan.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @class SfFlexCan
 * @brief Adds mutex protection to FlexCan operations.
 */
class SfFlexCan {
public:
    /**
     * @brief Construct a thread-safe CAN driver.
     * @param port CAN controller port
     * @param baudRate Bus bitrate in bit/s
     * @param mutexHandle Mutex used for locking
     */
    SfFlexCan(uint8_t port, uint32_t baudRate, SemaphoreHandle_t mutexHandle) noexcept;
    ~SfFlexCan() noexcept;
    SfFlexCan(const SfFlexCan&) = delete;
    SfFlexCan& operator=(const SfFlexCan&) = delete;

    bool Open() noexcept;  ///< Initialize the underlying driver
    bool Close() noexcept; ///< Stop and uninstall

    bool Write(const FlexCan::Frame& frame, uint32_t timeoutMsec = 1000) noexcept;
    bool Read(FlexCan::Frame& frame, uint32_t timeoutMsec = 0) noexcept;

    bool Lock(uint32_t timeoutMsec = portMAX_DELAY) noexcept;
    bool Unlock() noexcept;

    bool IsInitialized() const noexcept { return initialized; }
    uint32_t GetBaudRate() const noexcept { return base.GetBaudRate(); }

private:
    FlexCan base;
    SemaphoreHandle_t mutex;
    bool initialized;
};

#endif // SFFLEXCAN_H_
