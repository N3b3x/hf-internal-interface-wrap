# 🚌 BaseI2c API Reference

<div align="center">

**📋 Navigation**

[← Previous: BasePwm](BasePwm.md) | [Back to API Index](README.md) | [Next: BaseSpi →](BaseSpi.md)

</div>

---

## 🌟 Overview

`BaseI2c` is the abstract base class for I2C (Inter-Integrated Circuit) device communication in the
HardFOC system.
It provides a unified interface for communicating with sensors, displays, memory devices,
and other I2C peripherals with comprehensive error handling and device management.

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
```text

## 🎯 Type Definitions

### 🚨 Error Codes

```cpp
enum class hf*i2c*err*t : hf*u8*t {
    I2C*SUCCESS = 0,                    // ✅ Success
    I2C*ERR*FAILURE = 1,                // ❌ General failure
    I2C*ERR*NOT*INITIALIZED = 2,        // ⚠️ Not initialized
    I2C*ERR*ALREADY*INITIALIZED = 3,    // ⚠️ Already initialized
    I2C*ERR*INVALID*PARAMETER = 4,      // 🚫 Invalid parameter
    I2C*ERR*NULL*POINTER = 5,           // 🚫 Null pointer
    I2C*ERR*OUT*OF*MEMORY = 6,          // 💾 Out of memory
    I2C*ERR*BUS*BUSY = 7,               // 🔄 Bus busy
    I2C*ERR*BUS*ERROR = 8,              // 💥 Bus error
    I2C*ERR*BUS*ARBITRATION*LOST = 9,   // ⚔️ Arbitration lost
    I2C*ERR*BUS*NOT*AVAILABLE = 10,     // 🚫 Bus not available
    I2C*ERR*BUS*TIMEOUT = 11,           // ⏰ Bus timeout
    I2C*ERR*DEVICE*NOT*FOUND = 12,      // 🔍 Device not found
    I2C*ERR*DEVICE*NACK = 13,           // 🚫 Device NACK
    I2C*ERR*DEVICE*NOT*RESPONDING = 14, // 🔇 Device not responding
    I2C*ERR*INVALID*ADDRESS = 15,       // 🏠 Invalid device address
    I2C*ERR*DATA*TOO*LONG = 16,         // 📏 Data too long
    I2C*ERR*READ*FAILURE = 17,          // 📖 Read failure
    I2C*ERR*WRITE*FAILURE = 18,         // ✍️ Write failure
    I2C*ERR*TIMEOUT = 19,               // ⏰ Operation timeout
    I2C*ERR*HARDWARE*FAULT = 20,        // 💥 Hardware fault
    I2C*ERR*COMMUNICATION*FAILURE = 21, // 📡 Communication failure
    I2C*ERR*VOLTAGE*OUT*OF*RANGE = 22,  // ⚡ Voltage out of range
    I2C*ERR*CLOCK*STRETCH*TIMEOUT = 23, // ⏱️ Clock stretch timeout
    I2C*ERR*INVALID*CONFIGURATION = 24, // ⚙️ Invalid configuration
    I2C*ERR*UNSUPPORTED*OPERATION = 25, // 🚫 Unsupported operation
    I2C*ERR*INVALID*CLOCK*SPEED = 26,   // 📻 Invalid clock speed
    I2C*ERR*PIN*CONFIGURATION*ERROR = 27, // 🔌 Pin configuration error
    I2C*ERR*SYSTEM*ERROR = 28,          // 💻 System error
    I2C*ERR*PERMISSION*DENIED = 29,     // 🔒 Permission denied
    I2C*ERR*OPERATION*ABORTED = 30      // 🛑 Operation aborted
};
```text

### 📊 Statistics Structure

```cpp
struct hf*i2c*statistics*t {
    hf*u32*t total*transactions;        // 🔄 Total I2C transactions
    hf*u32*t successful*transactions;   // ✅ Successful transactions
    hf*u32*t failed*transactions;       // ❌ Failed transactions
    hf*u32*t timeout*count;             // ⏰ Timeout occurrences
    hf*u32*t nack*count;                // 🚫 NACK count
    hf*u32*t arbitration*lost*count;    // ⚔️ Arbitration lost count
    hf*u32*t bus*error*count;           // 💥 Bus error count
    hf*u32*t bytes*transmitted;         // 📤 Total bytes transmitted
    hf*u32*t bytes*received;            // 📥 Total bytes received
    hf*u32*t average*transaction*time*us; // ⏱️ Average transaction time
};
```text

### 🩺 Diagnostics Structure

```cpp
struct hf*i2c*diagnostics*t {
    bool bus*healthy;                   // 🟢 Overall bus health
    bool sda*line*state;                // 📡 SDA line state (true = high)
    bool scl*line*state;                // 🕐 SCL line state (true = high)
    bool bus*locked;                    // 🔒 Bus lock status
    hf*i2c*err*t last*error*code;       // ⚠️ Last error code
    hf*u32*t last*error*timestamp*us;   // 🕐 Last error timestamp
    hf*u32*t consecutive*errors;        // 📊 Consecutive error count
    hf*u32*t error*recovery*attempts;   // 🔄 Error recovery attempts
    float bus*utilization*percent;      // 📈 Bus utilization percentage
    hf*u32*t average*response*time*us;  // ⏱️ Average device response time
    hf*u32*t clock*stretching*events;   // 🕐 Clock stretching events
    hf*u32*t active*device*count;       // 📟 Active device count
};
```text

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
    virtual hf*i2c*err*t Write(const hf*u8*t* data, hf*u16*t length, 
                              hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept = 0;
    virtual hf*i2c*err*t Read(hf*u8*t* data, hf*u16*t length, 
                             hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept = 0;
    virtual hf*i2c*err*t WriteRead(const hf*u8*t* write*data, hf*u16*t write*length,
                                  hf*u8*t* read*data, hf*u16*t read*length,
                                  hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept = 0;

    // 📋 Register operations (convenience methods)
    virtual hf*i2c*err*t WriteRegister(hf*u8*t register*address, hf*u8*t value,
                                      hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept = 0;
    virtual hf*i2c*err*t ReadRegister(hf*u8*t register*address, hf*u8*t& value,
                                     hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept = 0;
    virtual hf*i2c*err*t WriteRegisters(hf*u8*t register*address, const hf*u8*t* data, hf*u16*t length,
                                       hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept = 0;
    virtual hf*i2c*err*t ReadRegisters(hf*u8*t register*address, hf*u8*t* data, hf*u16*t length,
                                      hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept = 0;

    // 🔍 Device information and management
    virtual bool IsDevicePresent(hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept = 0;
    virtual hf*u8*t GetDeviceAddress() const noexcept = 0;
    virtual hf*frequency*hz*t GetClockSpeed() const noexcept = 0;
    virtual hf*i2c*err*t SetClockSpeed(hf*frequency*hz*t clock*speed*hz) noexcept = 0;

    // 📊 Diagnostics and monitoring
    virtual hf*i2c*err*t GetStatistics(hf*i2c*statistics*t& statistics) const noexcept = 0;
    virtual hf*i2c*err*t GetDiagnostics(hf*i2c*diagnostics*t& diagnostics) const noexcept = 0;
    virtual hf*i2c*err*t ResetStatistics() noexcept = 0;
    virtual hf*i2c*err*t ResetDiagnostics() noexcept = 0;
};
```text

