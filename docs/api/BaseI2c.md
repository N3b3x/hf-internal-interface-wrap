# 🚌 BaseI2c API Reference

## 🌟 Overview

`BaseI2c` is the abstract base class for I2C (Inter-Integrated Circuit) device communication in the HardFOC system. It provides a unified interface for communicating with sensors, displays, memory devices, and other I2C peripherals with comprehensive error handling and device management.

## ✨ Features

- **🎯 Device-Centric Design** - Each instance represents a single I2C device with pre-configured address
- **🚀 Multi-Speed Support** - Standard (100kHz), Fast (400kHz), and Fast+ (1MHz) modes
- **📡 Register Operations** - Convenient read/write register methods for easy sensor access
- **🔍 Device Discovery** - Built-in device scanning and presence detection
- **⏰ Configurable Timeouts** - Per-operation timeout control for reliable communication
- **🛡️ Error Recovery** - Automatic bus recovery and error handling mechanisms
- **📊 Performance Monitoring** - Comprehensive statistics and bus health diagnostics
- **🔧 Lazy Initialization** - Resources allocated only when needed

## 📁 Header File

```cpp
#include "inc/base/BaseI2c.h"
```

## 🎯 Type Definitions

### 🚨 Error Codes

```cpp
enum class hf_i2c_err_t : hf_u8_t {
    I2C_SUCCESS = 0,                    // ✅ Success
    I2C_ERR_FAILURE = 1,                // ❌ General failure
    I2C_ERR_NOT_INITIALIZED = 2,        // ⚠️ Not initialized
    I2C_ERR_ALREADY_INITIALIZED = 3,    // ⚠️ Already initialized
    I2C_ERR_INVALID_PARAMETER = 4,      // 🚫 Invalid parameter
    I2C_ERR_NULL_POINTER = 5,           // 🚫 Null pointer
    I2C_ERR_OUT_OF_MEMORY = 6,          // 💾 Out of memory
    I2C_ERR_BUS_BUSY = 7,               // 🔄 Bus busy
    I2C_ERR_BUS_ERROR = 8,              // 💥 Bus error
    I2C_ERR_BUS_ARBITRATION_LOST = 9,   // ⚔️ Arbitration lost
    I2C_ERR_BUS_NOT_AVAILABLE = 10,     // 🚫 Bus not available
    I2C_ERR_BUS_TIMEOUT = 11,           // ⏰ Bus timeout
    I2C_ERR_DEVICE_NOT_FOUND = 12,      // 🔍 Device not found
    I2C_ERR_DEVICE_NACK = 13,           // 🚫 Device NACK
    I2C_ERR_DEVICE_NOT_RESPONDING = 14, // 🔇 Device not responding
    I2C_ERR_INVALID_ADDRESS = 15,       // 🏠 Invalid device address
    I2C_ERR_DATA_TOO_LONG = 16,         // 📏 Data too long
    I2C_ERR_READ_FAILURE = 17,          // 📖 Read failure
    I2C_ERR_WRITE_FAILURE = 18,         // ✍️ Write failure
    I2C_ERR_TIMEOUT = 19,               // ⏰ Operation timeout
    I2C_ERR_HARDWARE_FAULT = 20,        // 💥 Hardware fault
    I2C_ERR_COMMUNICATION_FAILURE = 21, // 📡 Communication failure
    I2C_ERR_VOLTAGE_OUT_OF_RANGE = 22,  // ⚡ Voltage out of range
    I2C_ERR_CLOCK_STRETCH_TIMEOUT = 23, // ⏱️ Clock stretch timeout
    I2C_ERR_INVALID_CONFIGURATION = 24, // ⚙️ Invalid configuration
    I2C_ERR_UNSUPPORTED_OPERATION = 25, // 🚫 Unsupported operation
    I2C_ERR_INVALID_CLOCK_SPEED = 26,   // 📻 Invalid clock speed
    I2C_ERR_PIN_CONFIGURATION_ERROR = 27, // 🔌 Pin configuration error
    I2C_ERR_SYSTEM_ERROR = 28,          // 💻 System error
    I2C_ERR_PERMISSION_DENIED = 29,     // 🔒 Permission denied
    I2C_ERR_OPERATION_ABORTED = 30      // 🛑 Operation aborted
};
```

