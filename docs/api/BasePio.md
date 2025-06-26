# 🎛️ BasePio API Documentation

## 📋 Overview

The `BasePio` class is an abstract base class that provides a unified, platform-agnostic interface for Programmable I/O operations in the HardFOC system. This class enables precise, hardware-accelerated digital signal generation and capture for protocols like WS2812 LEDs, IR remote control, 1-Wire, and custom communication protocols.

## 🏗️ Class Hierarchy

```
BasePio (Abstract Base Class)
    ├── McuPio (ESP32 RMT implementation)
    ├── RpiPio (Raspberry Pi Pico PIO implementation)
    ├── StmPio (STM32 Timer+DMA implementation)
    └── SfPio (Thread-safe wrapper)
```

## 🔧 Features

- ✅ **Hardware Acceleration**: Utilizes dedicated PIO peripherals (RMT, PIO, Timer+DMA)
- ✅ **Precise Timing**: Nanosecond-level timing resolution
- ✅ **Protocol Support**: WS2812, NeoPixel, IR remote, 1-Wire, DHT22, custom protocols
- ✅ **Buffered Operation**: Large symbol buffers for complex sequences
- ✅ **Bidirectional**: Both transmit and receive modes
- ✅ **Non-blocking**: Interrupt-driven with callback support
- ✅ **Flexible Configuration**: Configurable polarity, idle state, and timing
- ✅ **Error Detection**: Comprehensive error reporting and recovery

## 📊 Error Codes

| Error Code | Value | Description |
|------------|-------|-------------|
| `PIO_SUCCESS` | 0 | Operation successful |
| `PIO_ERR_NOT_INITIALIZED` | 2 | PIO not initialized |
| `PIO_ERR_INVALID_CHANNEL` | 7 | Invalid PIO channel number |
| `PIO_ERR_CHANNEL_BUSY` | 8 | Channel already in use |
| `PIO_ERR_INVALID_RESOLUTION` | 11 | Unsupported time resolution |
| `PIO_ERR_DURATION_TOO_LONG` | 14 | Symbol duration exceeds limits |
| `PIO_ERR_BUFFER_OVERFLOW` | 16 | Symbol buffer overflow |
| `PIO_ERR_HARDWARE_FAULT` | 20 | Hardware malfunction |
| `PIO_ERR_PIN_CONFLICT` | 26 | GPIO pin already in use |

*See header file for complete list of 31 error codes*

## 🏗️ Data Structures

### PioChannelConfig
Configuration structure for PIO channel setup:

```cpp
struct PioChannelConfig {
    hf_gpio_num_t gpio_pin;           // GPIO pin for PIO signal
    PioDirection direction;           // Transmit/Receive/Bidirectional
    uint32_t resolution_ns;           // Time resolution in nanoseconds
    PioPolarity polarity;             // Normal/Inverted polarity
    PioIdleState idle_state;          // Low/High idle state
    uint32_t timeout_us;              // Operation timeout in microseconds
    size_t buffer_size;               // Buffer size for symbols
};
```

### PioDirection
Channel operation direction:

```cpp
enum class PioDirection : uint8_t {
    Transmit = 0,     // Transmit mode (output)
    Receive = 1,      // Receive mode (input)
    Bidirectional = 2 // Bidirectional mode (if supported)
};
```

### PioSymbol
Precise timing symbol for signal generation:

```cpp
struct PioSymbol {
    uint32_t duration;                // Duration in resolution units
    bool level;                       // Signal level (true=high, false=low)
};
```

### PioPolarity & PioIdleState
Signal configuration enums:

```cpp
enum class PioPolarity : uint8_t {
    Normal = 0,       // Normal polarity (idle low, active high)
    Inverted = 1      // Inverted polarity (idle high, active low)
};

enum class PioIdleState : uint8_t {
    Low = 0,          // Idle state is low
    High = 1          // Idle state is high
};
```

## 🔨 Core Methods

### Initialization

#### `EnsureInitialized()`
```cpp
bool EnsureInitialized() noexcept
```
**Description**: Lazy initialization - initializes PIO controller on first call  
**Returns**: `true` if initialized successfully, `false` on failure  
**Thread-Safe**: Yes  

### Channel Management

