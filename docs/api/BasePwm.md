# 🎛️ BasePwm API Reference

<div align="center">

**📋 Navigation**

[← Previous: BaseAdc](BaseAdc.md) | [Back to API Index](README.md) | [Next: BaseI2c →](BaseI2c.md)

</div>

---

## 🌟 Overview

`BasePwm` is the abstract base class for all PWM (Pulse Width Modulation) implementations in the
HardFOC system.
It provides a unified interface for motor control, LED dimming, servo control,
and other PWM applications with comprehensive multi-channel support.

## ✨ Features

- **🔢 Multi-Channel Support** - Control multiple PWM channels simultaneously
- **⚡ Variable Frequency** - Configurable frequency per channel with wide range support
- **🎯 Precise Duty Control** - High-resolution duty cycle control with hardware precision
- **🔄 Fade Operations** - Smooth transitions between duty cycle values
- **🛡️ Hardware Protection** - Built-in fault detection and recovery mechanisms
- **📊 Performance Monitoring** - Comprehensive statistics and diagnostics
- **🔧 Lazy Initialization** - Resources allocated only when needed
- **🏎️ Real-Time Optimized** - Designed for time-critical motor control applications

## 📁 Header File

```cpp
#include "inc/base/BasePwm.h"
```text

## 🎯 Type Definitions

### 🚨 Error Codes

```cpp
enum class hf*pwm*err*t : hf*u32*t {
    PWM*SUCCESS = 0,                    // ✅ Success
    PWM*ERR*FAILURE = 1,                // ❌ General failure
    PWM*ERR*NOT*INITIALIZED = 2,        // ⚠️ Not initialized
    PWM*ERR*ALREADY*INITIALIZED = 3,    // ⚠️ Already initialized
    PWM*ERR*INVALID*PARAMETER = 4,      // 🚫 Invalid parameter
    PWM*ERR*NULL*POINTER = 5,           // 🚫 Null pointer
    PWM*ERR*OUT*OF*MEMORY = 6,          // 💾 Out of memory
    PWM*ERR*INVALID*CHANNEL = 7,        // 🔍 Invalid PWM channel
    PWM*ERR*CHANNEL*BUSY = 8,           // 🔄 Channel already in use
    PWM*ERR*CHANNEL*NOT*AVAILABLE = 9,  // 🚫 Channel not available
    PWM*ERR*INSUFFICIENT*CHANNELS = 10, // 📉 Insufficient channels
    PWM*ERR*INVALID*FREQUENCY = 11,     // 📻 Invalid frequency
    PWM*ERR*FREQUENCY*TOO*HIGH = 12,    // 📈 Frequency too high
    PWM*ERR*FREQUENCY*TOO*LOW = 13,     // 📉 Frequency too low
    PWM*ERR*RESOLUTION*NOT*SUPPORTED = 14, // 🎯 Resolution not supported
    PWM*ERR*INVALID*DUTY*CYCLE = 15,    // 🎛️ Invalid duty cycle
    PWM*ERR*DUTY*OUT*OF*RANGE = 16,     // 📏 Duty cycle out of range
    PWM*ERR*HARDWARE*FAULT = 17,        // 💥 Hardware fault
    PWM*ERR*TIMER*CONFLICT = 18,        // ⏱️ Timer resource conflict
    PWM*ERR*PIN*CONFLICT = 19,          // 🔌 Pin already in use
    PWM*ERR*COMMUNICATION*TIMEOUT = 20, // ⏰ Communication timeout
    PWM*ERR*COMMUNICATION*FAILURE = 21, // 📡 Communication failure
    PWM*ERR*DEVICE*NOT*RESPONDING = 22, // 🔇 Device not responding
    PWM*ERR*INVALID*DEVICE*ID = 23,     // 🆔 Invalid device ID
    PWM*ERR*UNSUPPORTED*OPERATION = 24  // 🚫 Unsupported operation
};
```text

### 📊 Statistics Structure

```cpp
struct hf*pwm*statistics*t {
    hf*u32*t duty*updates*count;        // 🔄 Total duty cycle updates
    hf*u32*t frequency*changes*count;   // 📻 Total frequency changes
    hf*u32*t fade*operations*count;     // 🌟 Total fade operations
    hf*u32*t error*count;               // ❌ Total error count
    hf*u32*t channel*enables*count;     // ✅ Total channel enable operations
    hf*u32*t channel*disables*count;    // ❌ Total channel disable operations
};
```text

