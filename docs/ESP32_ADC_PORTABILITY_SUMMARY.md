# ESP32 ADC Portability Implementation Summary

## Overview

The ESP32 ADC implementation has been completely redesigned to be portable across all ESP32 variants while maintaining single ADC unit encapsulation. This allows developers to write code once and use it across different ESP32 boards by simply changing the MCU selection.

## Key Changes Made

### 1. **Variant-Specific Configuration System**

**File: `inc/mcu/esp32/EspAdc.h`**

Added comprehensive variant-specific configuration using preprocessor directives:

```cpp
// ESP32-C6 Configuration
#ifdef HF_MCU_ESP32C6
#define HF_ESP32_ADC_MAX_UNITS 1                    ///< ESP32-C6 has 1 ADC unit (ADC1)
#define HF_ESP32_ADC_MAX_CHANNELS 7                 ///< ESP32-C6 has 7 ADC channels (0-6)
#define HF_ESP32_ADC_MAX_FILTERS 2                  ///< ESP32-C6 supports 2 IIR filters
#define HF_ESP32_ADC_MAX_MONITORS 2                 ///< ESP32-C6 supports 2 threshold monitors
// ... other configuration constants

// ESP32 Classic Configuration
#elif defined(HF_MCU_ESP32)
#define HF_ESP32_ADC_MAX_UNITS 2                    ///< ESP32 has 2 ADC units (ADC1, ADC2)
#define HF_ESP32_ADC_MAX_CHANNELS 8                 ///< ESP32 has 8 ADC channels per unit (0-7)
// ... configuration for all other variants
```

### 2. **Dynamic GPIO Mapping System**

**File: `src/mcu/esp32/EspAdc.cpp`**

Implemented variant-specific GPIO mappings with helper function:

```cpp
// ESP32-C6 Channel to GPIO mapping
#ifdef HF_MCU_ESP32C6
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_ADC_MAX_CHANNELS] = {
    GPIO_NUM_0,  // ADC_CHANNEL_0
    GPIO_NUM_1,  // ADC_CHANNEL_1  
    // ... mappings for all channels
};

// ESP32 Classic has different mappings for ADC1 and ADC2
#elif defined(HF_MCU_ESP32)
static constexpr gpio_num_t CHANNEL_TO_GPIO_ADC1[HF_ESP32_ADC_MAX_CHANNELS] = {
    GPIO_NUM_36, // ADC1_CHANNEL_0
    // ... ADC1 mappings
};

static constexpr gpio_num_t CHANNEL_TO_GPIO_ADC2[HF_ESP32_ADC_MAX_CHANNELS] = {
    GPIO_NUM_4,  // ADC2_CHANNEL_0
    // ... ADC2 mappings
};
```

### 3. **Enhanced MCU Selection System**

**File: `inc/mcu/utils/McuSelect.h`**

Added support for all ESP32 variants:

```cpp
// ESP32-S2 Configuration
#elif defined(HF_TARGET_MCU_ESP32S2)
#define HF_MCU_ESP32S2
#define HF_MCU_FAMILY_ESP32
#define HF_MCU_NAME "ESP32-S2"
#define HF_MCU_ARCHITECTURE "Xtensa LX7"
#define HF_MCU_VARIANT_S2

// ESP32-S3 Configuration
#elif defined(HF_TARGET_MCU_ESP32S3)
#define HF_MCU_ESP32S3
#define HF_MCU_FAMILY_ESP32
#define HF_MCU_NAME "ESP32-S3"
#define HF_MCU_ARCHITECTURE "Xtensa LX7"
#define HF_MCU_VARIANT_S3

// ... configurations for ESP32-C3, ESP32-C2, ESP32-H2
```

### 4. **Portable Implementation**

**File: `src/mcu/esp32/EspAdc.cpp`**

Updated all hardcoded ESP32-C6 specific values to use variant-specific constants:

```cpp
// Before (ESP32-C6 specific)
static constexpr uint8_t ADC_UNIT_1 = 0;
static constexpr uint8_t MAX_CHANNELS_ESP32C6 = 7;

// After (variant-specific)
static constexpr uint32_t ADC_MAX_RAW_VALUE = HF_ESP32_ADC_MAX_RAW_VALUE;
static constexpr uint8_t HF_ADC_MAX_CHANNELS = HF_ESP32_ADC_MAX_CHANNELS;
```

### 5. **Dynamic Unit Validation**

Updated validation to work with all variants:

```cpp
// Before (ESP32-C6 specific)
if (config_.unit_id != 0) {
    ESP_LOGE(TAG, "Invalid ADC unit %d (ESP32-C6 only has ADC1, unit 0)", config_.unit_id);
    return hf_adc_err_t::ADC_ERR_INVALID_CONFIGURATION;
}

// After (variant-specific)
if (config_.unit_id >= HF_ESP32_ADC_MAX_UNITS) {
    ESP_LOGE(TAG, "Invalid ADC unit %d (this ESP32 variant supports %d units)", 
            config_.unit_id, HF_ESP32_ADC_MAX_UNITS);
    return hf_adc_err_t::ADC_ERR_INVALID_CONFIGURATION;
}
```

