# EspPio Class API Reference

## Overview

The `EspPio` class provides a comprehensive C++ wrapper for the ESP-IDF v5.5 RMT (Remote Control
Transceiver) driver, enabling precise timing control for digital signal generation and capture
across all ESP32 variants.

## Class Declaration

```cpp
class EspPio : public BasePio {
public:
    EspPio() noexcept;
    ~EspPio() noexcept override;
    
    // BasePio interface implementation
    hf*pio*err*t Initialize() noexcept override;
    hf*pio*err*t Deinitialize() noexcept override;
    
    hf*pio*err*t ConfigureChannel(hf*u8*t channel*id,
                                  const hf*pio*channel*config*t& config) noexcept override;
    
    hf*pio*err*t Transmit(hf*u8*t channel*id, const hf*pio*symbol*t* symbols, 
                          size*t symbol*count, bool wait*completion = false) noexcept override;
    
    hf*pio*err*t StartReceive(hf*u8*t channel*id, hf*pio*symbol*t* buffer, 
                              size*t buffer*size, uint32*t timeout*us = 0) noexcept override;
    hf*pio*err*t StopReceive(hf*u8*t channel*id, size*t& symbols*received) noexcept override;
    
    bool IsChannelBusy(hf*u8*t channel*id) const noexcept override;
    hf*pio*err*t GetChannelStatus(hf*u8*t channel*id,
                                  hf*pio*channel*status*t& status) const noexcept override;
    hf*pio*err*t GetCapabilities(hf*pio*capabilities*t& capabilities) const noexcept override;
    
    // Channel-specific callback management
    void SetTransmitCallback(hf*u8*t channel*id, hf*pio*transmit*callback*t callback,
                            void* user*data = nullptr) noexcept override;
    void SetReceiveCallback(hf*u8*t channel*id, hf*pio*receive*callback*t callback,
                           void* user*data = nullptr) noexcept override;
    void SetErrorCallback(hf*u8*t channel*id, hf*pio*error*callback*t callback,
                         void* user*data = nullptr) noexcept override;
    void ClearChannelCallbacks(hf*u8*t channel*id) noexcept override;
    void ClearCallbacks() noexcept override;
    
    // Statistics and diagnostics
    hf*pio*err*t GetStatistics(hf*u8*t channel*id, hf*pio*statistics*t& statistics) const noexcept override;
    hf*pio*err*t GetDiagnostics(hf*u8*t channel*id, hf*pio*diagnostics*t& diagnostics) const noexcept override;
    
    // ESP32-specific advanced features
    hf*pio*err*t ConfigureCarrier(hf*u8*t channel*id, uint32*t carrier*freq*hz,
                                  float duty*cycle) noexcept;
    hf*pio*err*t EnableLoopback(hf*u8*t channel*id, bool enable) noexcept;
    hf*pio*err*t ConfigureAdvancedRmt(hf*u8*t channel*id, size*t memory*blocks = 64,
                                      bool enable*dma = false, uint32*t queue*depth = 4) noexcept;
    
    // Raw RMT symbol operations
    hf*pio*err*t TransmitRawRmtSymbols(hf*u8*t channel*id, const rmt*symbol*word*t* rmt*symbols,
                                       size*t symbol*count, bool wait*completion = false) noexcept;
    hf*pio*err*t ReceiveRawRmtSymbols(hf*u8*t channel*id, rmt*symbol*word*t* rmt*buffer,
                                      size*t buffer*size, size*t& symbols*received,
                                      uint32*t timeout*us = 10000) noexcept;
    
    // Utility methods
    size*t GetMaxSymbolCount() const noexcept;
    bool ValidatePioSystem() noexcept;
};
```text

## Constructor and Destructor

### EspPio()

Creates a new EspPio instance.

**Parameters:** None

**Example:**
```cpp
EspPio pio;  // Default construction
```text

### ~EspPio()

Destructor that automatically deinitializes the PIO system if still initialized.

**Note:** Follows RAII principles for safe resource cleanup.

## Core PIO Operations

### Initialize()

Initializes the PIO peripheral system.