## 🏗️ Class Interface

```cpp
class BasePwm {
public:
    // 🔧 Lifecycle management
    virtual ~BasePwm() noexcept = default;
    virtual hf*pwm*err*t Initialize() noexcept = 0;
    virtual hf*pwm*err*t Deinitialize() noexcept = 0;
    bool IsInitialized() const noexcept;
    bool EnsureInitialized() noexcept;
    bool EnsureDeinitialized() noexcept;

    // 📡 Channel management
    virtual hf*pwm*err*t EnableChannel(hf*channel*id*t channel*id) noexcept = 0;
    virtual hf*pwm*err*t DisableChannel(hf*channel*id*t channel*id) noexcept = 0;
    virtual bool IsChannelEnabled(hf*channel*id*t channel*id) const noexcept = 0;

    // 🎛️ PWM control
    virtual hf*pwm*err*t SetDutyCycle(hf*channel*id*t channel*id, float duty*cycle) noexcept = 0;
virtual float GetDutyCycle(hf*channel*id*t channel*id) const noexcept = 0;
    virtual hf*pwm*err*t SetFrequency(hf*channel*id*t channel*id, hf*frequency*hz*t frequency) noexcept = 0;
    virtual hf*pwm*err*t GetFrequency(hf*channel*id*t channel*id, hf*frequency*hz*t& frequency) const noexcept = 0;

    // 🌟 Advanced features
    virtual hf*pwm*err*t StartFade(hf*channel*id*t channel*id, float target*duty*percent, 
                                  hf*time*t fade*time*ms) noexcept = 0;
    virtual hf*pwm*err*t StopFade(hf*channel*id*t channel*id) noexcept = 0;
    virtual bool IsFading(hf*channel*id*t channel*id) const noexcept = 0;

    // 📊 Information and diagnostics
    virtual hf*u8*t GetMaxChannels() const noexcept = 0;
    virtual bool IsChannelAvailable(hf*channel*id*t channel*id) const noexcept = 0;
    virtual hf*pwm*err*t GetStatistics(hf*pwm*statistics*t& stats) const noexcept = 0;
    virtual hf*pwm*err*t ResetStatistics() noexcept = 0;
};
```text

## 🎯 Core Methods

### 🔧 Initialization

```cpp
bool EnsureInitialized() noexcept;
```text
**Purpose:** 🚀 Lazy initialization - automatically initializes PWM if not already done  
**Returns:** `true` if successful, `false` on failure  
**Usage:** Call before any PWM operations

### 📡 Channel Control

```cpp
hf*pwm*err*t EnableChannel(hf*channel*id*t channel*id) noexcept;
hf*pwm*err*t DisableChannel(hf*channel*id*t channel*id) noexcept;
bool IsChannelEnabled(hf*channel*id*t channel*id) const noexcept;
```text
**Purpose:** 🎛️ Enable/disable individual PWM channels  
**Parameters:** Channel ID (0-based indexing)  
**Returns:** Error code or boolean status

### 🎛️ Duty Cycle Control

```cpp
hf*pwm*err*t SetDutyCycle(hf*channel*id*t channel*id, float duty*percent) noexcept;
hf*pwm*err*t GetDutyCycle(hf*channel*id*t channel*id, float& duty*percent) const noexcept;
```text
**Purpose:** 🎯 Set/get PWM duty cycle as percentage (0.0 - 100.0)  
**Parameters:** 
- `channel*id` - Target PWM channel
- `duty*percent` - Duty cycle percentage (0.0 = 0%, 100.0 = 100%)

### 📻 Frequency Control

```cpp
hf*pwm*err*t SetFrequency(hf*channel*id*t channel*id, hf*frequency*hz*t frequency) noexcept;
hf*pwm*err*t GetFrequency(hf*channel*id*t channel*id, hf*frequency*hz*t& frequency) const noexcept;
```text
**Purpose:** ⚡ Set/get PWM frequency in Hz  
**Parameters:**
- `channel*id` - Target PWM channel  
- `frequency` - Frequency in Hz

### 🌟 Fade Operations

```cpp
hf*pwm*err*t StartFade(hf*channel*id*t channel*id, float target*duty*percent, 
                      hf*time*t fade*time*ms) noexcept;
hf*pwm*err*t StopFade(hf*channel*id*t channel*id) noexcept;
bool IsFading(hf*channel*id*t channel*id) const noexcept;
```text
**Purpose:** 🌅 Smooth transitions between duty cycle values  
**Parameters:**
- `target*duty*percent` - Target duty cycle (0.0 - 100.0)
- `fade*time*ms` - Fade duration in milliseconds

