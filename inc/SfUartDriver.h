#ifndef SFUARTDRIVER_H
#define SFUARTDRIVER_H

#include <cstdint>
#include <driver/uart.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @file SfUartDriver.h
 * @brief Thread-safe UART driver wrapper for ESP-IDF.
 *
 * Mirrors the API of UartDriver but guards read and write
 * operations using a FreeRTOS mutex.
 */
class SfUartDriver {
public:
  /**
   * @brief Construct a thread-safe UART driver.
   * @param port UART port number
   * @param config UART configuration structure
   * @param txPin GPIO used for TX
   * @param rxPin GPIO used for RX
   * @param mutexHandle Mutex used for locking during transfers
   */
  SfUartDriver(uart_port_t port, const uart_config_t &config, int txPin, int rxPin,
               SemaphoreHandle_t mutexHandle) noexcept;
  ~SfUartDriver() noexcept;
  SfUartDriver(const SfUartDriver &) = delete;
  SfUartDriver &operator=(const SfUartDriver &) = delete;

  bool Open() noexcept;  ///< Install and configure the driver
  bool Close() noexcept; ///< Delete the driver
  bool Write(const uint8_t *data, uint16_t length,
             uint32_t timeoutMsec = 1000) noexcept; ///< Blocking write
  bool Read(uint8_t *data, uint16_t length,
            TickType_t ticksToWait = portMAX_DELAY) noexcept; ///< Blocking read

  bool Lock() noexcept;   ///< Manually lock the mutex
  bool Unlock() noexcept; ///< Unlock the mutex

  bool IsInitialized() const noexcept {
    return initialized;
  }

private:
  uart_port_t port;
  uart_config_t config;
  int txPin;
  int rxPin;
  SemaphoreHandle_t mutex;
  bool initialized;
};

#endif // SFUARTDRIVER_H
