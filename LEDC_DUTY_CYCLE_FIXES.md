# ESP32-C6 LEDC Duty Cycle Fixes - Comprehensive Analysis & Implementation

## Executive Summary

This document details the comprehensive analysis and implementation of critical fixes for ESP32-C6 LEDC (LED Controller) duty cycle issues in the ESP-IDF v5.5 PWM wrapping implementation. The fixes address fundamental problems in resolution handling, timer state management, and duty cycle calculations that were causing incorrect duty cycle behavior in multiple duty cycle tests.

## Critical Issues Identified and Fixed

### 1. **Timer Resolution Tracking Failure** ⚠️ **CRITICAL**
**Location**: `src/mcu/esp32/EspPwm.cpp:ConfigurePlatformTimer()`
**Problem**: Timer resolution was not being stored in timer state after hardware configuration
**Impact**: All duty cycle calculations used uninitialized resolution (0), causing division by zero or incorrect calculations

**Fix Implemented**:
```cpp
// CRITICAL FIX: Ensure timer state reflects actual hardware configuration
// This is essential for correct duty cycle calculations
timers_[timer_id].frequency_hz = frequency_hz;
timers_[timer_id].resolution_bits = resolution_bits;
timers_[timer_id].in_use = true;
```

### 2. **Resolution Inconsistency Between Functions** ⚠️ **CRITICAL**
**Location**: `src/mcu/esp32/EspPwm.cpp:ConfigurePlatformChannel()`
**Problem**: Used hardcoded `HF_PWM_DEFAULT_RESOLUTION` instead of timer's actual resolution
**Impact**: Channel configuration and duty calculations used different resolution values

**Fix Implemented**:
```cpp
// CRITICAL FIX: Use timer's actual resolution, not hardcoded default
// This ensures duty cycle calculations are consistent across all functions
hf_u8_t timer_resolution = timers_[timer_id].resolution_bits;
if (timer_resolution == 0) {
  ESP_LOGE(TAG, "Timer %d resolution not set! Using default %d bits", timer_id, HF_PWM_DEFAULT_RESOLUTION);
  timer_resolution = HF_PWM_DEFAULT_RESOLUTION;
}

uint32_t max_duty_value = (1u << timer_resolution) - 1u;
uint32_t initial_duty = std::min<hf_u32_t>(config.duty_initial, max_duty_value);
```

### 3. **Duty Cycle Value Semantic Error** ⚠️ **CRITICAL**
**Location**: `src/mcu/esp32/EspPwm.cpp:ConfigureChannel()`
**Problem**: `BasePwm::DutyCycleToRaw()` expects float percentage (0.0-1.0) but was passed uint32_t raw value
**Impact**: Incorrect type conversion causing wrong duty cycle calculations

**Fix Implemented**:
```cpp
// CRITICAL FIX: Properly handle duty_initial as RAW value, not percentage
// config.duty_initial is a RAW value, not a percentage - validate and store directly
hf_u32_t max_duty_raw = (1U << timer_resolution) - 1;
channels_[channel_id].raw_duty_value = std::min<hf_u32_t>(config.duty_initial, max_duty_raw);
```

### 4. **Missing ESP32-C6 Hardware Validation** 🆕 **NEW FEATURE**
**Location**: New function `ValidateFrequencyResolutionCombination()`
**Problem**: No validation of ESP32-C6 LEDC hardware constraints
**Impact**: Invalid frequency/resolution combinations could be configured

**Fix Implemented**:
```cpp
bool EspPwm::ValidateFrequencyResolutionCombination(hf_u32_t frequency_hz, hf_u8_t resolution_bits) const noexcept {
  // ESP32-C6 LEDC hardware constraint validation
  // The LEDC timer clock must satisfy: freq_hz * (2^resolution_bits) <= source_clock_hz
  
  uint64_t required_clock = static_cast<uint64_t>(frequency_hz) * (1ULL << resolution_bits);
  const uint64_t max_source_clock = 80000000ULL; // 80MHz
  const uint64_t practical_limit = max_source_clock * 95 / 100; // 95% of max for safety margin
  
  return required_clock <= practical_limit;
}
```

### 5. **GetChannelStatus Resolution Bug** ⚠️ **CRITICAL**
**Location**: `src/mcu/esp32/EspPwm.cpp:GetChannelStatus()`
**Problem**: Used hardcoded default resolution instead of timer's actual resolution
**Impact**: Status reporting showed incorrect duty cycle values

**Fix Implemented**:
```cpp
// CRITICAL FIX: Use actual timer resolution, not hardcoded default
status.resolution_bits = timers_[timer_id].resolution_bits;
status.current_duty_cycle = BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value, 
                                                   timers_[timer_id].resolution_bits);
```

## ESP32-C6 Specific Constraints Addressed

### LEDC Hardware Limitations
- **Maximum Resolution**: 14 bits
- **Clock Constraint**: `frequency_hz * (2^resolution_bits) ≤ 80MHz`
- **Practical Examples**:
  - 1 kHz @ 14-bit: 1000 * 16384 = 16.384 MHz ✅ Valid
  - 10 kHz @ 10-bit: 10000 * 1024 = 10.24 MHz ✅ Valid  
  - 100 kHz @ 10-bit: 100000 * 1024 = 102.4 MHz ❌ Invalid