## 💡 Usage Examples

### 🎯 Basic Motor Speed Control

```cpp
#include "inc/mcu/esp32/EspPwm.h"

// 🏗️ Create PWM instance for motor control
EspPwm motor*pwm;

void setup*motor*control() {
    // 🚀 Initialize PWM system
    if (!motor*pwm.EnsureInitialized()) {
        printf("❌ Failed to initialize PWM\n");
        return;
    }
    
    // 📡 Enable channel 0 for motor speed control
    hf*pwm*err*t result = motor*pwm.EnableChannel(0);
    if (result != hf*pwm*err*t::PWM*SUCCESS) {
        printf("❌ Failed to enable PWM channel: %s\n", HfPwmErrToString(result));
        return;
    }
    
    // 📻 Set PWM frequency to 20kHz (typical for motor control)
    result = motor*pwm.SetFrequency(0, 20000);
    if (result != hf*pwm*err*t::PWM*SUCCESS) {
        printf("❌ Failed to set frequency: %s\n", HfPwmErrToString(result));
        return;
    }
    
    printf("✅ Motor PWM initialized successfully\n");
}

void set*motor*speed(float speed*percent) {
    // 🎛️ Set motor speed (0-100%)
    hf*pwm*err*t result = motor*pwm.SetDutyCycle(0, speed*percent);
    if (result == hf*pwm*err*t::PWM*SUCCESS) {
        printf("🏎️ Motor speed set to %.1f%%\n", speed*percent);
    } else {
        printf("❌ Failed to set motor speed: %s\n", HfPwmErrToString(result));
    }
}

void motor*control*demo() {
    setup*motor*control();
    
    // 🚀 Gradually increase motor speed
    for (float speed = 0.0f; speed <= 100.0f; speed += 10.0f) {
        set*motor*speed(speed);
        vTaskDelay(pdMS*TO*TICKS(500));  // Wait 500ms
    }
    
    // 🛑 Stop motor
    set*motor*speed(0.0f);
}
```text

### 💡 LED Dimming with Fade Effects

```cpp
#include "inc/mcu/esp32/EspPwm.h"

class SmartLED {
private:
    EspPwm led*pwm*;
    hf*channel*id*t channel*;
    
public:
    SmartLED(hf*channel*id*t channel) : channel*(channel) {}
    
    bool initialize() {
        // 🚀 Initialize PWM for LED control
        if (!led*pwm*.EnsureInitialized()) {
            printf("❌ Failed to initialize LED PWM\n");
            return false;
        }
        
        // 📡 Enable LED channel
        hf*pwm*err*t result = led*pwm*.EnableChannel(channel*);
        if (result != hf*pwm*err*t::PWM*SUCCESS) {
            printf("❌ Failed to enable LED channel: %s\n", HfPwmErrToString(result));
            return false;
        }
        
        // 📻 Set frequency to 1kHz (good for LED dimming)
        result = led*pwm*.SetFrequency(channel*, 1000);
        if (result != hf*pwm*err*t::PWM*SUCCESS) {
            printf("❌ Failed to set LED frequency: %s\n", HfPwmErrToString(result));
            return false;
        }
        
        printf("✅ Smart LED initialized on channel %u\n", channel*);
        return true;
    }
    
    void set*brightness(float brightness*percent) {
        // 💡 Set LED brightness instantly
        hf*pwm*err*t result = led*pwm*.SetDutyCycle(channel*, brightness*percent);
        if (result == hf*pwm*err*t::PWM*SUCCESS) {
            printf("💡 LED brightness set to %.1f%%\n", brightness*percent);
        } else {
            printf("❌ Failed to set brightness: %s\n", HfPwmErrToString(result));
        }
    }
    
    void fade*to(float target*brightness, hf*time*t fade*time*ms) {
        // 🌟 Start smooth fade to target brightness
        hf*pwm*err*t result = led*pwm*.StartFade(channel*, target*brightness, fade*time*ms);
        if (result == hf*pwm*err*t::PWM*SUCCESS) {
            printf("🌅 Starting fade to %.1f%% over %u ms\n", target*brightness, fade*time*ms);
        } else {
            printf("❌ Failed to start fade: %s\n", HfPwmErrToString(result));
        }
    }
    
    void breathing*effect() {
        printf("🫁 Starting breathing effect...\n");
        
        // 🌟 Fade in over 2 seconds
        fade*to(100.0f, 2000);
        vTaskDelay(pdMS*TO*TICKS(2500));  // Wait for fade + extra
        
        // 🌙 Fade out over 2 seconds
        fade*to(0.0f, 2000);
        vTaskDelay(pdMS*TO*TICKS(2500));  // Wait for fade + extra
    }
    
    bool is*fading() {
        return led*pwm*.IsFading(channel*);
    }
};
```text