#### `ConfigureChannel()`
```cpp
virtual HfPioErr ConfigureChannel(uint8_t channel, const PioChannelConfig& config) noexcept = 0
```
**Description**: Configure a PIO channel with specified settings  
**Parameters**:
- `channel`: Channel number (0-based)
- `config`: Channel configuration structure

**Returns**: `HfPioErr` result code  
**Thread-Safe**: Implementation dependent  

#### `EnableChannel()`
```cpp
virtual HfPioErr EnableChannel(uint8_t channel) noexcept = 0
```
**Description**: Enable PIO channel for operation  
**Parameters**:
- `channel`: Channel number to enable

**Returns**: `HfPioErr` result code  
**Thread-Safe**: Implementation dependent  

#### `DisableChannel()`
```cpp
virtual HfPioErr DisableChannel(uint8_t channel) noexcept = 0
```
**Description**: Disable PIO channel and stop operation  
**Parameters**:
- `channel`: Channel number to disable

**Returns**: `HfPioErr` result code  
**Thread-Safe**: Implementation dependent  

### Symbol Transmission

#### `TransmitSymbols()`
```cpp
virtual HfPioErr TransmitSymbols(uint8_t channel, const PioSymbol* symbols, 
                                size_t count, bool blocking = false) noexcept = 0
```
**Description**: Transmit an array of symbols with precise timing  
**Parameters**:
- `channel`: Channel number
- `symbols`: Array of symbols to transmit
- `count`: Number of symbols
- `blocking`: Wait for completion if true

**Returns**: `HfPioErr` result code  
**Thread-Safe**: Implementation dependent  

#### `TransmitPattern()`
```cpp
virtual HfPioErr TransmitPattern(uint8_t channel, const uint32_t* durations, 
                                const bool* levels, size_t count, bool blocking = false) noexcept = 0
```
**Description**: Transmit pattern using separate duration and level arrays  
**Parameters**:
- `channel`: Channel number
- `durations`: Array of durations in resolution units
- `levels`: Array of signal levels
- `count`: Number of elements
- `blocking`: Wait for completion if true

**Returns**: `HfPioErr` result code  
**Thread-Safe**: Implementation dependent  

### Symbol Reception

#### `ReceiveSymbols()`
```cpp
virtual HfPioErr ReceiveSymbols(uint8_t channel, PioSymbol* symbols, 
                               size_t max_count, size_t& received_count, 
                               uint32_t timeout_us = 0) noexcept = 0
```
**Description**: Receive symbols with precise timing measurement  
**Parameters**:
- `channel`: Channel number
- `symbols`: Buffer to store received symbols
- `max_count`: Maximum symbols to receive
- `received_count`: Reference to store actual count received
- `timeout_us`: Receive timeout in microseconds

**Returns**: `HfPioErr` result code  
**Thread-Safe**: Implementation dependent  

### Callback Management

#### `SetTransmitCompleteCallback()`
```cpp
virtual HfPioErr SetTransmitCompleteCallback(uint8_t channel, 
                                           PioTransmitCallback callback) noexcept = 0
```
**Description**: Set callback for transmission complete events  
**Parameters**:
- `channel`: Channel number
- `callback`: Function to call when transmission completes

**Returns**: `HfPioErr` result code  
**Thread-Safe**: Implementation dependent  

#### `SetReceiveCallback()`
```cpp
virtual HfPioErr SetReceiveCallback(uint8_t channel, 
                                  PioReceiveCallback callback) noexcept = 0
```
**Description**: Set callback for symbol reception events  
**Parameters**:
- `channel`: Channel number
- `callback`: Function to call when symbols are received

**Returns**: `HfPioErr` result code  
**Thread-Safe**: Implementation dependent  

## 💡 Usage Examples

