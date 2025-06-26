# 🔗 BaseI2cBus API Documentation

## 📋 Overview

The `BaseI2cBus` class is an abstract base class that provides a unified, platform-agnostic interface for I2C (Inter-Integrated Circuit) bus communication in the HardFOC system. This class enables communication with various I2C devices such as sensors, EEPROMs, DACs, and other peripherals through a consistent API.

## 🏗️ Class Hierarchy

```
BaseI2cBus (Abstract Base Class)
    ├── McuI2cBus (ESP32/STM32 hardware I2C)
    ├── BitBangI2c (Software I2C implementation)
    └── SfI2cBus (Thread-safe wrapper)
```

## 🔧 Features

- ✅ **Platform Agnostic**: Works with different I2C controllers
- ✅ **Multi-Speed Support**: Standard (100kHz) and Fast (400kHz) modes
- ✅ **Comprehensive Operations**: Read, Write, WriteRead operations
- ✅ **Device Detection**: Scan and probe I2C devices
- ✅ **Register Access**: Convenient register read/write methods
- ✅ **Error Handling**: 30+ specific error codes
- ✅ **Lazy Initialization**: Initialize on first use
- ✅ **Configurable Timeouts**: Per-operation timeout control

## 📊 Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| `I2C_SUCCESS` | 0 | Operation successful |
| `I2C_ERR_NOT_INITIALIZED` | 2 | I2C bus not initialized |
| `I2C_ERR_BUS_BUSY` | 7 | I2C bus is busy |
| `I2C_ERR_BUS_ARBITRATION_LOST` | 9 | Lost arbitration |
| `I2C_ERR_DEVICE_NOT_FOUND` | 12 | Device address not responding |
| `I2C_ERR_DEVICE_NACK` | 13 | Device sent NACK |
| `I2C_ERR_DATA_TOO_LONG` | 16 | Data exceeds buffer size |
| `I2C_ERR_TIMEOUT` | 19 | Operation timed out |
| `I2C_ERR_CLOCK_STRETCH_TIMEOUT` | 23 | Clock stretching timeout |
| `I2C_ERR_INVALID_CLOCK_SPEED` | 26 | Unsupported clock speed |

*See header file for complete list of 30 error codes*

## 🏗️ Data Structures

### I2cBusConfig
Configuration structure for I2C bus initialization:

```cpp
struct I2cBusConfig {
    hf_i2c_port_t port;                 // I2C port number (0, 1, etc.)
    hf_gpio_num_t sda_pin;              // SDA (data) pin
    hf_gpio_num_t scl_pin;              // SCL (clock) pin
    uint32_t clock_speed_hz;            // Clock frequency (100000 or 400000)
    bool enable_pullups;                // Enable internal pull-ups
    uint16_t timeout_ms;                // Default operation timeout
    uint16_t tx_buffer_size;            // TX buffer size (0 = blocking)
    uint16_t rx_buffer_size;            // RX buffer size (0 = blocking)
};
```

**Default Configuration:**
- Clock Speed: 100kHz (Standard Mode)
- Pullups: Enabled
- Timeout: 1000ms
- Buffers: Blocking mode (size = 0)

## 🔨 Core Methods

### Initialization

#### `EnsureInitialized()`
```cpp
bool EnsureInitialized() noexcept
```
**Description**: Lazy initialization - initializes I2C bus on first call  
**Returns**: `true` if initialized successfully, `false` on failure  
**Thread-Safe**: Implementation dependent  

#### `IsInitialized()`
```cpp
bool IsInitialized() const noexcept
```
**Description**: Check if I2C bus is initialized  
**Returns**: `true` if initialized, `false` otherwise  
**Thread-Safe**: Yes  

### Data Transfer Operations

#### `Write()`
```cpp
HfI2cErr Write(uint8_t device_addr, const uint8_t* data, 
               uint16_t length, uint32_t timeout_ms = 0) noexcept
```
**Description**: Write data to an I2C slave device  
**Parameters**:
- `device_addr`: 7-bit I2C device address
- `data`: Pointer to data buffer
- `length`: Number of bytes to write
- `timeout_ms`: Timeout in milliseconds (0 = use default)

**Returns**: `HfI2cErr` error code  
**Thread-Safe**: Implementation dependent  