### 🎵 Multi-Channel RGB LED Control

```cpp
class RGBController {
private:
    EspPwm rgb*pwm*;
    static constexpr hf*channel*id*t RED*CHANNEL = 0;
    static constexpr hf*channel*id*t GREEN*CHANNEL = 1;
    static constexpr hf*channel*id*t BLUE*CHANNEL = 2;
    
public:
    bool initialize() {
        // 🚀 Initialize RGB PWM controller
        if (!rgb*pwm*.EnsureInitialized()) {
            printf("❌ Failed to initialize RGB PWM\n");
            return false;
        }
        
        // 📡 Enable all RGB channels
        const hf*channel*id*t channels[] = {RED*CHANNEL, GREEN*CHANNEL, BLUE*CHANNEL};
        const char* colors[] = {"🔴 Red", "🟢 Green", "🔵 Blue"};
        
        for (int i = 0; i < 3; i++) {
            hf*pwm*err*t result = rgb*pwm*.EnableChannel(channels[i]);
            if (result != hf*pwm*err*t::PWM*SUCCESS) {
                printf("❌ Failed to enable %s channel: %s\n", colors[i], HfPwmErrToString(result));
                return false;
            }
            
            // 📻 Set frequency to 1kHz for all channels
            result = rgb*pwm*.SetFrequency(channels[i], 1000);
            if (result != hf*pwm*err*t::PWM*SUCCESS) {
                printf("❌ Failed to set %s frequency: %s\n", colors[i], HfPwmErrToString(result));
                return false;
            }
        }
        
        printf("🌈 RGB Controller initialized successfully\n");
        return true;
    }
    
    void set*rgb*color(float red*percent, float green*percent, float blue*percent) {
        // 🎨 Set RGB color components
        struct {
            hf*channel*id*t channel;
            float value;
            const char* name;
            const char* emoji;
        } components[] = {
            {RED*CHANNEL, red*percent, "Red", "🔴"},
            {GREEN*CHANNEL, green*percent, "Green", "🟢"},
            {BLUE*CHANNEL, blue*percent, "Blue", "🔵"}
        };
        
        printf("🎨 Setting RGB color: R=%.1f%%, G=%.1f%%, B=%.1f%%\n", 
               red*percent, green*percent, blue*percent);
        
        for (const auto& comp : components) {
            hf*pwm*err*t result = rgb*pwm*.SetDutyCycle(comp.channel, comp.value);
            if (result != hf*pwm*err*t::PWM*SUCCESS) {
                printf("❌ Failed to set %s %s: %s\n", comp.emoji, comp.name, HfPwmErrToString(result));
            }
        }
    }
    
    void color*demo() {
        printf("🌈 Starting RGB color demo...\n");
        
        // 🔴 Pure red
        set*rgb*color(100.0f, 0.0f, 0.0f);
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // 🟢 Pure green  
        set*rgb*color(0.0f, 100.0f, 0.0f);
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // 🔵 Pure blue
        set*rgb*color(0.0f, 0.0f, 100.0f);
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // 🟡 Yellow (red + green)
        set*rgb*color(100.0f, 100.0f, 0.0f);
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // 🟣 Magenta (red + blue)
        set*rgb*color(100.0f, 0.0f, 100.0f);
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // 🟦 Cyan (green + blue)
        set*rgb*color(0.0f, 100.0f, 100.0f);
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // ⚪ White (all colors)
        set*rgb*color(100.0f, 100.0f, 100.0f);
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // ⚫ Off
        set*rgb*color(0.0f, 0.0f, 0.0f);
    }
    
    void rainbow*fade() {
        printf("🌈 Starting rainbow fade effect...\n");
        
        // Start fade operations for smooth color transitions
        rgb*pwm*.StartFade(RED*CHANNEL, 100.0f, 2000);
        vTaskDelay(pdMS*TO*TICKS(500));
        rgb*pwm*.StartFade(GREEN*CHANNEL, 100.0f, 2000);
        vTaskDelay(pdMS*TO*TICKS(500));
        rgb*pwm*.StartFade(BLUE*CHANNEL, 100.0f, 2000);
    }
};
```text