### 📊 Statistics Structure

```cpp
struct hf_i2c_statistics_t {
    hf_u32_t total_transactions;        // 🔄 Total I2C transactions
    hf_u32_t successful_transactions;   // ✅ Successful transactions
    hf_u32_t failed_transactions;       // ❌ Failed transactions
    hf_u32_t timeout_count;             // ⏰ Timeout occurrences
    hf_u32_t nack_count;                // 🚫 NACK count
    hf_u32_t arbitration_lost_count;    // ⚔️ Arbitration lost count
    hf_u32_t bus_error_count;           // 💥 Bus error count
    hf_u32_t bytes_transmitted;         // 📤 Total bytes transmitted
    hf_u32_t bytes_received;            // 📥 Total bytes received
    hf_u32_t average_transaction_time_us; // ⏱️ Average transaction time
};
```

### 🩺 Diagnostics Structure

```cpp
struct hf_i2c_diagnostics_t {
    bool bus_healthy;                   // 🟢 Overall bus health
    bool sda_line_state;                // 📡 SDA line state (true = high)
    bool scl_line_state;                // 🕐 SCL line state (true = high)
    bool bus_locked;                    // 🔒 Bus lock status
    hf_i2c_err_t last_error_code;       // ⚠️ Last error code
    hf_u32_t last_error_timestamp_us;   // 🕐 Last error timestamp
    hf_u32_t consecutive_errors;        // 📊 Consecutive error count
    hf_u32_t error_recovery_attempts;   // 🔄 Error recovery attempts
    float bus_utilization_percent;      // 📈 Bus utilization percentage
    hf_u32_t average_response_time_us;  // ⏱️ Average device response time
    hf_u32_t clock_stretching_events;   // 🕐 Clock stretching events
    hf_u32_t active_device_count;       // 📟 Active device count
};
```

## 🏗️ Class Interface

```cpp
class BaseI2c {
public:
    // 🔧 Lifecycle management
    virtual ~BaseI2c() noexcept = default;
    BaseI2c(const BaseI2c&) = delete;
    BaseI2c& operator=(const BaseI2c&) = delete;
    bool EnsureInitialized() noexcept;
    bool EnsureDeinitialized() noexcept;
    bool IsInitialized() const noexcept;

    // 🚀 Core operations (pure virtual)
    virtual bool Initialize() noexcept = 0;
    virtual bool Deinitialize() noexcept = 0;
    
    // 📡 Basic I2C operations
    virtual hf_i2c_err_t Write(const hf_u8_t* data, hf_u16_t length, 
                              hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept = 0;
    virtual hf_i2c_err_t Read(hf_u8_t* data, hf_u16_t length, 
                             hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept = 0;
    virtual hf_i2c_err_t WriteRead(const hf_u8_t* write_data, hf_u16_t write_length,
                                  hf_u8_t* read_data, hf_u16_t read_length,
                                  hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept = 0;

    // 📋 Register operations (convenience methods)
    virtual hf_i2c_err_t WriteRegister(hf_u8_t register_address, hf_u8_t value,
                                      hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept = 0;
    virtual hf_i2c_err_t ReadRegister(hf_u8_t register_address, hf_u8_t& value,
                                     hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept = 0;
    virtual hf_i2c_err_t WriteRegisters(hf_u8_t register_address, const hf_u8_t* data, hf_u16_t length,
                                       hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept = 0;
    virtual hf_i2c_err_t ReadRegisters(hf_u8_t register_address, hf_u8_t* data, hf_u16_t length,
                                      hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept = 0;

    // 🔍 Device information and management
    virtual bool IsDevicePresent(hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept = 0;
    virtual hf_u8_t GetDeviceAddress() const noexcept = 0;
    virtual hf_frequency_hz_t GetClockSpeed() const noexcept = 0;
    virtual hf_i2c_err_t SetClockSpeed(hf_frequency_hz_t clock_speed_hz) noexcept = 0;

    // 📊 Diagnostics and monitoring
    virtual hf_i2c_err_t GetStatistics(hf_i2c_statistics_t& statistics) const noexcept = 0;
    virtual hf_i2c_err_t GetDiagnostics(hf_i2c_diagnostics_t& diagnostics) const noexcept = 0;
    virtual hf_i2c_err_t ResetStatistics() noexcept = 0;
    virtual hf_i2c_err_t ResetDiagnostics() noexcept = 0;
};
```

