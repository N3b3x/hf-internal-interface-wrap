# 📻 PIO Guide

Programmable I/O (PIO) enables precise signal generation for protocols that lack native hardware support. On the ESP32 family this is implemented using the RMT peripheral.

## Basic Usage

```cpp
#include "McuPio.h"

McuPio pio(HF_RMT_CHANNEL_0);
pio.Open();
```

## Sending Pulses

```cpp
hf_rmt_item_t items[2];
items[0].duration0 = 10;
items[0].level0 = 1;
items[0].duration1 = 20;
items[0].level1 = 0;

pio.Transmit(items, 1);
```

This example sends a single high/low pulse sequence.

## WS2812 Example

For controlling addressable LEDs refer to [WS2812 Example](../examples/ws2812-pio.md) which builds on this API.

## Tips

- Use DMA capable memory for large sequences.
- Match the RMT clock divider to achieve the desired timing resolution.

## 🧩 Advanced Protocols
- Combine multiple channels to generate differential signals
- Use interrupts to queue successive frames for continuous output

## 🔗 Related Examples
- [🎯 Custom Protocol](../examples/custom-protocol.md)
- [🌈 WS2812 PIO](../examples/ws2812-pio.md)
