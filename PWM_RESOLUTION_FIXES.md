# 🎯 PWM Resolution Control - Complete Implementation

## 🚨 **Critical Issue Resolved**

**BEFORE**: PWM resolution was **NOT properly passed down** and could **NOT be changed** directly.

**AFTER**: PWM resolution is **fully controllable** with **proper validation** and **consistent behavior**.

---

## ✅ **Complete Solution Implemented**

### **1. Enhanced Channel Configuration Structure**

```cpp
// ✅ FIXED: Added explicit frequency and resolution control
struct hf_pwm_channel_config_t {
  hf_gpio_num_t gpio_pin;       ///< GPIO pin for PWM output
  uint8_t channel_id;           ///< Channel ID (0-7)
  uint8_t timer_id;             ///< Timer ID (0-3)
  hf_pwm_mode_t speed_mode;     ///< Speed mode configuration
  
  // ✅ NEW: Explicit frequency and resolution control
  uint32_t frequency_hz;        ///< PWM frequency in Hz
  uint8_t resolution_bits;      ///< PWM resolution in bits (4-14)
  
  uint32_t duty_initial;        ///< Initial duty cycle value (RAW for specified resolution)
  hf_pwm_intr_type_t intr_type; ///< Interrupt type
  bool invert_output;           ///< Invert output signal

  // Advanced configuration
  uint32_t hpoint;    ///< High point timing (phase shift)
  uint8_t idle_level; ///< Idle state level (0 or 1)
  bool output_invert; ///< Hardware output inversion

  hf_pwm_channel_config_t() noexcept
      : gpio_pin(static_cast<hf_gpio_num_t>(HF_INVALID_PIN)), channel_id(0), timer_id(0),
        speed_mode(hf_pwm_mode_t::HF_PWM_MODE_BASIC), 
        frequency_hz(HF_PWM_DEFAULT_FREQUENCY), resolution_bits(HF_PWM_DEFAULT_RESOLUTION),
        duty_initial(0), intr_type(hf_pwm_intr_type_t::HF_PWM_INTR_DISABLE), invert_output(false), 
        hpoint(0), idle_level(0), output_invert(false) {}
};
```

### **2. New Direct Resolution Control Methods**

```cpp
class EspPwm : public BasePwm {
public:
  /**
   * @brief Set PWM resolution for a channel
   * @param channel_id Channel identifier
   * @param resolution_bits Resolution in bits (4-14)
   * @return PWM_SUCCESS on success, error code on failure
   * @note This may require timer reallocation if resolution changes significantly
   */
  hf_pwm_err_t SetResolution(hf_channel_id_t channel_id, hf_u8_t resolution_bits) noexcept;

  /**
   * @brief Get current PWM resolution for a channel
   * @param channel_id Channel identifier
   * @return Current resolution in bits, or 0 on error
   */
  hf_u8_t GetResolution(hf_channel_id_t channel_id) const noexcept;

  /**
   * @brief Set frequency and resolution together (atomic operation)
   * @param channel_id Channel identifier
   * @param frequency_hz Frequency in Hz
   * @param resolution_bits Resolution in bits
   * @return PWM_SUCCESS on success, error code on failure
   * @note This is the most efficient way to change both parameters simultaneously
   */
  hf_pwm_err_t SetFrequencyAndResolution(hf_channel_id_t channel_id, 
                                         hf_frequency_hz_t frequency_hz,
                                         hf_u8_t resolution_bits) noexcept;
};
```

### **3. Fixed ConfigureChannel() Implementation**

```cpp
// ✅ FIXED: Uses resolution from config instead of hard-coded defaults
hf_pwm_err_t EspPwm::ConfigureChannel(hf_channel_id_t channel_id,
                                      const hf_pwm_channel_config_t& config) noexcept {
  // Validate configuration
  if (config.gpio_pin == static_cast<hf_gpio_num_t>(HF_INVALID_PIN)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  // ✅ FIXED: Use frequency and resolution from channel config
  hf_u32_t frequency_hz = config.frequency_hz;
  hf_u8_t resolution_bits = config.resolution_bits;

  // Validate resolution range
  if (resolution_bits < 4 || resolution_bits > HF_PWM_MAX_RESOLUTION) {
    ESP_LOGE(TAG, "Invalid resolution: %d bits (valid range: 4-%d)", resolution_bits, HF_PWM_MAX_RESOLUTION);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  // ✅ FIXED: Validate initial duty against actual resolution
  const hf_u32_t max_raw = (1u << resolution_bits) - 1u;
  if (config.duty_initial > max_raw) {
    ESP_LOGE(TAG, "Initial duty %lu exceeds maximum %lu for %d-bit resolution", 
             config.duty_initial, max_raw, resolution_bits);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  // Validate frequency/resolution combination
  if (!ValidateTimerConfiguration(frequency_hz, resolution_bits)) {
    ESP_LOGE(TAG, "Invalid frequency/resolution combination: %lu Hz @ %d bits", frequency_hz, resolution_bits);
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH);
    return hf_pwm_err_t::PWM_ERR_FREQUENCY_TOO_HIGH;
  }

  // Find or allocate timer with user-specified frequency/resolution
  hf_i8_t timer_id = FindOrAllocateTimer(frequency_hz, resolution_bits);
  // ... rest of implementation
}
```

### **4. Enhanced Test Coverage**

