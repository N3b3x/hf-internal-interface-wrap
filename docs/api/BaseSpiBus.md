# 🔄 BaseSpiBus API Documentation

## 📋 Overview

The `BaseSpiBus` class is an abstract base class that provides a unified, platform-agnostic interface for SPI (Serial Peripheral Interface) bus communication in the HardFOC system. This class enables high-speed communication with various SPI devices such as ADCs, DACs, sensors, displays, and external controllers.

## 🏗️ Class Hierarchy

```
BaseSpiBus (Abstract Base Class)
    ├── McuSpiBus (ESP32 SPI implementation)
    ├── StmSpiBus (STM32 SPI implementation)
    ├── BitBangSpiBus (Software SPI implementation)
    └── SfSpiBus (Thread-safe wrapper)
```

## 🔧 Features

- ✅ **High-Speed Communication**: Up to 80MHz SPI clock speeds
- ✅ **Four SPI Modes**: Complete CPOL/CPHA mode support (0-3)
- ✅ **Flexible Transfers**: Full-duplex, write-only, and read-only operations
- ✅ **Variable Word Size**: 8-bit, 16-bit, and custom bit transfers
- ✅ **Automatic CS Control**: Hardware and software chip select management
- ✅ **DMA Support**: High-performance transfers with DMA acceleration
- ✅ **Multi-Device Support**: Multiple devices on same bus with different settings
- ✅ **Comprehensive Error Handling**: 30 specific error codes

## 📊 Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| `SPI_SUCCESS` | 0 | Operation successful |
| `SPI_ERR_NOT_INITIALIZED` | 2 | SPI not initialized |
| `SPI_ERR_BUS_BUSY` | 7 | SPI bus is busy |
| `SPI_ERR_TRANSFER_FAILED` | 11 | Transfer operation failed |
| `SPI_ERR_TRANSFER_TIMEOUT` | 12 | Transfer timed out |
| `SPI_ERR_DEVICE_NOT_RESPONDING` | 16 | Device not responding |
| `SPI_ERR_CS_CONTROL_FAILED` | 17 | Chip select control failed |
| `SPI_ERR_INVALID_CLOCK_SPEED` | 24 | Unsupported clock speed |
| `SPI_ERR_INVALID_MODE` | 25 | Invalid SPI mode |

*See header file for complete list of 30 error codes*

## 🏗️ Data Structures

### SpiBusConfig
Configuration structure for SPI bus setup:

```cpp
struct SpiBusConfig {
    hf_spi_host_t host;                 // SPI host/controller
    hf_gpio_num_t mosi_pin;             // MOSI (Master Out Slave In) pin
    hf_gpio_num_t miso_pin;             // MISO (Master In Slave Out) pin
    hf_gpio_num_t sclk_pin;             // SCLK (Serial Clock) pin
    hf_gpio_num_t cs_pin;               // CS (Chip Select) pin
    uint32_t clock_speed_hz;            // Clock speed in Hz
    uint8_t mode;                       // SPI mode (0-3)
    uint8_t bits_per_word;              // Bits per transfer (8 or 16)
    bool cs_active_low;                 // CS polarity
    uint16_t timeout_ms;                // Operation timeout
};
```

### SPI Modes
Standard SPI modes with CPOL/CPHA combinations:

| Mode | CPOL | CPHA | Clock Polarity | Clock Phase |
|------|------|------|----------------|-------------|
| 0 | 0 | 0 | Idle Low | Sample on Rising Edge |
| 1 | 0 | 1 | Idle Low | Sample on Falling Edge |
| 2 | 1 | 0 | Idle High | Sample on Falling Edge |
| 3 | 1 | 1 | Idle High | Sample on Rising Edge |

### SpiDeviceConfig
Per-device configuration for multi-device buses:

```cpp
struct SpiDeviceConfig {
    hf_gpio_num_t cs_pin;               // Device-specific CS pin
    uint32_t clock_speed_hz;            // Device-specific clock speed
    uint8_t mode;                       // Device-specific SPI mode
    uint8_t bits_per_word;              // Device-specific word size
    uint16_t cs_ena_pretrans;           // CS setup time
    uint16_t cs_ena_posttrans;          // CS hold time
    bool use_dma;                       // Enable DMA for transfers
};
```

## 🔨 Core Methods

### Initialization