### Resolution vs Frequency Trade-offs
| Frequency | Max Resolution | Raw Duty Range |
|-----------|----------------|----------------|
| 1 kHz     | 14-bit         | 0-16383        |
| 10 kHz    | 13-bit         | 0-8191         |
| 20 kHz    | 12-bit         | 0-4095         |
| 50 kHz    | 10-bit         | 0-1023         |

## New Comprehensive Tests Added

### 1. Resolution-Specific Duty Cycle Accuracy Test
```cpp
bool test_resolution_specific_duty_cycles() noexcept
```
**Purpose**: Validates precise duty cycle calculations at 10-bit resolution
**Tests**:
- 0% duty (raw=0) → 0.0000
- 25% duty (raw=255) → 0.2500  
- 50% duty (raw=511) → 0.5000
- 75% duty (raw=767) → 0.7500
- 100% duty (raw=1023) → 1.0000

### 2. Frequency/Resolution Validation Test
```cpp
bool test_frequency_resolution_validation() noexcept
```
**Purpose**: Validates ESP32-C6 hardware constraint enforcement
**Tests**:
- Valid combinations (should succeed)
- Invalid combinations (should fail with proper error)

## Technical Deep Dive: Root Cause Analysis

### The Core Problem Chain
1. **Timer State Initialization**: `ConfigurePlatformTimer()` configured hardware but didn't update software state
2. **Resolution Mismatch**: Different functions used different resolution values for the same timer
3. **Type Confusion**: Raw duty values treated as percentages in conversion functions
4. **Validation Gap**: No hardware constraint checking allowed invalid configurations

### Impact on Multiple Duty Cycle Tests
The comprehensive test `test_duty_cycle_control()` was failing because:

```cpp
// Test was setting: 0.0f, 0.25f, 0.5f, 0.75f, 1.0f
for (float duty : test_duties) {
    pwm.SetDutyCycle(0, duty);           // Used timer resolution = 0 (uninitialized)
    float actual_duty = pwm.GetDutyCycle(0); // Also used resolution = 0
    // Result: Division by zero or garbage values
}
```

### Before vs After Fix Comparison

**Before Fixes**:
```
SetDutyCycle(0, 0.5f):
├── Uses timers_[0].resolution_bits = 0 (uninitialized)
├── DutyCycleToRaw(0.5f, 0) = 0 (wrong!)
└── Hardware gets wrong duty value

GetDutyCycle(0):
├── Uses HF_PWM_DEFAULT_RESOLUTION = 10 (inconsistent!)
├── RawToDutyCycle(hardware_value, 10) = wrong percentage
└── Returns incorrect duty cycle
```

**After Fixes**:
```
SetDutyCycle(0, 0.5f):
├── Uses timers_[0].resolution_bits = 10 (properly initialized)
├── DutyCycleToRaw(0.5f, 10) = 511 (correct!)
└── Hardware gets correct duty value

GetDutyCycle(0):
├── Uses timers_[0].resolution_bits = 10 (consistent!)
├── RawToDutyCycle(511, 10) = 0.4990 ≈ 0.5f (correct!)
└── Returns accurate duty cycle
```

## Validation Results

### Expected Test Outcomes
With these fixes, the comprehensive PWM test should now pass with:
- ✅ Accurate duty cycle setting and readback
- ✅ Consistent resolution handling across all functions
- ✅ Proper ESP32-C6 hardware constraint validation
- ✅ Enhanced error reporting and diagnostics

### Performance Impact
- **Minimal**: Added validation functions are O(1) operations
- **Memory**: No additional memory usage
- **Safety**: Significantly improved with hardware constraint checking

## Future Recommendations

### 1. Enhanced Resolution Configuration
Consider adding per-channel resolution configuration:
```cpp
struct hf_pwm_channel_config_t {
    // ... existing fields ...
    hf_u8_t preferred_resolution_bits;  // Allow per-channel resolution preference
    bool auto_adjust_resolution;        // Auto-adjust for frequency compatibility
};
```

### 2. Advanced Frequency/Resolution Optimization
Implement automatic resolution optimization based on frequency:
```cpp
hf_u8_t EspPwm::CalculateOptimalResolution(hf_u32_t frequency_hz) const noexcept {
    // Calculate maximum achievable resolution for given frequency
    for (hf_u8_t res = HF_PWM_MAX_RESOLUTION; res >= 8; res--) {
        if (ValidateFrequencyResolutionCombination(frequency_hz, res)) {
            return res;
        }
    }
    return 8; // Minimum viable resolution
}
```

### 3. Runtime Resolution Switching
Add capability to change resolution dynamically:
```cpp
hf_pwm_err_t SetResolution(hf_channel_id_t channel_id, hf_u8_t resolution_bits) noexcept;
```

## Conclusion

These comprehensive fixes address the fundamental issues causing LEDC duty cycle problems in ESP32-C6 with ESP-IDF v5.5. The implementation ensures:

1. **Consistency**: All functions use the same resolution values
2. **Accuracy**: Proper type handling and calculations
3. **Validation**: Hardware constraints are enforced
4. **Reliability**: Enhanced error checking and reporting
5. **Testability**: Comprehensive test coverage for validation

The fixes maintain backward compatibility while significantly improving the reliability and accuracy of PWM duty cycle operations.