**Returns:** `hf*pio*err*t`
- `PIO*SUCCESS`: Initialization successful
- `PIO*ERR*ALREADY*INITIALIZED`: Already initialized

**Example:**
```cpp
EspPio pio;
hf*pio*err*t result = pio.Initialize();
if (result != hf*pio*err*t::PIO*SUCCESS) {
    ESP*LOGE(TAG, "PIO initialization failed: %s", HfPioErrToString(result).data());
}
```text

### Deinitialize()

Deinitializes the PIO peripheral and cleans up all channels.

**Returns:** `hf*pio*err*t`
- `PIO*SUCCESS`: Deinitialization successful
- `PIO*ERR*NOT*INITIALIZED`: Not initialized

### ConfigureChannel()

Configures a PIO channel with the specified parameters.

**Parameters:**
- `channel*id`: Channel identifier (must be valid for current ESP32 variant)
- `config`: Channel configuration structure

**Returns:** `hf*pio*err*t`
- `PIO*SUCCESS`: Configuration successful
- `PIO*ERR*INVALID*CHANNEL`: Invalid channel ID for current ESP32 variant
- `PIO*ERR*INVALID*PARAMETER`: Invalid configuration parameters
- `PIO*ERR*CHANNEL*BUSY`: Channel is currently busy

**Example:**
```cpp
// Get appropriate TX channel for current ESP32 variant
uint8*t tx*channel = HfRmtGetTxChannel(0);

hf*pio*channel*config*t config;
config.gpio*pin = 8;
config.direction = hf*pio*direction*t::Transmit;
config.resolution*hz = 8000000;  // 8MHz for precise WS2812 timing
config.polarity = hf*pio*polarity*t::Normal;
config.idle*state = hf*pio*idle*state*t::Low;

hf*pio*err*t result = pio.ConfigureChannel(tx*channel, config);
```text

### Transmit()

Transmits a sequence of symbols on the specified channel.

**Parameters:**
- `channel*id`: Channel identifier
- `symbols`: Array of symbols to transmit
- `symbol*count`: Number of symbols in the array
- `wait*completion`: If true, blocks until transmission completes

**Returns:** `hf*pio*err*t`

**Example:**
```cpp
// WS2812 RGB color transmission
hf*pio*symbol*t rgb*symbols[] = {
    {6, true}, {5, false},  // Bit 1: Green MSB
    {3, true}, {7, false},  // Bit 0: Green bit 6
    // ... more color bits
};

hf*pio*err*t result = pio.Transmit(tx*channel, rgb*symbols, 
                                   sizeof(rgb*symbols) / sizeof(rgb*symbols[0]), 
                                   false);  // Async transmission
```text

### StartReceive() / StopReceive()

Starts and stops symbol reception on a channel.

**StartReceive Parameters:**
- `channel*id`: Channel identifier
- `buffer`: Buffer to store received symbols
- `buffer*size`: Size of the buffer
- `timeout*us`: Timeout in microseconds (0 = no timeout)

**StopReceive Parameters:**
- `channel*id`: Channel identifier
- `symbols*received`: [out] Number of symbols actually received

**Example:**
```cpp
uint8*t rx*channel = HfRmtGetRxChannel(0);
hf*pio*symbol*t rx*buffer[64];

// Start reception
hf*pio*err*t result = pio.StartReceive(rx*channel, rx*buffer, 64, 5000);  // 5ms timeout

// Later, stop reception
size*t symbols*received;
result = pio.StopReceive(rx*channel, symbols*received);
ESP*LOGI(TAG, "Received %zu symbols", symbols*received);
```text

## Channel-Specific Callback Management

### SetTransmitCallback()

Sets a transmission completion callback for a specific channel.

**Parameters:**
- `channel*id`: Channel identifier
- `callback`: Callback function
- `user*data`: User data passed to callback

**Callback Signature:**
```cpp
void callback(hf*u8*t channel*id, size*t symbols*sent, void* user*data);
```text

