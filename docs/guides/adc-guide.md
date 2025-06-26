# 📊 ADC Guide

The **Analog-to-Digital Converter (ADC)** API provides high level access to analog inputs while keeping the implementation platform independent. This guide covers initialization, configuration and common usage patterns.

## Initialization

```cpp
#include "McuAdc.h"

McuAdc adc(HF_ADC_UNIT_1, HF_ADC_CHANNEL_0, HF_ADC_ATTEN_DB_11);
adc.Open();
```

- `HF_ADC_UNIT_1` selects the ADC unit.
- `HF_ADC_CHANNEL_0` selects the input channel.
- `HF_ADC_ATTEN_DB_11` configures the attenuation for full voltage range.

## Reading Values

```cpp
hf_u32_t raw = adc.Read();
float voltage = adc.ReadVoltage();
```

Use `Read()` for the raw value and `ReadVoltage()` to convert to volts automatically.

### Averaged Samples

```cpp
hf_u32_t average = adc.ReadAveraged(16);
```

Averaging multiple samples helps reduce noise for sensors.

## Calibration

On some MCUs calibration improves accuracy. Call `adc.Calibrate()` if supported. The wrapper falls back to default values when calibration is not available.

## Tips

- Use the thread‑safe wrapper `SfAdc` when accessing from multiple tasks.
- Configure the ADC clock and sampling resolution in the MCU implementation for best performance.
- Refer to the [Porting Guide](porting-guide.md) when adding support for a new ADC peripheral.

## 🚀 DMA & Continuous Sampling

For high-speed acquisition some MCUs support DMA-driven ADC sampling. Check your platform's implementation for `StartContinuous()` and `ReadBuffer()` functions.

## 🛠️ Troubleshooting
- Ensure the ADC pin is configured correctly; some pins are input-only
- Verify the attenuation setting matches the expected voltage range
- Use a low-impedance signal source to improve accuracy

## 🔗 Related Examples
- [📊 Basic ADC Example](../examples/basic-adc.md)
- [🏭 Industrial I/O](../examples/industrial-io.md)
