/**
 * @file SfUartDriver.h
 * @brief Thread-safe UART driver wrapper with platform-agnostic interface.
 *
 * This class provides thread-safe UART operations using a platform-agnostic
 * interface. It wraps the UartDriver class and adds mutex protection for
 * multi-threaded environments. The implementation supports configurable
 * serial communication, data transmission, flow control, and comprehensive
 * error handling with thread safety guarantees.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "BaseUart.h"
#include "RtosMutex.h"
#include <cstdint>
#include <memory>

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
  explicit SfUartDriver(std::unique_ptr<BaseUart> uart_impl) noexcept;

  ~SfUartDriver() noexcept;

  SfUartDriver(const SfUartDriver &) = delete;
  SfUartDriver &operator=(const SfUartDriver &) = delete;

  bool Open() noexcept;
  bool Close() noexcept;

  bool Write(const uint8_t *data, uint16_t length,
             uint32_t timeoutMsec = HF_TIMEOUT_DEFAULT) noexcept;
  bool Read(uint8_t *data, uint16_t length, TickType_t ticksToWait) noexcept;

  bool Lock() noexcept;
  bool Unlock() noexcept;

  bool IsInitialized() const noexcept {
    return initialized_;
  }

private:
  std::unique_ptr<BaseUart> uart_driver_;
  RtosMutex mutex_;
  bool initialized_;
};