## 🎯 Core Methods

### 🔧 Initialization

```cpp
bool EnsureInitialized() noexcept;
```text
**Purpose:** 🚀 Lazy initialization - automatically initializes I2C if not already done  
**Returns:** `true` if successful, `false` on failure  
**Usage:** Call before any I2C operations

### 📡 Basic Communication

```cpp
hf*i2c*err*t Write(const hf*u8*t* data, hf*u16*t length, 
                  hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept;
hf*i2c*err*t Read(hf*u8*t* data, hf*u16*t length, 
                 hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept;
hf*i2c*err*t WriteRead(const hf*u8*t* write*data, hf*u16*t write*length,
                      hf*u8*t* read*data, hf*u16*t read*length,
                      hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept;
```text
**Purpose:** 📤📥 Basic I2C read/write operations  
**Parameters:** Data buffers, lengths, and optional timeout  
**Returns:** Error code indicating success or failure

### 📋 Register Operations

```cpp
hf*i2c*err*t WriteRegister(hf*u8*t register*address, hf*u8*t value,
                          hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept;
hf*i2c*err*t ReadRegister(hf*u8*t register*address, hf*u8*t& value,
                         hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept;
```text
**Purpose:** 📋 Convenient register read/write for sensor devices  
**Parameters:** Register address, data value, optional timeout  
**Returns:** Error code indicating success or failure