## 🎯 Core Methods

### 🔧 Initialization

```cpp
bool EnsureInitialized() noexcept;
```
**Purpose:** 🚀 Lazy initialization - automatically initializes I2C if not already done  
**Returns:** `true` if successful, `false` on failure  
**Usage:** Call before any I2C operations

### 📡 Basic Communication

```cpp
hf_i2c_err_t Write(const hf_u8_t* data, hf_u16_t length, 
                  hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept;
hf_i2c_err_t Read(hf_u8_t* data, hf_u16_t length, 
                 hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept;
hf_i2c_err_t WriteRead(const hf_u8_t* write_data, hf_u16_t write_length,
                      hf_u8_t* read_data, hf_u16_t read_length,
                      hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept;
```
**Purpose:** 📤📥 Basic I2C read/write operations  
**Parameters:** Data buffers, lengths, and optional timeout  
**Returns:** Error code indicating success or failure

### 📋 Register Operations

```cpp
hf_i2c_err_t WriteRegister(hf_u8_t register_address, hf_u8_t value,
                          hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept;
hf_i2c_err_t ReadRegister(hf_u8_t register_address, hf_u8_t& value,
                         hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept;
```
**Purpose:** 📋 Convenient register read/write for sensor devices  
**Parameters:** Register address, data value, optional timeout  
**Returns:** Error code indicating success or failure

### 🔍 Device Management

```cpp
bool IsDevicePresent(hf_timeout_ms_t timeout_ms = HF_TIMEOUT_DEFAULT_MS) noexcept;
hf_u8_t GetDeviceAddress() const noexcept;
hf_frequency_hz_t GetClockSpeed() const noexcept;
```
**Purpose:** 🔍 Device detection and configuration query  
**Returns:** Device presence, address, or clock speed information

## 💡 Usage Examples

### 🌡️ Temperature Sensor (LM75A)

```cpp
#include "inc/mcu/esp32/EspI2c.h"

class LM75ATemperatureSensor {
private:
    EspI2c sensor_;
    static constexpr hf_u8_t LM75A_ADDRESS = 0x48;
    static constexpr hf_u8_t TEMP_REGISTER = 0x00;
    static constexpr hf_u8_t CONFIG_REGISTER = 0x01;
    
public:
    LM75ATemperatureSensor() : sensor_(I2C_NUM_0, LM75A_ADDRESS, 400000) {}
    
    bool initialize() {
        // 🚀 Initialize I2C communication
        if (!sensor_.EnsureInitialized()) {
            printf("❌ Failed to initialize LM75A I2C\n");
            return false;
        }
        
        // 🔍 Check if device is present
        if (!sensor_.IsDevicePresent()) {
            printf("❌ LM75A not found at address 0x%02X\n", LM75A_ADDRESS);
            return false;
        }
        
        // ⚙️ Configure sensor for normal operation
        hf_i2c_err_t result = sensor_.WriteRegister(CONFIG_REGISTER, 0x00);
        if (result != hf_i2c_err_t::I2C_SUCCESS) {
            printf("❌ Failed to configure LM75A: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        printf("✅ LM75A temperature sensor initialized\n");
        return true;
    }
    
    float read_temperature() {
        // 📖 Read temperature register (2 bytes)
        hf_u8_t temp_data[2];
        hf_i2c_err_t result = sensor_.ReadRegisters(TEMP_REGISTER, temp_data, 2);
        
        if (result != hf_i2c_err_t::I2C_SUCCESS) {
            printf("❌ Failed to read temperature: %s\n", HfI2CErrToString(result).data());
            return -999.0f;  // Error value
        }
        
        // 🧮 Convert raw data to temperature (LM75A format: 9-bit, 0.5°C resolution)
        hf_i16_t raw_temp = (temp_data[0] << 8) | temp_data[1];
        raw_temp >>= 7;  // Shift to get 9-bit value
        
        float temperature = raw_temp * 0.5f;
        printf("🌡️ Temperature: %.1f°C\n", temperature);
        return temperature;
    }
    
    bool is_connected() {
        return sensor_.IsDevicePresent(100);  // 100ms timeout
    }
};
```

