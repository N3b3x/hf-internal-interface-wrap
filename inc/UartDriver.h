#ifndef UARTDRIVER_H
#define UARTDRIVER_H

#include <driver/uart.h>
#include <cstdint>

/**
 * @file UartDriver.h
 * @brief Simple UART driver wrapper for ESP-IDF.
 *
 * Provides minimal helpers for opening, closing and
 * blocking read/write over a UART port.
 */
class UartDriver {
public:
    /**
     * @brief Construct a UartDriver.
     * @param port UART port number
     * @param config UART configuration structure
     * @param txPin GPIO used for TX
     * @param rxPin GPIO used for RX
     */
    UartDriver(uart_port_t port, const uart_config_t& config,
               int txPin, int rxPin) noexcept;
    ~UartDriver() noexcept;
    UartDriver(const UartDriver&) = delete;
    UartDriver& operator=(const UartDriver&) = delete;

    bool Open() noexcept;   ///< Install and configure the driver
    bool Close() noexcept;  ///< Delete the driver
    bool Write(const uint8_t* data, uint16_t length) noexcept; ///< Blocking write
    bool Read(uint8_t* data, uint16_t length,
              TickType_t ticksToWait = portMAX_DELAY) noexcept; ///< Blocking read

    bool IsInitialized() const noexcept { return initialized; }

private:
    uart_port_t port;
    uart_config_t config;
    int txPin;
    int rxPin;
    bool initialized;
};

#endif // UARTDRIVER_H