### 🔍 Device Management

```cpp
bool IsDevicePresent(hf*timeout*ms*t timeout*ms = HF*TIMEOUT*DEFAULT*MS) noexcept;
hf*u8*t GetDeviceAddress() const noexcept;
hf*frequency*hz*t GetClockSpeed() const noexcept;
```text
**Purpose:** 🔍 Device detection and configuration query  
**Returns:** Device presence, address, or clock speed information

## 💡 Usage Examples

### 🌡️ Temperature Sensor (LM75A)

```cpp
#include "inc/mcu/esp32/EspI2c.h"

class LM75ATemperatureSensor {
private:
    EspI2c sensor*;
    static constexpr hf*u8*t LM75A*ADDRESS = 0x48;
    static constexpr hf*u8*t TEMP*REGISTER = 0x00;
    static constexpr hf*u8*t CONFIG*REGISTER = 0x01;
    
public:
    LM75ATemperatureSensor() : sensor*(I2C*NUM*0, LM75A*ADDRESS, 400000) {}
    
    bool initialize() {
        // 🚀 Initialize I2C communication
        if (!sensor*.EnsureInitialized()) {
            printf("❌ Failed to initialize LM75A I2C\n");
            return false;
        }
        
        // 🔍 Check if device is present
        if (!sensor*.IsDevicePresent()) {
            printf("❌ LM75A not found at address 0x%02X\n", LM75A*ADDRESS);
            return false;
        }
        
        // ⚙️ Configure sensor for normal operation
        hf*i2c*err*t result = sensor*.WriteRegister(CONFIG*REGISTER, 0x00);
        if (result != hf*i2c*err*t::I2C*SUCCESS) {
            printf("❌ Failed to configure LM75A: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        printf("✅ LM75A temperature sensor initialized\n");
        return true;
    }
    
    float read*temperature() {
        // 📖 Read temperature register (2 bytes)
        hf*u8*t temp*data[2];
        hf*i2c*err*t result = sensor*.ReadRegisters(TEMP*REGISTER, temp*data, 2);
        
        if (result != hf*i2c*err*t::I2C*SUCCESS) {
            printf("❌ Failed to read temperature: %s\n", HfI2CErrToString(result).data());
            return -999.0f;  // Error value
        }
        
        // 🧮 Convert raw data to temperature (LM75A format: 9-bit, 0.5°C resolution)
        hf*i16*t raw*temp = (temp*data[0] << 8) | temp*data[1];
        raw*temp >>= 7;  // Shift to get 9-bit value
        
        float temperature = raw*temp * 0.5f;
        printf("🌡️ Temperature: %.1f°C\n", temperature);
        return temperature;
    }
    
    bool is*connected() {
        return sensor*.IsDevicePresent(100);  // 100ms timeout
    }
};
```text

### 📟 OLED Display (SSD1306)

```cpp
class SSD1306Display {
private:
    EspI2c display*;
    static constexpr hf*u8*t SSD1306*ADDRESS = 0x3C;
    static constexpr hf*u8*t COMMAND*MODE = 0x00;
    static constexpr hf*u8*t DATA*MODE = 0x40;
    
public:
    SSD1306Display() : display*(I2C*NUM*0, SSD1306*ADDRESS, 400000) {}
    
    bool initialize() {
        // 🚀 Initialize display I2C
        if (!display*.EnsureInitialized()) {
            printf("❌ Failed to initialize SSD1306 I2C\n");
            return false;
        }
        
        // 🔍 Check if display is connected
        if (!display*.IsDevicePresent()) {
            printf("❌ SSD1306 not found at address 0x%02X\n", SSD1306*ADDRESS);
            return false;
        }
        
        // 🎨 Initialize display with command sequence
        const hf*u8*t init*commands[] = {
            COMMAND*MODE,
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
        
        hf*i2c*err*t result = display*.Write(init*commands, sizeof(init*commands));
        if (result != hf*i2c*err*t::I2C*SUCCESS) {
            printf("❌ Failed to initialize SSD1306: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        printf("✅ SSD1306 OLED display initialized\n");
        return true;
    }
    
    void clear*display() {
        // 🧹 Clear display buffer
        hf*u8*t clear*data[129];  // Command byte + 128 data bytes
        clear*data[0] = DATA*MODE;
        for (int i = 1; i < 129; i++) {
            clear*data[i] = 0x00;
        }
        
        // 📝 Send clear command for each page (8 pages total)
        for (int page = 0; page < 8; page++) {
            // Set page address
            hf*u8*t page*cmd[] = {COMMAND*MODE, 0xB0 + page, 0x00, 0x10};
            display*.Write(page*cmd, sizeof(page*cmd));
            
            // Clear the page
            display*.Write(clear*data, sizeof(clear*data));
        }
        
        printf("🧹 Display cleared\n");
    }
    
    void write*pixel(hf*u8*t x, hf*u8*t y, bool on) {
        // 📍 Write single pixel (simplified implementation)
        if (x >= 128 || y >= 64) return;
        
        hf*u8*t page = y / 8;
        hf*u8*t bit = y % 8;
        
        // Set position
        hf*u8*t pos*cmd[] = {COMMAND*MODE, 0xB0 + page, x & 0x0F, 0x10 + (x >> 4)};
        display*.Write(pos*cmd, sizeof(pos*cmd));
        
        // Write pixel data
        hf*u8*t pixel*data[] = {DATA*MODE, on ? (1 << bit) : 0};
        display*.Write(pixel*data, sizeof(pixel*data));
    }
};
```text

