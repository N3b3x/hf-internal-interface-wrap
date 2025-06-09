# SfUartDriver Class Guide

Thread‑safe UART driver using a FreeRTOS mutex. Wraps the ESP‑IDF UART driver and mirrors the `UartDriver` API.

## Highlights
- Mutex protected read and write operations
- Identical usage to `UartDriver` with extra `Lock()` and `Unlock()` helpers

## Example
```cpp
SemaphoreHandle_t m = xSemaphoreCreateMutex();
SfUartDriver serial(UART_NUM_0, cfg, GPIO_NUM_1, GPIO_NUM_3, m);
serial.Open();
serial.Write(reinterpret_cast<const uint8_t*>("hi"), 2);
```

---

[← Previous](UartDriver.md) | [Documentation Index](index.md) | [Next →](DacOutput.md)