### 📟 OLED Display (SSD1306)

```cpp
class SSD1306Display {
private:
    EspI2c display_;
    static constexpr hf_u8_t SSD1306_ADDRESS = 0x3C;
    static constexpr hf_u8_t COMMAND_MODE = 0x00;
    static constexpr hf_u8_t DATA_MODE = 0x40;
    
public:
    SSD1306Display() : display_(I2C_NUM_0, SSD1306_ADDRESS, 400000) {}
    
    bool initialize() {
        // 🚀 Initialize display I2C
        if (!display_.EnsureInitialized()) {
            printf("❌ Failed to initialize SSD1306 I2C\n");
            return false;
        }
        
        // 🔍 Check if display is connected
        if (!display_.IsDevicePresent()) {
            printf("❌ SSD1306 not found at address 0x%02X\n", SSD1306_ADDRESS);
            return false;
        }
        
        // 🎨 Initialize display with command sequence
        const hf_u8_t init_commands[] = {
            COMMAND_MODE,
            0xAE,  // Display OFF
            0x20, 0x00,  // Memory addressing mode = horizontal
            0xB0,  // Page start address = 0
            0xC8,  // COM scan direction
            0x00,  // Low column start address
            0x10,  // High column start address
            0x40,  // Display start line = 0
            0x81, 0x3F,  // Contrast control
            0xA1,  // Segment re-map
            0xA6,  // Normal display
            0xA8, 0x3F,  // Multiplex ratio
            0xA4,  // Resume to RAM content display
            0xD3, 0x00,  // Display offset
            0xD5, 0xF0,  // Display clock divide ratio
            0xD9, 0x22,  // Pre-charge period
            0xDA, 0x12,  // COM pins hardware configuration
            0xDB, 0x20,  // VCOM deselect level
            0x8D, 0x14,  // Charge pump setting = enable
            0xAF   // Display ON
        };
        
        hf_i2c_err_t result = display_.Write(init_commands, sizeof(init_commands));
        if (result != hf_i2c_err_t::I2C_SUCCESS) {
            printf("❌ Failed to initialize SSD1306: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        printf("✅ SSD1306 OLED display initialized\n");
        return true;
    }
    
    void clear_display() {
        // 🧹 Clear display buffer
        hf_u8_t clear_data[129];  // Command byte + 128 data bytes
        clear_data[0] = DATA_MODE;
        for (int i = 1; i < 129; i++) {
            clear_data[i] = 0x00;
        }
        
        // 📝 Send clear command for each page (8 pages total)
        for (int page = 0; page < 8; page++) {
            // Set page address
            hf_u8_t page_cmd[] = {COMMAND_MODE, 0xB0 + page, 0x00, 0x10};
            display_.Write(page_cmd, sizeof(page_cmd));
            
            // Clear the page
            display_.Write(clear_data, sizeof(clear_data));
        }
        
        printf("🧹 Display cleared\n");
    }
    
    void write_pixel(hf_u8_t x, hf_u8_t y, bool on) {
        // 📍 Write single pixel (simplified implementation)
        if (x >= 128 || y >= 64) return;
        
        hf_u8_t page = y / 8;
        hf_u8_t bit = y % 8;
        
        // Set position
        hf_u8_t pos_cmd[] = {COMMAND_MODE, 0xB0 + page, x & 0x0F, 0x10 + (x >> 4)};
        display_.Write(pos_cmd, sizeof(pos_cmd));
        
        // Write pixel data
        hf_u8_t pixel_data[] = {DATA_MODE, on ? (1 << bit) : 0};
        display_.Write(pixel_data, sizeof(pixel_data));
    }
};
```

### 💾 EEPROM Memory (24C256)