### 💾 EEPROM Memory (24C256)

```cpp
class EEPROM24C256 {
private:
    EspI2c eeprom*;
    static constexpr hf*u8*t EEPROM*ADDRESS = 0x50;
    static constexpr hf*u16*t PAGE*SIZE = 64;     // 64-byte page size
    static constexpr hf*u16*t TOTAL*SIZE = 32768; // 32KB total
    
public:
    EEPROM24C256() : eeprom*(I2C*NUM*0, EEPROM*ADDRESS, 400000) {}
    
    bool initialize() {
        // 🚀 Initialize EEPROM I2C
        if (!eeprom*.EnsureInitialized()) {
            printf("❌ Failed to initialize EEPROM I2C\n");
            return false;
        }
        
        // 🔍 Test EEPROM presence by reading first byte
        if (!eeprom*.IsDevicePresent()) {
            printf("❌ EEPROM not found at address 0x%02X\n", EEPROM*ADDRESS);
            return false;
        }
        
        printf("✅ 24C256 EEPROM initialized (32KB)\n");
        return true;
    }
    
    bool write*byte(hf*u16*t address, hf*u8*t value) {
        if (address >= TOTAL*SIZE) {
            printf("❌ Address 0x%04X out of range\n", address);
            return false;
        }
        
        // 📝 Write single byte (address + data)
        hf*u8*t write*data[] = {
            static*cast<hf*u8*t>(address >> 8),   // High address byte
            static*cast<hf*u8*t>(address & 0xFF), // Low address byte
            value
        };
        
        hf*i2c*err*t result = eeprom*.Write(write*data, sizeof(write*data));
        if (result != hf*i2c*err*t::I2C*SUCCESS) {
            printf("❌ Failed to write EEPROM byte: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        // ⏰ Wait for write cycle to complete (typical 5ms)
        vTaskDelay(pdMS*TO*TICKS(10));
        return true;
    }
    
    bool read*byte(hf*u16*t address, hf*u8*t& value) {
        if (address >= TOTAL*SIZE) {
            printf("❌ Address 0x%04X out of range\n", address);
            return false;
        }
        
        // 📖 Set address then read data
        hf*u8*t addr*data[] = {
            static*cast<hf*u8*t>(address >> 8),   // High address byte
            static*cast<hf*u8*t>(address & 0xFF)  // Low address byte
        };
        
        hf*i2c*err*t result = eeprom*.WriteRead(addr*data, sizeof(addr*data), &value, 1);
        if (result != hf*i2c*err*t::I2C*SUCCESS) {
            printf("❌ Failed to read EEPROM byte: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        return true;
    }
    
    bool write*page(hf*u16*t start*address, const hf*u8*t* data, hf*u16*t length) {
        // ✅ Validate parameters
        if (start*address >= TOTAL*SIZE || length == 0 || length > PAGE*SIZE) {
            printf("❌ Invalid page write parameters\n");
            return false;
        }
        
        if ((start*address + length) > TOTAL*SIZE) {
            printf("❌ Page write would exceed EEPROM size\n");
            return false;
        }
        
        // 📄 Check page boundary alignment
        hf*u16*t page*start = start*address & ~(PAGE*SIZE - 1);
        if (start*address != page*start) {
            printf("⚠️ Warning: Write not page-aligned\n");
        }
        
        // 📝 Prepare write data (address + data)
        hf*u8*t write*buffer[PAGE*SIZE + 2];
        write*buffer[0] = static*cast<hf*u8*t>(start*address >> 8);
        write*buffer[1] = static*cast<hf*u8*t>(start*address & 0xFF);
        
        for (hf*u16*t i = 0; i < length; i++) {
            write*buffer[i + 2] = data[i];
        }
        
        hf*i2c*err*t result = eeprom*.Write(write*buffer, length + 2);
        if (result != hf*i2c*err*t::I2C*SUCCESS) {
            printf("❌ Failed to write EEPROM page: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        // ⏰ Wait for write cycle to complete
        vTaskDelay(pdMS*TO*TICKS(10));
        printf("✅ Wrote %u bytes to EEPROM at address 0x%04X\n", length, start*address);
        return true;
    }
    
    bool read*sequential(hf*u16*t start*address, hf*u8*t* data, hf*u16*t length) {
        if (start*address >= TOTAL*SIZE || length == 0) {
            return false;
        }
        
        if ((start*address + length) > TOTAL*SIZE) {
            length = TOTAL*SIZE - start*address;  // Clamp to available space
        }
        
        // 📖 Set starting address
        hf*u8*t addr*data[] = {
            static*cast<hf*u8*t>(start*address >> 8),
            static*cast<hf*u8*t>(start*address & 0xFF)
        };
        
        hf*i2c*err*t result = eeprom*.WriteRead(addr*data, sizeof(addr*data), data, length);
        if (result != hf*i2c*err*t::I2C*SUCCESS) {
            printf("❌ Failed to read EEPROM sequence: %s\n", HfI2CErrToString(result).data());
            return false;
        }
        
        printf("✅ Read %u bytes from EEPROM starting at 0x%04X\n", length, start*address);
        return true;
    }
};
```text

