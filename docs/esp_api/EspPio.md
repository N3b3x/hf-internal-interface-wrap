# EspPio Class API Reference

## Overview

The `EspPio` class provides a comprehensive C++ wrapper for the ESP-IDF v5.5 RMT (Remote Control Transceiver) driver, enabling precise timing control for digital signal generation and capture across all ESP32 variants.

## Class Declaration

```cpp
class EspPio : public BasePio {
public:
    EspPio() noexcept;
    ~EspPio() noexcept override;
    
    // BasePio interface implementation
    hf_pio_err_t Initialize() noexcept override;
    hf_pio_err_t Deinitialize() noexcept override;
    
    hf_pio_err_t ConfigureChannel(hf_u8_t channel_id,
                                  const hf_pio_channel_config_t& config) noexcept override;
    
    hf_pio_err_t Transmit(hf_u8_t channel_id, const hf_pio_symbol_t* symbols, 
                          size_t symbol_count, bool wait_completion = false) noexcept override;
    
    hf_pio_err_t StartReceive(hf_u8_t channel_id, hf_pio_symbol_t* buffer, 
                              size_t buffer_size, uint32_t timeout_us = 0) noexcept override;
    hf_pio_err_t StopReceive(hf_u8_t channel_id, size_t& symbols_received) noexcept override;
    
    bool IsChannelBusy(hf_u8_t channel_id) const noexcept override;
    hf_pio_err_t GetChannelStatus(hf_u8_t channel_id,
                                  hf_pio_channel_status_t& status) const noexcept override;
    hf_pio_err_t GetCapabilities(hf_pio_capabilities_t& capabilities) const noexcept override;
    
    // Channel-specific callback management
    void SetTransmitCallback(hf_u8_t channel_id, hf_pio_transmit_callback_t callback,
                            void* user_data = nullptr) noexcept override;
    void SetReceiveCallback(hf_u8_t channel_id, hf_pio_receive_callback_t callback,
                           void* user_data = nullptr) noexcept override;
    void SetErrorCallback(hf_u8_t channel_id, hf_pio_error_callback_t callback,
                         void* user_data = nullptr) noexcept override;
    void ClearChannelCallbacks(hf_u8_t channel_id) noexcept override;
    void ClearCallbacks() noexcept override;
    
    // Statistics and diagnostics
    hf_pio_err_t GetStatistics(hf_u8_t channel_id, hf_pio_statistics_t& statistics) const noexcept override;
    hf_pio_err_t GetDiagnostics(hf_u8_t channel_id, hf_pio_diagnostics_t& diagnostics) const noexcept override;
    
    // ESP32-specific advanced features
    hf_pio_err_t ConfigureCarrier(hf_u8_t channel_id, uint32_t carrier_freq_hz,
                                  float duty_cycle) noexcept;
    hf_pio_err_t EnableLoopback(hf_u8_t channel_id, bool enable) noexcept;
    hf_pio_err_t ConfigureAdvancedRmt(hf_u8_t channel_id, size_t memory_blocks = 64,
                                      bool enable_dma = false, uint32_t queue_depth = 4) noexcept;
    
    // Raw RMT symbol operations
    hf_pio_err_t TransmitRawRmtSymbols(hf_u8_t channel_id, const rmt_symbol_word_t* rmt_symbols,
                                       size_t symbol_count, bool wait_completion = false) noexcept;
    hf_pio_err_t ReceiveRawRmtSymbols(hf_u8_t channel_id, rmt_symbol_word_t* rmt_buffer,
                                      size_t buffer_size, size_t& symbols_received,
                                      uint32_t timeout_us = 10000) noexcept;
    
    // Utility methods
    size_t GetMaxSymbolCount() const noexcept;
    bool ValidatePioSystem() noexcept;
};
```

## Constructor and Destructor

### EspPio()

Creates a new EspPio instance.

**Parameters:** None

**Example:**
```cpp
EspPio pio;  // Default construction
```

### ~EspPio()

Destructor that automatically deinitializes the PIO system if still initialized.

**Note:** Follows RAII principles for safe resource cleanup.

## Core PIO Operations

### Initialize()

Initializes the PIO peripheral system.

**Returns:** `hf_pio_err_t`
- `PIO_SUCCESS`: Initialization successful
- `PIO_ERR_ALREADY_INITIALIZED`: Already initialized

**Example:**
```cpp
EspPio pio;
hf_pio_err_t result = pio.Initialize();
if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "PIO initialization failed: %s", HfPioErrToString(result).data());
}
```

### Deinitialize()

Deinitializes the PIO peripheral and cleans up all channels.

**Returns:** `hf_pio_err_t`
- `PIO_SUCCESS`: Deinitialization successful
- `PIO_ERR_NOT_INITIALIZED`: Not initialized

### ConfigureChannel()

Configures a PIO channel with the specified parameters.

**Parameters:**
- `channel_id`: Channel identifier (must be valid for current ESP32 variant)
- `config`: Channel configuration structure