## Supported ESP32 Variants

| Variant | ADC Units | Channels per Unit | Max Sampling | Architecture |
|---------|-----------|-------------------|--------------|--------------|
| ESP32-C6 | 1 | 7 | 100kSPS | RISC-V RV32IMAC |
| ESP32 Classic | 2 | 8 | 200kSPS | Xtensa LX6 |
| ESP32-S2 | 1 | 10 | 200kSPS | Xtensa LX7 |
| ESP32-S3 | 2 | 10 | 200kSPS | Xtensa LX7 |
| ESP32-C3 | 1 | 6 | 100kSPS | RISC-V RV32IMC |
| ESP32-C2 | 1 | 4 | 100kSPS | RISC-V RV32IMC |
| ESP32-H2 | 1 | 6 | 100kSPS | RISC-V RV32IMC |

## Usage Examples

### Single Unit Variants (ESP32-C6, ESP32-S2, ESP32-C3, ESP32-C2, ESP32-H2)

```cpp
// Select variant in McuSelect.h
#define HF_TARGET_MCU_ESP32C6

// Create ADC configuration
hf_adc_unit_config_t adc_config = {
    .unit_id = 0,  // Only unit available
    .mode = hf_adc_mode_t::ONESHOT,
    // ... other configuration
};

EspAdc adc(adc_config);
```

### Dual Unit Variants (ESP32 Classic, ESP32-S3)

```cpp
// Select variant in McuSelect.h
#define HF_TARGET_MCU_ESP32

// Create two ADC instances
hf_adc_unit_config_t adc1_config = {
    .unit_id = 0,  // ADC1
    // ... configuration
};

hf_adc_unit_config_t adc2_config = {
    .unit_id = 1,  // ADC2
    // ... configuration
};

EspAdc adc1(adc1_config);
EspAdc adc2(adc2_config);
```

## Key Benefits

### 1. **Write Once, Run Anywhere**
- Single codebase works across all ESP32 variants
- No need to maintain separate implementations
- Automatic configuration based on MCU selection

### 2. **Compile-Time Optimization**
- All variant-specific values are resolved at compile time
- No runtime overhead for variant detection
- Efficient memory usage

### 3. **Maintainable Architecture**
- Clear separation of variant-specific configurations
- Easy to add new ESP32 variants
- Consistent API across all variants

### 4. **Flexible Unit Management**
- Each EspAdc instance represents one ADC unit
- Higher-level applications can manage multiple units
- Proper resource isolation between units

### 5. **Comprehensive Documentation**
- Complete GPIO mapping reference
- Usage examples for all variants
- Best practices and error handling

## Migration Guide

### For Existing ESP32-C6 Users

1. **No code changes required** - existing code will continue to work
2. **Optional**: Update to use the new portable patterns
3. **Benefit**: Can now easily switch to other ESP32 variants

### For New Projects

1. **Select target ESP32 variant** in `McuSelect.h`
2. **Use the portable examples** from the documentation
3. **Leverage automatic configuration** - no manual GPIO mapping needed

## Technical Implementation Details

### Compile-Time Configuration

The system uses preprocessor directives to:
- Select variant-specific constants
- Configure GPIO mappings
- Set hardware limits
- Define feature availability

### Runtime Flexibility

The implementation provides:
- Dynamic unit validation
- Flexible channel configuration
- Adaptive error messages
- Variant-aware diagnostics

### Memory Efficiency

- No runtime variant detection overhead
- Compile-time constant resolution
- Efficient array sizing based on variant
- Minimal memory footprint

## Future Extensibility

### Adding New ESP32 Variants

To add support for a new ESP32 variant:

1. **Add MCU selection define** in `McuSelect.h`
2. **Add variant configuration** in `EspAdc.h`
3. **Add GPIO mappings** in `EspAdc.cpp`
4. **Update documentation** with new variant details

### Example for New Variant

```cpp
// In McuSelect.h
#elif defined(HF_TARGET_MCU_ESP32NEW)
#define HF_MCU_ESP32NEW
#define HF_MCU_FAMILY_ESP32
#define HF_MCU_NAME "ESP32-NEW"
#define HF_MCU_ARCHITECTURE "RISC-V RV32IMC"
#define HF_MCU_VARIANT_NEW

// In EspAdc.h
#elif defined(HF_MCU_ESP32NEW)
#define HF_ESP32_ADC_MAX_UNITS 1
#define HF_ESP32_ADC_MAX_CHANNELS 8
// ... other configuration

// In EspAdc.cpp
#elif defined(HF_MCU_ESP32NEW)
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_ADC_MAX_CHANNELS] = {
    // ... GPIO mappings
};
```

## Conclusion

The ESP32 ADC implementation is now fully portable across all ESP32 variants while maintaining excellent performance, comprehensive feature support, and ease of use. The single ADC unit encapsulation design allows for flexible multi-unit configurations while providing a consistent API across all supported platforms.

This implementation demonstrates best practices for embedded software portability and serves as a template for other peripheral drivers in the HardFOC system. 