```cpp
class EEPROM24C256 {
private:
    EspI2c eeprom_;
    static constexpr hf_u8_t EEPROM_ADDRESS = 0x50;
    static constexpr hf_u16_t PAGE_SIZE = 64;     // 64-byte page size
    static constexpr hf_u16_t TOTAL_SIZE = 32768; // 32KB total
    
public:
    EEPROM24C256() : eeprom_(I2C_NUM_0, EEPROM_ADDRESS, 400000) {}
    
    bool initialize() {
        // 🚀 Initialize EEPROM I2C
        if (!eeprom_.EnsureInitialized()) {
            printf("❌ Failed to initialize EEPROM I2C\n");
            return false;
        }
        
        // 🔍 Test EEPROM presence by reading first byte
        if (!eeprom_.IsDevicePresent()) {
            printf("❌ EEPROM not found at address 0x%02X\n", EEPROM_ADDRESS);
            return false;
        }
        
        printf("✅ 24C256 EEPROM initialized (32KB)\n");
        return true;
    }
    
    bool write_byte(hf_u16_t address, hf_u8_t value) {
        if (address >= TOTAL_SIZE) {
            printf("❌ Address 0x%04X out of range\n", address);
            return false;
        }
        
        // 📝 Write single byte (address + data)
        hf_u8_t write_data[] = {
            static_cast<hf_u8_t>(address >> 8),   // High address byte
            static_cast<hf_u8_t>(address & 0xFF), // Low address byte
            value
        };
        
        hf_i2c_err_t result = eeprom_.Write(write_data, sizeof(write_data));
        if (result != hf_i2c_err_t::I2C_SUCCESS) {
            printf("❌ Failed to write EEPROM byte: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        // ⏰ Wait for write cycle to complete (typical 5ms)
        vTaskDelay(pdMS_TO_TICKS(10));
        return true;
    }
    
    bool read_byte(hf_u16_t address, hf_u8_t& value) {
        if (address >= TOTAL_SIZE) {
            printf("❌ Address 0x%04X out of range\n", address);
            return false;
        }
        
        // 📖 Set address then read data
        hf_u8_t addr_data[] = {
            static_cast<hf_u8_t>(address >> 8),   // High address byte
            static_cast<hf_u8_t>(address & 0xFF)  // Low address byte
        };
        
        hf_i2c_err_t result = eeprom_.WriteRead(addr_data, sizeof(addr_data), &value, 1);
        if (result != hf_i2c_err_t::I2C_SUCCESS) {
            printf("❌ Failed to read EEPROM byte: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        return true;
    }
    
    bool write_page(hf_u16_t start_address, const hf_u8_t* data, hf_u16_t length) {
        // ✅ Validate parameters
        if (start_address >= TOTAL_SIZE || length == 0 || length > PAGE_SIZE) {
            printf("❌ Invalid page write parameters\n");
            return false;
        }
        
        if ((start_address + length) > TOTAL_SIZE) {
            printf("❌ Page write would exceed EEPROM size\n");
            return false;
        }
        
        // 📄 Check page boundary alignment
        hf_u16_t page_start = start_address & ~(PAGE_SIZE - 1);
        if (start_address != page_start) {
            printf("⚠️ Warning: Write not page-aligned\n");
        }
        
        // 📝 Prepare write data (address + data)
        hf_u8_t write_buffer[PAGE_SIZE + 2];
        write_buffer[0] = static_cast<hf_u8_t>(start_address >> 8);
        write_buffer[1] = static_cast<hf_u8_t>(start_address & 0xFF);
        
        for (hf_u16_t i = 0; i < length; i++) {
            write_buffer[i + 2] = data[i];
        }
        
        hf_i2c_err_t result = eeprom_.Write(write_buffer, length + 2);
        if (result != hf_i2c_err_t::I2C_SUCCESS) {
            printf("❌ Failed to write EEPROM page: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        // ⏰ Wait for write cycle to complete
        vTaskDelay(pdMS_TO_TICKS(10));
        printf("✅ Wrote %u bytes to EEPROM at address 0x%04X\n", length, start_address);
        return true;
    }
    
    bool read_sequential(hf_u16_t start_address, hf_u8_t* data, hf_u16_t length) {
        if (start_address >= TOTAL_SIZE || length == 0) {
            return false;
        }
        
        if ((start_address + length) > TOTAL_SIZE) {
            length = TOTAL_SIZE - start_address;  // Clamp to available space
        }
        
        // 📖 Set starting address
        hf_u8_t addr_data[] = {
            static_cast<hf_u8_t>(start_address >> 8),
            static_cast<hf_u8_t>(start_address & 0xFF)
        };
        
        hf_i2c_err_t result = eeprom_.WriteRead(addr_data, sizeof(addr_data), data, length);
        if (result != hf_i2c_err_t::I2C_SUCCESS) {
            printf("❌ Failed to read EEPROM sequence: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        printf("✅ Read %u bytes from EEPROM starting at 0x%04X\n", length, start_address);
        return true;
    }
};
```