### WS2812 LED Control (NeoPixel)
```cpp
#include "mcu/McuPio.h"

// Create PIO instance
auto pio = McuPio::Create();

// Configure for WS2812 timing (800kHz, 1.25μs period)
PioChannelConfig ws2812_config;
ws2812_config.gpio_pin = 8;              // Data pin
ws2812_config.direction = PioDirection::Transmit;
ws2812_config.resolution_ns = 50;        // 50ns resolution
ws2812_config.polarity = PioPolarity::Normal;
ws2812_config.idle_state = PioIdleState::Low;

if (pio->EnsureInitialized()) {
    pio->ConfigureChannel(0, ws2812_config);
    pio->EnableChannel(0);
}

// Create color data for 10 RGB LEDs
auto GenerateWS2812Data = [](uint8_t r, uint8_t g, uint8_t b) -> std::vector<PioSymbol> {
    std::vector<PioSymbol> symbols;
    
    // GRB order for WS2812
    uint32_t color = (g << 16) | (r << 8) | b;
    
    for (int bit = 23; bit >= 0; bit--) {
        if (color & (1 << bit)) {
            // '1' bit: 800ns high, 450ns low
            symbols.push_back({16, true});   // 800ns / 50ns = 16
            symbols.push_back({9, false});   // 450ns / 50ns = 9
        } else {
            // '0' bit: 400ns high, 850ns low
            symbols.push_back({8, true});    // 400ns / 50ns = 8
            symbols.push_back({17, false});  // 850ns / 50ns = 17
        }
    }
    
    return symbols;
};

// Send rainbow pattern to 10 LEDs
std::vector<PioSymbol> led_data;
for (int i = 0; i < 10; ++i) {
    uint8_t hue = (i * 255) / 10;
    auto color_symbols = GenerateWS2812Data(hue, 255 - hue, 128);
    led_data.insert(led_data.end(), color_symbols.begin(), color_symbols.end());
}

// Add reset pulse (>50μs low)
led_data.push_back({1000, false});  // 50μs / 50ns = 1000

pio->TransmitSymbols(0, led_data.data(), led_data.size(), true);
```

### IR Remote Control Transmission
```cpp
// Configure for IR remote control (38kHz carrier)
PioChannelConfig ir_config;
ir_config.gpio_pin = 19;
ir_config.direction = PioDirection::Transmit;
ir_config.resolution_ns = 500;           // 500ns resolution
ir_config.polarity = PioPolarity::Normal;
ir_config.idle_state = PioIdleState::Low;

pio->ConfigureChannel(1, ir_config);
pio->EnableChannel(1);

// NEC protocol: Send power button (0x00FF827D)
auto GenerateNECCommand = [](uint32_t command) -> std::vector<PioSymbol> {
    std::vector<PioSymbol> symbols;
    
    // Start burst: 9ms on, 4.5ms off
    symbols.push_back({18000, true});     // 9ms / 500ns = 18000
    symbols.push_back({9000, false});     // 4.5ms / 500ns = 9000
    
    // Send 32 bits
    for (int bit = 31; bit >= 0; bit--) {
        symbols.push_back({1120, true});   // 560μs / 500ns = 1120
        
        if (command & (1U << bit)) {
            symbols.push_back({3360, false}); // '1': 1.68ms space
        } else {
            symbols.push_back({1120, false}); // '0': 560μs space
        }
    }
    
    // Stop bit
    symbols.push_back({1120, true});
    
    return symbols;
};

auto ir_symbols = GenerateNECCommand(0x00FF827D);
pio->TransmitSymbols(1, ir_symbols.data(), ir_symbols.size(), false);
```

### DHT22 Temperature Sensor Reading
```cpp
// Configure for DHT22 bidirectional communication
PioChannelConfig dht_config;
dht_config.gpio_pin = 22;
dht_config.direction = PioDirection::Bidirectional;
dht_config.resolution_ns = 1000;         // 1μs resolution
dht_config.timeout_us = 5000;            // 5ms timeout

pio->ConfigureChannel(2, dht_config);
pio->EnableChannel(2);

// DHT22 start sequence: 1ms low, 30μs high
std::vector<PioSymbol> start_sequence = {
    {1000, false},  // 1ms low
    {30, true}      // 30μs high
};

// Send start sequence and switch to receive mode
pio->TransmitSymbols(2, start_sequence.data(), start_sequence.size(), true);

// Receive response
PioSymbol received_symbols[84];  // Max 84 symbols for DHT22 response
size_t received_count = 0;

HfPioErr result = pio->ReceiveSymbols(2, received_symbols, 84, received_count, 5000);
if (result == HfPioErr::PIO_SUCCESS) {
    // Parse DHT22 data
    printf("DHT22 received %zu symbols\n", received_count);
    
    // Extract humidity and temperature from symbols
    // ... parsing logic here ...
}
```

