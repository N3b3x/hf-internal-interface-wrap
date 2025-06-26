/**
 * @file SfUartDriver.h
 * @brief Thread-safe UART driver wrapper with platform-agnostic interface.
 *
 * This class provides thread-safe UART operations using a platform-agnostic
 * interface. It wraps the UartDriver class and adds mutex protection for
 * multi-threaded environments.
 */
#ifndef SFUARTDRIVER_H
#define SFUARTDRIVER_H

#include "../mcu/McuUartDriver.h"
#include "../mcu/McuTypes.h"
#include <cstdint>
#include <memory>
#include <mutex>

/**
 * @class SfUartDriver
 * @brief Thread-safe UART driver wrapper.
 */
class SfUartDriver {
public:
  /**
   * @brief Constructor for thread-safe UART driver.
   * @param port Platform-agnostic UART port number
   * @param config UART configuration
   */
  SfUartDriver(HfPortNumber port, const UartConfig &config) noexcept;

  ~SfUartDriver() noexcept;

  SfUartDriver(const SfUartDriver &) = delete;
  SfUartDriver &operator=(const SfUartDriver &) = delete;

  bool Open() noexcept;
  bool Close() noexcept;

  bool Write(const uint8_t *data, uint16_t length,
             uint32_t timeoutMsec = 1000) noexcept;
  bool Read(uint8_t *data, uint16_t length,
            uint32_t timeoutMsec = UINT32_MAX) noexcept;

  bool Lock(uint32_t timeoutMsec = UINT32_MAX) noexcept;
  bool Unlock() noexcept;

  bool IsInitialized() const noexcept;
  const UartConfig &GetConfig() const noexcept;

private:
  std::unique_ptr<UartDriver> uart_driver_;
  mutable std::mutex mutex_;
  bool initialized_;
};

#endif // SFUARTDRIVER_H
