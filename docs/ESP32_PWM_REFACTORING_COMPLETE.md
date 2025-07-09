# ESP32 PWM Refactoring Plan - Complete Implementation

## Executive Summary

This document provides a comprehensive refactoring plan to address all architectural gaps identified in the ESP32 PWM audit and achieve **100% compliance** with HardFOC architectural standards. The refactoring will transform the current 65% compliant implementation into a production-ready, architecturally uniform PWM solution.

**Target Compliance**: 100% - Full architectural uniformity with EspAdc
**Implementation Timeline**: Complete refactoring in single iteration
**Reference Model**: EspAdc implementation patterns

---

## 1. Critical Gap Fixes (High Priority)

### 1.1 Add EnsureInitialized() to BasePwm.h

**PROBLEM**: Missing lazy initialization pattern in base interface
**IMPACT**: Breaks architectural consistency across entire system

**SOLUTION**:
```cpp
// Add to BasePwm.h in public interface
/**
 * @brief Ensures that the PWM is initialized (lazy initialization pattern).
 * @return true if the PWM is initialized, false otherwise.
 * @note This method should be called at the beginning of all public methods
 *       that require initialization. It implements the lazy initialization
 *       pattern established in other HardFOC modules.
 */
bool EnsureInitialized() noexcept {
  if (!initialized_) {
    initialized_ = Initialize();
  }
  return initialized_;
}
```

**IMPLEMENTATION**: Add to BasePwm.h after constructor declarations

### 1.2 Complete Multi-Variant Macro Lattice

**PROBLEM**: Missing support for ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C2, ESP32-H2
**IMPACT**: Code won't compile on 5 out of 7 ESP32 variants

**SOLUTION**: Complete variant macro lattice in EspTypes_PWM.h