**Returns:** `hf_pio_err_t`
- `PIO_SUCCESS`: Configuration successful
- `PIO_ERR_INVALID_CHANNEL`: Invalid channel ID for current ESP32 variant
- `PIO_ERR_INVALID_PARAMETER`: Invalid configuration parameters
- `PIO_ERR_CHANNEL_BUSY`: Channel is currently busy

**Example:**
```cpp
// Get appropriate TX channel for current ESP32 variant
uint8_t tx_channel = HfRmtGetTxChannel(0);

hf_pio_channel_config_t config;
config.gpio_pin = 8;
config.direction = hf_pio_direction_t::Transmit;
config.resolution_hz = 8000000;  // 8MHz for precise WS2812 timing
config.polarity = hf_pio_polarity_t::Normal;
config.idle_state = hf_pio_idle_state_t::Low;

hf_pio_err_t result = pio.ConfigureChannel(tx_channel, config);
```

### Transmit()

Transmits a sequence of symbols on the specified channel.

**Parameters:**
- `channel_id`: Channel identifier
- `symbols`: Array of symbols to transmit
- `symbol_count`: Number of symbols in the array
- `wait_completion`: If true, blocks until transmission completes

**Returns:** `hf_pio_err_t`

**Example:**
```cpp
// WS2812 RGB color transmission
hf_pio_symbol_t rgb_symbols[] = {
    {6, true}, {5, false},  // Bit 1: Green MSB
    {3, true}, {7, false},  // Bit 0: Green bit 6
    // ... more color bits
};

hf_pio_err_t result = pio.Transmit(tx_channel, rgb_symbols, 
                                   sizeof(rgb_symbols) / sizeof(rgb_symbols[0]), 
                                   false);  // Async transmission
```

### StartReceive() / StopReceive()

Starts and stops symbol reception on a channel.

**StartReceive Parameters:**
- `channel_id`: Channel identifier
- `buffer`: Buffer to store received symbols
- `buffer_size`: Size of the buffer
- `timeout_us`: Timeout in microseconds (0 = no timeout)

**StopReceive Parameters:**
- `channel_id`: Channel identifier
- `symbols_received`: [out] Number of symbols actually received

**Example:**
```cpp
uint8_t rx_channel = HfRmtGetRxChannel(0);
hf_pio_symbol_t rx_buffer[64];

// Start reception
hf_pio_err_t result = pio.StartReceive(rx_channel, rx_buffer, 64, 5000);  // 5ms timeout

// Later, stop reception
size_t symbols_received;
result = pio.StopReceive(rx_channel, symbols_received);
ESP_LOGI(TAG, "Received %zu symbols", symbols_received);
```

## Channel-Specific Callback Management

### SetTransmitCallback()

Sets a transmission completion callback for a specific channel.

**Parameters:**
- `channel_id`: Channel identifier
- `callback`: Callback function
- `user_data`: User data passed to callback

**Callback Signature:**
```cpp
void callback(hf_u8_t channel_id, size_t symbols_sent, void* user_data);
```

**Example:**
```cpp
void OnWS2812Complete(hf_u8_t channel_id, size_t symbols_sent, void* user_data) {
    const char* led_name = static_cast<const char*>(user_data);
    ESP_LOGI(TAG, "LED %s on channel %d: sent %zu symbols", led_name, channel_id, symbols_sent);
}

const char* led_context = "Built-in RGB";
pio.SetTransmitCallback(tx_channel, OnWS2812Complete, const_cast<char*>(led_context));
```

### SetReceiveCallback()

Sets a reception completion callback for a specific channel.

**Callback Signature:**
```cpp
void callback(hf_u8_t channel_id, const hf_pio_symbol_t* symbols, 
              size_t symbol_count, void* user_data);
```

### SetErrorCallback()

Sets an error callback for a specific channel.

**Callback Signature:**
```cpp
void callback(hf_u8_t channel_id, hf_pio_err_t error, void* user_data);
```

### ClearChannelCallbacks()

Clears all callbacks for a specific channel.

**Parameters:**
- `channel_id`: Channel identifier

### ClearCallbacks()

Clears all callbacks for all channels.

## ESP32-Specific Advanced Features

### ConfigureCarrier()

Configures carrier modulation for IR protocols.

**Parameters:**
- `channel_id`: Channel identifier
- `carrier_freq_hz`: Carrier frequency in Hz (0 to disable)
- `duty_cycle`: Carrier duty cycle (0.0 to 1.0)

**Example:**
```cpp
// Configure 38kHz IR carrier with 33% duty cycle
pio.ConfigureCarrier(tx_channel, 38000, 0.33f);
```

### ConfigureAdvancedRmt()

Configures advanced RMT channel settings.

**Parameters:**
- `channel_id`: Channel identifier
- `memory_blocks`: Number of memory blocks to allocate (default: 64)
- `enable_dma`: Enable DMA mode for large transfers (default: false)
- `queue_depth`: Transmit queue depth (default: 4)

**Example:**
```cpp
// Configure for high-throughput DMA operation
pio.ConfigureAdvancedRmt(tx_channel, 256, true, 8);
```

### TransmitRawRmtSymbols()

