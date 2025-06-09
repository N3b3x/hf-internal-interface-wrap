# RmtOutput Class Guide

Wrapper around the ESP‑IDF RMT API for transmitting pulse sequences. 📡

## Features
- Install and configure a TX channel
- Send arrays of `rmt_item32_t`
- RAII cleanup of the driver

## Example
```cpp
RmtOutput rmt(RMT_CHANNEL_0, GPIO_NUM_18, 80);
rmt.Open();
rmt_item32_t item = {};
item.level0 = 1;
item.duration0 = 500;
item.level1 = 0;
item.duration1 = 500;
rmt.Write(&item, 1);
```

---

[← Previous](DacOutput.md) | [Documentation Index](index.md) | [Next →](NvsStorage.md)