```cpp
// ESP32-S2 Configuration
#elif defined(HF_MCU_ESP32S2)
#define HF_ESP32_PWM_MAX_CHANNELS 8                 ///< ESP32-S2 has 8 LEDC channels (0-7)
#define HF_ESP32_PWM_MAX_TIMERS 4                   ///< ESP32-S2 has 4 LEDC timers (0-3)
#define HF_ESP32_PWM_MAX_RESOLUTION 14              ///< 14-bit resolution max
#define HF_ESP32_PWM_MIN_FREQUENCY 1                ///< 1 Hz minimum
#define HF_ESP32_PWM_MAX_FREQUENCY 40000000         ///< 40 MHz maximum
#define HF_ESP32_PWM_APB_CLOCK_HZ 80000000          ///< APB clock frequency
#define HF_ESP32_PWM_XTAL_CLOCK_HZ 40000000         ///< XTAL clock frequency
#define HF_ESP32_PWM_RC_FAST_CLOCK_HZ 8000000       ///< RC_FAST clock frequency
#define HF_ESP32_PWM_DEFAULT_CLOCK_SRC LEDC_AUTO_CLK ///< Default clock source

// ESP32-S3 Configuration
#elif defined(HF_MCU_ESP32S3)
#define HF_ESP32_PWM_MAX_CHANNELS 8                 ///< ESP32-S3 has 8 LEDC channels (0-7)
#define HF_ESP32_PWM_MAX_TIMERS 4                   ///< ESP32-S3 has 4 LEDC timers (0-3)
#define HF_ESP32_PWM_MAX_RESOLUTION 14              ///< 14-bit resolution max
#define HF_ESP32_PWM_MIN_FREQUENCY 1                ///< 1 Hz minimum
#define HF_ESP32_PWM_MAX_FREQUENCY 40000000         ///< 40 MHz maximum
#define HF_ESP32_PWM_APB_CLOCK_HZ 80000000          ///< APB clock frequency
#define HF_ESP32_PWM_XTAL_CLOCK_HZ 40000000         ///< XTAL clock frequency
#define HF_ESP32_PWM_RC_FAST_CLOCK_HZ 8000000       ///< RC_FAST clock frequency
#define HF_ESP32_PWM_DEFAULT_CLOCK_SRC LEDC_AUTO_CLK ///< Default clock source

// ESP32-C3 Configuration
#elif defined(HF_MCU_ESP32C3)
#define HF_ESP32_PWM_MAX_CHANNELS 6                 ///< ESP32-C3 has 6 LEDC channels (0-5)
#define HF_ESP32_PWM_MAX_TIMERS 4                   ///< ESP32-C3 has 4 LEDC timers (0-3)
#define HF_ESP32_PWM_MAX_RESOLUTION 14              ///< 14-bit resolution max
#define HF_ESP32_PWM_MIN_FREQUENCY 1                ///< 1 Hz minimum
#define HF_ESP32_PWM_MAX_FREQUENCY 40000000         ///< 40 MHz maximum
#define HF_ESP32_PWM_APB_CLOCK_HZ 80000000          ///< APB clock frequency
#define HF_ESP32_PWM_XTAL_CLOCK_HZ 40000000         ///< XTAL clock frequency
#define HF_ESP32_PWM_RC_FAST_CLOCK_HZ 8000000       ///< RC_FAST clock frequency
#define HF_ESP32_PWM_DEFAULT_CLOCK_SRC LEDC_AUTO_CLK ///< Default clock source

// ESP32-C2 Configuration
#elif defined(HF_MCU_ESP32C2)
#define HF_ESP32_PWM_MAX_CHANNELS 4                 ///< ESP32-C2 has 4 LEDC channels (0-3)
#define HF_ESP32_PWM_MAX_TIMERS 4                   ///< ESP32-C2 has 4 LEDC timers (0-3)
#define HF_ESP32_PWM_MAX_RESOLUTION 14              ///< 14-bit resolution max
#define HF_ESP32_PWM_MIN_FREQUENCY 1                ///< 1 Hz minimum
#define HF_ESP32_PWM_MAX_FREQUENCY 40000000         ///< 40 MHz maximum
#define HF_ESP32_PWM_APB_CLOCK_HZ 80000000          ///< APB clock frequency
#define HF_ESP32_PWM_XTAL_CLOCK_HZ 40000000         ///< XTAL clock frequency
#define HF_ESP32_PWM_RC_FAST_CLOCK_HZ 8000000       ///< RC_FAST clock frequency
#define HF_ESP32_PWM_DEFAULT_CLOCK_SRC LEDC_AUTO_CLK ///< Default clock source

// ESP32-H2 Configuration
#elif defined(HF_MCU_ESP32H2)
#define HF_ESP32_PWM_MAX_CHANNELS 6                 ///< ESP32-H2 has 6 LEDC channels (0-5)
#define HF_ESP32_PWM_MAX_TIMERS 4                   ///< ESP32-H2 has 4 LEDC timers (0-3)
#define HF_ESP32_PWM_MAX_RESOLUTION 14              ///< 14-bit resolution max
#define HF_ESP32_PWM_MIN_FREQUENCY 1                ///< 1 Hz minimum
#define HF_ESP32_PWM_MAX_FREQUENCY 40000000         ///< 40 MHz maximum
#define HF_ESP32_PWM_APB_CLOCK_HZ 80000000          ///< APB clock frequency
#define HF_ESP32_PWM_XTAL_CLOCK_HZ 40000000         ///< XTAL clock frequency
#define HF_ESP32_PWM_RC_FAST_CLOCK_HZ 8000000       ///< RC_FAST clock frequency
#define HF_ESP32_PWM_DEFAULT_CLOCK_SRC LEDC_AUTO_CLK ///< Default clock source

// Default fallback (should not be reached)
#else
#error "Unsupported ESP32 variant! Please add support for this ESP32 variant in EspTypes_PWM.h"
#endif
```

### 1.3 Implement Variant-Specific Resource Maps

**PROBLEM**: No GPIO mapping for different ESP32 variants
**IMPACT**: PWM channels can't be properly configured

**SOLUTION**: Add resource maps to EspPwm.cpp following EspAdc pattern