### 🔍 I2C Bus Scanner

```cpp
class I2CBusScanner {
private:
    EspI2c scanner_;
    static constexpr hf_u8_t SCAN_START = 0x08;
    static constexpr hf_u8_t SCAN_END = 0x77;
    
public:
    I2CBusScanner() : scanner_(I2C_NUM_0, SCAN_START, 100000) {}  // Any address for scanning
    
    bool initialize() {
        return scanner_.EnsureInitialized();
    }
    
    void scan_bus() {
        printf("🔍 Scanning I2C bus...\n");
        printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
        
        hf_u16_t found_count = 0;
        
        for (hf_u8_t row = 0; row < 8; row++) {
            printf("%02X: ", row * 16);
            
            for (hf_u8_t col = 0; col < 16; col++) {
                hf_u8_t address = row * 16 + col;
                
                if (address < SCAN_START || address > SCAN_END) {
                    printf("   ");
                    continue;
                }
                
                // 🔍 Test device presence at this address
                EspI2c test_device(I2C_NUM_0, address, 100000);
                test_device.EnsureInitialized();
                
                if (test_device.IsDevicePresent(50)) {  // 50ms timeout
                    printf("%02X ", address);
                    found_count++;
                } else {
                    printf("-- ");
                }
            }
            printf("\n");
        }
        
        printf("\n🎯 Found %u I2C device(s)\n", found_count);
        
        if (found_count > 0) {
            print_common_devices();
        }
    }
    
private:
    void print_common_devices() {
        printf("\n📋 Common I2C devices:\n");
        printf("   0x20-0x27: PCF8574 I/O Expander\n");
        printf("   0x3C, 0x3D: SSD1306 OLED Display\n");
        printf("   0x48-0x4F: LM75A Temperature Sensor\n");
        printf("   0x50-0x57: 24C256 EEPROM\n");
        printf("   0x68: DS1307 RTC, MPU6050 IMU\n");
        printf("   0x76, 0x77: BMP280 Pressure Sensor\n");
    }
};
```

## 📊 Performance and Diagnostics

### 📈 Statistics Monitoring

```cpp
void monitor_i2c_performance(BaseI2c& device) {
    hf_i2c_statistics_t stats;
    hf_i2c_err_t result = device.GetStatistics(stats);
    
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
        printf("📊 I2C Performance Statistics:\n");
        printf("   🔄 Total Transactions: %u\n", stats.total_transactions);
        printf("   ✅ Successful: %u (%.1f%%)\n", 
               stats.successful_transactions, 
               (float)stats.successful_transactions / stats.total_transactions * 100.0f);
        printf("   ❌ Failed: %u\n", stats.failed_transactions);
        printf("   ⏰ Timeouts: %u\n", stats.timeout_count);
        printf("   🚫 NACKs: %u\n", stats.nack_count);
        printf("   📤 Bytes TX: %u\n", stats.bytes_transmitted);
        printf("   📥 Bytes RX: %u\n", stats.bytes_received);
        printf("   ⏱️ Avg Time: %u μs\n", stats.average_transaction_time_us);
    }
}

void monitor_i2c_health(BaseI2c& device) {
    hf_i2c_diagnostics_t diag;
    hf_i2c_err_t result = device.GetDiagnostics(diag);
    
    if (result == hf_i2c_err_t::I2C_SUCCESS) {
        printf("🩺 I2C Bus Health:\n");
        printf("   🟢 Bus Healthy: %s\n", diag.bus_healthy ? "Yes" : "No");
        printf("   📡 SDA Line: %s\n", diag.sda_line_state ? "HIGH" : "LOW");
        printf("   🕐 SCL Line: %s\n", diag.scl_line_state ? "HIGH" : "LOW");
        printf("   🔒 Bus Locked: %s\n", diag.bus_locked ? "Yes" : "No");
        printf("   📈 Utilization: %.1f%%\n", diag.bus_utilization_percent);
        printf("   📟 Active Devices: %u\n", diag.active_device_count);
        printf("   🔄 Recovery Attempts: %u\n", diag.error_recovery_attempts);
        
        if (diag.consecutive_errors > 0) {
            printf("   ⚠️ Consecutive Errors: %u\n", diag.consecutive_errors);
            printf("   ⚠️ Last Error: %s\n", HfI2CErrToString(diag.last_error_code).data());
        }
    }
}
```