#### `Read()`
```cpp
HfI2cErr Read(uint8_t device_addr, uint8_t* data, 
              uint16_t length, uint32_t timeout_ms = 0) noexcept
```
**Description**: Read data from an I2C slave device  
**Parameters**:
- `device_addr`: 7-bit I2C device address
- `data`: Buffer to store received data
- `length`: Number of bytes to read
- `timeout_ms`: Timeout in milliseconds (0 = use default)

**Returns**: `HfI2cErr` error code  
**Thread-Safe**: Implementation dependent  

#### `WriteRead()`
```cpp
HfI2cErr WriteRead(uint8_t device_addr, const uint8_t* tx_data, uint16_t tx_length,
                   uint8_t* rx_data, uint16_t rx_length, uint32_t timeout_ms = 0) noexcept
```
**Description**: Write then read without releasing the bus (register access)  
**Parameters**:
- `device_addr`: 7-bit I2C device address
- `tx_data`: Data to transmit (usually register address)
- `tx_length`: Number of bytes to write
- `rx_data`: Buffer for received data
- `rx_length`: Number of bytes to read
- `timeout_ms`: Timeout in milliseconds (0 = use default)

**Returns**: `HfI2cErr` error code  
**Thread-Safe**: Implementation dependent  

### Convenience Methods

#### `WriteRegister()`
```cpp
HfI2cErr WriteRegister(uint8_t device_addr, uint8_t reg_addr, 
                       uint8_t value, uint32_t timeout_ms = 0) noexcept
```
**Description**: Write a single byte to a device register  
**Returns**: `HfI2cErr` error code  

#### `ReadRegister()`
```cpp
HfI2cErr ReadRegister(uint8_t device_addr, uint8_t reg_addr, 
                      uint8_t& value, uint32_t timeout_ms = 0) noexcept
```
**Description**: Read a single byte from a device register  
**Returns**: `HfI2cErr` error code  

#### `WriteRegisterBuffer()`
```cpp
HfI2cErr WriteRegisterBuffer(uint8_t device_addr, uint8_t reg_addr, 
                             const uint8_t* data, uint16_t length, 
                             uint32_t timeout_ms = 0) noexcept
```
**Description**: Write multiple bytes starting from a register address  
**Returns**: `HfI2cErr` error code  

#### `ReadRegisterBuffer()`
```cpp
HfI2cErr ReadRegisterBuffer(uint8_t device_addr, uint8_t reg_addr, 
                            uint8_t* data, uint16_t length, 
                            uint32_t timeout_ms = 0) noexcept
```
**Description**: Read multiple bytes starting from a register address  
**Returns**: `HfI2cErr` error code  

### Device Detection

#### `ProbeDevice()`
```cpp
bool ProbeDevice(uint8_t device_addr, uint32_t timeout_ms = 0) noexcept
```
**Description**: Check if a device responds at the given address  
**Parameters**:
- `device_addr`: 7-bit I2C device address to probe

**Returns**: `true` if device responds, `false` otherwise  
**Thread-Safe**: Implementation dependent  

#### `ScanBus()`
```cpp
std::vector<uint8_t> ScanBus(uint32_t timeout_ms = 0) noexcept
```
**Description**: Scan the I2C bus for responding devices  
**Returns**: Vector of responding device addresses  
**Thread-Safe**: Implementation dependent  

## 💡 Usage Examples

### Basic I2C Communication
```cpp
#include "mcu/McuI2cBus.h"

// Configure I2C bus
I2cBusConfig config;
config.port = 0;
config.sda_pin = 21;
config.scl_pin = 22;
config.clock_speed_hz = 400000;  // 400kHz Fast Mode

// Create I2C instance
auto i2c = std::make_unique<McuI2cBus>(config);

// Initialize
if (!i2c->EnsureInitialized()) {
    printf("Failed to initialize I2C\n");
    return;
}

// Write data to device
uint8_t data[] = {0x10, 0x20, 0x30};
HfI2cErr result = i2c->Write(0x48, data, sizeof(data));
if (result == HfI2cErr::I2C_SUCCESS) {
    printf("Write successful\n");
}
```

### Register-Based Communication
```cpp
const uint8_t DEVICE_ADDR = 0x48;
const uint8_t TEMP_REG = 0x00;
const uint8_t CONFIG_REG = 0x01;

// Write to configuration register
uint8_t config_value = 0x60;  // Configuration data
result = i2c->WriteRegister(DEVICE_ADDR, CONFIG_REG, config_value);

if (result == HfI2cErr::I2C_SUCCESS) {
    printf("Configuration written successfully\n");
}

// Read temperature register
uint8_t temp_data[2];
result = i2c->ReadRegisterBuffer(DEVICE_ADDR, TEMP_REG, temp_data, 2);

if (result == HfI2cErr::I2C_SUCCESS) {
    uint16_t temp_raw = (temp_data[0] << 8) | temp_data[1];
    float temperature = temp_raw * 0.0625f;  // Convert to Celsius
    printf("Temperature: %.2f°C\n", temperature);
}
```