**Example:**
```cpp
void OnWS2812Complete(hf*u8*t channel*id, size*t symbols*sent, void* user*data) {
    const char* led*name = static*cast<const char*>(user*data);
    ESP*LOGI(TAG, "LED %s on channel %d: sent %zu symbols", led*name, channel*id, symbols*sent);
}

const char* led*context = "Built-in RGB";
pio.SetTransmitCallback(tx*channel, OnWS2812Complete, const*cast<char*>(led*context));
```text

### SetReceiveCallback()

Sets a reception completion callback for a specific channel.

**Callback Signature:**
```cpp
void callback(hf*u8*t channel*id, const hf*pio*symbol*t* symbols, 
              size*t symbol*count, void* user*data);
```text

### SetErrorCallback()

Sets an error callback for a specific channel.

**Callback Signature:**
```cpp
void callback(hf*u8*t channel*id, hf*pio*err*t error, void* user*data);
```text

### ClearChannelCallbacks()

Clears all callbacks for a specific channel.

**Parameters:**
- `channel*id`: Channel identifier

### ClearCallbacks()

Clears all callbacks for all channels.

## ESP32-Specific Advanced Features

### ConfigureCarrier()

Configures carrier modulation for IR protocols.

**Parameters:**
- `channel*id`: Channel identifier
- `carrier*freq*hz`: Carrier frequency in Hz (0 to disable)
- `duty*cycle`: Carrier duty cycle (0.0 to 1.0)

**Example:**
```cpp
// Configure 38kHz IR carrier with 33% duty cycle
pio.ConfigureCarrier(tx*channel, 38000, 0.33f);
```text

### ConfigureAdvancedRmt()

Configures advanced RMT channel settings.

**Parameters:**
- `channel*id`: Channel identifier
- `memory*blocks`: Number of memory blocks to allocate (default: 64)
- `enable*dma`: Enable DMA mode for large transfers (default: false)
- `queue*depth`: Transmit queue depth (default: 4)

**Example:**
```cpp
// Configure for high-throughput DMA operation
pio.ConfigureAdvancedRmt(tx*channel, 256, true, 8);
```text

### TransmitRawRmtSymbols()

Transmits raw RMT symbols directly, bypassing HardFOC symbol conversion.

**Parameters:**
- `channel*id`: Channel identifier
- `rmt*symbols`: Array of raw RMT symbols
- `symbol*count`: Number of RMT symbols
- `wait*completion`: If true, blocks until transmission completes

**Use Case:** Maximum performance when you already have RMT-formatted symbols.

### GetMaxSymbolCount()

Returns the maximum number of symbols that can be transmitted in one operation.

**Returns:** Maximum symbol count (typically 4096 for ESP32-C6)

## Status and Diagnostics

### IsChannelBusy()

Checks if a channel is currently busy with an operation.

**Parameters:**
- `channel*id`: Channel identifier

**Returns:** `true` if busy, `false` if idle

### GetChannelStatus()

Gets detailed status information for a channel.

**Parameters:**
- `channel*id`: Channel identifier
- `status`: [out] Status information structure

### GetCapabilities()

Gets PIO system capabilities.

**Parameters:**
- `capabilities`: [out] Capability information structure

**Example:**
```cpp
hf*pio*capabilities*t caps;
pio.GetCapabilities(caps);
ESP*LOGI(TAG, "Max channels: %d, Min resolution: %u Hz, Max resolution: %u Hz", 
         caps.max*channels, caps.min*resolution*ns, caps.max*resolution*ns);
```text

### ValidatePioSystem()

Performs comprehensive system validation and diagnostics.

**Returns:** `true` if all systems pass validation, `false` otherwise

**Use Case:** System health checks and troubleshooting.

## ESP32 Variant Helper Functions

These functions are defined in `EspTypes*PIO.h` and work with the EspPio class:

### HfRmtGetTxChannel()

Gets the appropriate TX channel for the current ESP32 variant.

```cpp
inline constexpr int8*t HfRmtGetTxChannel(uint8*t index) noexcept;
```text

**Parameters:**
- `index`: Channel index (0-based within available TX channels)

**Returns:** Actual channel number, or -1 if invalid

### HfRmtGetRxChannel()

Gets the appropriate RX channel for the current ESP32 variant.

```cpp
inline constexpr int8*t HfRmtGetRxChannel(uint8*t index) noexcept;
```text