#### `EnsureInitialized()`
```cpp
bool EnsureInitialized() noexcept
```
**Description**: Lazy initialization - initializes SPI bus on first call  
**Returns**: `true` if initialized successfully, `false` on failure  
**Thread-Safe**: Yes  

#### `IsInitialized()`
```cpp
bool IsInitialized() const noexcept
```
**Description**: Check if SPI bus is initialized  
**Returns**: `true` if initialized, `false` otherwise  
**Thread-Safe**: Yes  

### Device Management

#### `AddDevice()`
```cpp
virtual HfSpiErr AddDevice(uint8_t device_id, const SpiDeviceConfig& config) noexcept = 0
```
**Description**: Add a device to the SPI bus with specific configuration  
**Parameters**:
- `device_id`: Unique device identifier
- `config`: Device-specific configuration

**Returns**: `HfSpiErr` result code  
**Thread-Safe**: Implementation dependent  

#### `RemoveDevice()`
```cpp
virtual HfSpiErr RemoveDevice(uint8_t device_id) noexcept = 0
```
**Description**: Remove a device from the SPI bus  
**Parameters**:
- `device_id`: Device identifier to remove

**Returns**: `HfSpiErr` result code  
**Thread-Safe**: Implementation dependent  

### Data Transfer

#### `Transfer()`
```cpp
virtual HfSpiErr Transfer(uint8_t device_id, const uint8_t* tx_data, uint8_t* rx_data, 
                         size_t length, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Full-duplex SPI transfer (simultaneous send and receive)  
**Parameters**:
- `device_id`: Target device identifier
- `tx_data`: Data to transmit (can be nullptr for receive-only)
- `rx_data`: Buffer for received data (can be nullptr for transmit-only)
- `length`: Number of bytes to transfer
- `timeout_ms`: Timeout in milliseconds (0 = use default)

**Returns**: `HfSpiErr` result code  
**Thread-Safe**: Implementation dependent  

#### `Write()`
```cpp
virtual HfSpiErr Write(uint8_t device_id, const uint8_t* data, 
                      size_t length, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Write-only SPI transfer  
**Parameters**:
- `device_id`: Target device identifier
- `data`: Data to transmit
- `length`: Number of bytes to write
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfSpiErr` result code  
**Thread-Safe**: Implementation dependent  

#### `Read()`
```cpp
virtual HfSpiErr Read(uint8_t device_id, uint8_t* data, 
                     size_t length, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Read-only SPI transfer  
**Parameters**:
- `device_id`: Target device identifier
- `data`: Buffer for received data
- `length`: Number of bytes to read
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfSpiErr` result code  
**Thread-Safe**: Implementation dependent  

#### `WriteRead()`
```cpp
virtual HfSpiErr WriteRead(uint8_t device_id, const uint8_t* tx_data, size_t tx_length,
                          uint8_t* rx_data, size_t rx_length, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Write then read operation (useful for command-response protocols)  
**Parameters**:
- `device_id`: Target device identifier
- `tx_data`: Command/data to transmit
- `tx_length`: Number of bytes to write
- `rx_data`: Buffer for received data
- `rx_length`: Number of bytes to read
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfSpiErr` result code  
**Thread-Safe**: Implementation dependent  

### Register Access Helpers

#### `WriteRegister()`
```cpp
virtual HfSpiErr WriteRegister(uint8_t device_id, uint8_t reg_addr, 
                              uint8_t value, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Write a single register value  
**Parameters**:
- `device_id`: Target device identifier
- `reg_addr`: Register address
- `value`: Value to write
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfSpiErr` result code  
**Thread-Safe**: Implementation dependent  

#### `ReadRegister()`
```cpp
virtual HfSpiErr ReadRegister(uint8_t device_id, uint8_t reg_addr, 
                             uint8_t& value, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Read a single register value  
**Parameters**:
- `device_id`: Target device identifier
- `reg_addr`: Register address
- `value`: Reference to store read value
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfSpiErr` result code  
**Thread-Safe**: Implementation dependent  

## 💡 Usage Examples

### Basic SPI Setup and Communication
```cpp
#include "mcu/McuSpiBus.h"

// Create SPI instance
auto spi = McuSpiBus::Create();

// Configure SPI bus
SpiBusConfig config;
config.host = SPI2_HOST;
config.mosi_pin = 23;                // GPIO 23
config.miso_pin = 19;                // GPIO 19
config.sclk_pin = 18;                // GPIO 18
config.clock_speed_hz = 1000000;     // 1MHz
config.mode = 0;                     // SPI Mode 0

// Initialize bus
if (!spi->Configure(config) || !spi->EnsureInitialized()) {
    printf("Failed to initialize SPI bus\n");
    return;
}

// Add device (e.g., ADC MCP3008)
SpiDeviceConfig adc_config;
adc_config.cs_pin = 5;               // GPIO 5 for CS
adc_config.clock_speed_hz = 1000000; // 1MHz for ADC
adc_config.mode = 0;                 // Mode 0
adc_config.bits_per_word = 8;

spi->AddDevice(0, adc_config);

// Read ADC channel 0
uint8_t cmd[] = {0x01, 0x80, 0x00};  // Start bit, SGL/DIFF, channel 0
uint8_t response[3];

HfSpiErr result = spi->Transfer(0, cmd, response, 3);
if (result == HfSpiErr::SPI_SUCCESS) {
    uint16_t adc_value = ((response[1] & 0x03) << 8) | response[2];
    printf("ADC Value: %d\n", adc_value);
}
```

### Multiple Device Management
```cpp
// Configure different devices on same bus
SpiDeviceConfig eeprom_config;
eeprom_config.cs_pin = 4;            // Different CS pin
eeprom_config.clock_speed_hz = 2000000; // 2MHz for EEPROM
eeprom_config.mode = 0;

SpiDeviceConfig display_config;
display_config.cs_pin = 15;          // Another CS pin
display_config.clock_speed_hz = 8000000; // 8MHz for display
display_config.mode = 0;

// Add multiple devices
spi->AddDevice(1, eeprom_config);    // Device ID 1: EEPROM
spi->AddDevice(2, display_config);   // Device ID 2: Display

// Use different devices
uint8_t eeprom_data[] = {0x02, 0x00, 0x00, 0x55}; // Write command
spi->Write(1, eeprom_data, 4);       // Write to EEPROM

uint8_t display_cmd[] = {0x21, 0x00, 0x7F}; // Set column address
spi->Write(2, display_cmd, 3);       // Send to display
```

### Register-Based Device Communication
```cpp
// Example: MAX31855 Thermocouple Amplifier
spi->AddDevice(3, thermocouple_config);

// Read temperature data
uint8_t temp_data[4];
if (spi->Read(3, temp_data, 4) == HfSpiErr::SPI_SUCCESS) {
    // Parse thermocouple data
    int32_t raw_temp = (temp_data[0] << 24) | (temp_data[1] << 16) | 
                       (temp_data[2] << 8) | temp_data[3];
    
    if (!(raw_temp & 0x01)) {  // Check fault bit
        float temperature = (raw_temp >> 18) * 0.25f;
        printf("Temperature: %.2f°C\n", temperature);
    } else {
        printf("Thermocouple fault detected\n");
    }
}

// Example: Using register helpers for MCP23S17 GPIO expander
spi->AddDevice(4, gpio_expander_config);

// Configure all pins as outputs
spi->WriteRegister(4, 0x00, 0x00);  // IODIRA register
spi->WriteRegister(4, 0x01, 0x00);  // IODIRB register

// Set all pins high
spi->WriteRegister(4, 0x12, 0xFF);  // GPIOA register
spi->WriteRegister(4, 0x13, 0xFF);  // GPIOB register

// Read current pin states
uint8_t gpio_a_state, gpio_b_state;
spi->ReadRegister(4, 0x12, gpio_a_state);
spi->ReadRegister(4, 0x13, gpio_b_state);
printf("GPIO A: 0x%02X, GPIO B: 0x%02X\n", gpio_a_state, gpio_b_state);
```

### High-Speed Data Transfer with DMA
```cpp
// Configure high-speed device with DMA
SpiDeviceConfig high_speed_config;
high_speed_config.cs_pin = 21;
high_speed_config.clock_speed_hz = 20000000; // 20MHz
high_speed_config.mode = 0;
high_speed_config.use_dma = true;            // Enable DMA

spi->AddDevice(5, high_speed_config);

// Large data transfer (e.g., image data to display)
std::vector<uint8_t> image_data(1024 * 64); // 64KB image
std::fill(image_data.begin(), image_data.end(), 0x55); // Test pattern

// Send large data block efficiently
auto start_time = esp_timer_get_time();
HfSpiErr result = spi->Write(5, image_data.data(), image_data.size());
auto end_time = esp_timer_get_time();

if (result == HfSpiErr::SPI_SUCCESS) {
    uint32_t transfer_time = end_time - start_time;
    float transfer_rate = (image_data.size() * 8.0f * 1000000.0f) / transfer_time;
    printf("Transferred %zu bytes in %u μs (%.2f Mbps)\n", 
           image_data.size(), transfer_time, transfer_rate / 1000000.0f);
}
```

### Error Handling and Recovery
```cpp
HfSpiErr result = spi->Transfer(0, tx_data, rx_data, length);
if (result != HfSpiErr::SPI_SUCCESS) {
    printf("SPI Error: %s\n", HfSpiErrToString(result).data());
    
    switch (result) {
        case HfSpiErr::SPI_ERR_BUS_BUSY:
            printf("Bus is busy, retrying...\n");
            vTaskDelay(pdMS_TO_TICKS(1));
            // Retry operation
            break;
            
        case HfSpiErr::SPI_ERR_TRANSFER_TIMEOUT:
            printf("Transfer timeout, checking device\n");
            // Check device power, connections
            break;
            
        case HfSpiErr::SPI_ERR_DEVICE_NOT_RESPONDING:
            printf("Device not responding, reinitializing...\n");
            spi->RemoveDevice(0);
            spi->AddDevice(0, device_config);
            break;
            
        case HfSpiErr::SPI_ERR_INVALID_CLOCK_SPEED:
            printf("Clock speed too high, reducing...\n");
            device_config.clock_speed_hz /= 2;
            spi->RemoveDevice(0);
            spi->AddDevice(0, device_config);
            break;
            
        default:
            printf("Unhandled SPI error\n");
            break;
    }
}
```

### Performance Optimization
```cpp
// Benchmark different transfer methods
auto BenchmarkTransfer = [&spi](const char* method, std::function<void()> transfer_func) {
    const int iterations = 1000;
    
    auto start = esp_timer_get_time();
    for (int i = 0; i < iterations; ++i) {
        transfer_func();
    }
    auto end = esp_timer_get_time();
    
    uint32_t avg_time = (end - start) / iterations;
    printf("%s: %u μs average\n", method, avg_time);
};

uint8_t test_data[64];
uint8_t rx_buffer[64];

// Compare different transfer sizes
BenchmarkTransfer("1-byte transfers", [&]() {
    for (int i = 0; i < 64; ++i) {
        spi->Transfer(0, &test_data[i], &rx_buffer[i], 1);
    }
});

BenchmarkTransfer("64-byte transfer", [&]() {
    spi->Transfer(0, test_data, rx_buffer, 64);
});

BenchmarkTransfer("Write-only 64 bytes", [&]() {
    spi->Write(0, test_data, 64);
});
```

## 🧪 Testing

The BaseSpiBus class can be tested using:

```cpp
#include "tests/SpiBusTests.h"

// Run comprehensive SPI tests
bool success = TestSpiBusFunctionality();
```

## ⚠️ Important Notes

1. **Abstract Class**: Cannot be instantiated directly - use concrete implementations
2. **Thread Safety**: Depends on concrete implementation (use SfSpiBus for thread safety)
3. **Clock Speed Limits**: Maximum speed depends on hardware and wire length
4. **Signal Integrity**: Use proper termination and short wires for high speeds
5. **CS Management**: Ensure CS pins don't conflict with other peripherals
6. **DMA Limitations**: DMA transfers may have alignment and size requirements
7. **Mode Compatibility**: Ensure device SPI mode matches configuration

## 🔗 Related Classes

- [`BaseI2cBus`](BaseI2cBus.md) - I2C interface following same pattern
- [`BaseGpio`](BaseGpio.md) - GPIO interface for CS and other control pins
- [`McuSpiBus`](../mcu/McuSpiBus.md) - ESP32 SPI implementation
- [`SfSpiBus`](../thread_safe/SfSpiBus.md) - Thread-safe wrapper

## 📝 See Also

- [HardFOC SPI Architecture](../guides/spi-architecture.md)
- [SPI Device Integration Guide](../guides/spi-device-integration.md)
- [High-Speed SPI Best Practices](../guides/high-speed-spi.md)
