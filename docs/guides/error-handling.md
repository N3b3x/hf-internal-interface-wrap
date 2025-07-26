# 🛡️ Error Handling Guide

<div align="center">

![Error Handling](https://img.shields.io/badge/Error%20Handling-Best%20Practices-red?style=for-the-badge&logo=exclamationtriangle)

**🎯 Comprehensive guide to robust error handling in HardFOC applications**

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Error Code System**](#️-error-code-system)
- [✅ **Best Practices**](#-best-practices)
- [🔧 **Implementation Patterns**](#-implementation-patterns)
- [🔄 **Recovery Strategies**](#-recovery-strategies)
- [📊 **Monitoring and Logging**](#-monitoring-and-logging)
- [💡 **Common Scenarios**](#-common-scenarios)

---

## 🎯 **Overview**

Robust error handling is critical for reliable embedded systems, especially in motor control applications where failures can have physical consequences. The HardFOC Internal Interface Wrapper provides a comprehensive error handling system designed for embedded safety and reliability.

### ✨ **Key Principles**

- **🛡️ Fail-Safe Operation**: Always fail to a safe state
- **📊 Comprehensive Coverage**: Handle all possible error conditions
- **🔄 Graceful Recovery**: Implement recovery where possible
- **📝 Clear Reporting**: Provide actionable error information
- **⚡ Performance Conscious**: Minimal overhead in critical paths

### 🏆 **Benefits**

- **🔒 Safety**: Prevents dangerous operating conditions
- **📈 Reliability**: Reduces system crashes and hangs
- **🔍 Debuggability**: Clear error reporting aids troubleshooting
- **📊 Maintainability**: Consistent error handling patterns

---

## 🏗️ **Error Code System**

### 📋 **Hierarchical Structure**

The HardFOC error system uses a hierarchical approach:

```cpp
// Interface-specific error types
enum class hf_gpio_err_t : hf_u8_t {
    GPIO_SUCCESS = 0,                    // Always 0 for success
    GPIO_ERR_FAILURE = 1,                // General errors start at 1
    GPIO_ERR_INVALID_PIN = 11,           // Specific errors start at 11
    // ...
};

enum class hf_adc_err_t : hf_u8_t {
    ADC_SUCCESS = 0,
    ADC_ERR_FAILURE = 1,
    ADC_ERR_INVALID_CHANNEL = 11,
    // ...
};
```

### 🎯 **Error Categories**

| Category | Range | Purpose | Examples |
|----------|-------|---------|----------|
| **Success** | 0 | Successful operation | `GPIO_SUCCESS`, `ADC_SUCCESS` |
| **General** | 1-10 | Common errors | `NOT_INITIALIZED`, `TIMEOUT` |
| **Specific** | 11+ | Interface-specific | `INVALID_PIN`, `NO_ACK` |

### 🔧 **Utility Functions**

```cpp
// Generic success/error checking
template<typename ErrorType>
bool IsSuccess(ErrorType error) {
    return static_cast<hf_u8_t>(error) == 0;
}

template<typename ErrorType>
bool IsError(ErrorType error) {
    return static_cast<hf_u8_t>(error) != 0;
}

// Usage examples
if (IsSuccess(gpio_result)) { /* success path */ }
if (IsError(adc_result)) { /* error handling */ }
```

---

## ✅ **Best Practices**

### 1. **Always Check Return Values**

```cpp
// ✅ Good - Always check error codes
hf_gpio_err_t result = gpio.SetHigh();
if (IsError(result)) {
    printf("GPIO error: %d\n", static_cast<int>(result));
    return false;  // Propagate error
}

// ❌ Bad - Ignoring return values
gpio.SetHigh();  // What if it fails?
```

### 2. **Use Specific Error Handling**

```cpp
// ✅ Good - Handle specific cases
hf_i2c_err_t result = i2c.Write(address, data, size);
switch (result) {
    case hf_i2c_err_t::I2C_SUCCESS:
        printf("✅ I2C write successful\n");
        break;
    case hf_i2c_err_t::I2C_ERR_NO_ACK:
        printf("⚠️ Device not responding - check connections\n");
        return RetryOperation();
    case hf_i2c_err_t::I2C_ERR_TIMEOUT:
        printf("⏰ I2C timeout - reducing bus speed\n");
        return ReduceBusSpeed();
    default:
        printf("❌ I2C error: %d\n", static_cast<int>(result));
        return false;
}

// ❌ Bad - Generic error handling
if (IsError(result)) {
    printf("Error occurred\n");  // Not helpful!
    return false;
}
```

### 3. **Implement Error Propagation**

```cpp
// ✅ Good - Propagate errors up the call stack
bool InitializeHardware() {
    // Initialize GPIO
    hf_gpio_err_t gpio_result = gpio_.Initialize();
    if (IsError(gpio_result)) {
        printf("❌ GPIO initialization failed: %d\n", 
               static_cast<int>(gpio_result));
        return false;
    }
    
    // Initialize ADC
    hf_adc_err_t adc_result = adc_.Initialize();
    if (IsError(adc_result)) {
        printf("❌ ADC initialization failed: %d\n", 
               static_cast<int>(adc_result));
        return false;
    }
    
    return true;
}
```

### 4. **Use RAII for Resource Management**

```cpp
// ✅ Good - RAII ensures cleanup on errors
class SafeGpioOperation {
private:
    BaseGpio& gpio_;
    bool was_high_;
    bool initialized_;

public:
    SafeGpioOperation(BaseGpio& gpio) 
        : gpio_(gpio), was_high_(false), initialized_(false) {
        // Save current state
        was_high_ = gpio_.Read();
        
        // Set to safe state
        if (IsSuccess(gpio_.SetLow())) {
            initialized_ = true;
        }
    }
    
    ~SafeGpioOperation() {
        if (initialized_) {
            // Restore original state
            if (was_high_) {
                gpio_.SetHigh();
            }
        }
    }
    
    bool IsValid() const { return initialized_; }
};

// Usage
{
    SafeGpioOperation safe_op(motor_enable_pin);
    if (!safe_op.IsValid()) {
        return false;
    }
    
    // Perform operations - GPIO will be restored on scope exit
    // even if an exception occurs or early return happens
}
```

---

## 🔧 **Implementation Patterns**

### 🎯 **Pattern 1: Early Return on Error**

```cpp
bool ConfigureMotorSystem() {
    // Configure GPIO
    hf_gpio_err_t gpio_result = motor_enable_.SetAsOutput();
    if (IsError(gpio_result)) {
        return false;
    }
    
    // Configure PWM
    hf_pwm_err_t pwm_result = motor_pwm_.SetFrequency(20000);
    if (IsError(pwm_result)) {
        return false;
    }
    
    // Configure ADC
    hf_adc_err_t adc_result = current_sensor_.ConfigureChannel(0);
    if (IsError(adc_result)) {
        return false;
    }
    
    return true;
}
```

### 🔄 **Pattern 2: Error Accumulation**

```cpp
struct InitResult {
    bool success;
    std::vector<std::string> errors;
};

InitResult InitializeAllSensors() {
    InitResult result = {true, {}};
    
    // Try to initialize all sensors, collect errors
    hf_adc_err_t adc_result = voltage_sensor_.Initialize();
    if (IsError(adc_result)) {
        result.success = false;
        result.errors.push_back("Voltage sensor failed");
    }
    
    hf_i2c_err_t temp_result = temperature_sensor_.Initialize();
    if (IsError(temp_result)) {
        result.success = false;
        result.errors.push_back("Temperature sensor failed");
    }
    
    hf_spi_err_t encoder_result = encoder_.Initialize();
    if (IsError(encoder_result)) {
        result.success = false;
        result.errors.push_back("Encoder failed");
    }
    
    return result;
}
```

### 🛡️ **Pattern 3: Error Result Type**

```cpp
template<typename T>
struct Result {
    bool is_success;
    T value;
    std::string error_message;
    
    static Result Success(const T& val) {
        return {true, val, ""};
    }
    
    static Result Error(const std::string& msg) {
        return {false, T{}, msg};
    }
    
    bool IsSuccess() const { return is_success; }
    bool IsError() const { return !is_success; }
};

Result<float> ReadMotorCurrent() {
    float current;
    hf_adc_err_t result = current_adc_.ReadChannelV(0, current);
    
    if (IsError(result)) {
        return Result<float>::Error("Failed to read current sensor");
    }
    
    // Validate range
    if (current < 0.0f || current > 10.0f) {
        return Result<float>::Error("Current reading out of range");
    }
    
    return Result<float>::Success(current);
}

// Usage
auto current_result = ReadMotorCurrent();
if (current_result.IsSuccess()) {
    float current = current_result.value;
    // Use current value
} else {
    printf("Error: %s\n", current_result.error_message.c_str());
}
```

---

## 🔄 **Recovery Strategies**

### 1. **Retry with Backoff**

```cpp
bool WriteWithRetry(BaseI2c& i2c, hf_u8_t address, 
                   const hf_u8_t* data, size_t size) {
    constexpr int MAX_RETRIES = 3;
    constexpr int BASE_DELAY_MS = 10;
    
    for (int retry = 0; retry < MAX_RETRIES; ++retry) {
        hf_i2c_err_t result = i2c.Write(address, data, size);
        
        if (IsSuccess(result)) {
            return true;
        }
        
        // Check if error is recoverable
        if (result == hf_i2c_err_t::I2C_ERR_NO_ACK ||
            result == hf_i2c_err_t::I2C_ERR_TIMEOUT) {
            
            printf("⚠️ I2C retry %d/%d\n", retry + 1, MAX_RETRIES);
            
            // Exponential backoff
            int delay_ms = BASE_DELAY_MS * (1 << retry);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
            continue;
        }
        
        // Non-recoverable error
        printf("❌ Fatal I2C error: %d\n", static_cast<int>(result));
        break;
    }
    
    return false;
}
```

### 2. **Fallback Mechanisms**

```cpp
float ReadVoltageWithFallback() {
    // Try primary ADC
    float voltage;
    hf_adc_err_t result = primary_adc_.ReadChannelV(0, voltage);
    if (IsSuccess(result)) {
        return voltage;
    }
    
    printf("⚠️ Primary ADC failed, trying backup\n");
    
    // Try backup ADC
    result = backup_adc_.ReadChannelV(0, voltage);
    if (IsSuccess(result)) {
        return voltage;
    }
    
    printf("❌ Both ADCs failed, using safe default\n");
    return 0.0f;  // Safe default value
}
```

### 3. **Graceful Degradation**

```cpp
struct SystemStatus {
    bool primary_sensor_ok;
    bool backup_sensor_ok;
    bool can_operate;
    std::string status_message;
};

SystemStatus AssessSystemHealth() {
    SystemStatus status = {};
    
    // Check primary sensor
    float primary_reading;
    status.primary_sensor_ok = IsSuccess(
        primary_sensor_.ReadChannelV(0, primary_reading)
    );
    
    // Check backup sensor
    float backup_reading;
    status.backup_sensor_ok = IsSuccess(
        backup_sensor_.ReadChannelV(0, backup_reading)
    );
    
    // Determine operating capability
    if (status.primary_sensor_ok && status.backup_sensor_ok) {
        status.can_operate = true;
        status.status_message = "All systems operational";
    } else if (status.primary_sensor_ok || status.backup_sensor_ok) {
        status.can_operate = true;
        status.status_message = "Operating with degraded performance";
    } else {
        status.can_operate = false;
        status.status_message = "Critical sensor failure - shutdown required";
    }
    
    return status;
}
```

---

## 📊 **Monitoring and Logging**

### 📝 **Structured Error Logging**

```cpp
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class ErrorLogger {
private:
    static constexpr size_t MAX_LOG_ENTRIES = 100;
    
    struct LogEntry {
        LogLevel level;
        uint32_t timestamp;
        std::string component;
        int error_code;
        std::string message;
    };
    
    std::array<LogEntry, MAX_LOG_ENTRIES> log_buffer_;
    size_t write_index_;
    
public:
    void Log(LogLevel level, const std::string& component,
             int error_code, const std::string& message) {
        
        LogEntry& entry = log_buffer_[write_index_];
        entry.level = level;
        entry.timestamp = GetCurrentTimeMs();
        entry.component = component;
        entry.error_code = error_code;
        entry.message = message;
        
        write_index_ = (write_index_ + 1) % MAX_LOG_ENTRIES;
        
        // Print critical errors immediately
        if (level >= LogLevel::ERROR) {
            printf("[%s] %s: %s (code: %d)\n",
                   LevelToString(level), component.c_str(),
                   message.c_str(), error_code);
        }
    }
    
    template<typename ErrorType>
    void LogError(const std::string& component, ErrorType error,
                  const std::string& context) {
        if (IsError(error)) {
            Log(LogLevel::ERROR, component, 
                static_cast<int>(error), context);
        }
    }
};

// Global logger instance
ErrorLogger g_error_logger;

// Usage macros
#define LOG_GPIO_ERROR(error, context) \
    g_error_logger.LogError("GPIO", error, context)

#define LOG_ADC_ERROR(error, context) \
    g_error_logger.LogError("ADC", error, context)
```

### 📊 **Error Statistics**

```cpp
class ErrorStatistics {
private:
    std::map<int, uint32_t> error_counts_;
    uint32_t total_operations_;
    uint32_t total_errors_;
    
public:
    template<typename ErrorType>
    void RecordOperation(ErrorType result) {
        total_operations_++;
        
        if (IsError(result)) {
            total_errors_++;
            error_counts_[static_cast<int>(result)]++;
        }
    }
    
    double GetErrorRate() const {
        return total_operations_ > 0 ? 
               static_cast<double>(total_errors_) / total_operations_ : 0.0;
    }
    
    void PrintStatistics() const {
        printf("=== Error Statistics ===\n");
        printf("Total operations: %u\n", total_operations_);
        printf("Total errors: %u\n", total_errors_);
        printf("Error rate: %.2f%%\n", GetErrorRate() * 100.0);
        
        printf("Error breakdown:\n");
        for (const auto& pair : error_counts_) {
            printf("  Error %d: %u occurrences\n", pair.first, pair.second);
        }
    }
};
```

---

## 💡 **Common Scenarios**

### 🔌 **Motor Control Error Handling**

```cpp
class MotorController {
private:
    BaseGpio& enable_pin_;
    BasePwm& speed_pwm_;
    BaseAdc& current_sensor_;
    bool emergency_stop_;
    
public:
    bool SetMotorSpeed(float speed_percent) {
        // Validate input
        if (speed_percent < 0.0f || speed_percent > 100.0f) {
            LOG_ERROR("Invalid speed: %.2f%%", speed_percent);
            return false;
        }
        
        // Check emergency stop
        if (emergency_stop_) {
            LOG_WARNING("Motor disabled due to emergency stop");
            return false;
        }
        
        // Read current before changing speed
        float current;
        hf_adc_err_t adc_result = current_sensor_.ReadChannelV(0, current);
        if (IsError(adc_result)) {
            LOG_ADC_ERROR(adc_result, "Failed to read motor current");
            EmergencyStop();
            return false;
        }
        
        // Check overcurrent
        if (current > MAX_MOTOR_CURRENT) {
            LOG_ERROR("Overcurrent detected: %.2f A", current);
            EmergencyStop();
            return false;
        }
        
        // Set PWM duty cycle
        hf_u32_t duty_cycle = static_cast<hf_u32_t>(speed_percent * 10.0f);
        hf_pwm_err_t pwm_result = speed_pwm_.SetDutyCycle(duty_cycle);
        if (IsError(pwm_result)) {
            LOG_PWM_ERROR(pwm_result, "Failed to set motor PWM");
            EmergencyStop();
            return false;
        }
        
        return true;
    }
    
    void EmergencyStop() {
        emergency_stop_ = true;
        
        // Set motor to safe state
        enable_pin_.SetLow();      // Disable motor
        speed_pwm_.SetDutyCycle(0); // Zero speed
        
        LOG_CRITICAL("Emergency stop activated");
    }
};
```

### 🌡️ **Sensor Validation**

```cpp
struct SensorReading {
    bool valid;
    float value;
    std::string error_message;
};

SensorReading ReadTemperatureSensor() {
    const float MIN_TEMP = -40.0f;
    const float MAX_TEMP = 125.0f;
    
    float temperature;
    hf_adc_err_t result = temp_adc_.ReadChannelV(0, temperature);
    
    if (IsError(result)) {
        return {false, 0.0f, "ADC read failed"};
    }
    
    // Convert voltage to temperature (example: linear sensor)
    temperature = (temperature - 0.5f) * 100.0f;  // LM35 formula
    
    // Validate range
    if (temperature < MIN_TEMP || temperature > MAX_TEMP) {
        return {false, temperature, "Temperature out of range"};
    }
    
    return {true, temperature, ""};
}

void MonitorTemperature() {
    auto reading = ReadTemperatureSensor();
    
    if (!reading.valid) {
        printf("❌ Temperature sensor error: %s\n", 
               reading.error_message.c_str());
        
        // Take safety action
        if (reading.value > MAX_SAFE_TEMP) {
            EmergencyShutdown();
        }
        return;
    }
    
    printf("🌡️ Temperature: %.1f°C\n", reading.value);
    
    // Check thermal limits
    if (reading.value > WARNING_TEMP) {
        printf("⚠️ High temperature warning\n");
        ReducePerformance();
    }
}
```

---

<div align="center">

**🛡️ Robust error handling is the foundation of reliable embedded systems**

*Implement these patterns consistently to build resilient motor control applications*

</div>