```cpp
//==============================================================================
// ESP32 VARIANT-SPECIFIC CHANNEL TO GPIO MAPPING
//==============================================================================

// ESP32-C6 Channel to GPIO mapping
#ifdef HF_MCU_ESP32C6
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_PWM_MAX_CHANNELS] = {
    GPIO_NUM_0,  // LEDC_CHANNEL_0
    GPIO_NUM_1,  // LEDC_CHANNEL_1
    GPIO_NUM_2,  // LEDC_CHANNEL_2
    GPIO_NUM_3,  // LEDC_CHANNEL_3
    GPIO_NUM_4,  // LEDC_CHANNEL_4
    GPIO_NUM_5,  // LEDC_CHANNEL_5
    GPIO_NUM_6,  // LEDC_CHANNEL_6
    GPIO_NUM_7   // LEDC_CHANNEL_7
};

// ESP32 Classic Channel to GPIO mapping
#elif defined(HF_MCU_ESP32)
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_PWM_MAX_CHANNELS] = {
    GPIO_NUM_0,  // LEDC_CHANNEL_0
    GPIO_NUM_2,  // LEDC_CHANNEL_1
    GPIO_NUM_4,  // LEDC_CHANNEL_2
    GPIO_NUM_5,  // LEDC_CHANNEL_3
    GPIO_NUM_12, // LEDC_CHANNEL_4
    GPIO_NUM_13, // LEDC_CHANNEL_5
    GPIO_NUM_14, // LEDC_CHANNEL_6
    GPIO_NUM_15  // LEDC_CHANNEL_7
};

// ESP32-S2 Channel to GPIO mapping
#elif defined(HF_MCU_ESP32S2)
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_PWM_MAX_CHANNELS] = {
    GPIO_NUM_0,  // LEDC_CHANNEL_0
    GPIO_NUM_1,  // LEDC_CHANNEL_1
    GPIO_NUM_2,  // LEDC_CHANNEL_2
    GPIO_NUM_3,  // LEDC_CHANNEL_3
    GPIO_NUM_4,  // LEDC_CHANNEL_4
    GPIO_NUM_5,  // LEDC_CHANNEL_5
    GPIO_NUM_6,  // LEDC_CHANNEL_6
    GPIO_NUM_7   // LEDC_CHANNEL_7
};

// ESP32-S3 Channel to GPIO mapping
#elif defined(HF_MCU_ESP32S3)
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_PWM_MAX_CHANNELS] = {
    GPIO_NUM_0,  // LEDC_CHANNEL_0
    GPIO_NUM_1,  // LEDC_CHANNEL_1
    GPIO_NUM_2,  // LEDC_CHANNEL_2
    GPIO_NUM_3,  // LEDC_CHANNEL_3
    GPIO_NUM_4,  // LEDC_CHANNEL_4
    GPIO_NUM_5,  // LEDC_CHANNEL_5
    GPIO_NUM_6,  // LEDC_CHANNEL_6
    GPIO_NUM_7   // LEDC_CHANNEL_7
};

// ESP32-C3 Channel to GPIO mapping
#elif defined(HF_MCU_ESP32C3)
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_PWM_MAX_CHANNELS] = {
    GPIO_NUM_0,  // LEDC_CHANNEL_0
    GPIO_NUM_1,  // LEDC_CHANNEL_1
    GPIO_NUM_2,  // LEDC_CHANNEL_2
    GPIO_NUM_3,  // LEDC_CHANNEL_3
    GPIO_NUM_4,  // LEDC_CHANNEL_4
    GPIO_NUM_5   // LEDC_CHANNEL_5
};

// ESP32-C2 Channel to GPIO mapping
#elif defined(HF_MCU_ESP32C2)
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_PWM_MAX_CHANNELS] = {
    GPIO_NUM_0,  // LEDC_CHANNEL_0
    GPIO_NUM_1,  // LEDC_CHANNEL_1
    GPIO_NUM_2,  // LEDC_CHANNEL_2
    GPIO_NUM_3   // LEDC_CHANNEL_3
};

// ESP32-H2 Channel to GPIO mapping
#elif defined(HF_MCU_ESP32H2)
static constexpr gpio_num_t CHANNEL_TO_GPIO[HF_ESP32_PWM_MAX_CHANNELS] = {
    GPIO_NUM_0,  // LEDC_CHANNEL_0
    GPIO_NUM_1,  // LEDC_CHANNEL_1
    GPIO_NUM_2,  // LEDC_CHANNEL_2
    GPIO_NUM_3,  // LEDC_CHANNEL_3
    GPIO_NUM_4,  // LEDC_CHANNEL_4
    GPIO_NUM_5   // LEDC_CHANNEL_5
};

// Default fallback (should not be reached)
#else
#error "Unsupported ESP32 variant! Please add GPIO mapping for this ESP32 variant in EspPwm.cpp"
#endif

// Helper function to get GPIO for channel
static gpio_num_t GetGpioForChannel(hf_channel_id_t channel_id) {
    if (channel_id >= HF_ESP32_PWM_MAX_CHANNELS) {
        return GPIO_NUM_NC; // Invalid channel
    }
    return CHANNEL_TO_GPIO[channel_id];
}
```