Transmits raw RMT symbols directly, bypassing HardFOC symbol conversion.

**Parameters:**
- `channel_id`: Channel identifier
- `rmt_symbols`: Array of raw RMT symbols
- `symbol_count`: Number of RMT symbols
- `wait_completion`: If true, blocks until transmission completes

**Use Case:** Maximum performance when you already have RMT-formatted symbols.

### GetMaxSymbolCount()

Returns the maximum number of symbols that can be transmitted in one operation.

**Returns:** Maximum symbol count (typically 4096 for ESP32-C6)

## Status and Diagnostics

### IsChannelBusy()

Checks if a channel is currently busy with an operation.

**Parameters:**
- `channel_id`: Channel identifier

**Returns:** `true` if busy, `false` if idle

### GetChannelStatus()

Gets detailed status information for a channel.

**Parameters:**
- `channel_id`: Channel identifier
- `status`: [out] Status information structure

### GetCapabilities()

Gets PIO system capabilities.

**Parameters:**
- `capabilities`: [out] Capability information structure

**Example:**
```cpp
hf_pio_capabilities_t caps;
pio.GetCapabilities(caps);
ESP_LOGI(TAG, "Max channels: %d, Min resolution: %u Hz, Max resolution: %u Hz", 
         caps.max_channels, caps.min_resolution_ns, caps.max_resolution_ns);
```

### ValidatePioSystem()

Performs comprehensive system validation and diagnostics.

**Returns:** `true` if all systems pass validation, `false` otherwise

**Use Case:** System health checks and troubleshooting.

## ESP32 Variant Helper Functions

These functions are defined in `EspTypes_PIO.h` and work with the EspPio class:

### HfRmtGetTxChannel()

Gets the appropriate TX channel for the current ESP32 variant.

```cpp
inline constexpr int8_t HfRmtGetTxChannel(uint8_t index) noexcept;
```

**Parameters:**
- `index`: Channel index (0-based within available TX channels)

**Returns:** Actual channel number, or -1 if invalid

### HfRmtGetRxChannel()

Gets the appropriate RX channel for the current ESP32 variant.

```cpp
inline constexpr int8_t HfRmtGetRxChannel(uint8_t index) noexcept;
```

### HfRmtIsChannelValidForDirection()

Validates if a channel is valid for a specific direction on the current ESP32 variant.

```cpp
inline constexpr bool HfRmtIsChannelValidForDirection(uint8_t channel_id, 
                                                     hf_pio_direction_t direction) noexcept;
```

### HfRmtGetVariantName()

Returns the name of the current ESP32 variant.

```cpp
inline constexpr const char* HfRmtGetVariantName() noexcept;
```

## Error Handling

All methods return `hf_pio_err_t` error codes. Use `HfPioErrToString()` to convert error codes to human-readable strings:

```cpp
hf_pio_err_t result = pio.ConfigureChannel(channel_id, config);
if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Configuration failed: %s", HfPioErrToString(result).data());
}
```

## Thread Safety

The EspPio class is thread-safe. All public methods use internal mutex protection to ensure safe concurrent access from multiple tasks.

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
#include "mcu/esp32/utils/EspTypes_PIO.h"

EspPio pio;
uint8_t led_channel;

void setup_ws2812() {
    // Initialize PIO
    pio.Initialize();
    
    // Get appropriate TX channel for current ESP32 variant
    led_channel = HfRmtGetTxChannel(0);
    if (led_channel < 0) {
        ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
        return;
    }
    
    // Configure channel for WS2812
    hf_pio_channel_config_t config;
    config.gpio_pin = 8;  // Built-in LED on ESP32-C6
    config.direction = hf_pio_direction_t::Transmit;
    config.resolution_hz = 8000000;  // 8MHz for 125ns precision
    config.polarity = hf_pio_polarity_t::Normal;
    config.idle_state = hf_pio_idle_state_t::Low;
    
    hf_pio_err_t result = pio.ConfigureChannel(led_channel, config);
    if (result != hf_pio_err_t::PIO_SUCCESS) {
        ESP_LOGE(TAG, "Channel configuration failed: %s", HfPioErrToString(result).data());
        return;
    }
    
    // Set completion callback
    pio.SetTransmitCallback(led_channel, [](hf_u8_t ch, size_t symbols, void* ctx) {
        ESP_LOGI(TAG, "WS2812 transmission complete: %zu symbols", symbols);
    }, nullptr);
}

void set_led_color(uint8_t red, uint8_t green, uint8_t blue) {
    hf_pio_symbol_t symbols[24];  // 8 bits × 3 colors
    
    // Convert RGB to WS2812 symbols (8MHz resolution: 125ns per tick)
    uint32_t color = (green << 16) | (red << 8) | blue;  // GRB order
    
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
    
    pio.Transmit(led_channel, symbols, 24, false);  // Async transmission
}
```

## See Also

- [BasePio API Reference](../api/BasePio.md)
- [ESP32 PIO Test Documentation](../../examples/esp32/docs/README_PIO_TEST.md)
- [HardwareTypes Reference](../api/HardwareTypes.md)