## 🛡️ Error Handling Best Practices

### 🎯 Robust Communication

```cpp
hf_i2c_err_t safe_i2c_read_with_retry(BaseI2c& device, hf_u8_t reg, hf_u8_t& value) {
    const int max_retries = 3;
    int retry_count = 0;
    
    while (retry_count < max_retries) {
        hf_i2c_err_t result = device.ReadRegister(reg, value, 100);  // 100ms timeout
        
        switch (result) {
            case hf_i2c_err_t::I2C_SUCCESS:
                return result;  // Success!
                
            case hf_i2c_err_t::I2C_ERR_BUS_BUSY:
            case hf_i2c_err_t::I2C_ERR_TIMEOUT:
                // 🔄 Transient errors - retry after delay
                retry_count++;
                printf("⚠️ I2C busy/timeout, retry %d/%d\n", retry_count, max_retries);
                vTaskDelay(pdMS_TO_TICKS(10));
                break;
                
            case hf_i2c_err_t::I2C_ERR_DEVICE_NACK:
            case hf_i2c_err_t::I2C_ERR_DEVICE_NOT_RESPONDING:
                // 🔍 Check device presence
                if (!device.IsDevicePresent(50)) {
                    printf("❌ Device not present\n");
                    return result;
                }
                retry_count++;
                vTaskDelay(pdMS_TO_TICKS(5));
                break;
                
            default:
                // 💥 Permanent error
                printf("❌ I2C Error: %s\n", HfI2CErrToString(result).data());
                return result;
        }
    }
    
    printf("❌ I2C operation failed after %d retries\n", max_retries);
    return hf_i2c_err_t::I2C_ERR_TIMEOUT;
}
```

## 🏎️ Performance Considerations

### ⚡ Optimization Tips

- **🚀 Clock Speed** - Use highest speed supported by all devices (100kHz, 400kHz, 1MHz)
- **📏 Transaction Size** - Larger transactions are more efficient than many small ones
- **⏰ Timeouts** - Use appropriate timeouts based on device characteristics
- **🔄 Retries** - Implement retry logic for transient errors
- **📊 Monitoring** - Use statistics to identify performance bottlenecks

### 📊 Typical Performance Ranges

| **Clock Speed** | **Throughput** | **Use Cases** |
|-----------------|----------------|---------------|
| **100kHz (Standard)** | ~10KB/s | Basic sensors, simple devices |
| **400kHz (Fast)** | ~40KB/s | Most sensors, displays, memory |
| **1MHz (Fast+)** | ~100KB/s | High-speed data acquisition |

## 🧵 Thread Safety

The `BaseI2c` class is **not thread-safe**. For concurrent access from multiple tasks, use appropriate synchronization mechanisms.

## 🔗 Related Documentation

- **[EspI2c API Reference](EspI2c.md)** - ESP32-C6 I2C implementation
- **[BaseGpio API Reference](BaseGpio.md)** - GPIO interface for I2C pins
- **[HardwareTypes Reference](HardwareTypes.md)** - Platform-agnostic type definitions

---

<div align="center">

**🚌 BaseI2c - Connecting the HardFOC Ecosystem** 🌐

*From sensors to displays - BaseI2c bridges the gap between devices* 🔗

</div> 