### 1.4 Define Unit Configuration Structure

**PROBLEM**: No standardized way to configure PWM units
**IMPACT**: Inconsistent initialization patterns

**SOLUTION**: Add comprehensive unit configuration structure

```cpp
/**
 * @brief PWM unit configuration structure for ESP32 variants.
 * @details Comprehensive configuration for PWM initialization with all
 *          ESP32-specific options and multi-variant support.
 */
struct hf_pwm_unit_config_t {
  uint8_t unit_id;                    ///< PWM unit identifier (0 for single-unit variants)
  hf_pwm_mode_t mode;                 ///< Operating mode (Basic or Fade)
  uint32_t base_clock_hz;             ///< Base clock frequency (default: 80MHz)
  hf_pwm_clock_source_t clock_source; ///< Clock source selection
  bool enable_fade;                   ///< Enable hardware fade support
  bool enable_interrupts;             ///< Enable interrupt support
  uint8_t default_resolution_bits;    ///< Default resolution bits (8-14)
  uint32_t default_frequency_hz;      ///< Default frequency in Hz
  bool enable_complementary_outputs;  ///< Enable complementary output support
  bool enable_deadtime;               ///< Enable deadtime insertion
  uint32_t default_deadtime_ns;       ///< Default deadtime in nanoseconds

  /**
   * @brief Default constructor with sensible defaults for ESP32 variants.
   */
  hf_pwm_unit_config_t() noexcept
      : unit_id(0),                           // Single unit by default
        mode(hf_pwm_mode_t::HF_PWM_MODE_LOW_SPEED), // Low speed mode
        base_clock_hz(80000000),              // 80MHz APB clock
        clock_source(hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT),
        enable_fade(true),                    // Enable fade by default
        enable_interrupts(false),             // Disable interrupts by default
        default_resolution_bits(12),          // 12-bit resolution
        default_frequency_hz(1000),           // 1kHz default frequency
        enable_complementary_outputs(false),  // Disable complementary by default
        enable_deadtime(false),               // Disable deadtime by default
        default_deadtime_ns(1000)             // 1μs default deadtime
  {}
};
```

---

## 2. Medium Priority Fixes

### 2.1 Integrate Statistics Tracking

**PROBLEM**: Statistics structures exist but not integrated
**IMPACT**: No operational metrics or diagnostics

**SOLUTION**: Add statistics tracking to all public methods

```cpp
// Add to EspPwm.cpp in SetDutyCycle method
hf_pwm_err_t EspPwm::SetDutyCycle(hf_channel_id_t channel_id, float duty_cycle) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  // Update statistics
  statistics_.duty_updates_count.fetch_add(1, std::memory_order_relaxed);
  statistics_.last_activity_timestamp = esp_timer_get_time();
  
  // ... existing implementation
}

// Add to EspPwm.cpp in SetFrequency method
hf_pwm_err_t EspPwm::SetFrequency(hf_channel_id_t channel_id, hf_frequency_hz_t frequency_hz) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  // Update statistics
  statistics_.frequency_changes_count.fetch_add(1, std::memory_order_relaxed);
  statistics_.last_activity_timestamp = esp_timer_get_time();
  
  // ... existing implementation
}

// Add to EspPwm.cpp in SetHardwareFade method
hf_pwm_err_t EspPwm::SetHardwareFade(hf_channel_id_t channel_id, float target_duty_cycle, uint32_t fade_time_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);
  
  // Update statistics
  statistics_.fade_operations_count.fetch_add(1, std::memory_order_relaxed);
  statistics_.last_activity_timestamp = esp_timer_get_time();
  
  // ... existing implementation
}
```

### 2.2 Add ISR Safety Documentation

