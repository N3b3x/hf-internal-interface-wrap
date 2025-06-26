# 📡 BaseUartDriver API Documentation

## 📋 Overview

The `BaseUartDriver` class is an abstract base class that provides a unified, platform-agnostic interface for UART (Universal Asynchronous Receiver-Transmitter) communication in the HardFOC system. This class enables serial communication with external devices, debugging interfaces, wireless modules, and other UART-based peripherals.

## 🏗️ Class Hierarchy

```
BaseUartDriver (Abstract Base Class)
    ├── McuUartDriver (ESP32 UART implementation)
    ├── StmUartDriver (STM32 UART implementation)
    ├── VirtualUartDriver (USB CDC/Virtual COM port)
    └── SfUartDriver (Thread-safe wrapper)
```

## 🔧 Features

- ✅ **High Baud Rates**: Up to 5Mbps depending on hardware
- ✅ **Flexible Configuration**: Data bits (5-8), parity (none/even/odd), stop bits (1-2)
- ✅ **Hardware Flow Control**: RTS/CTS for reliable high-speed communication
- ✅ **Buffered I/O**: Configurable TX/RX buffers with interrupt handling
- ✅ **Non-blocking Operations**: Asynchronous send/receive with callbacks
- ✅ **Printf Support**: Formatted output for debugging and logging
- ✅ **Error Detection**: Frame, parity, overrun, and noise error detection
- ✅ **Multiple Protocols**: Support for various serial protocols

## 📊 Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| `UART_SUCCESS` | 0 | Operation successful |
| `UART_ERR_NOT_INITIALIZED` | 2 | UART not initialized |
| `UART_ERR_TIMEOUT` | 7 | Operation timed out |
| `UART_ERR_BUFFER_FULL` | 8 | Transmit buffer is full |
| `UART_ERR_BUFFER_EMPTY` | 9 | Receive buffer is empty |
| `UART_ERR_FRAME_ERROR` | 12 | Frame error detected |
| `UART_ERR_PARITY_ERROR` | 13 | Parity error detected |
| `UART_ERR_OVERRUN_ERROR` | 14 | Data overrun error |
| `UART_ERR_INVALID_BAUD_RATE` | 23 | Unsupported baud rate |
| `UART_ERR_FLOW_CONTROL_ERROR` | 28 | Flow control error |

*See header file for complete list of 32 error codes*

## 🏗️ Data Structures

### UartConfig
Configuration structure for UART setup:

```cpp
struct UartConfig {
    uint32_t baud_rate;                      // Baud rate (bits per second)
    uint8_t data_bits;                       // Data bits (5-8, typically 8)
    uint8_t parity;                          // Parity: 0=None, 1=Even, 2=Odd
    uint8_t stop_bits;                       // Stop bits (1-2, typically 1)
    bool use_hardware_flow_control;          // Enable RTS/CTS flow control
    hf_gpio_num_t tx_pin;                    // TX (transmit) pin
    hf_gpio_num_t rx_pin;                    // RX (receive) pin
    hf_gpio_num_t rts_pin;                   // RTS pin (if flow control enabled)
    hf_gpio_num_t cts_pin;                   // CTS pin (if flow control enabled)
    uint16_t tx_buffer_size;                 // TX buffer size in bytes
    uint16_t rx_buffer_size;                 // RX buffer size in bytes
    uint32_t timeout_ms;                     // Default operation timeout
};
```

### Common UART Configurations
Standard configurations for typical use cases:

| Use Case | Baud Rate | Data | Parity | Stop | Flow Control |
|----------|-----------|------|--------|------|--------------|
| Debug Console | 115200 | 8 | None | 1 | No |
| GPS Module | 9600 | 8 | None | 1 | No |
| Bluetooth Module | 38400 | 8 | None | 1 | No |
| High-Speed Data | 921600 | 8 | None | 1 | Yes |
| Industrial RS485 | 19200 | 8 | Even | 1 | No |
| Modbus RTU | 9600 | 8 | Even | 1 | No |

### UartStatus
Real-time UART status information:

```cpp
struct UartStatus {
    bool is_initialized;                     // UART is initialized
    uint32_t tx_bytes_sent;                  // Total bytes transmitted
    uint32_t rx_bytes_received;              // Total bytes received
    uint16_t tx_buffer_free;                 // Free space in TX buffer
    uint16_t rx_buffer_used;                 // Used space in RX buffer
    uint32_t frame_errors;                   // Frame error count
    uint32_t parity_errors;                  // Parity error count
    uint32_t overrun_errors;                 // Overrun error count
    bool cts_state;                          // Current CTS signal state
    bool rts_state;                          // Current RTS signal state
};
```

