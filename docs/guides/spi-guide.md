# ⚡ SPI Guide

The SPI bus wrapper supports full-duplex transfers with configurable modes.

## Configuration

```cpp
#include "McuSpi.h"

McuSpi spi(HF_SPI_HOST_1);
spi.Open(1000000, HF_SPI_MODE_0); // 1 MHz, mode 0
```

## Transfers

```cpp
hf_u8_t tx[2] = {0x9F, 0};
hf_u8_t rx[2];
spi.Transfer(tx, rx, sizeof(tx));
```

`Transfer()` simultaneously sends and receives data.

## DMA Transfers

Use `EnableDma()` to allow large transfers with minimal CPU overhead.

```cpp
spi.EnableDma();
```

## Tips

- Keep chip select handling consistent; consider wrapping in a driver class.
- When performance matters, use the [Performance Guide](performance-guide.md) for optimization advice.

## 🧰 Advanced Settings
- Adjust bit order with `SetBitOrder()` if needed
- Use `SetCsActiveLow()` for non-standard CS polarity

## 🛠️ Troubleshooting
- Verify the clock mode (CPOL/CPHA) matches the peripheral
- Ensure DMA buffers are 32-bit aligned when DMA is enabled

## 🔗 Related Examples
- [⚡ SPI Communication Example](../examples/basic-i2c.md)