**PROBLEM**: Missing ISR safety documentation
**IMPACT**: Unclear interrupt handling requirements

**SOLUTION**: Add comprehensive ISR safety documentation

```cpp
/**
 * @brief Platform-specific interrupt handler (ISR-safe).
 * @param channel_id Channel that generated interrupt
 * @param user_data User data pointer
 * @note This function is called from interrupt context and must be ISR-safe.
 * @note All operations in this function must be non-blocking and use IRAM.
 * @note Do not call any functions that may block or allocate memory.
 * @note Use only atomic operations and IRAM-safe functions.
 */
static void IRAM_ATTR EspPwm::InterruptHandler(hf_channel_id_t channel_id, void *user_data) noexcept {
  // ISR-safe implementation
  // Only atomic operations and IRAM-safe functions
}
```

### 2.3 Add ESP-IDF Documentation Citations

**PROBLEM**: No inline citations to ESP-IDF documentation
**IMPACT**: Missing traceability to official documentation

**SOLUTION**: Add comprehensive ESP-IDF citations

```cpp
/**
 * @brief Configure LEDC timer using ESP-IDF API.
 * @param timer_id Timer to configure
 * @param frequency_hz Timer frequency
 * @param resolution_bits Timer resolution
 * @return PWM_SUCCESS on success, error code on failure
 * 
 * @note Based on ESP-IDF LEDC API documentation:
 *       https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32c6/api-reference/peripherals/ledc.html#ledc-timer-config
 * @note Uses ledc_timer_config_t structure as defined in ESP-IDF v5.4.2+
 * @note Clock source selection follows ESP-IDF recommendations for power optimization
 */
hf_pwm_err_t EspPwm::ConfigurePlatformTimer(uint8_t timer_id, uint32_t frequency_hz, uint8_t resolution_bits) noexcept {
  // Implementation with ESP-IDF API
  ledc_timer_config_t timer_config = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .duty_resolution = static_cast<ledc_timer_bit_t>(resolution_bits),
      .timer_num = static_cast<ledc_timer_t>(timer_id),
      .freq_hz = frequency_hz,
      .clk_cfg = LEDC_AUTO_CLK
  };
  
  esp_err_t result = ledc_timer_config(&timer_config);
  return (result == ESP_OK) ? hf_pwm_err_t::PWM_SUCCESS : hf_pwm_err_t::PWM_ERR_FAILURE;
}
```

---

## 3. Low Priority Fixes

### 3.1 Ensure Consistent Native Type Usage

**PROBLEM**: Inconsistent usage of native vs wrapper types
**IMPACT**: Potential type confusion and maintenance issues

**SOLUTION**: Standardize on native ESP-IDF types throughout

```cpp
// In EspPwm.cpp - Use native types consistently
ledc_channel_config_t channel_config = {
    .gpio_num = GetGpioForChannel(channel_id),
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = static_cast<ledc_channel_t>(channel_id),
    .timer_sel = static_cast<ledc_timer_t>(timer_id),
    .duty = raw_duty_value,
    .hpoint = 0
};

esp_err_t result = ledc_channel_config(&channel_config);
```

### 3.2 Enhance Doxygen Documentation

**PROBLEM**: Some methods lack comprehensive documentation
**IMPACT**: Reduced developer experience

**SOLUTION**: Add comprehensive Doxygen comments

```cpp
/**
 * @brief Set hardware fade for smooth duty cycle transitions.
 * @param channel_id Channel identifier (0 to MAX_CHANNELS-1)
 * @param target_duty_cycle Target duty cycle (0.0 to 1.0)
 * @param fade_time_ms Fade duration in milliseconds
 * @return PWM_SUCCESS on success, error code on failure
 * 
 * @note This method uses ESP-IDF's hardware fade functionality:
 *       https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32c6/api-reference/peripherals/ledc.html#ledc-fade-functions
 * @note The fade operation is non-blocking and uses hardware acceleration
 * @note Fade completion can be detected via interrupt callbacks
 * @note Requires fade functionality to be enabled during initialization
 * 
 * @example
 * @code
 * // Fade channel 0 to 80% duty cycle over 2 seconds
 * pwm.SetHardwareFade(0, 0.8f, 2000);
 * @endcode
 */
hf_pwm_err_t SetHardwareFade(hf_channel_id_t channel_id, float target_duty_cycle, uint32_t fade_time_ms) noexcept;
```