## 🔨 Core Methods

### Initialization

#### `EnsureInitialized()`
```cpp
bool EnsureInitialized() noexcept
```
**Description**: Lazy initialization - initializes UART on first call  
**Returns**: `true` if initialized successfully, `false` on failure  
**Thread-Safe**: Yes  

#### `IsInitialized()`
```cpp
bool IsInitialized() const noexcept
```
**Description**: Check if UART is initialized  
**Returns**: `true` if initialized, `false` otherwise  
**Thread-Safe**: Yes  

### Data Transmission

#### `Write()`
```cpp
virtual HfUartErr Write(const uint8_t* data, size_t length, 
                       uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Write data to UART  
**Parameters**:
- `data`: Data buffer to transmit
- `length`: Number of bytes to write
- `timeout_ms`: Timeout in milliseconds (0 = use default)

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

#### `WriteString()`
```cpp
virtual HfUartErr WriteString(const char* str, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Write null-terminated string to UART  
**Parameters**:
- `str`: Null-terminated string to transmit
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

#### `Printf()`
```cpp
virtual HfUartErr Printf(const char* format, ...) noexcept = 0
```
**Description**: Write formatted string to UART (printf-style)  
**Parameters**:
- `format`: Printf-style format string
- `...`: Variable arguments for formatting

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

### Data Reception

#### `Read()`
```cpp
virtual HfUartErr Read(uint8_t* data, size_t max_length, 
                      size_t& bytes_read, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Read data from UART  
**Parameters**:
- `data`: Buffer to store received data
- `max_length`: Maximum bytes to read
- `bytes_read`: Reference to store actual bytes read
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

#### `ReadLine()`
```cpp
virtual HfUartErr ReadLine(char* buffer, size_t buffer_size, 
                          size_t& line_length, uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Read a line of text (until newline or timeout)  
**Parameters**:
- `buffer`: Buffer to store the line
- `buffer_size`: Size of the buffer
- `line_length`: Reference to store actual line length
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

#### `Available()`
```cpp
virtual size_t Available() noexcept = 0
```
**Description**: Get number of bytes available for reading  
**Returns**: Number of bytes in receive buffer  
**Thread-Safe**: Yes  

### Buffer Management

#### `FlushTx()`
```cpp
virtual HfUartErr FlushTx(uint32_t timeout_ms = 0) noexcept = 0
```
**Description**: Wait for all transmitted data to be sent  
**Parameters**:
- `timeout_ms`: Timeout in milliseconds

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

#### `FlushRx()`
```cpp
virtual HfUartErr FlushRx() noexcept = 0
```
**Description**: Clear the receive buffer  
**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

### Status and Control

#### `GetStatus()`
```cpp
virtual HfUartErr GetStatus(UartStatus& status) noexcept = 0
```
**Description**: Get comprehensive UART status information  
**Parameters**:
- `status`: Reference to store status information

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Yes  

#### `SetBaudRate()`
```cpp
virtual HfUartErr SetBaudRate(uint32_t baud_rate) noexcept = 0
```
**Description**: Change UART baud rate dynamically  
**Parameters**:
- `baud_rate`: New baud rate in bits per second

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

### Callback Management

#### `SetReceiveCallback()`
```cpp
virtual HfUartErr SetReceiveCallback(UartReceiveCallback callback) noexcept = 0
```
**Description**: Set callback for data reception events  
**Parameters**:
- `callback`: Function to call when data is received

**Returns**: `HfUartErr` result code  
**Thread-Safe**: Implementation dependent  

## 💡 Usage Examples

### Basic UART Communication
```cpp
#include "mcu/McuUartDriver.h"

// Create UART instance
auto uart = McuUartDriver::Create();

// Configure UART for debug console
UartConfig config;
config.baud_rate = 115200;
config.data_bits = 8;
config.parity = 0;                    // No parity
config.stop_bits = 1;
config.tx_pin = 1;                    // GPIO 1 (U0TXD)
config.rx_pin = 3;                    // GPIO 3 (U0RXD)
config.tx_buffer_size = 512;
config.rx_buffer_size = 512;

// Initialize
if (!uart->Configure(config) || !uart->EnsureInitialized()) {
    printf("Failed to initialize UART\n");
    return;
}

// Send data
const char* message = "Hello, UART!\n";
HfUartErr result = uart->WriteString(message);
if (result == HfUartErr::UART_SUCCESS) {
    printf("Message sent successfully\n");
}

// Read response
char response[64];
size_t bytes_read;
result = uart->Read((uint8_t*)response, sizeof(response) - 1, bytes_read, 1000);
if (result == HfUartErr::UART_SUCCESS && bytes_read > 0) {
    response[bytes_read] = '\0';  // Null terminate
    printf("Received: %s\n", response);
}
```

### GPS Module Communication
```cpp
// Configure for GPS module (typical 9600 baud)
UartConfig gps_config;
gps_config.baud_rate = 9600;
gps_config.tx_pin = 17;
gps_config.rx_pin = 16;
gps_config.rx_buffer_size = 1024;     // Larger buffer for GPS sentences

uart->Configure(gps_config);
uart->EnsureInitialized();

// Read NMEA sentences
while (true) {
    char nmea_line[128];
    size_t line_length;
    
    HfUartErr result = uart->ReadLine(nmea_line, sizeof(nmea_line), line_length, 5000);
    if (result == HfUartErr::UART_SUCCESS) {
        printf("GPS: %s\n", nmea_line);
        
        // Parse NMEA sentence
        if (strncmp(nmea_line, "$GPGGA", 6) == 0) {
            // Parse GGA sentence for position
            printf("Position data received\n");
        }
    } else if (result == HfUartErr::UART_ERR_TIMEOUT) {
        printf("No GPS data received\n");
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
}
```

### Bluetooth Module AT Commands
```cpp
// Configure for Bluetooth module
UartConfig bt_config;
bt_config.baud_rate = 38400;
bt_config.tx_pin = 4;
bt_config.rx_pin = 5;

uart->Configure(bt_config);
uart->EnsureInitialized();

// Send AT command and wait for response
auto SendATCommand = [&uart](const char* command) -> bool {
    // Send command
    uart->Printf("%s\r\n", command);
    
    // Wait for response
    char response[64];
    size_t bytes_read;
    HfUartErr result = uart->Read((uint8_t*)response, sizeof(response) - 1, bytes_read, 2000);
    
    if (result == HfUartErr::UART_SUCCESS && bytes_read > 0) {
        response[bytes_read] = '\0';
        printf("BT Response: %s\n", response);
        return strstr(response, "OK") != nullptr;
    }
    return false;
};

// Initialize Bluetooth module
if (SendATCommand("AT")) {
    printf("Bluetooth module responding\n");
    SendATCommand("AT+NAME=HardFOC_Device");
    SendATCommand("AT+BAUD8");  // Set to 115200 baud
    
    // Update UART to new baud rate
    uart->SetBaudRate(115200);
}
```

### High-Speed Data Transfer with Flow Control
```cpp
// Configure for high-speed transfer with hardware flow control
UartConfig high_speed_config;
high_speed_config.baud_rate = 921600;
high_speed_config.use_hardware_flow_control = true;
high_speed_config.tx_pin = 4;
high_speed_config.rx_pin = 5;
high_speed_config.rts_pin = 18;
high_speed_config.cts_pin = 19;
high_speed_config.tx_buffer_size = 2048;
high_speed_config.rx_buffer_size = 2048;

uart->Configure(high_speed_config);
uart->EnsureInitialized();

// Send large data block
std::vector<uint8_t> data(1024);
std::iota(data.begin(), data.end(), 0);  // Fill with 0-255 pattern

auto start_time = esp_timer_get_time();
HfUartErr result = uart->Write(data.data(), data.size(), 5000);
auto end_time = esp_timer_get_time();

if (result == HfUartErr::UART_SUCCESS) {
    uint32_t transfer_time = end_time - start_time;
    float transfer_rate = (data.size() * 8.0f * 1000000.0f) / transfer_time;
    printf("Transferred %zu bytes in %u μs (%.2f kbps)\n", 
           data.size(), transfer_time, transfer_rate / 1000.0f);
}
```

### Callback-Based Reception
```cpp
// Set up receive callback for continuous monitoring
uart->SetReceiveCallback([](const uint8_t* data, size_t length) {
    printf("Received %zu bytes: ", length);
    for (size_t i = 0; i < length; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
    
    // Process received data
    // (Note: Keep callback short - runs in interrupt context)
});

// Main loop can focus on other tasks
while (true) {
    // Do other work while UART receives data in background
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Check UART status periodically
    UartStatus status;
    uart->GetStatus(status);
    printf("RX Buffer: %u/%u bytes, Errors: %u frame, %u parity, %u overrun\n",
           status.rx_buffer_used, status.rx_buffer_used + status.rx_buffer_free,
           status.frame_errors, status.parity_errors, status.overrun_errors);
}
```

### Protocol Implementation (Modbus RTU)
```cpp
// Configure for Modbus RTU
UartConfig modbus_config;
modbus_config.baud_rate = 9600;
modbus_config.data_bits = 8;
modbus_config.parity = 2;             // Even parity
modbus_config.stop_bits = 1;
modbus_config.tx_pin = 17;
modbus_config.rx_pin = 16;

uart->Configure(modbus_config);
uart->EnsureInitialized();

// Modbus CRC calculation
auto CalculateModbusCRC = [](const uint8_t* data, size_t length) -> uint16_t {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
};

// Send Modbus read holding registers command
auto SendModbusRead = [&](uint8_t slave_id, uint16_t start_addr, uint16_t count) {
    uint8_t frame[8];
    frame[0] = slave_id;
    frame[1] = 0x03;                  // Read holding registers
    frame[2] = start_addr >> 8;
    frame[3] = start_addr & 0xFF;
    frame[4] = count >> 8;
    frame[5] = count & 0xFF;
    
    uint16_t crc = CalculateModbusCRC(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;
    
    uart->Write(frame, 8, 1000);
    
    // Wait for response
    uint8_t response[256];
    size_t bytes_read;
    HfUartErr result = uart->Read(response, sizeof(response), bytes_read, 2000);
    
    if (result == HfUartErr::UART_SUCCESS && bytes_read >= 5) {
        // Verify CRC and process response
        uint16_t received_crc = response[bytes_read - 1] << 8 | response[bytes_read - 2];
        uint16_t calculated_crc = CalculateModbusCRC(response, bytes_read - 2);
        
        if (received_crc == calculated_crc) {
            printf("Modbus response received: %u bytes\n", (unsigned)bytes_read);
            // Process register data...
        } else {
            printf("Modbus CRC error\n");
        }
    }
};

// Read registers from slave device
SendModbusRead(1, 0, 10);  // Read 10 registers starting from address 0
```

### Error Handling and Recovery
```cpp
HfUartErr result = uart->Write(data, length);
if (result != HfUartErr::UART_SUCCESS) {
    printf("UART Error: %s\n", HfUartErrToString(result).data());
    
    switch (result) {
        case HfUartErr::UART_ERR_BUFFER_FULL:
            printf("TX buffer full, waiting...\n");
            uart->FlushTx(5000);  // Wait for buffer to empty
            // Retry operation
            break;
            
        case HfUartErr::UART_ERR_TIMEOUT:
            printf("Operation timed out\n");
            uart->FlushRx();      // Clear stale data
            break;
            
        case HfUartErr::UART_ERR_FRAME_ERROR:
            printf("Frame error - check baud rate and connections\n");
            // Log error for diagnostics
            break;
            
        case HfUartErr::UART_ERR_OVERRUN_ERROR:
            printf("Data overrun - increase buffer size or read more frequently\n");
            uart->FlushRx();
            break;
            
        default:
            printf("Unhandled UART error\n");
            break;
    }
}
```

## 🧪 Testing

The BaseUartDriver class can be tested using:

```cpp
#include "tests/UartTests.h"

// Run comprehensive UART tests
bool success = TestUartFunctionality();
```

## ⚠️ Important Notes

1. **Abstract Class**: Cannot be instantiated directly - use concrete implementations
2. **Thread Safety**: Depends on concrete implementation (use SfUartDriver for thread safety)
3. **Buffer Overflow**: Monitor buffer usage to prevent data loss
4. **Baud Rate Accuracy**: Higher baud rates may have more timing sensitivity
5. **Flow Control**: Use hardware flow control for reliable high-speed communication
6. **Pin Conflicts**: Ensure UART pins don't conflict with other peripherals
7. **Interrupt Context**: Callbacks execute in interrupt context - keep them short

## 🔗 Related Classes

- [`BaseI2cBus`](BaseI2cBus.md) - I2C interface for different communication needs
- [`BaseSpiBus`](BaseSpiBus.md) - SPI interface for high-speed synchronous communication
- [`McuUartDriver`](../mcu/McuUartDriver.md) - ESP32 UART implementation
- [`SfUartDriver`](../thread_safe/SfUartDriver.md) - Thread-safe wrapper

## 📝 See Also

- [HardFOC UART Architecture](../guides/uart-architecture.md)
- [Serial Protocol Implementation Guide](../guides/serial-protocols.md)
- [UART Debugging and Troubleshooting](../guides/uart-debugging.md)