### 🤖 Servo Motor Control

```cpp
class ServoController {
private:
    EspPwm servo*pwm*;
    hf*channel*id*t channel*;
    static constexpr float SERVO*MIN*DUTY = 2.5f;   // 2.5% duty cycle (0 degrees)
    static constexpr float SERVO*MAX*DUTY = 12.5f;  // 12.5% duty cycle (180 degrees)
    static constexpr hf*frequency*hz*t SERVO*FREQ = 50; // 50Hz for standard servos
    
public:
    ServoController(hf*channel*id*t channel) : channel*(channel) {}
    
    bool initialize() {
        // 🚀 Initialize servo PWM
        if (!servo*pwm*.EnsureInitialized()) {
            printf("❌ Failed to initialize servo PWM\n");
            return false;
        }
        
        // 📡 Enable servo channel
        hf*pwm*err*t result = servo*pwm*.EnableChannel(channel*);
        if (result != hf*pwm*err*t::PWM*SUCCESS) {
            printf("❌ Failed to enable servo channel: %s\n", HfPwmErrToString(result));
            return false;
        }
        
        // 📻 Set servo frequency to 50Hz
        result = servo*pwm*.SetFrequency(channel*, SERVO*FREQ);
        if (result != hf*pwm*err*t::PWM*SUCCESS) {
            printf("❌ Failed to set servo frequency: %s\n", HfPwmErrToString(result));
            return false;
        }
        
        // 🎯 Set to center position (90 degrees)
        set*angle(90.0f);
        
        printf("🤖 Servo controller initialized on channel %u\n", channel*);
        return true;
    }
    
    void set*angle(float angle*degrees) {
        // 🎯 Convert angle to PWM duty cycle
        // Servo range: 0-180 degrees maps to 2.5%-12.5% duty cycle
        if (angle*degrees < 0.0f) angle*degrees = 0.0f;
        if (angle*degrees > 180.0f) angle*degrees = 180.0f;
        
        float duty*percent = SERVO*MIN*DUTY + (angle*degrees / 180.0f) * (SERVO*MAX*DUTY - SERVO*MIN*DUTY);
        
        hf*pwm*err*t result = servo*pwm*.SetDutyCycle(channel*, duty*percent);
        if (result == hf*pwm*err*t::PWM*SUCCESS) {
            printf("🤖 Servo angle set to %.1f° (%.2f%% duty)\n", angle*degrees, duty*percent);
        } else {
            printf("❌ Failed to set servo angle: %s\n", HfPwmErrToString(result));
        }
    }
    
    void smooth*move*to(float target*angle, hf*time*t move*time*ms) {
        // 🌟 Smooth movement to target angle
        float current*duty, target*duty;
        servo*pwm*.GetDutyCycle(channel*, current*duty);
        
        // Calculate target duty cycle
        if (target*angle < 0.0f) target*angle = 0.0f;
        if (target*angle > 180.0f) target*angle = 180.0f;
        target*duty = SERVO*MIN*DUTY + (target*angle / 180.0f) * (SERVO*MAX*DUTY - SERVO*MIN*DUTY);
        
        hf*pwm*err*t result = servo*pwm*.StartFade(channel*, target*duty, move*time*ms);
        if (result == hf*pwm*err*t::PWM*SUCCESS) {
            printf("🌟 Servo smoothly moving to %.1f° over %u ms\n", target*angle, move*time*ms);
        } else {
            printf("❌ Failed to start smooth movement: %s\n", HfPwmErrToString(result));
        }
    }
    
    void sweep*demo() {
        printf("🔄 Starting servo sweep demo...\n");
        
        // 🔄 Sweep from 0 to 180 degrees
        for (float angle = 0.0f; angle <= 180.0f; angle += 30.0f) {
            set*angle(angle);
            vTaskDelay(pdMS*TO*TICKS(1000));
        }
        
        // 🔄 Sweep back from 180 to 0 degrees
        for (float angle = 180.0f; angle >= 0.0f; angle -= 30.0f) {
            set*angle(angle);
            vTaskDelay(pdMS*TO*TICKS(1000));
        }
        
        // 🎯 Return to center
        set*angle(90.0f);
    }
};
```text