### HfRmtIsChannelValidForDirection()

Validates if a channel is valid for a specific direction on the current ESP32 variant.

```cpp
inline constexpr bool HfRmtIsChannelValidForDirection(uint8*t channel*id, 
                                                     hf*pio*direction*t direction) noexcept;
```text

### HfRmtGetVariantName()

Returns the name of the current ESP32 variant.

```cpp
inline constexpr const char* HfRmtGetVariantName() noexcept;
```text

## Error Handling

All methods return `hf*pio*err*t` error codes.
Use `HfPioErrToString()` to convert error codes to human-readable strings:

```cpp
hf*pio*err*t result = pio.ConfigureChannel(channel*id, config);
if (result != hf*pio*err*t::PIO*SUCCESS) {
    ESP*LOGE(TAG, "Configuration failed: %s", HfPioErrToString(result).data());
}
```text

## Thread Safety

The EspPio class is thread-safe.
All public methods use internal mutex protection to ensure safe concurrent access from multiple
tasks.

## Performance Considerations

### Memory Usage
- **Zero Dynamic Allocation**: All callback storage uses fixed-size arrays
- **Cache-Friendly**: Contiguous memory layout for optimal performance
- **Embedded-Optimized**: Suitable for real-time embedded applications

### Timing Performance
- **ISR-Safe Callbacks**: Minimal overhead in interrupt context
- **O(1) Channel Access**: Direct array indexing for callback lookup
- **Deterministic Execution**: Predictable timing for real-time applications

## Examples

### Complete WS2812 LED Control
```cpp
#include "mcu/esp32/EspPio.h"
#include "mcu/esp32/utils/EspTypes*PIO.h"

EspPio pio;
uint8*t led*channel;

void setup*ws2812() {
    // Initialize PIO
    pio.Initialize();
    
    // Get appropriate TX channel for current ESP32 variant
    led*channel = HfRmtGetTxChannel(0);
    if (led*channel < 0) {
        ESP*LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
        return;
    }
    
    // Configure channel for WS2812
    hf*pio*channel*config*t config;
    config.gpio*pin = 8;  // Built-in LED on ESP32-C6
    config.direction = hf*pio*direction*t::Transmit;
    config.resolution*hz = 8000000;  // 8MHz for 125ns precision
    config.polarity = hf*pio*polarity*t::Normal;
    config.idle*state = hf*pio*idle*state*t::Low;
    
    hf*pio*err*t result = pio.ConfigureChannel(led*channel, config);
    if (result != hf*pio*err*t::PIO*SUCCESS) {
        ESP*LOGE(TAG, "Channel configuration failed: %s", HfPioErrToString(result).data());
        return;
    }
    
    // Set completion callback
    pio.SetTransmitCallback(led*channel, [](hf*u8*t ch, size*t symbols, void* ctx) {
        ESP*LOGI(TAG, "WS2812 transmission complete: %zu symbols", symbols);
    }, nullptr);
}

void set*led*color(uint8*t red, uint8*t green, uint8*t blue) {
    hf*pio*symbol*t symbols[24];  // 8 bits × 3 colors
    
    // Convert RGB to WS2812 symbols (8MHz resolution: 125ns per tick)
    uint32*t color = (green << 16) | (red << 8) | blue;  // GRB order
    
    for (int i = 0; i < 24; i++) {
        bool bit = (color >> (23 - i)) & 1;
        if (bit) {
            symbols[i] = {6, true};   // T1H: 750ns ≈ 6 ticks
            symbols[i+1] = {5, false}; // T1L: 625ns ≈ 5 ticks
        } else {
            symbols[i] = {3, true};   // T0H: 375ns ≈ 3 ticks
            symbols[i+1] = {7, false}; // T0L: 875ns ≈ 7 ticks
        }
    }
    
    pio.Transmit(led*channel, symbols, 24, false);  // Async transmission
}
```text

## See Also

- [BasePio API Reference](../api/BasePio.md)
- [ESP32 PIO Test Documentation](../../examples/esp32/docs/README_PIO_TEST.md)
- [HardwareTypes Reference](../api/HardwareTypes.md)