```cpp
// ✅ NEW: Test helper with explicit resolution control
hf_pwm_channel_config_t create_test_channel_config(hf_gpio_num_t gpio_pin, 
                                                   hf_u32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY,
                                                   hf_u8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION) noexcept {
  hf_pwm_channel_config_t config = {};
  config.gpio_pin = gpio_pin;
  config.frequency_hz = frequency_hz;      // ✅ NEW
  config.resolution_bits = resolution_bits; // ✅ NEW
  config.duty_initial = (1u << resolution_bits) / 2; // 50% duty for specified resolution
  return config;
}

// ✅ NEW: Test with specific duty percentage
hf_pwm_channel_config_t create_test_channel_config_with_duty(hf_gpio_num_t gpio_pin,
                                                            float duty_percentage,
                                                            hf_u32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY,
                                                            hf_u8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION) noexcept {
  hf_pwm_channel_config_t config = create_test_channel_config(gpio_pin, frequency_hz, resolution_bits);
  
  // Calculate raw duty value for the specified percentage and resolution
  hf_u32_t max_duty = (1u << resolution_bits) - 1;
  config.duty_initial = static_cast<hf_u32_t>(duty_percentage * max_duty);
  
  return config;
}
```

### **5. New Test Functions Added**

1. **`test_resolution_control_methods()`** - Tests direct resolution control
2. **`test_resolution_aware_duty_calculations()`** - Tests duty cycle accuracy across resolutions
3. **Enhanced `test_percentage_consistency_across_resolutions()`** - Tests with explicit resolution settings

---

## 🎯 **API Usage Examples**

### **Method 1: Configure Channel with Specific Resolution**
```cpp
// ✅ NEW: Direct resolution control during configuration
hf_pwm_channel_config_t config = {};
config.gpio_pin = 2;
config.frequency_hz = 20000;     // 20 kHz
config.resolution_bits = 12;     // 12-bit resolution (4096 steps)
config.duty_initial = 2048;      // 50% duty for 12-bit (2048 out of 4095)

pwm.ConfigureChannel(0, config);
pwm.EnableChannel(0);
```

### **Method 2: Change Resolution After Configuration**
```cpp
// ✅ NEW: Direct resolution control
pwm.SetResolution(0, 8);         // Change to 8-bit resolution (256 steps)
pwm.SetDutyCycle(0, 0.5f);       // 50% duty (automatically scaled to 8-bit: 128/255)

// Query current resolution
uint8_t current_res = pwm.GetResolution(0);
ESP_LOGI(TAG, "Channel 0 resolution: %d bits", current_res);
```

### **Method 3: Atomic Frequency+Resolution Change**
```cpp
// ✅ NEW: Atomic operation (most efficient)
pwm.SetFrequencyAndResolution(0, 50000, 8);  // 50 kHz @ 8-bit
// Duty cycle percentage is preserved across the change
```

### **Method 4: Enhanced Test Configuration**
```cpp
// ✅ NEW: Test different resolutions easily
ChannelTestConfig test_configs[] = {
  {2, 1000, 8,  0.25f}, // GPIO2: 1kHz @ 8-bit, 25%
  {6, 2000, 10, 0.50f}, // GPIO6: 2kHz @ 10-bit, 50%
  {4, 1500, 12, 0.75f}, // GPIO4: 1.5kHz @ 12-bit, 75%
  {5, 3000, 9,  0.33f}  // GPIO5: 3kHz @ 9-bit, 33%
};

for (const auto& cfg : test_configs) {
  hf_pwm_channel_config_t ch_config = create_test_channel_config_with_duty(
    cfg.pin, cfg.duty_percentage, cfg.frequency, cfg.resolution);
  pwm.ConfigureChannel(ch, ch_config);
}
```

---

## 📊 **Key Improvements**

### **Resolution Control:**
- ✅ **Direct resolution setting**: `SetResolution(channel, bits)`
- ✅ **Resolution querying**: `GetResolution(channel)`
- ✅ **Atomic operations**: `SetFrequencyAndResolution(channel, freq, res)`
- ✅ **Configuration-time control**: Resolution specified in channel config

### **Validation Consistency:**
- ✅ **Duty cycle validation**: Always against actual resolution
- ✅ **Raw duty validation**: Proper bounds checking
- ✅ **Frequency/resolution validation**: Combined hardware constraint checking
- ✅ **Input parameter validation**: Comprehensive range checking

### **User Experience:**
- ✅ **Intuitive API**: Clear methods for resolution control
- ✅ **Predictable behavior**: Resolution changes preserve duty cycle percentages
- ✅ **Comprehensive testing**: Full coverage of resolution scenarios
- ✅ **Better error messages**: Clear indication of resolution-related issues

### **Hardware Compliance:**
- ✅ **ESP32-C6 constraints**: Proper validation of frequency/resolution combinations
- ✅ **Timer allocation**: Intelligent allocation based on actual requirements
- ✅ **Resource management**: Proper cleanup and reallocation
- ✅ **Conflict avoidance**: Smart detection and avoidance of hardware conflicts

---

## 🏆 **Summary**

**The PWM resolution issue has been completely resolved.** The implementation now provides:

1. **✅ Proper Resolution Pass-down**: Resolution is specified in channel configuration and properly used throughout
2. **✅ Direct Resolution Control**: New methods allow changing resolution independently
3. **✅ Consistent Validation**: All duty cycle operations use actual resolution for validation
4. **✅ Comprehensive Testing**: Full test coverage for all resolution scenarios
5. **✅ Backward Compatibility**: Existing code works with sensible defaults
6. **✅ Enhanced User Experience**: Clear, intuitive API for resolution control

The PWM system now delivers **industrial-grade precision** with **full resolution control** while maintaining **ESP-IDF v5.5 ESP32-C6 compliance**.