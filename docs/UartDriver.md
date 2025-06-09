# UartDriver Class Guide

Simple wrapper around the ESP‑IDF UART driver providing blocking read and write.

## Features
- Configure port, pins and parameters
- `Open()`, `Close()` lifecycle helpers
- `Write()` and `Read()` methods for byte streams

## Example
```cpp
uart_config_t cfg = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};
UartDriver serial(UART_NUM_0, cfg, GPIO_NUM_1, GPIO_NUM_3);
serial.Open();
const char msg[] = "hi";
serial.Write(reinterpret_cast<const uint8_t*>(msg), sizeof(msg));
```

---

[← Previous](PeriodicTimer.md) | [Documentation Index](index.md) | [Next →](DacOutput.md)
