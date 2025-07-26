# 🚨 Error Codes API Reference

<div align="center">

![Error Codes](https://img.shields.io/badge/Error%20Codes-Comprehensive%20System-red?style=for-the-badge&logo=exclamationtriangle)

**🛡️ Robust error handling system for all hardware interfaces**

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Error Code Structure**](#️-error-code-structure)
- [🔌 **GPIO Error Codes**](#-gpio-error-codes)
- [📊 **ADC Error Codes**](#-adc-error-codes)
- [🔄 **I2C Error Codes**](#-i2c-error-codes)
- [⚡ **SPI Error Codes**](#-spi-error-codes)
- [📡 **UART Error Codes**](#-uart-error-codes)
- [🚗 **CAN Error Codes**](#-can-error-codes)
- [🎛️ **PWM Error Codes**](#️-pwm-error-codes)
- [📻 **PIO Error Codes**](#-pio-error-codes)
- [💾 **NVS Error Codes**](#-nvs-error-codes)
- [⏰ **Timer Error Codes**](#-timer-error-codes)
- [💡 **Error Handling Best Practices**](#-error-handling-best-practices)

---

## 🎯 **Overview**

The HardFOC Internal Interface Wrapper implements a comprehensive error code system that provides detailed error information for all hardware operations. Each interface has its own error enumeration with specific error codes tailored to that interface's operations.

### ✨ **Key Features**

- **🎯 Interface-Specific**: Each hardware interface has dedicated error codes
- **📊 Comprehensive Coverage**: All possible error conditions are represented
- **🔄 Consistent Structure**: Similar naming and organization across interfaces
- **🛡️ Type Safety**: Strongly-typed enumerations prevent mixing error types
- **📝 Clear Documentation**: Each error code has clear meaning and resolution guidance

### 🏗️ **Design Principles**

- **Zero means success**: All interfaces use 0 for successful operations
- **Positive error codes**: All errors use positive integer values
- **Logical grouping**: Related errors are grouped together
- **Clear naming**: Error names clearly indicate the problem

---

## 🏗️ **Error Code Structure**

### 📋 **Common Pattern**

All error enumerations follow this consistent pattern:

```cpp
enum class hf_interface_err_t : hf_u8_t {
    INTERFACE_SUCCESS = 0,              // Success (always 0)
    
    // General errors (1-10)
    INTERFACE_ERR_FAILURE = 1,          // General failure
    INTERFACE_ERR_NOT_INITIALIZED = 2,   // Not initialized
    INTERFACE_ERR_INVALID_PARAMETER = 3, // Invalid parameter
    INTERFACE_ERR_TIMEOUT = 4,          // Operation timeout
    INTERFACE_ERR_BUSY = 5,             // Resource busy
    
    // Interface-specific errors (11+)
    INTERFACE_ERR_SPECIFIC_1 = 11,      // First specific error
    INTERFACE_ERR_SPECIFIC_2 = 12,      // Second specific error
    // ... more specific errors
};
```

### 🔧 **Utility Functions**

```cpp
/**
 * @brief Check if an operation was successful
 * @param error Error code to check
 * @return true if successful, false otherwise
 */
template<typename ErrorType>
inline bool IsSuccess(ErrorType error) {
    return static_cast<hf_u8_t>(error) == 0;
}

/**
 * @brief Check if an operation failed
 * @param error Error code to check
 * @return true if failed, false otherwise
 */
template<typename ErrorType>
inline bool IsError(ErrorType error) {
    return static_cast<hf_u8_t>(error) != 0;
}
```

---

## 🔌 **GPIO Error Codes**

### 📋 **hf_gpio_err_t Enumeration**

```cpp
enum class hf_gpio_err_t : hf_u8_t {
    GPIO_SUCCESS = 0,                    ///< ✅ Operation successful
    
    // General GPIO errors (1-10)
    GPIO_ERR_FAILURE = 1,                ///< ❌ General GPIO failure
    GPIO_ERR_NOT_INITIALIZED = 2,        ///< ⚠️ GPIO not initialized
    GPIO_ERR_INVALID_PARAMETER = 3,      ///< 🚫 Invalid parameter
    GPIO_ERR_TIMEOUT = 4,                ///< ⏰ Operation timeout
    GPIO_ERR_BUSY = 5,                   ///< 🔄 GPIO resource busy
    
    // GPIO-specific errors (11-20)
    GPIO_ERR_INVALID_PIN = 11,           ///< 📍 Invalid pin number
    GPIO_ERR_PIN_NOT_CONFIGURED = 12,    ///< ⚙️ Pin not configured
    GPIO_ERR_DIRECTION_MISMATCH = 13,    ///< 🔄 Wrong direction for operation
    GPIO_ERR_INTERRUPT_FAILED = 14,      ///< ⚡ Interrupt setup failed
    GPIO_ERR_PULL_RESISTOR_FAILED = 15,  ///< 🔧 Pull resistor config failed
    GPIO_ERR_HARDWARE_FAILURE = 16,      ///< 🔧 Hardware failure
    GPIO_ERR_PIN_RESERVED = 17,          ///< 🚫 Pin reserved by system
    GPIO_ERR_INVALID_MODE = 18,          ///< ⚙️ Invalid GPIO mode
};
```

### 🎯 **Usage Examples**

```cpp
#include "mcu/esp32/EspGpio.h"

EspGpio led_pin(2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);

// Check initialization
if (led_pin.Initialize()) {
    // Set pin high and check for errors
    hf_gpio_err_t result = led_pin.SetHigh();
    if (IsSuccess(result)) {
        printf("✅ LED turned on successfully\n");
    } else {
        printf("❌ Failed to turn on LED: %d\n", static_cast<int>(result));
    }
} else {
    printf("⚠️ Failed to initialize GPIO pin\n");
}
```

---

## 📊 **ADC Error Codes**

### 📋 **hf_adc_err_t Enumeration**

```cpp
enum class hf_adc_err_t : hf_u8_t {
    ADC_SUCCESS = 0,                     ///< ✅ Operation successful
    
    // General ADC errors (1-10)
    ADC_ERR_FAILURE = 1,                 ///< ❌ General ADC failure
    ADC_ERR_NOT_INITIALIZED = 2,         ///< ⚠️ ADC not initialized
    ADC_ERR_INVALID_PARAMETER = 3,       ///< 🚫 Invalid parameter
    ADC_ERR_TIMEOUT = 4,                 ///< ⏰ Operation timeout
    ADC_ERR_BUSY = 5,                    ///< 🔄 ADC resource busy
    
    // ADC-specific errors (11-25)
    ADC_ERR_INVALID_CHANNEL = 11,        ///< 📍 Invalid channel number
    ADC_ERR_CHANNEL_NOT_CONFIGURED = 12, ///< ⚙️ Channel not configured
    ADC_ERR_CALIBRATION_FAILED = 13,     ///< 📏 Calibration failed
    ADC_ERR_CONVERSION_FAILED = 14,      ///< 🔄 ADC conversion failed
    ADC_ERR_OUT_OF_RANGE = 15,          ///< 📊 Value out of range
    ADC_ERR_UNIT_NOT_AVAILABLE = 16,     ///< 🏭 ADC unit not available
    ADC_ERR_ATTENUATION_INVALID = 17,    ///< ⚡ Invalid attenuation setting
    ADC_ERR_CONTINUOUS_MODE_FAILED = 18, ///< 🔄 Continuous mode error
    ADC_ERR_DMA_ERROR = 19,              ///< 💾 DMA operation error
    ADC_ERR_HARDWARE_FAILURE = 20,       ///< 🔧 Hardware failure
};
```

### 🎯 **Usage Examples**

```cpp
#include "mcu/esp32/EspAdc.h"

EspAdc adc(ADC_UNIT_1, ADC_ATTEN_DB_11);

float voltage;
hf_adc_err_t result = adc.ReadChannelV(0, voltage);

switch (result) {
    case hf_adc_err_t::ADC_SUCCESS:
        printf("✅ Voltage: %.3f V\n", voltage);
        break;
    case hf_adc_err_t::ADC_ERR_INVALID_CHANNEL:
        printf("❌ Invalid ADC channel\n");
        break;
    case hf_adc_err_t::ADC_ERR_NOT_INITIALIZED:
        printf("⚠️ ADC not initialized\n");
        break;
    default:
        printf("❌ ADC error: %d\n", static_cast<int>(result));
        break;
}
```

---

## 🔄 **I2C Error Codes**

### 📋 **hf_i2c_err_t Enumeration**

```cpp
enum class hf_i2c_err_t : hf_u8_t {
    I2C_SUCCESS = 0,                     ///< ✅ Operation successful
    
    // General I2C errors (1-10)
    I2C_ERR_FAILURE = 1,                 ///< ❌ General I2C failure
    I2C_ERR_NOT_INITIALIZED = 2,         ///< ⚠️ I2C not initialized
    I2C_ERR_INVALID_PARAMETER = 3,       ///< 🚫 Invalid parameter
    I2C_ERR_TIMEOUT = 4,                 ///< ⏰ Operation timeout
    I2C_ERR_BUSY = 5,                    ///< 🔄 I2C bus busy
    
    // I2C-specific errors (11-25)
    I2C_ERR_INVALID_PORT = 11,           ///< 📍 Invalid I2C port
    I2C_ERR_INVALID_ADDRESS = 12,        ///< 📍 Invalid device address
    I2C_ERR_NO_ACK = 13,                 ///< 📡 No acknowledgment from device
    I2C_ERR_ARBITRATION_LOST = 14,       ///< 🔄 Bus arbitration lost
    I2C_ERR_BUS_ERROR = 15,              ///< 🚌 Bus error detected
    I2C_ERR_INVALID_FREQUENCY = 16,      ///< ⚡ Invalid frequency setting
    I2C_ERR_SDA_SCL_ERROR = 17,          ///< 📡 SDA/SCL line error
    I2C_ERR_DEVICE_NOT_FOUND = 18,       ///< 🔍 Device not found on bus
    I2C_ERR_BUFFER_TOO_SMALL = 19,       ///< 📦 Buffer too small
    I2C_ERR_HARDWARE_FAILURE = 20,       ///< 🔧 Hardware failure
};
```

---

## ⚡ **SPI Error Codes**

### 📋 **hf_spi_err_t Enumeration**

```cpp
enum class hf_spi_err_t : hf_u8_t {
    SPI_SUCCESS = 0,                     ///< ✅ Operation successful
    
    // General SPI errors (1-10)
    SPI_ERR_FAILURE = 1,                 ///< ❌ General SPI failure
    SPI_ERR_NOT_INITIALIZED = 2,         ///< ⚠️ SPI not initialized
    SPI_ERR_INVALID_PARAMETER = 3,       ///< 🚫 Invalid parameter
    SPI_ERR_TIMEOUT = 4,                 ///< ⏰ Operation timeout
    SPI_ERR_BUSY = 5,                    ///< 🔄 SPI resource busy
    
    // SPI-specific errors (11-25)
    SPI_ERR_INVALID_HOST = 11,           ///< 📍 Invalid SPI host
    SPI_ERR_INVALID_MODE = 12,           ///< ⚙️ Invalid SPI mode
    SPI_ERR_INVALID_FREQUENCY = 13,      ///< ⚡ Invalid frequency
    SPI_ERR_TRANSFER_FAILED = 14,        ///< 🔄 Transfer failed
    SPI_ERR_CS_ERROR = 15,               ///< 📍 Chip select error
    SPI_ERR_DMA_ERROR = 16,              ///< 💾 DMA operation error
    SPI_ERR_BUFFER_NULL = 17,            ///< 📦 Null buffer pointer
    SPI_ERR_INVALID_LENGTH = 18,         ///< 📏 Invalid transfer length
    SPI_ERR_CLOCK_ERROR = 19,            ///< ⏰ Clock configuration error
    SPI_ERR_HARDWARE_FAILURE = 20,       ///< 🔧 Hardware failure
};
```

---

## 📡 **UART Error Codes**

### 📋 **hf_uart_err_t Enumeration**

```cpp
enum class hf_uart_err_t : hf_u8_t {
    UART_SUCCESS = 0,                    ///< ✅ Operation successful
    
    // General UART errors (1-10)
    UART_ERR_FAILURE = 1,                ///< ❌ General UART failure
    UART_ERR_NOT_INITIALIZED = 2,        ///< ⚠️ UART not initialized
    UART_ERR_INVALID_PARAMETER = 3,      ///< 🚫 Invalid parameter
    UART_ERR_TIMEOUT = 4,                ///< ⏰ Operation timeout
    UART_ERR_BUSY = 5,                   ///< 🔄 UART resource busy
    
    // UART-specific errors (11-25)
    UART_ERR_INVALID_PORT = 11,          ///< 📍 Invalid UART port
    UART_ERR_INVALID_BAUDRATE = 12,      ///< ⚡ Invalid baud rate
    UART_ERR_FRAME_ERROR = 13,           ///< 📦 Frame error
    UART_ERR_PARITY_ERROR = 14,          ///< 🔍 Parity error
    UART_ERR_OVERRUN_ERROR = 15,         ///< 📊 Data overrun
    UART_ERR_BREAK_ERROR = 16,           ///< 🔌 Break condition
    UART_ERR_BUFFER_FULL = 17,           ///< 📦 Buffer full
    UART_ERR_BUFFER_EMPTY = 18,          ///< 📦 Buffer empty
    UART_ERR_FLOW_CONTROL_ERROR = 19,    ///< 🔄 Flow control error
    UART_ERR_HARDWARE_FAILURE = 20,      ///< 🔧 Hardware failure
};
```

---

## 🚗 **CAN Error Codes**

### 📋 **hf_can_err_t Enumeration**

```cpp
enum class hf_can_err_t : hf_u8_t {
    CAN_SUCCESS = 0,                     ///< ✅ Operation successful
    
    // General CAN errors (1-10)
    CAN_ERR_FAILURE = 1,                 ///< ❌ General CAN failure
    CAN_ERR_NOT_INITIALIZED = 2,         ///< ⚠️ CAN not initialized
    CAN_ERR_INVALID_PARAMETER = 3,       ///< 🚫 Invalid parameter
    CAN_ERR_TIMEOUT = 4,                 ///< ⏰ Operation timeout
    CAN_ERR_BUSY = 5,                    ///< 🔄 CAN resource busy
    
    // CAN-specific errors (11-30)
    CAN_ERR_INVALID_ID = 11,             ///< 📍 Invalid message ID
    CAN_ERR_INVALID_DLC = 12,            ///< 📏 Invalid data length
    CAN_ERR_BUS_OFF = 13,                ///< 🚌 Bus off condition
    CAN_ERR_ERROR_PASSIVE = 14,          ///< ⚠️ Error passive state
    CAN_ERR_ERROR_WARNING = 15,          ///< ⚠️ Error warning state
    CAN_ERR_ARBITRATION_LOST = 16,       ///< 🔄 Arbitration lost
    CAN_ERR_BIT_ERROR = 17,              ///< 🔌 Bit error
    CAN_ERR_STUFF_ERROR = 18,            ///< 📦 Bit stuffing error
    CAN_ERR_CRC_ERROR = 19,              ///< 🔍 CRC error
    CAN_ERR_FORM_ERROR = 20,             ///< 📋 Form error
    CAN_ERR_ACK_ERROR = 21,              ///< 📡 Acknowledgment error
    CAN_ERR_TX_QUEUE_FULL = 22,          ///< 📤 TX queue full
    CAN_ERR_RX_QUEUE_EMPTY = 23,         ///< 📥 RX queue empty
    CAN_ERR_FILTER_CONFIG = 24,          ///< 🔍 Filter configuration error
    CAN_ERR_HARDWARE_FAILURE = 25,       ///< 🔧 Hardware failure
};
```

---

## 🎛️ **PWM Error Codes**

### 📋 **hf_pwm_err_t Enumeration**

```cpp
enum class hf_pwm_err_t : hf_u8_t {
    PWM_SUCCESS = 0,                     ///< ✅ Operation successful
    
    // General PWM errors (1-10)
    PWM_ERR_FAILURE = 1,                 ///< ❌ General PWM failure
    PWM_ERR_NOT_INITIALIZED = 2,         ///< ⚠️ PWM not initialized
    PWM_ERR_INVALID_PARAMETER = 3,       ///< 🚫 Invalid parameter
    PWM_ERR_TIMEOUT = 4,                 ///< ⏰ Operation timeout
    PWM_ERR_BUSY = 5,                    ///< 🔄 PWM resource busy
    
    // PWM-specific errors (11-25)
    PWM_ERR_INVALID_CHANNEL = 11,        ///< 📍 Invalid PWM channel
    PWM_ERR_INVALID_FREQUENCY = 12,      ///< ⚡ Invalid frequency
    PWM_ERR_INVALID_DUTY_CYCLE = 13,     ///< 📊 Invalid duty cycle
    PWM_ERR_CHANNEL_NOT_CONFIGURED = 14, ///< ⚙️ Channel not configured
    PWM_ERR_FREQUENCY_CONFLICT = 15,     ///< ⚡ Frequency conflict
    PWM_ERR_TIMER_ERROR = 16,            ///< ⏰ Timer error
    PWM_ERR_PIN_ERROR = 17,              ///< 📍 Pin configuration error
    PWM_ERR_FADE_ERROR = 18,             ///< 🌅 Fade operation error
    PWM_ERR_INTERRUPT_ERROR = 19,        ///< ⚡ Interrupt error
    PWM_ERR_HARDWARE_FAILURE = 20,       ///< 🔧 Hardware failure
};
```

---

## 📻 **PIO Error Codes**

### 📋 **hf_pio_err_t Enumeration**

```cpp
enum class hf_pio_err_t : hf_u8_t {
    PIO_SUCCESS = 0,                     ///< ✅ Operation successful
    
    // General PIO errors (1-10)
    PIO_ERR_FAILURE = 1,                 ///< ❌ General PIO failure
    PIO_ERR_NOT_INITIALIZED = 2,         ///< ⚠️ PIO not initialized
    PIO_ERR_INVALID_PARAMETER = 3,       ///< 🚫 Invalid parameter
    PIO_ERR_TIMEOUT = 4,                 ///< ⏰ Operation timeout
    PIO_ERR_BUSY = 5,                    ///< 🔄 PIO resource busy
    
    // PIO-specific errors (11-25)
    PIO_ERR_INVALID_CHANNEL = 11,        ///< 📍 Invalid PIO channel
    PIO_ERR_PROGRAM_LOAD_FAILED = 12,    ///< 📋 Program load failed
    PIO_ERR_ENCODING_ERROR = 13,         ///< 🔄 Encoding error
    PIO_ERR_TX_QUEUE_FULL = 14,          ///< 📤 TX queue full
    PIO_ERR_RX_QUEUE_EMPTY = 15,         ///< 📥 RX queue empty
    PIO_ERR_SYMBOL_ERROR = 16,           ///< 🔤 Symbol error
    PIO_ERR_TIMING_ERROR = 17,           ///< ⏰ Timing error
    PIO_ERR_MEMORY_ERROR = 18,           ///< 💾 Memory error
    PIO_ERR_INVALID_PROTOCOL = 19,       ///< 📡 Invalid protocol
    PIO_ERR_HARDWARE_FAILURE = 20,       ///< 🔧 Hardware failure
};
```

---

## 💾 **NVS Error Codes**

### 📋 **hf_nvs_err_t Enumeration**

```cpp
enum class hf_nvs_err_t : hf_u8_t {
    NVS_SUCCESS = 0,                     ///< ✅ Operation successful
    
    // General NVS errors (1-10)
    NVS_ERR_FAILURE = 1,                 ///< ❌ General NVS failure
    NVS_ERR_NOT_INITIALIZED = 2,         ///< ⚠️ NVS not initialized
    NVS_ERR_INVALID_PARAMETER = 3,       ///< 🚫 Invalid parameter
    NVS_ERR_TIMEOUT = 4,                 ///< ⏰ Operation timeout
    NVS_ERR_BUSY = 5,                    ///< 🔄 NVS resource busy
    
    // NVS-specific errors (11-25)
    NVS_ERR_KEY_NOT_FOUND = 11,          ///< 🔍 Key not found
    NVS_ERR_INVALID_KEY = 12,            ///< 🔑 Invalid key
    NVS_ERR_INVALID_NAMESPACE = 13,      ///< 📁 Invalid namespace
    NVS_ERR_STORAGE_FULL = 14,           ///< 💾 Storage full
    NVS_ERR_TYPE_MISMATCH = 15,          ///< 🔄 Type mismatch
    NVS_ERR_READ_ONLY = 16,              ///< 🔒 Read-only storage
    NVS_ERR_CORRUPTION = 17,             ///< 💥 Data corruption
    NVS_ERR_ENCRYPTION_ERROR = 18,       ///< 🔐 Encryption error
    NVS_ERR_ERASE_ERROR = 19,            ///< 🗑️ Erase operation failed
    NVS_ERR_HARDWARE_FAILURE = 20,       ///< 🔧 Hardware failure
};
```

---

## ⏰ **Timer Error Codes**

### 📋 **hf_timer_err_t Enumeration**

```cpp
enum class hf_timer_err_t : hf_u8_t {
    TIMER_SUCCESS = 0,                   ///< ✅ Operation successful
    
    // General Timer errors (1-10)
    TIMER_ERR_FAILURE = 1,               ///< ❌ General timer failure
    TIMER_ERR_NOT_INITIALIZED = 2,       ///< ⚠️ Timer not initialized
    TIMER_ERR_INVALID_PARAMETER = 3,     ///< 🚫 Invalid parameter
    TIMER_ERR_TIMEOUT = 4,               ///< ⏰ Operation timeout
    TIMER_ERR_BUSY = 5,                  ///< 🔄 Timer resource busy
    
    // Timer-specific errors (11-25)
    TIMER_ERR_INVALID_PERIOD = 11,       ///< ⏰ Invalid period
    TIMER_ERR_CALLBACK_NULL = 12,        ///< 📞 Null callback function
    TIMER_ERR_ALREADY_RUNNING = 13,      ///< 🔄 Timer already running
    TIMER_ERR_NOT_RUNNING = 14,          ///< ⏸️ Timer not running
    TIMER_ERR_PRECISION_LOST = 15,       ///< 📏 Precision lost
    TIMER_ERR_INTERRUPT_ERROR = 16,      ///< ⚡ Interrupt error
    TIMER_ERR_CLOCK_ERROR = 17,          ///< ⏰ Clock configuration error
    TIMER_ERR_OVERFLOW = 18,             ///< 📊 Timer overflow
    TIMER_ERR_UNDERFLOW = 19,            ///< 📊 Timer underflow
    TIMER_ERR_HARDWARE_FAILURE = 20,     ///< 🔧 Hardware failure
};
```

---

## 💡 **Error Handling Best Practices**

### ✅ **Recommended Practices**

#### 1. **Always Check Return Values**

```cpp
// ✅ Good - Always check error codes
hf_gpio_err_t result = gpio.SetHigh();
if (IsError(result)) {
    printf("GPIO error: %d\n", static_cast<int>(result));
    return false;
}

// ❌ Bad - Ignoring return values
gpio.SetHigh();  // What if it fails?
```

#### 2. **Use Switch Statements for Specific Handling**

```cpp
// ✅ Good - Handle specific error cases
hf_i2c_err_t result = i2c.Write(address, data, size);
switch (result) {
    case hf_i2c_err_t::I2C_SUCCESS:
        printf("✅ I2C write successful\n");
        break;
    case hf_i2c_err_t::I2C_ERR_NO_ACK:
        printf("⚠️ Device not responding\n");
        // Maybe retry or check connections
        break;
    case hf_i2c_err_t::I2C_ERR_TIMEOUT:
        printf("⏰ I2C timeout - check bus speed\n");
        break;
    default:
        printf("❌ I2C error: %d\n", static_cast<int>(result));
        break;
}
```

#### 3. **Implement Error Recovery**

```cpp
// ✅ Good - Implement retry logic
bool WriteWithRetry(BaseI2c& i2c, hf_u8_t address, 
                   const hf_u8_t* data, size_t size) {
    constexpr int MAX_RETRIES = 3;
    
    for (int retry = 0; retry < MAX_RETRIES; ++retry) {
        hf_i2c_err_t result = i2c.Write(address, data, size);
        
        if (IsSuccess(result)) {
            return true;
        }
        
        if (result == hf_i2c_err_t::I2C_ERR_TIMEOUT ||
            result == hf_i2c_err_t::I2C_ERR_NO_ACK) {
            // Recoverable errors - retry
            printf("⚠️ Retry %d/%d\n", retry + 1, MAX_RETRIES);
            vTaskDelay(pdMS_TO_TICKS(10));  // Brief delay
            continue;
        }
        
        // Non-recoverable error
        printf("❌ Fatal I2C error: %d\n", static_cast<int>(result));
        break;
    }
    
    return false;
}
```

#### 4. **Log Errors with Context**

```cpp
// ✅ Good - Provide context with errors
bool ConfigureAdc(EspAdc& adc, hf_channel_id_t channel) {
    hf_adc_err_t result = adc.ConfigureChannel(channel);
    if (IsError(result)) {
        printf("❌ Failed to configure ADC channel %u: %d\n", 
               channel, static_cast<int>(result));
        
        // Provide additional context
        if (result == hf_adc_err_t::ADC_ERR_INVALID_CHANNEL) {
            printf("💡 Valid channels: 0-7\n");
        }
        return false;
    }
    return true;
}
```

#### 5. **Use RAII for Resource Management**

```cpp
// ✅ Good - RAII ensures cleanup on errors
class SafeGpioOutput {
private:
    BaseGpio& gpio_;
    bool initialized_;

public:
    SafeGpioOutput(BaseGpio& gpio) : gpio_(gpio), initialized_(false) {
        if (gpio_.Initialize()) {
            if (IsSuccess(gpio_.SetAsOutput())) {
                initialized_ = true;
            }
        }
    }
    
    ~SafeGpioOutput() {
        if (initialized_) {
            gpio_.SetLow();  // Safe state on destruction
        }
    }
    
    bool IsValid() const { return initialized_; }
    
    hf_gpio_err_t SetHigh() {
        if (!initialized_) {
            return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
        }
        return gpio_.SetHigh();
    }
};
```

### ❌ **Common Mistakes to Avoid**

1. **Ignoring return values** - Always check error codes
2. **Generic error handling** - Use specific error types
3. **No error recovery** - Implement retry logic where appropriate
4. **Poor error messages** - Provide actionable information
5. **Resource leaks** - Ensure cleanup on error paths

### 🔧 **Error Code Conversion**

```cpp
// Utility function to convert error codes to strings
const char* ErrorToString(hf_gpio_err_t error) {
    switch (error) {
        case hf_gpio_err_t::GPIO_SUCCESS:
            return "Success";
        case hf_gpio_err_t::GPIO_ERR_INVALID_PIN:
            return "Invalid pin number";
        case hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED:
            return "GPIO not initialized";
        case hf_gpio_err_t::GPIO_ERR_DIRECTION_MISMATCH:
            return "Wrong pin direction";
        default:
            return "Unknown error";
    }
}
```

---

<div align="center">

**🚨 Robust error handling is essential for reliable embedded systems**

*Use these error codes consistently to build resilient hardware interfaces*

</div>