### 🔍 I2C Bus Scanner

```cpp
class I2CBusScanner {
private:
    EspI2c scanner*;
    static constexpr hf*u8*t SCAN*START = 0x08;
    static constexpr hf*u8*t SCAN*END = 0x77;
    
public:
    I2CBusScanner() : scanner*(I2C*NUM*0, SCAN*START, 100000) {}  // Any address for scanning
    
    bool initialize() {
        return scanner*.EnsureInitialized();
    }
    
    void scan*bus() {
        printf("🔍 Scanning I2C bus...\n");
        printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
        
        hf*u16*t found*count = 0;
        
        for (hf*u8*t row = 0; row < 8; row++) {
            printf("%02X: ", row * 16);
            
            for (hf*u8*t col = 0; col < 16; col++) {
                hf*u8*t address = row * 16 + col;
                
                if (address < SCAN*START || address > SCAN*END) {
                    printf("   ");
                    continue;
                }
                
                // 🔍 Test device presence at this address
                EspI2c test*device(I2C*NUM*0, address, 100000);
                test*device.EnsureInitialized();
                
                if (test*device.IsDevicePresent(50)) {  // 50ms timeout
                    printf("%02X ", address);
                    found*count++;
                } else {
                    printf("-- ");
                }
            }
            printf("\n");
        }
        
        printf("\n🎯 Found %u I2C device(s)\n", found*count);
        
        if (found*count > 0) {
            print*common*devices();
        }
    }
    
private:
    void print*common*devices() {
        printf("\n📋 Common I2C devices:\n");
        printf("   0x20-0x27: PCF8574 I/O Expander\n");
        printf("   0x3C, 0x3D: SSD1306 OLED Display\n");
        printf("   0x48-0x4F: LM75A Temperature Sensor\n");
        printf("   0x50-0x57: 24C256 EEPROM\n");
        printf("   0x68: DS1307 RTC, MPU6050 IMU\n");
        printf("   0x76, 0x77: BMP280 Pressure Sensor\n");
    }
};
```text

## 📊 Performance and Diagnostics

### 📈 Statistics Monitoring