### Device Detection and Scanning
```cpp
// Check if specific device is present
if (i2c->ProbeDevice(0x48)) {
    printf("Temperature sensor found at 0x48\n");
} else {
    printf("No device at 0x48\n");
}

// Scan entire bus
auto devices = i2c->ScanBus();
printf("Found %zu I2C devices:\n", devices.size());
for (uint8_t addr : devices) {
    printf("  Device at 0x%02X\n", addr);
}
```

### Error Handling
```cpp
HfI2cErr result = i2c->Write(0x48, data, length);

switch (result) {
    case HfI2cErr::I2C_SUCCESS:
        printf("Write successful\n");
        break;
        
    case HfI2cErr::I2C_ERR_DEVICE_NOT_FOUND:
        printf("Device not responding - check address and connections\n");
        break;
        
    case HfI2cErr::I2C_ERR_BUS_BUSY:
        printf("Bus busy - retry operation\n");
        // Implement retry logic
        break;
        
    case HfI2cErr::I2C_ERR_TIMEOUT:
        printf("Operation timed out - check clock speed and device\n");
        break;
        
    default:
        printf("I2C Error: %s\n", HfI2cErrToString(result).data());
        break;
}
```

### Multi-Device Communication
```cpp
// Multiple devices on same bus
const uint8_t EEPROM_ADDR = 0x50;
const uint8_t SENSOR_ADDR = 0x48;
const uint8_t DAC_ADDR = 0x60;

// Read from EEPROM
uint8_t eeprom_data[16];
result = i2c->ReadRegisterBuffer(EEPROM_ADDR, 0x00, eeprom_data, 16);

// Read sensor
uint8_t sensor_value;
result = i2c->ReadRegister(SENSOR_ADDR, 0x00, sensor_value);

// Write to DAC
uint16_t dac_value = 2048;  // Mid-scale
uint8_t dac_data[] = {(dac_value >> 8) & 0x0F, dac_value & 0xFF};
result = i2c->Write(DAC_ADDR, dac_data, 2);
```

### Advanced Configuration
```cpp
// Custom configuration for specific application
I2cBusConfig custom_config;
custom_config.port = 1;                    // Use I2C port 1
custom_config.sda_pin = 18;
custom_config.scl_pin = 19;
custom_config.clock_speed_hz = 100000;     // 100kHz for long cables
custom_config.enable_pullups = false;      // External pullups used
custom_config.timeout_ms = 2000;           // 2s timeout for slow devices
custom_config.tx_buffer_size = 128;        // Enable buffering
custom_config.rx_buffer_size = 128;

auto i2c_slow = std::make_unique<McuI2cBus>(custom_config);
```

## 🧪 Testing

The BaseI2cBus class can be tested using:

```cpp
#include "tests/CommBusTests.h"

// Run comprehensive I2C tests
bool success = TestI2cCommunication();
```

## ⚠️ Important Notes

1. **Abstract Class**: Cannot be instantiated directly - use concrete implementations
2. **7-bit Addresses**: All device addresses are 7-bit (0x00-0x7F)
3. **Pull-up Resistors**: Required for proper I2C operation (internal or external)
4. **Clock Stretching**: Some devices may hold SCL low - ensure adequate timeouts
5. **Multi-Master**: Not all implementations support multi-master configurations
6. **Thread Safety**: Depends on concrete implementation - use appropriate synchronization

## 🔗 Related Classes

- [`BaseGpio`](BaseGpio.md) - GPIO interface for I2C pin configuration
- [`BaseAdc`](BaseAdc.md) - ADC interface following same pattern
- [`McuI2cBus`](../mcu/McuI2cBus.md) - Hardware I2C implementation
- [`SfI2cBus`](../thread_safe/SfI2cBus.md) - Thread-safe wrapper

## 📝 See Also

- [I2C Communication Guide](../guides/i2c-communication.md)
- [Device Integration Examples](../examples/i2c-devices.md)
- [Error Handling Best Practices](../guides/error-handling.md)