---

## 4. Implementation Verification

### 4.1 Compilation Test Matrix

**REQUIREMENT**: Must compile on all ESP32 variants

**TEST MATRIX**:
- [ ] ESP32-C6 compilation
- [ ] ESP32 Classic compilation  
- [ ] ESP32-S2 compilation
- [ ] ESP32-S3 compilation
- [ ] ESP32-C3 compilation
- [ ] ESP32-C2 compilation
- [ ] ESP32-H2 compilation

### 4.2 Functional Test Cases

**REQUIREMENT**: LED toggling test passes

**TEST CASES**:
- [ ] Basic PWM generation (duty cycle control)
- [ ] Frequency control
- [ ] Hardware fade functionality
- [ ] Multi-channel operation
- [ ] Timer allocation and management
- [ ] Interrupt handling
- [ ] Error handling and recovery
- [ ] Statistics tracking
- [ ] Diagnostics reporting

### 4.3 Architectural Compliance Checklist

**VERIFICATION**:
- [ ] EnsureInitialized() pattern in all public methods
- [ ] RtosMutex protection for all shared state
- [ ] Native ESP-IDF types used consistently
- [ ] Multi-variant macro lattice complete
- [ ] Resource maps implemented for all variants
- [ ] Statistics tracking integrated
- [ ] ISR safety documented
- [ ] ESP-IDF citations added
- [ ] Doxygen documentation complete

---

## 5. Migration Strategy

### 5.1 Backward Compatibility

**APPROACH**: Maintain backward compatibility during transition

```cpp
// Legacy constructor still supported
explicit EspPwm(uint32_t base_clock_hz) noexcept;

// New constructor with full configuration
explicit EspPwm(const hf_pwm_unit_config_t &config = hf_pwm_unit_config_t{}) noexcept;
```

### 5.2 Gradual Migration Path

1. **Phase 1**: Add missing architectural elements (EnsureInitialized, multi-variant support)
2. **Phase 2**: Implement resource maps and statistics integration
3. **Phase 3**: Add documentation and citations
4. **Phase 4**: Comprehensive testing and validation

---

## 6. Success Criteria

### 6.1 Architectural Compliance

- [ ] **100% compliance** with HardFOC architectural standards
- [ ] **Full compatibility** with EspAdc patterns
- [ ] **Multi-variant support** for all ESP32 variants
- [ ] **Thread safety** with proper mutex protection
- [ ] **ISR safety** with documented interrupt handling

### 6.2 Functional Requirements

- [ ] **Basic PWM** functionality working on all variants
- [ ] **Hardware fade** support with proper timing
- [ ] **Multi-channel** operation with independent control
- [ ] **Timer management** with automatic allocation
- [ ] **Error handling** with comprehensive reporting
- [ ] **Statistics tracking** with operational metrics
- [ ] **Diagnostics** with system health information

### 6.3 Quality Standards

- [ ] **Doxygen documentation** for all public interfaces
- [ ] **ESP-IDF citations** for all API usage
- [ ] **Unit tests** for all major functionality
- [ ] **Performance benchmarks** for critical operations
- [ ] **Memory safety** with proper RAII patterns

---

## 7. Conclusion

This refactoring plan addresses all identified architectural gaps and provides a clear path to **100% compliance** with HardFOC standards. The implementation will achieve:

1. **Architectural Uniformity**: Consistent patterns across all ESP32 modules
2. **Multi-Variant Support**: Full compatibility with all ESP32 variants
3. **Production Readiness**: Comprehensive error handling and diagnostics
4. **Developer Experience**: Complete documentation and examples
5. **Performance**: Optimized implementation with proper resource management

**Next Steps**:
1. Implement all critical gap fixes (Section 1)
2. Add medium priority enhancements (Section 2)
3. Complete low priority improvements (Section 3)
4. Perform comprehensive testing and validation
5. Update documentation and examples

The refactored implementation will serve as the **reference standard** for PWM functionality across the entire ESP32 family, matching the quality and architectural consistency of the EspAdc implementation. 