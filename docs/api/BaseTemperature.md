# 🌡️ BaseTemperature API Reference

<div align="center">

![BaseTemperature](https://img.shields.io/badge/BaseTemperature-Abstract%20Base%20Class-red?style=for-the-badge&logo=thermometer)

**🎯 Unified temperature sensing abstraction for all thermal monitoring operations**

**📋 Navigation**

[← Previous: BaseLogger](BaseLogger.md) | [Back to API Index](README.md) | [Next: BasePeriodicTimer
→](BasePeriodicTimer.md)

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Class Hierarchy**](#-class-hierarchy)
- [📋 **Error Codes**](#-error-codes)
- [🔧 **Core API**](#-core-api)
- [📊 **Data Structures**](#-data-structures)
- [🌡️ **Temperature Units**](#-temperature-units)
- [📊 **Usage Examples**](#-usage-examples)
- [🧪 **Best Practices**](#-best-practices)

---

## 🎯 **Overview**

The `BaseTemperature` class provides a comprehensive temperature sensing abstraction that serves as
the unified interface for all thermal monitoring operations in the HardFOC system.
It supports multiple sensor types, calibration, alert thresholds,
and temperature unit conversions across different hardware implementations.

### ✨ **Key Features**

- 🌡️ **Multi-Sensor Support** - Support for various temperature sensor types
- 🎯 **Hardware Abstraction** - Works with internal and external temperature sensors
- ⚡ **High-Precision Reading** - Accurate temperature measurements with calibration
- 🔄 **Unit Conversion** - Celsius, Fahrenheit, and Kelvin support
- 📈 **Alert System** - Configurable temperature thresholds and alerts
- 🛡️ **Robust Error Handling** - Comprehensive validation and error reporting
- 🏎️ **Performance Optimized** - Minimal overhead for real-time applications
- 🔌 **Platform Agnostic** - Works across different MCU platforms

### 📊 **Supported Hardware**

| Implementation | Sensor Type | Range | Resolution | Accuracy |

|----------------|-------------|-------|------------|----------|

| `EspTemperature` | ESP32-C6 Internal | -40°C to +125°C | 0.1°C | ±2°C |

| `Ds18b20Temperature` | Digital OneWire | -55°C to +125°C | 0.0625°C | ±0.5°C |

| `Lm35Temperature` | Analog Linear | -55°C to +150°C | 0.1°C | ±1°C |

| `Ntc10kTemperature` | Analog Thermistor | -40°C to +125°C | 0.1°C | ±1°C |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BaseTemperature {
        <<abstract>>
        +EnsureInitialized() hf*temp*err*t
        +ReadTemperature(float&) hf*temp*err*t
        +ReadTemperatureF(float&) hf*temp*err*t
        +ReadTemperatureK(float&) hf*temp*err*t
        +SetAlertThreshold(float, hf*temp*alert*type*t) hf*temp*err*t
        +GetSensorInfo(hf*temp*sensor*info*t&) hf*temp*err*t
        +StartContinuousReading() hf*temp*err*t
        +StopContinuousReading() hf*temp*err*t
        +IsInitialized() bool
        +GetStatistics(hf*temp*statistics*t&) hf*temp*err*t
        #DoInitialize() hf*temp*err*t*
        #DoReadTemperature(float&) hf*temp*err*t*
        #DoSetAlert(float, hf*temp*alert*type*t) hf*temp*err*t*
    }

    class EspTemperature {
        +EspTemperature()
        +ReadRawTemperature(uint32*t&) hf*temp*err*t
        +CalibrateOffset(float) hf*temp*err*t
    }

    class Ds18b20Temperature {
        +Ds18b20Temperature(BaseGpio*)
        +SetResolution(hf*temp*resolution*t) hf*temp*err*t
        +GetDeviceAddress(uint64*t&) hf*temp*err*t
    }

    class Lm35Temperature {
        +Lm35Temperature(BaseAdc*, hf*channel*id*t)
        +SetSupplyVoltage(float) hf*temp*err*t
        +CalibrateLinear(float, float) hf*temp*err*t
    }

    BaseTemperature <|-- EspTemperature
    BaseTemperature <|-- Ds18b20Temperature
    BaseTemperature <|-- Lm35Temperature
```text

---

## 📋 **Error Codes**

### 🚨 **Temperature Error Enumeration**

```cpp
enum class hf*temp*err*t : hf*u32*t {
    // Success codes
    TEMP*SUCCESS = 0,
    
    // General errors
    TEMP*ERR*FAILURE = 1,
    TEMP*ERR*NOT*INITIALIZED = 2,
    TEMP*ERR*ALREADY*INITIALIZED = 3,
    TEMP*ERR*INVALID*PARAMETER = 4,
    TEMP*ERR*NULL*POINTER = 5,
    TEMP*ERR*OUT*OF*MEMORY = 6,
    
    // Sensor specific errors
    TEMP*ERR*SENSOR*NOT*AVAILABLE = 7,
    TEMP*ERR*SENSOR*BUSY = 8,
    TEMP*ERR*SENSOR*DISABLED = 9,
    TEMP*ERR*SENSOR*NOT*READY = 10,
    
    // Reading errors
    TEMP*ERR*READ*FAILED = 11,
    TEMP*ERR*READ*TIMEOUT = 12,
    TEMP*ERR*READ*CRC*ERROR = 13,
    TEMP*ERR*TEMPERATURE*OUT*OF*RANGE = 14,
    
    // Calibration errors
    TEMP*ERR*CALIBRATION*FAILED = 15,
    TEMP*ERR*CALIBRATION*INVALID = 16,
    TEMP*ERR*CALIBRATION*NOT*AVAILABLE = 17,
    
    // Alert errors
    TEMP*ERR*ALERT*NOT*SUPPORTED = 18,
    TEMP*ERR*ALERT*THRESHOLD*INVALID = 19,
    TEMP*ERR*ALERT*ALREADY*SET = 20,
    
    // Communication errors
    TEMP*ERR*COMMUNICATION*FAILURE = 21,
    TEMP*ERR*DEVICE*NOT*RESPONDING = 22,
    TEMP*ERR*BUS*ERROR = 23,
    
    // System errors
    TEMP*ERR*SYSTEM*ERROR = 24,
    TEMP*ERR*PERMISSION*DENIED = 25,
    TEMP*ERR*OPERATION*ABORTED = 26
};
```text

### 📊 **Error Code Categories**

| Category | Range | Description |

|----------|-------|-------------|

| **Success** | 0 | Successful operation |

| **General** | 1-6 | Basic initialization and parameter errors |

| **Sensor** | 7-10 | Sensor availability and status errors |

| **Reading** | 11-14 | Temperature measurement errors |

| **Calibration** | 15-17 | Calibration and accuracy errors |

| **Alert** | 18-20 | Temperature alert configuration errors |

| **Communication** | 21-23 | Sensor communication errors |

| **System** | 24-26 | System-level errors |

---

## 🔧 **Core API**

### 🎯 **Essential Methods**

#### **Initialization**
```cpp
/**
 * @brief Ensure the temperature sensor is initialized
 * @return hf*temp*err*t Error code
 */
virtual hf*temp*err*t EnsureInitialized() = 0;

/**
 * @brief Check if the temperature sensor is initialized
 * @return bool True if initialized
 */
virtual bool IsInitialized() const = 0;
```text

#### **Temperature Reading**
```cpp
/**
 * @brief Read temperature in Celsius
 * @param temperature*c Output temperature in degrees Celsius
 * @return hf*temp*err*t Error code
 */
virtual hf*temp*err*t ReadTemperature(float& temperature*c) = 0;

/**
 * @brief Read temperature in Fahrenheit
 * @param temperature*f Output temperature in degrees Fahrenheit
 * @return hf*temp*err*t Error code
 */
virtual hf*temp*err*t ReadTemperatureF(float& temperature*f) = 0;

/**
 * @brief Read temperature in Kelvin
 * @param temperature*k Output temperature in Kelvin
 * @return hf*temp*err*t Error code
 */
virtual hf*temp*err*t ReadTemperatureK(float& temperature*k) = 0;
```text

#### **Alert Management**
```cpp
/**
 * @brief Set temperature alert threshold
 * @param threshold*c Threshold temperature in Celsius
 * @param alert*type Type of alert (high/low/both)
 * @return hf*temp*err*t Error code
 */
virtual hf*temp*err*t SetAlertThreshold(float threshold*c, 
                                      hf*temp*alert*type*t alert*type) = 0;

/**
 * @brief Check if alert condition is active
 * @param alert*active Output alert status
 * @return hf*temp*err*t Error code
 */
virtual hf*temp*err*t IsAlertActive(bool& alert*active) = 0;
```text

#### **Continuous Monitoring**
```cpp
/**
 * @brief Start continuous temperature reading
 * @param interval*ms Reading interval in milliseconds
 * @return hf*temp*err*t Error code
 */
virtual hf*temp*err*t StartContinuousReading(hf*u32*t interval*ms = 1000) = 0;

/**
 * @brief Stop continuous temperature reading
 * @return hf*temp*err*t Error code
 */
virtual hf*temp*err*t StopContinuousReading() = 0;
```text

---

## 📊 **Data Structures**

### 🌡️ **Temperature Alert Types**

```cpp
enum class hf*temp*alert*type*t : hf*u8*t {
    TEMP*ALERT*NONE = 0,        ///< No alert
    TEMP*ALERT*HIGH = 1,        ///< High temperature alert
    TEMP*ALERT*LOW = 2,         ///< Low temperature alert
    TEMP*ALERT*BOTH = 3         ///< Both high and low alerts
};
```text

### 📊 **Sensor Information**

```cpp
struct hf*temp*sensor*info*t {
    hf*u32*t sensor*id;                    ///< Unique sensor identifier
    char sensor*name[32];                  ///< Sensor name string
    float min*temperature*c;               ///< Minimum measurable temperature
    float max*temperature*c;               ///< Maximum measurable temperature
    float resolution*c;                    ///< Temperature resolution
    float accuracy*c;                      ///< Temperature accuracy
    hf*u32*t response*time*ms;             ///< Sensor response time
    bool supports*alerts;                  ///< Alert capability
    bool supports*continuous;              ///< Continuous reading capability
};
```text

### 📈 **Temperature Statistics**

```cpp
struct hf*temp*statistics*t {
    hf*u32*t total*reads;                  ///< Total number of reads
    hf*u32*t successful*reads;             ///< Successful reads count
    hf*u32*t failed*reads;                 ///< Failed reads count
    float min*temperature*c;               ///< Minimum recorded temperature
    float max*temperature*c;               ///< Maximum recorded temperature
    float avg*temperature*c;               ///< Average temperature
    hf*u32*t last*read*time*ms;            ///< Last reading timestamp
    hf*u32*t total*alerts*triggered;       ///< Total alerts triggered
};
```text

---

## 🌡️ **Temperature Units**

### 🔄 **Unit Conversion Functions**

```cpp
/**
 * @brief Convert Celsius to Fahrenheit
 * @param celsius Temperature in Celsius
 * @return float Temperature in Fahrenheit
 */
static inline float CelsiusToFahrenheit(float celsius) {
    return (celsius * 9.0f / 5.0f) + 32.0f;
}

/**
 * @brief Convert Celsius to Kelvin
 * @param celsius Temperature in Celsius
 * @return float Temperature in Kelvin
 */
static inline float CelsiusToKelvin(float celsius) {
    return celsius + 273.15f;
}

/**
 * @brief Convert Fahrenheit to Celsius
 * @param fahrenheit Temperature in Fahrenheit
 * @return float Temperature in Celsius
 */
static inline float FahrenheitToCelsius(float fahrenheit) {
    return (fahrenheit - 32.0f) * 5.0f / 9.0f;
}
```text

---

## 📊 **Usage Examples**

### 🔥 **Basic Temperature Reading**

```cpp
#include "inc/mcu/esp32/EspTemperature.h"

class ThermalMonitor {
private:
    EspTemperature temp*sensor*;
    
public:
    bool initialize() {
        return temp*sensor*.EnsureInitialized() == hf*temp*err*t::TEMP*SUCCESS;
    }
    
    void read*temperature() {
        float temperature*c;
        
        if (temp*sensor*.ReadTemperature(temperature*c) == hf*temp*err*t::TEMP*SUCCESS) {
            printf("🌡️ Temperature: %.2f°C\n", temperature*c);
            
            // Convert to other units
            float temp*f = BaseTemperature::CelsiusToFahrenheit(temperature*c);
            float temp*k = BaseTemperature::CelsiusToKelvin(temperature*c);
            
            printf("   Fahrenheit: %.2f°F\n", temp*f);
            printf("   Kelvin: %.2f K\n", temp*k);
        } else {
            printf("❌ Failed to read temperature\n");
        }
    }
};
```text

### 🚨 **Temperature Alert System**

```cpp
#include "inc/external/Ds18b20Temperature.h"

class TemperatureAlertSystem {
private:
    Ds18b20Temperature temp*sensor*;
    bool alert*callback*registered*;
    
public:
    TemperatureAlertSystem(BaseGpio* one*wire*pin) 
        : temp*sensor*(one*wire*pin)
        , alert*callback*registered*(false) {}
    
    bool setup*thermal*protection() {
        // Initialize sensor
        if (temp*sensor*.EnsureInitialized() != hf*temp*err*t::TEMP*SUCCESS) {
            return false;
        }
        
        // Set high temperature alert at 85°C
        if (temp*sensor*.SetAlertThreshold(85.0f, 
                                         hf*temp*alert*type*t::TEMP*ALERT*HIGH) 
            != hf*temp*err*t::TEMP*SUCCESS) {
            return false;
        }
        
        // Set low temperature alert at -10°C
        if (temp*sensor*.SetAlertThreshold(-10.0f, 
                                         hf*temp*alert*type*t::TEMP*ALERT*LOW) 
            != hf*temp*err*t::TEMP*SUCCESS) {
            return false;
        }
        
        printf("🛡️ Thermal protection enabled (-10°C to 85°C)\n");
        return true;
    }
    
    void monitor*alerts() {
        bool alert*active;
        
        if (temp*sensor*.IsAlertActive(alert*active) == hf*temp*err*t::TEMP*SUCCESS) {
            if (alert*active) {
                float current*temp;
                temp*sensor*.ReadTemperature(current*temp);
                
                printf("🚨 TEMPERATURE ALERT: %.2f°C\n", current*temp);
                
                // Implement emergency response
                if (current*temp > 85.0f) {
                    printf("⚠️ OVERHEATING - Shutting down system\n");
                    emergency*shutdown();
                } else if (current*temp < -10.0f) {
                    printf("⚠️ FREEZING - Activating heater\n");
                    activate*heater();
                }
            }
        }
    }
    
private:
    void emergency*shutdown() {
        // Implement emergency shutdown logic
    }
    
    void activate*heater() {
        // Implement heater activation logic
    }
};
```text

### 📊 **Multi-Sensor Temperature Monitoring**

```cpp
#include "inc/external/Lm35Temperature.h"
#include "inc/external/Ntc10kTemperature.h"

class MultiSensorTempSystem {
private:
    EspTemperature internal*temp*;
    Lm35Temperature ambient*temp*;
    Ntc10kTemperature motor*temp*;
    
    struct TemperatureReading {
        float internal;
        float ambient;
        float motor;
        hf*u32*t timestamp;
    };
    
public:
    MultiSensorTempSystem(BaseAdc* adc) 
        : ambient*temp*(adc, ADC*CHANNEL*0)
        , motor*temp*(adc, ADC*CHANNEL*1) {}
    
    bool initialize() {
        bool success = true;
        
        success &= (internal*temp*.EnsureInitialized() == hf*temp*err*t::TEMP*SUCCESS);
        success &= (ambient*temp*.EnsureInitialized() == hf*temp*err*t::TEMP*SUCCESS);
        success &= (motor*temp*.EnsureInitialized() == hf*temp*err*t::TEMP*SUCCESS);
        
        if (success) {
            printf("🌡️ Multi-sensor temperature system initialized\n");
        }
        
        return success;
    }
    
    TemperatureReading read*all*temperatures() {
        TemperatureReading reading = {};
        reading.timestamp = esp*timer*get*time() / 1000; // Convert to ms
        
        // Read internal temperature
        if (internal*temp*.ReadTemperature(reading.internal) != hf*temp*err*t::TEMP*SUCCESS) {
            reading.internal = NAN;
        }
        
        // Read ambient temperature
        if (ambient*temp*.ReadTemperature(reading.ambient) != hf*temp*err*t::TEMP*SUCCESS) {
            reading.ambient = NAN;
        }
        
        // Read motor temperature
        if (motor*temp*.ReadTemperature(reading.motor) != hf*temp*err*t::TEMP*SUCCESS) {
            reading.motor = NAN;
        }
        
        return reading;
    }
    
    void log*temperature*data() {
        TemperatureReading reading = read*all*temperatures();
        
        printf("📊 Temperature Report [%lu ms]:\n", reading.timestamp);
        printf("   Internal: %.2f°C\n", reading.internal);
        printf("   Ambient:  %.2f°C\n", reading.ambient);
        printf("   Motor:    %.2f°C\n", reading.motor);
        
        // Check for thermal issues
        if (reading.motor > 80.0f) {
            printf("⚠️ Motor overheating detected!\n");
        }
        
        if (reading.internal > 70.0f) {
            printf("⚠️ MCU overheating detected!\n");
        }
    }
    
    void start*continuous*monitoring(hf*u32*t interval*ms = 5000) {
        // Start continuous reading on all sensors
        internal*temp*.StartContinuousReading(interval*ms);
        ambient*temp*.StartContinuousReading(interval*ms);
        motor*temp*.StartContinuousReading(interval*ms);
        
        printf("🔄 Continuous temperature monitoring started (%lu ms interval)\n", interval*ms);
    }
};
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Practices**

1. **🎯 Initialize Early**
   ```cpp
   // Initialize temperature sensors during system startup
   if (temp*sensor.EnsureInitialized() != hf*temp*err*t::TEMP*SUCCESS) {
       printf("❌ Temperature sensor initialization failed\n");
       // Handle initialization failure
   }
   ```

1. **🌡️ Use Appropriate Units**
   ```cpp
   // Be consistent with temperature units
   float temp*c;
   temp*sensor.ReadTemperature(temp*c);  // Always in Celsius
   
   // Convert when displaying to users
   float temp*f = BaseTemperature::CelsiusToFahrenheit(temp*c);
   printf("Temperature: %.1f°F\n", temp*f);
   ```

1. **🚨 Implement Thermal Protection**
   ```cpp
   // Set appropriate alert thresholds
   temp*sensor.SetAlertThreshold(85.0f, hf*temp*alert*type*t::TEMP*ALERT*HIGH);
   temp*sensor.SetAlertThreshold(-10.0f, hf*temp*alert*type*t::TEMP*ALERT*LOW);
   ```

1. **📊 Monitor Sensor Health**
   ```cpp
   // Regularly check sensor statistics
   hf*temp*statistics*t stats;
   if (temp*sensor.GetStatistics(stats) == hf*temp*err*t::TEMP*SUCCESS) {
       float success*rate = (float)stats.successful*reads / stats.total*reads;
       if (success*rate < 0.95f) {
           printf("⚠️ Temperature sensor reliability low: %.1f%%\n", success*rate * 100.0f);
       }
   }
   ```

### ❌ **Common Pitfalls**

1. **🚫 Not Checking Return Values**
   ```cpp
   // BAD: Ignoring error codes
   temp*sensor.ReadTemperature(temp);
   
   // GOOD: Always check return values
   if (temp*sensor.ReadTemperature(temp) != hf*temp*err*t::TEMP*SUCCESS) {
       // Handle error appropriately
   }
   ```

1. **🚫 Using Wrong Temperature Units**
   ```cpp
   // BAD: Mixing temperature units
   float temp*f;
   temp*sensor.ReadTemperature(temp*f);  // This returns Celsius!
   
   // GOOD: Use correct methods for units
   float temp*c, temp*f;
   temp*sensor.ReadTemperature(temp*c);    // Celsius
   temp*sensor.ReadTemperatureF(temp*f);   // Fahrenheit
   ```

1. **🚫 Ignoring Sensor Limitations**
   ```cpp
   // BAD: Not checking sensor range
   if (temperature > 100.0f) {
       // May be invalid for some sensors
   }
   
   // GOOD: Check sensor specifications
   hf*temp*sensor*info*t info;
   temp*sensor.GetSensorInfo(info);
   if (temperature > info.max*temperature*c) {
       printf("⚠️ Temperature exceeds sensor range\n");
   }
   ```

### 🎯 **Performance Tips**

1. **⚡ Use Continuous Reading for High-Frequency Monitoring**
   ```cpp
   // Start continuous reading for frequent updates
   temp*sensor.StartContinuousReading(100);  // 100ms interval
   ```

1. **🔄 Batch Multiple Sensor Reads**
   ```cpp
   // Read multiple sensors together for efficiency
   float temps[3];
   sensor1.ReadTemperature(temps[0]);
   sensor2.ReadTemperature(temps[1]);
   sensor3.ReadTemperature(temps[2]);
   ```

1. **📊 Use Statistics for Health Monitoring**
   ```cpp
   // Monitor sensor performance over time
   hf*temp*statistics*t stats;
   temp*sensor.GetStatistics(stats);
   ```

---

<div align="center">

**📋 Navigation**

[← Previous: BaseLogger](BaseLogger.md) | [Back to API Index](README.md) | [Next: BasePeriodicTimer
→](BasePeriodicTimer.md)

</div>

---

<div align="center">

**🌡️ Professional Temperature Monitoring for Critical Applications**

*Ensuring thermal safety and optimal performance across all operating conditions*

</div>