# 🎛️ BasePio API Reference

<div align="center">

![BasePio](https://img.shields.io/badge/BasePio-Abstract%20Base%20Class-blue?style=for-the-badge&logo=chip)

**⚡ Precise digital signal I/O for timing-critical operations**

**📋 Navigation**

[← Previous: BasePeriodicTimer](BasePeriodicTimer.md) | [Back to API Index](README.md) | [Next: API
Index](README.md)

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Class Hierarchy**](#-class-hierarchy)
- [📋 **Error Codes**](#-error-codes)
- [🔧 **Core API**](#-core-api)
- [📊 **Data Structures**](#-data-structures)
- [📊 **Usage Examples**](#-usage-examples)
- [🧪 **Best Practices**](#-best-practices)

---

## 🎯 **Overview**

The `BasePio` class provides a comprehensive abstraction for Programmable IO operations,
enabling precise timing control for digital signal generation and reception.
It's designed for timing-critical applications like WS2812 LED driving, IR communication,
stepper motor control, and custom protocols.

### ✨ **Key Features**

- ⚡ **Precise Timing** - Nanosecond resolution timing control
- 📊 **Buffered Operations** - Efficient symbol transmission and reception
- 🔄 **Asynchronous Operation** - Non-blocking with callback support
- 🎯 **Multi-Channel Support** - Simultaneous operation on multiple channels
- 🔧 **Flexible Configuration** - Configurable polarity, idle states, and timing
- 🛡️ **Robust Error Handling** - Comprehensive validation and error reporting
- 🏎️ **Performance Optimized** - Hardware-accelerated when available
- 🔌 **Platform Agnostic** - Works with various hardware backends

### 🎛️ **Supported Applications**

| Application | Description | Timing Requirements |

|-------------|-------------|-------------------|

| **WS2812 LEDs** | RGB LED strip control | 350ns/700ns pulses |

| **IR Communication** | Remote control protocols | 9-600μs pulses |

| **Stepper Motors** | Precise step timing | 1-100μs pulses |

| **Custom Protocols** | Proprietary signaling | Configurable timing |

| **PWM Generation** | High-frequency PWM | 1ns-1ms resolution |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BasePio {
        <<abstract>>
        +Initialize() hf*pio*err*t
        +Deinitialize() hf*pio*err*t
        +ConfigureChannel(channel*id, config) hf*pio*err*t
        +Transmit(channel*id, symbols, count) hf*pio*err*t
        +StartReceive(channel*id, buffer, size) hf*pio*err*t
        +StopReceive(channel*id, count) hf*pio*err*t
        +IsChannelBusy(channel*id) bool
        +GetChannelStatus(channel*id, status) hf*pio*err*t
        +GetCapabilities(capabilities) hf*pio*err*t
        +SetTransmitCallback(callback) void
        +SetReceiveCallback(callback) void
        +SetErrorCallback(callback) void
    }
    
    class EspPio {
        +EspPio(unit, channel)
        +GetUnit() rmt*channel*t
        +GetChannel() rmt*channel*t
    }
    
    BasePio <|-- EspPio
```text

---

## 📋 **Error Codes**

### ✅ **Success Codes**

| Code | Value | Description |

|------|-------|-------------|

| `PIO*SUCCESS` | 0 | ✅ Operation completed successfully |

### ❌ **General Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `PIO*ERR*FAILURE` | 1 | ❌ General operation failure | Check hardware and configuration |

| `PIO*ERR*NOT*INITIALIZED` | 2 | ⚠️ PIO not initialized | Call Initialize() first |

| `PIO*ERR*ALREADY*INITIALIZED` | 3 | ⚠️ PIO already initialized | Check initialization state |

| `PIO*ERR*INVALID*PARAMETER` | 4 | 🚫 Invalid parameter | Validate input parameters |

| `PIO*ERR*NULL*POINTER` | 5 | 🚫 Null pointer provided | Check pointer validity |

| `PIO*ERR*OUT*OF*MEMORY` | 6 | 💾 Memory allocation failed | Check system memory |

### 🔧 **Channel Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `PIO*ERR*INVALID*CHANNEL` | 7 | 🚫 Invalid PIO channel | Use valid channel numbers |

| `PIO*ERR*CHANNEL*BUSY` | 8 | 🔄 Channel already in use | Wait or use different channel |

| `PIO*ERR*CHANNEL*NOT*AVAILABLE` | 9 | ⚠️ Channel not available | Check channel availability |

| `PIO*ERR*INSUFFICIENT*CHANNELS` | 10 | 📊 Insufficient channels | Reduce channel count |

### ⏱️ **Timing Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `PIO*ERR*INVALID*RESOLUTION` | 11 | ⚙️ Invalid time resolution | Use supported resolution |

| `PIO*ERR*RESOLUTION*TOO*HIGH` | 12 | 📈 Resolution too high | Reduce resolution |

| `PIO*ERR*RESOLUTION*TOO*LOW` | 13 | 📉 Resolution too low | Increase resolution |

| `PIO*ERR*DURATION*TOO*LONG` | 14 | ⏰ Duration too long | Reduce duration |

| `PIO*ERR*DURATION*TOO*SHORT` | 15 | ⚡ Duration too short | Increase duration |

### 📊 **Buffer Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `PIO*ERR*BUFFER*OVERFLOW` | 16 | 📈 Buffer overflow | Increase buffer size |

| `PIO*ERR*BUFFER*UNDERFLOW` | 17 | 📉 Buffer underflow | Check data source |

| `PIO*ERR*BUFFER*TOO*SMALL` | 18 | 📏 Buffer too small | Increase buffer size |

| `PIO*ERR*BUFFER*TOO*LARGE` | 19 | 📐 Buffer too large | Reduce buffer size |

### 🌐 **Hardware Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `PIO*ERR*HARDWARE*FAULT` | 20 | 💥 Hardware fault | Check power and connections |

| `PIO*ERR*COMMUNICATION*TIMEOUT` | 21 | ⏰ Communication timeout | Check timing requirements |

| `PIO*ERR*COMMUNICATION*FAILURE` | 22 | 📡 Communication failure | Check bus connections |

| `PIO*ERR*DEVICE*NOT*RESPONDING` | 23 | 🔇 Device not responding | Check device power |

### ⚙️ **Configuration Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `PIO*ERR*INVALID*CONFIGURATION` | 24 | ⚙️ Invalid configuration | Check configuration parameters |

| `PIO*ERR*UNSUPPORTED*OPERATION` | 25 | 🚫 Unsupported operation | Check hardware capabilities |

| `PIO*ERR*PIN*CONFLICT` | 26 | 🔌 Pin already in use | Use different pin |

| `PIO*ERR*RESOURCE*BUSY` | 27 | 🔄 Resource busy | Wait for resource availability |

### 🔧 **System Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `PIO*ERR*SYSTEM*ERROR` | 28 | 💻 System error | Check system resources |

| `PIO*ERR*PERMISSION*DENIED` | 29 | 🚫 Permission denied | Check access permissions |

| `PIO*ERR*OPERATION*ABORTED` | 30 | ⏹️ Operation aborted | Check abort conditions |

---

## 🔧 **Core API**

### 🏗️ **Initialization Methods**

```cpp
/**
 * @brief Initialize the PIO peripheral
 * @return hf*pio*err*t error code
 * 
 * 📝 Sets up PIO hardware, configures channels, and prepares for operation.
 * Must be called before any PIO operations.
 * 
 * @example
 * EspPio pio(RMT*CHANNEL*0);
 * hf*pio*err*t result = pio.Initialize();
 * if (result == hf*pio*err*t::PIO*SUCCESS) {
 *     // PIO ready for use
 * }
 */
virtual hf*pio*err*t Initialize() noexcept = 0;

/**
 * @brief Deinitialize the PIO peripheral
 * @return hf*pio*err*t error code
 * 
 * 🧹 Cleanly shuts down PIO hardware and releases resources.
 */
virtual hf*pio*err*t Deinitialize() noexcept = 0;

/**
 * @brief Check if PIO is initialized
 * @return true if initialized, false otherwise
 * 
 * ❓ Query initialization status without side effects.
 */
[[nodiscard]] bool IsInitialized() const noexcept;

/**
 * @brief Ensure PIO is initialized (lazy initialization)
 * @return true if initialized successfully, false otherwise
 * 
 * 🔄 Automatically initializes PIO if not already initialized.
 */
bool EnsureInitialized() noexcept;
```text

### ⚙️ **Channel Configuration**

```cpp
/**
 * @brief Configure a PIO channel
 * @param channel*id Channel identifier
 * @param config Channel configuration structure
 * @return hf*pio*err*t error code
 * 
 * ⚙️ Configures channel parameters including GPIO pin, direction, timing,
 * polarity, and buffer settings.
 * 
 * @example
 * hf*pio*channel*config*t config;
 * config.gpio*pin = 18;
 * config.direction = hf*pio*direction*t::Transmit;
 * config.resolution*ns = 1000;  // 1μs resolution (will be adjusted to closest achievable)
 * config.polarity = hf*pio*polarity*t::Normal;
 * config.idle*state = hf*pio*idle*state*t::Low;
 * 
 * hf*pio*err*t result = pio.ConfigureChannel(0, config);
 */
virtual hf*pio*err*t ConfigureChannel(uint8*t channel*id,
                                    const hf*pio*channel*config*t &config) noexcept = 0;
```text

### 📤 **Transmission Methods**

```cpp
/**
 * @brief Transmit a sequence of symbols
 * @param channel*id Channel identifier
 * @param symbols Array of symbols to transmit
 * @param symbol*count Number of symbols in the array
 * @param wait*completion If true, block until transmission is complete
 * @return hf*pio*err*t error code
 * 
 * 📤 Transmits precise timing sequences. Each symbol defines duration
 * and signal level for precise waveform generation.
 * 
 * @example
 * hf*pio*symbol*t ws2812*data[] = {
 *     {350, true},   // 350ns high
 *     {800, false},  // 800ns low
 *     {700, true},   // 700ns high
 *     {600, false}   // 600ns low
 * };
 * 
 * hf*pio*err*t result = pio.Transmit(0, ws2812*data, 4, true);
 */
virtual hf*pio*err*t Transmit(uint8*t channel*id, const hf*pio*symbol*t *symbols,
                            size*t symbol*count, bool wait*completion = false) noexcept = 0;
```text

### 📥 **Reception Methods**

```cpp
/**
 * @brief Start receiving symbols
 * @param channel*id Channel identifier
 * @param buffer Buffer to store received symbols
 * @param buffer*size Size of the buffer
 * @param timeout*us Timeout in microseconds (0 = no timeout)
 * @return hf*pio*err*t error code
 * 
 * 📥 Begins asynchronous symbol reception. Received symbols are stored
 * in the provided buffer with precise timing information.
 * 
 * @example
 * hf*pio*symbol*t receive*buffer[64];
 * hf*pio*err*t result = pio.StartReceive(0, receive*buffer, 64, 10000);
 */
virtual hf*pio*err*t StartReceive(uint8*t channel*id, hf*pio*symbol*t *buffer, 
                                size*t buffer*size, uint32*t timeout*us = 0) noexcept = 0;

/**
 * @brief Stop receiving and get the number of symbols received
 * @param channel*id Channel identifier
 * @param symbols*received [out] Number of symbols actually received
 * @return hf*pio*err*t error code
 * 
 * ⏹️ Stops reception and returns the count of symbols received.
 */
virtual hf*pio*err*t StopReceive(uint8*t channel*id, size*t &symbols*received) noexcept = 0;
```text

### 📊 **Status and Capabilities**

```cpp
/**
 * @brief Check if a channel is currently busy
 * @param channel*id Channel identifier
 * @return true if channel is busy, false otherwise
 * 
 * ❓ Query channel busy status for flow control.
 */
virtual bool IsChannelBusy(uint8*t channel*id) const noexcept = 0;

/**
 * @brief Get channel status information
 * @param channel*id Channel identifier
 * @param status [out] Status information structure
 * @return hf*pio*err*t error code
 * 
 * 📊 Retrieves comprehensive status information about a channel.
 */
virtual hf*pio*err*t GetChannelStatus(uint8*t channel*id,
                                    hf*pio*channel*status*t &status) const noexcept = 0;

/**
 * @brief Get PIO capabilities
 * @param capabilities [out] Capability information structure
 * @return hf*pio*err*t error code
 * 
 * 📋 Retrieves hardware capabilities and limitations.
 */
virtual hf*pio*err*t GetCapabilities(hf*pio*capabilities*t &capabilities) const noexcept = 0;
```text

### 🔄 **Callback Management**

```cpp
/**
 * @brief Set callback for transmission complete events
 * @param callback Callback function
 * @param user*data User data to pass to callback
 * 
 * 🔔 Registers callback for transmission completion events.
 */
virtual void SetTransmitCallback(hf*pio*transmit*callback*t callback,
                               void *user*data = nullptr) noexcept = 0;

/**
 * @brief Set callback for reception complete events
 * @param callback Callback function
 * @param user*data User data to pass to callback
 * 
 * 🔔 Registers callback for reception completion events.
 */
virtual void SetReceiveCallback(hf*pio*receive*callback*t callback,
                              void *user*data = nullptr) noexcept = 0;

/**
 * @brief Set callback for error events
 * @param callback Callback function
 * @param user*data User data to pass to callback
 * 
 * 🔔 Registers callback for error events.
 */
virtual void SetErrorCallback(hf*pio*error*callback*t callback,
                            void *user*data = nullptr) noexcept = 0;

/**
 * @brief Clear all callbacks
 * 
 * 🧹 Removes all registered callbacks.
 */
virtual void ClearCallbacks() noexcept = 0;
```text

---

## 📊 **Data Structures**

### ⚙️ **Channel Configuration**

```cpp
struct hf*pio*channel*config*t {
    hf*pin*num*t gpio*pin;          ///< GPIO pin for PIO signal
    hf*pio*direction*t direction;   ///< Channel direction
    uint32*t resolution*ns;         ///< Time resolution in nanoseconds (user-friendly interface)
    hf*pio*polarity*t polarity;     ///< Signal polarity
    hf*pio*idle*state*t idle*state; ///< Idle state
    uint32*t timeout*us;            ///< Operation timeout in microseconds
    size*t buffer*size;             ///< Buffer size for symbols/durations
};
```text

### 📈 **PIO Symbol**

```cpp
struct hf*pio*symbol*t {
    uint32*t duration; ///< Duration in resolution units
    bool level;        ///< Signal level (true = high, false = low)
};
```text

### 📊 **Channel Status**

```cpp
struct hf*pio*channel*status*t {
    bool is*initialized;      ///< Channel is initialized
    bool is*busy;             ///< Channel is currently busy
    bool is*transmitting;     ///< Channel is transmitting
    bool is*receiving;        ///< Channel is receiving
    size*t symbols*queued;    ///< Number of symbols in queue
    size*t symbols*processed; ///< Number of symbols processed
    hf*pio*err*t last*error;  ///< Last error that occurred
    uint32*t timestamp*us;    ///< Timestamp of last operation
};
```text

### 📋 **PIO Capabilities**

```cpp
struct hf*pio*capabilities*t {
    uint8*t max*channels;        ///< Maximum number of channels
    uint32*t min*resolution*ns;  ///< Minimum time resolution
    uint32*t max*resolution*ns;  ///< Maximum time resolution
    uint32*t max*duration;       ///< Maximum single duration
    size*t max*buffer*size;      ///< Maximum buffer size
    bool supports*bidirectional; ///< Supports bidirectional mode
    bool supports*loopback;      ///< Supports loopback mode
    bool supports*carrier;       ///< Supports carrier modulation
};
```text

### 📈 **PIO Statistics**

```cpp
struct hf*pio*statistics*t {
    uint32*t totalTransmissions;    ///< Total transmissions performed
    uint32*t successfulTransmissions; ///< Successful transmissions
    uint32*t failedTransmissions;   ///< Failed transmissions
    uint32*t totalReceptions;       ///< Total receptions performed
    uint32*t successfulReceptions;  ///< Successful receptions
    uint32*t failedReceptions;      ///< Failed receptions
    uint32*t symbolsTransmitted;    ///< Total symbols transmitted
    uint32*t symbolsReceived;       ///< Total symbols received
    uint32*t averageTransmissionTimeUs; ///< Average transmission time
    uint32*t maxTransmissionTimeUs; ///< Maximum transmission time
    uint32*t minTransmissionTimeUs; ///< Minimum transmission time
    uint32*t timingErrors;          ///< Number of timing errors
    uint32*t bufferOverflows;       ///< Number of buffer overflows
};
```text

---

## 📊 **Usage Examples**

### 🎨 **WS2812 LED Control**

```cpp
#include "mcu/esp32/EspPio.h"

class WS2812Controller {
private:
    EspPio pio*;
    static constexpr uint32*t T0H*NS = 350;   // 0-bit high time
    static constexpr uint32*t T0L*NS = 800;   // 0-bit low time
    static constexpr uint32*t T1H*NS = 700;   // 1-bit high time
    static constexpr uint32*t T1L*NS = 600;   // 1-bit low time
    
public:
    bool initialize() {
        // Configure PIO channel for WS2812
        hf*pio*channel*config*t config;
        config.gpio*pin = 18;  // WS2812 data pin
        config.direction = hf*pio*direction*t::Transmit;
        config.resolution*ns = 1000;  // 1μs resolution (will be adjusted to closest achievable)
        config.polarity = hf*pio*polarity*t::Normal;
        config.idle*state = hf*pio*idle*state*t::Low;
        
        hf*pio*err*t result = pio*.ConfigureChannel(0, config);
        return (result == hf*pio*err*t::PIO*SUCCESS);
    }
    
    void send*color(uint8*t r, uint8*t g, uint8*t b) {
        // Convert RGB to GRB (WS2812 format)
        uint8*t grb[3] = {g, r, b};
        
        // Create symbol array for 24 bits
        hf*pio*symbol*t symbols[24];
        int symbol*index = 0;
        
        for (int i = 0; i < 3; i++) {
            for (int bit = 7; bit >= 0; bit--) {
                bool bit*value = (grb[i] >> bit) & 1;
                
                if (bit*value) {
                    // 1-bit: 700ns high, 600ns low
                    symbols[symbol*index++] = {7, true};   // 700ns high
                    symbols[symbol*index++] = {6, false};  // 600ns low
                } else {
                    // 0-bit: 350ns high, 800ns low
                    symbols[symbol*index++] = {4, true};   // 350ns high
                    symbols[symbol*index++] = {8, false};  // 800ns low
                }
            }
        }
        
        // Transmit the color data
        pio*.Transmit(0, symbols, 24, true);
    }
    
    void set*all*leds(uint8*t r, uint8*t g, uint8*t b, int count) {
        for (int i = 0; i < count; i++) {
            send*color(r, g, b);
            // Small delay between LEDs
            esp*rom*delay*us(50);
        }
    }
};
```text

### 📡 **IR Signal Transmission**

```cpp
#include "mcu/esp32/EspPio.h"

class IRTransmitter {
private:
    EspPio pio*;
    static constexpr uint32*t CARRIER*FREQ*KHZ = 38;  // 38kHz carrier
    static constexpr uint32*t CARRIER*PERIOD*NS = 26316;  // 1/38kHz
    
public:
    bool initialize() {
        hf*pio*channel*config*t config;
        config.gpio*pin = 4;  // IR LED pin
        config.direction = hf*pio*direction*t::Transmit;
        config.resolution*ns = 1000;  // 1μs resolution
        config.polarity = hf*pio*polarity*t::Normal;
        config.idle*state = hf*pio*idle*state*t::Low;
        
        return (pio*.ConfigureChannel(0, config) == hf*pio*err*t::PIO*SUCCESS);
    }
    
    void send*nec*code(uint32*t address, uint32*t command) {
        // NEC protocol: 9ms leader + 4.5ms space + address + command + stop
        std::vector<hf*pio*symbol*t> symbols;
        
        // 9ms leader pulse (9000μs high)
        symbols.push*back({9000, true});
        symbols.push*back({4500, false});  // 4.5ms space
        
        // Send address (LSB first)
        for (int i = 0; i < 16; i++) {
            bool bit = (address >> i) & 1;
            if (bit) {
                symbols.push*back({560, true});   // 560μs pulse
                symbols.push*back({1690, false}); // 1690μs space
            } else {
                symbols.push*back({560, true});   // 560μs pulse
                symbols.push*back({560, false});  // 560μs space
            }
        }
        
        // Send command (LSB first)
        for (int i = 0; i < 16; i++) {
            bool bit = (command >> i) & 1;
            if (bit) {
                symbols.push*back({560, true});   // 560μs pulse
                symbols.push*back({1690, false}); // 1690μs space
            } else {
                symbols.push*back({560, true});   // 560μs pulse
                symbols.push*back({560, false});  // 560μs space
            }
        }
        
        // Stop bit
        symbols.push*back({560, true});
        symbols.push*back({56000, false});  // 56ms space
        
        // Transmit the IR code
        pio*.Transmit(0, symbols.data(), symbols.size(), true);
    }
    
    void send*sony*code(uint32*t command, uint32*t address = 0) {
        // Sony SIRC protocol: 2.4ms leader + 12 bits data
        std::vector<hf*pio*symbol*t> symbols;
        
        // 2.4ms leader pulse
        symbols.push*back({2400, true});
        symbols.push*back({600, false});  // 600μs space
        
        // Send command (7 bits, MSB first)
        for (int i = 6; i >= 0; i--) {
            bool bit = (command >> i) & 1;
            symbols.push*back({600, true});  // 600μs pulse
            if (bit) {
                symbols.push*back({1200, false}); // 1200μs space
            } else {
                symbols.push*back({600, false});  // 600μs space
            }
        }
        
        // Send address (5 bits, MSB first)
        for (int i = 4; i >= 0; i--) {
            bool bit = (address >> i) & 1;
            symbols.push*back({600, true});  // 600μs pulse
            if (bit) {
                symbols.push*back({1200, false}); // 1200μs space
            } else {
                symbols.push*back({600, false});  // 600μs space
            }
        }
        
        // Transmit the IR code
        pio*.Transmit(0, symbols.data(), symbols.size(), true);
    }
};
```text

### 🔄 **Stepper Motor Control**

```cpp
#include "mcu/esp32/EspPio.h"

class StepperController {
private:
    EspPio pio*;
    static constexpr uint32*t STEP*PULSE*US = 10;  // 10μs step pulse
    static constexpr uint32*t STEP*DELAY*US = 1000; // 1ms between steps
    
public:
    bool initialize() {
        hf*pio*channel*config*t config;
        config.gpio*pin = 26;  // Step pin
        config.direction = hf*pio*direction*t::Transmit;
        config.resolution*ns = 1000;  // 1μs resolution
        config.polarity = hf*pio*polarity*t::Normal;
        config.idle*state = hf*pio*idle*state*t::Low;
        
        return (pio*.ConfigureChannel(0, config) == hf*pio*err*t::PIO*SUCCESS);
    }
    
    void step*single() {
        // Single step: 10μs high pulse
        hf*pio*symbol*t step*pulse[] = {
            {10, true},   // 10μs high
            {10, false}   // 10μs low
        };
        
        pio*.Transmit(0, step*pulse, 2, true);
    }
    
    void step*continuous(int steps, uint32*t delay*us) {
        // Generate continuous stepping pattern
        std::vector<hf*pio*symbol*t> symbols;
        
        for (int i = 0; i < steps; i++) {
            symbols.push*back({10, true});    // 10μs high
            symbols.push*back({delay*us, false}); // Delay between steps
        }
        
        pio*.Transmit(0, symbols.data(), symbols.size(), true);
    }
    
    void set*speed*rpm(float rpm, int steps*per*rev = 200) {
        // Calculate delay for desired RPM
        float steps*per*second = (rpm * steps*per*rev) / 60.0f;
        uint32*t delay*us = static*cast<uint32*t>(1000000.0f / steps*per*second);
        
        printf("Speed: %.1f RPM, Delay: %u μs\n", rpm, delay*us);
    }
};
```text

### 📡 **IR Signal Reception**

```cpp
#include "mcu/esp32/EspPio.h"

class IRReceiver {
private:
    EspPio pio*;
    hf*pio*symbol*t receive*buffer*[128];
    bool receiving* = false;
    
public:
    bool initialize() {
        hf*pio*channel*config*t config;
        config.gpio*pin = 5;  // IR receiver pin
        config.direction = hf*pio*direction*t::Receive;
        config.resolution*ns = 1000;  // 1μs resolution
        config.polarity = hf*pio*polarity*t::Normal;
        config.idle*state = hf*pio*idle*state*t::High;  // IR receivers idle high
        
        return (pio*.ConfigureChannel(0, config) == hf*pio*err*t::PIO*SUCCESS);
    }
    
    void start*receiving() {
        if (!receiving*) {
            hf*pio*err*t result = pio*.StartReceive(0, receive*buffer*, 128, 50000); // 50ms timeout
            if (result == hf*pio*err*t::PIO*SUCCESS) {
                receiving* = true;
                printf("🎯 Started IR reception\n");
            }
        }
    }
    
    void stop*receiving() {
        if (receiving*) {
            size*t symbols*received;
            hf*pio*err*t result = pio*.StopReceive(0, symbols*received);
            if (result == hf*pio*err*t::PIO*SUCCESS) {
                printf("📥 Received %zu symbols\n", symbols*received);
                process*ir*data(symbols*received);
            }
            receiving* = false;
        }
    }
    
private:
    void process*ir*data(size*t symbol*count) {
        if (symbol*count < 4) return;
        
        // Simple NEC protocol decoder
        uint32*t address = 0;
        uint32*t command = 0;
        
        // Check for NEC leader (9ms high, 4.5ms low)
        if (receive*buffer*[0].level && receive*buffer*[0].duration >= 8000 &&  // 8ms+ high
            !receive*buffer*[1].level && receive*buffer*[1].duration >= 4000) { // 4ms+ low
            
            printf("📡 NEC protocol detected\n");
            
            // Decode address and command (simplified)
            int bit*index = 0;
            for (size*t i = 2; i < symbol*count - 2; i += 2) {
                if (receive*buffer*[i].level && receive*buffer*[i].duration >= 500) {
                    // Valid pulse
                    if (receive*buffer*[i + 1].duration >= 1500) {
                        // Long space = 1 bit
                        if (bit*index < 16) {
                            address |= (1 << bit*index);
                        } else {
                            command |= (1 << (bit*index - 16));
                        }
                    }
                    bit*index++;
                }
            }
            
            printf("📋 Address: 0x%04X, Command: 0x%04X\n", address, command);
        }
    }
};
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Patterns**

```cpp
// ✅ Always check initialization
if (!pio.EnsureInitialized()) {
    printf("❌ PIO initialization failed\n");
    return false;
}

// ✅ Validate channel configuration
hf*pio*capabilities*t caps;
if (pio.GetCapabilities(caps) == hf*pio*err*t::PIO*SUCCESS) {
    if (channel*id >= caps.max*channels) {
        printf("❌ Channel %u exceeds maximum (%u)\n", channel*id, caps.max*channels);
        return;
    }
}

// ✅ Use appropriate timing resolution
uint32*t resolution*ns = 1000;  // 1μs for most applications
if (precise*timing*needed) {
    resolution*ns = 100;  // 100ns for precise timing (hardware permitting)
}

// ✅ Query actual achieved resolution (ESP32 specific)
uint32*t actual*resolution*ns;
if (pio.GetActualResolution(channel*id, actual*resolution*ns) == hf*pio*err*t::PIO*SUCCESS) {
    ESP*LOGI(TAG, "Requested: %uns, Achieved: %uns", resolution*ns, actual*resolution*ns);
}

// ✅ Check hardware constraints before configuration
uint32*t min*ns, max*ns, clock*hz;
if (pio.GetResolutionConstraints(min*ns, max*ns, clock*hz) == hf*pio*err*t::PIO*SUCCESS) {
    ESP*LOGI(TAG, "Hardware limits: %u-%uns with %u Hz clock", min*ns, max*ns, clock*hz);
}

// ✅ Handle transmission errors gracefully
hf*pio*err*t result = pio.Transmit(channel*id, symbols, count);
if (result != hf*pio*err*t::PIO*SUCCESS) {
    printf("⚠️ Transmission error: %s\n", HfPioErrToString(result));
    // Implement retry logic or error recovery
}

// ✅ Use callbacks for asynchronous operation
pio.SetTransmitCallback([](uint8*t ch, size*t sent, void* data) {
    printf("✅ Transmitted %zu symbols on channel %u\n", sent, ch);
});

// ✅ Monitor channel status
hf*pio*channel*status*t status;
if (pio.GetChannelStatus(channel*id, status) == hf*pio*err*t::PIO*SUCCESS) {
    if (status.is*busy) {
        printf("⏳ Channel %u is busy\n", channel*id);
    }
}
```text

### ❌ **Common Pitfalls**

```cpp
// ❌ Don't ignore timing requirements
// WS2812 requires precise 350ns/700ns timing
hf*pio*symbol*t wrong*timing[] = {
    {4, true},   // 400ns - too long!
    {8, false}   // 800ns - too long!
};

// ❌ Don't use invalid channel numbers
pio.ConfigureChannel(99, config);  // Invalid channel

// ❌ Don't ignore buffer size limits
hf*pio*symbol*t huge*buffer[10000];  // May exceed hardware limits

// ❌ Don't assume all protocols work the same
// Different IR protocols have different timing requirements

// ❌ Don't forget to stop reception
pio.StartReceive(0, buffer, 64);
// Missing: pio.StopReceive(0, count);
```text

### 🎯 **Performance Optimization**

```cpp
// 🚀 Use appropriate buffer sizes
size*t optimal*buffer*size = 64;  // Balance between memory and performance

// 🚀 Minimize symbol count for efficiency
// Combine similar symbols when possible
hf*pio*symbol*t optimized[] = {
    {1000, true},   // 1ms high
    {500, false}    // 500μs low
};
// Instead of: {100, true}, {100, true}, ..., {100, false}

// 🚀 Use hardware-accelerated timing when available
// ESP32 RMT provides precise timing without CPU intervention

// 🚀 Batch operations for multiple channels
// Configure all channels before starting transmission

// 🚀 Use appropriate idle states
// Match idle state to protocol requirements
```text

---

## 🔗 **Related Documentation**

- [⚙️ **EspPio**](../esp_api/EspPio.md) - ESP32-C6 implementation
- [🎛️ **Hardware Types**](HardwareTypes.md) - Platform-agnostic types

---

<div align="center">

**📋 Navigation**

[← Previous: BasePeriodicTimer](BasePeriodicTimer.md) | [Back to API Index](README.md) | [Next: API
Index](README.md)

</div>

---

<div align="center">

**🎛️ BasePio - Precision Digital Signal Control for HardFOC**

*Part of the HardFOC Internal Interface Wrapper Documentation*

</div> 