```cpp
void monitor*i2c*performance(BaseI2c& device) {
    hf*i2c*statistics*t stats;
    hf*i2c*err*t result = device.GetStatistics(stats);
    
    if (result == hf*i2c*err*t::I2C*SUCCESS) {
        printf("📊 I2C Performance Statistics:\n");
        printf("   🔄 Total Transactions: %u\n", stats.total*transactions);
        printf("   ✅ Successful: %u (%.1f%%)\n", 
               stats.successful*transactions, 
               (float)stats.successful*transactions / stats.total*transactions * 100.0f);
        printf("   ❌ Failed: %u\n", stats.failed*transactions);
        printf("   ⏰ Timeouts: %u\n", stats.timeout*count);
        printf("   🚫 NACKs: %u\n", stats.nack*count);
        printf("   📤 Bytes TX: %u\n", stats.bytes*transmitted);
        printf("   📥 Bytes RX: %u\n", stats.bytes*received);
        printf("   ⏱️ Avg Time: %u μs\n", stats.average*transaction*time*us);
    }
}

void monitor*i2c*health(BaseI2c& device) {
    hf*i2c*diagnostics*t diag;
    hf*i2c*err*t result = device.GetDiagnostics(diag);
    
    if (result == hf*i2c*err*t::I2C*SUCCESS) {
        printf("🩺 I2C Bus Health:\n");
        printf("   🟢 Bus Healthy: %s\n", diag.bus*healthy ? "Yes" : "No");
        printf("   📡 SDA Line: %s\n", diag.sda*line*state ? "HIGH" : "LOW");
        printf("   🕐 SCL Line: %s\n", diag.scl*line*state ? "HIGH" : "LOW");
        printf("   🔒 Bus Locked: %s\n", diag.bus*locked ? "Yes" : "No");
        printf("   📈 Utilization: %.1f%%\n", diag.bus*utilization*percent);
        printf("   📟 Active Devices: %u\n", diag.active*device*count);
        printf("   🔄 Recovery Attempts: %u\n", diag.error*recovery*attempts);
        
        if (diag.consecutive*errors > 0) {
            printf("   ⚠️ Consecutive Errors: %u\n", diag.consecutive*errors);
            printf("   ⚠️ Last Error: %s\n", HfI2CErrToString(diag.last*error*code).data());
        }
    }
}
```text

## 🛡️ Error Handling Best Practices

### 🎯 Robust Communication

```cpp
hf*i2c*err*t safe*i2c*read*with*retry(BaseI2c& device, hf*u8*t reg, hf*u8*t& value) {
    const int max*retries = 3;
    int retry*count = 0;
    
    while (retry*count < max*retries) {
        hf*i2c*err*t result = device.ReadRegister(reg, value, 100);  // 100ms timeout
        
        switch (result) {
            case hf*i2c*err*t::I2C*SUCCESS:
                return result;  // Success!
                
            case hf*i2c*err*t::I2C*ERR*BUS*BUSY:
            case hf*i2c*err*t::I2C*ERR*TIMEOUT:
                // 🔄 Transient errors - retry after delay
                retry*count++;
                printf("⚠️ I2C busy/timeout, retry %d/%d\n", retry*count, max*retries);
                vTaskDelay(pdMS*TO*TICKS(10));
                break;
                
            case hf*i2c*err*t::I2C*ERR*DEVICE*NACK:
            case hf*i2c*err*t::I2C*ERR*DEVICE*NOT*RESPONDING:
                // 🔍 Check device presence
                if (!device.IsDevicePresent(50)) {
                    printf("❌ Device not present\n");
                    return result;
                }
                retry*count++;
                vTaskDelay(pdMS*TO*TICKS(5));
                break;
                
            default:
                // 💥 Permanent error
                printf("❌ I2C Error: %s\n", HfI2CErrToString(result).data());
                return result;
        }
    }
    
    printf("❌ I2C operation failed after %d retries\n", max*retries);
    return hf*i2c*err*t::I2C*ERR*TIMEOUT;
}
```text

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

The `BaseI2c` class is **not thread-safe**.
For concurrent access from multiple tasks, use appropriate synchronization mechanisms.

## 🔗 Related Documentation

- **[EspI2c API Reference](../esp_api/EspI2c.md)** - ESP32-C6 I2C implementation
- **[BaseGpio API Reference](BaseGpio.md)** - GPIO interface for I2C pins
- **[HardwareTypes Reference](HardwareTypes.md)** - Platform-agnostic type definitions

---

<div align="center">

**📋 Navigation**

[← Previous: BasePwm](BasePwm.md) | [Back to API Index](README.md) | [Next: BaseSpi →](BaseSpi.md)

</div>

**🚌 BaseI2c - Connecting the HardFOC Ecosystem** 🌐

*From sensors to displays - BaseI2c bridges the gap between devices* 🔗

</div> 