### Custom Protocol with Callbacks
```cpp
// Set up callbacks for non-blocking operation
pio->SetTransmitCompleteCallback(0, [](uint8_t channel, bool success) {
    if (success) {
        printf("Channel %d transmission completed successfully\n", channel);
    } else {
        printf("Channel %d transmission failed\n", channel);
    }
});

pio->SetReceiveCallback(1, [](uint8_t channel, const PioSymbol* symbols, size_t count) {
    printf("Channel %d received %zu symbols:\n", channel, count);
    for (size_t i = 0; i < count; ++i) {
        printf("  Symbol %zu: %u units, %s\n", i, symbols[i].duration, 
               symbols[i].level ? "HIGH" : "LOW");
    }
});

// Start non-blocking reception
pio->ReceiveSymbols(1, nullptr, 0, received_count, 0);  // Continuous receive
```

### Precise Timing Validation
```cpp
// Test timing accuracy
auto TestTiming = [&pio](uint32_t duration_us) {
    PioSymbol test_symbol = {duration_us * 1000, true};  // Convert to ns
    
    auto start_time = esp_timer_get_time();
    pio->TransmitSymbols(0, &test_symbol, 1, true);
    auto end_time = esp_timer_get_time();
    
    uint32_t actual_duration = end_time - start_time;
    printf("Expected: %u μs, Actual: %u μs, Error: %d μs\n", 
           duration_us, actual_duration, (int)(actual_duration - duration_us));
};

// Test various durations
TestTiming(10);    // 10μs
TestTiming(100);   // 100μs
TestTiming(1000);  // 1ms
TestTiming(10000); // 10ms
```

### Error Handling and Recovery
```cpp
HfPioErr result = pio->TransmitSymbols(0, symbols, count, false);
if (result != HfPioErr::PIO_SUCCESS) {
    printf("PIO Error: %s\n", HfPioErrToString(result).data());
    
    switch (result) {
        case HfPioErr::PIO_ERR_CHANNEL_BUSY:
            printf("Channel is busy, waiting...\n");
            vTaskDelay(pdMS_TO_TICKS(10));
            // Retry operation
            break;
            
        case HfPioErr::PIO_ERR_BUFFER_OVERFLOW:
            printf("Buffer too small, reducing symbol count\n");
            // Reduce symbol count and retry
            break;
            
        case HfPioErr::PIO_ERR_HARDWARE_FAULT:
            printf("Hardware fault, reinitializing...\n");
            pio->DisableChannel(0);
            pio->ConfigureChannel(0, config);
            pio->EnableChannel(0);
            break;
            
        default:
            printf("Unhandled error occurred\n");
            break;
    }
}
```

## 🧪 Testing

The BasePio class can be tested using:

```cpp
#include "tests/PioTests.h"

// Run comprehensive PIO tests
bool success = TestPioFunctionality();
```

## ⚠️ Important Notes

1. **Abstract Class**: Cannot be instantiated directly - use concrete implementations
2. **Hardware Dependent**: Timing resolution and capabilities vary by platform
3. **Interrupt Context**: Callbacks execute in interrupt context - keep them short
4. **Buffer Management**: Large symbol buffers consume significant memory
5. **Pin Conflicts**: Ensure GPIO pins are not used by other peripherals
6. **Timing Precision**: Actual timing depends on system clock and hardware limits
7. **Resource Limits**: Number of channels limited by hardware (typically 2-8)

## 🔗 Related Classes

- [`BaseGpio`](BaseGpio.md) - GPIO interface for pin control
- [`BasePwm`](BasePwm.md) - PWM interface for simpler timing needs
- [`McuPio`](../mcu/McuPio.md) - ESP32 RMT implementation
- [`SfPio`](../thread_safe/SfPio.md) - Thread-safe wrapper

## 📝 See Also

- [HardFOC PIO Architecture](../guides/pio-architecture.md)
- [WS2812 LED Control Guide](../guides/ws2812-control.md)
- [IR Remote Control Guide](../guides/ir-remote-control.md)
- [Custom Protocol Development](../guides/custom-protocols.md)
