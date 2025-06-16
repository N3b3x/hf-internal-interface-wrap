# RMT Class Guide

Wrapper around the ESP‑IDF RMT API for transmitting or receiving pulse sequences. 📡

## Features
- Configure a channel for TX or RX on demand
- Send arrays of `rmt_symbol_word_t`
- Receive `rmt_symbol_word_t` buffers
- RAII cleanup of the driver

## Example
```cpp
RMT rmt(RMT_CHANNEL_0, GPIO_NUM_18, 80);
rmt.OpenTx();
rmt_symbol_word_t item = {};
item.level0 = 1;
item.duration0 = 500;
item.level1 = 0;
item.duration1 = 500;
rmt.Write(&item, 1);

// Switch to RX later
// rmt.OpenRx();
```

---

[← Previous](SfUartDriver.md) | [Documentation Index](index.md) | [Next →](NvsStorage.md)