## 📊 Performance and Diagnostics

### 📈 Statistics Monitoring

```cpp
void monitor*pwm*performance(BasePwm& pwm) {
    hf*pwm*statistics*t stats;
    hf*pwm*err*t result = pwm.GetStatistics(stats);
    
    if (result == hf*pwm*err*t::PWM*SUCCESS) {
        printf("📊 PWM Performance Statistics:\n");
        printf("   🔄 Duty Updates: %u\n", stats.duty*updates*count);
        printf("   📻 Frequency Changes: %u\n", stats.frequency*changes*count);
        printf("   🌟 Fade Operations: %u\n", stats.fade*operations*count);
        printf("   ✅ Channel Enables: %u\n", stats.channel*enables*count);
        printf("   ❌ Channel Disables: %u\n", stats.channel*disables*count);
        printf("   ⚠️ Total Errors: %u\n", stats.error*count);
    }
}
```text

## 🛡️ Error Handling Best Practices

### 🎯 Comprehensive Error Checking

```cpp
hf*pwm*err*t safe*set*duty*cycle(BasePwm& pwm, hf*channel*id*t channel, float duty) {
    // ✅ Validate duty cycle range
    if (duty < 0.0f || duty > 100.0f) {
        printf("❌ Invalid duty cycle: %.2f%% (must be 0-100%%)\n", duty);
        return hf*pwm*err*t::PWM*ERR*DUTY*OUT*OF*RANGE;
    }
    
    // ✅ Check if channel is available
    if (!pwm.IsChannelAvailable(channel)) {
        printf("❌ Channel %u not available\n", channel);
        return hf*pwm*err*t::PWM*ERR*INVALID*CHANNEL;
    }
    
    // ✅ Ensure PWM is initialized
    if (!pwm.EnsureInitialized()) {
        printf("❌ Failed to initialize PWM\n");
        return hf*pwm*err*t::PWM*ERR*NOT*INITIALIZED;
    }
    
    // ✅ Enable channel if not already enabled
    if (!pwm.IsChannelEnabled(channel)) {
        hf*pwm*err*t result = pwm.EnableChannel(channel);
        if (result != hf*pwm*err*t::PWM*SUCCESS) {
            printf("❌ Failed to enable channel %u: %s\n", channel, HfPwmErrToString(result));
            return result;
        }
    }
    
    // 🎛️ Set duty cycle
    return pwm.SetDutyCycle(channel, duty);
}
```text

## 🏎️ Performance Considerations

### ⚡ Optimization Tips

- **🔢 Channel Limits** - Check `GetMaxChannels()` before allocating channels
- **📻 Frequency Ranges** - Respect hardware frequency limitations
- **🎯 Resolution Trade-offs** - Higher frequencies may reduce duty cycle resolution
- **🌟 Fade Performance** - Hardware-based fading is faster than software loops
- **💾 Memory Usage** - Use lazy initialization to save memory

### 📊 Typical Performance Ranges

| **Hardware** | **Channels** | **Frequency Range** | **Resolution** |

|--------------|--------------|---------------------|----------------|

| **ESP32-C6 LEDC** | 8 | 1Hz - 40MHz | 1-20 bits |

| **External PWM ICs** | 4-16 | 1Hz - 1.5MHz | 8-16 bits |

| **Motor Controllers** | 2-6 | 1kHz - 100kHz | 10-16 bits |

## 🧵 Thread Safety

The `BasePwm` class is **not thread-safe**.
For concurrent access,
use appropriate synchronization or consider thread-safe wrapper implementations.

## 🔗 Related Documentation

- **[EspPwm API Reference](../esp_api/EspPwm.md)** - ESP32-C6 PWM implementation
- **[BaseGpio API Reference](BaseGpio.md)** - GPIO interface for PWM output pins
- **[HardwareTypes Reference](HardwareTypes.md)** - Platform-agnostic type definitions

---

<div align="center">

**📋 Navigation**

[← Previous: BaseAdc](BaseAdc.md) | [Back to API Index](README.md) | [Next: BaseI2c →](BaseI2c.md)

</div>

**🎛️ BasePwm - Powering Precise Control in HardFOC Systems** 🚀

*From motor control to LED artistry - BasePwm delivers the precision you need* ⚡

</div> 