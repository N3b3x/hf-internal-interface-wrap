# ESP32 PIO/RMT Implementation Guide

## Overview

This guide documents the comprehensive improvements made to the ESP32 PIO implementation, which wraps the ESP-IDF v5.5 RMT (Remote Control Transceiver) driver. The implementation now provides proper channel-specific callback management, enhanced timing control, and robust support for all ESP32 variants.

## Supported ESP32 Variants

The implementation automatically adapts to different ESP32 variants with their specific RMT channel allocation constraints:

| ESP32 Variant | Total Channels | TX Channels | RX Channels | Channel Allocation |
|---------------|----------------|-------------|-------------|-------------------|
| ESP32         | 8              | 8 (0-7)     | 8 (0-7)     | Any channel can be TX or RX |
| ESP32-S2      | 4              | 4 (0-3)     | 4 (0-3)     | Any channel can be TX or RX |
| ESP32-S3      | 8              | 4 (0-3)     | 4 (4-7)     | **Hardcoded allocation** |
| ESP32-C3      | 4              | 2 (0-1)     | 2 (2-3)     | **Hardcoded allocation** |
| ESP32-C6      | 4              | 2 (0-1)     | 2 (2-3)     | **Hardcoded allocation** |
| ESP32-H2      | 4              | 2 (0-1)     | 2 (2-3)     | **Hardcoded allocation** |

## Key Improvements

### 1. Channel-Specific Callback System

#### Problem Solved
The original implementation used global callbacks that couldn't distinguish between channels, making it impossible to have different handlers for different RMT channels.

#### Solution Implementation
```cpp
// New API requiring channel ID
virtual void SetTransmitCallback(hf_u8_t channel_id, hf_pio_transmit_callback_t callback, 
                                void* user_data) = 0;
virtual void SetReceiveCallback(hf_u8_t channel_id, hf_pio_receive_callback_t callback,
                               void* user_data) = 0;
virtual void SetErrorCallback(hf_u8_t channel_id, hf_pio_error_callback_t callback,
                             void* user_data) = 0;
virtual void ClearChannelCallbacks(hf_u8_t channel_id) = 0;
```

#### Storage Implementation
```cpp
struct ChannelState {
    // Per-channel callbacks and user data
    hf_pio_transmit_callback_t transmit_callback;
    void* transmit_user_data;
    hf_pio_receive_callback_t receive_callback;
    void* receive_user_data;
    hf_pio_error_callback_t error_callback;
    void* error_user_data;
    // ... other channel state
};

std::array<ChannelState, MAX_CHANNELS> channels_;  // Embedded storage
```

### 2. Resolution Hz Instead of Resolution Ns

#### Problem Solved
The ESP-IDF RMT driver uses `resolution_hz` parameters, but the original implementation used `resolution_ns`, requiring error-prone conversions.

#### Solution Implementation
```cpp
struct hf_pio_channel_config_t {
    hf_u32_t resolution_hz;  // Direct RMT compatibility
    // ... other fields
};
```

#### Clock Divider Calculation
```cpp
hf_u32_t EspPio::CalculateClockDivider(hf_u32_t resolution_hz) const noexcept {
    // Direct calculation: divider = source_freq / target_freq
    hf_u32_t divider = RMT_CLK_SRC_FREQ / resolution_hz;
    
    // Clamp to valid hardware range (1-255)
    return std::max(1U, std::min(255U, divider));
}
```

### 3. ESP32 Variant-Specific Channel Validation

#### Validation Functions
```cpp
// Compile-time channel validation based on ESP32 variant
inline constexpr bool HfRmtIsChannelValidForDirection(uint8_t channel_id, 
                                                     hf_pio_direction_t direction) noexcept {
    switch (direction) {
        case hf_pio_direction_t::Transmit:
            return HF_RMT_IS_VALID_TX_CHANNEL(channel_id);
        case hf_pio_direction_t::Receive:
            return HF_RMT_IS_VALID_RX_CHANNEL(channel_id);
        case hf_pio_direction_t::Bidirectional:
            return HF_RMT_IS_VALID_TX_CHANNEL(channel_id) && HF_RMT_IS_VALID_RX_CHANNEL(channel_id);
    }
}
```

#### Helper Functions
```cpp
// Get appropriate channels for each variant
inline constexpr int8_t HfRmtGetTxChannel(uint8_t index) noexcept;
inline constexpr int8_t HfRmtGetRxChannel(uint8_t index) noexcept;
inline constexpr const char* HfRmtGetVariantName() noexcept;
```

### 4. Enhanced Static Callback Dispatch

#### Problem Solved
ESP-IDF RMT driver requires C-compatible callbacks, but the C++ wrapper needs to dispatch to member functions and maintain state.

#### Solution Implementation
```cpp
// Static callback compatible with ESP-IDF
static bool OnTransmitComplete(rmt_channel_handle_t channel, 
                              const rmt_tx_done_event_data_t* edata, 
                              void* user_ctx) {
    auto* instance = static_cast<EspPio*>(user_ctx);
    
    // Find channel and dispatch to channel-specific callback
    for (hf_u8_t i = 0; i < MAX_CHANNELS; ++i) {
        if (instance->channels_[i].tx_channel == channel) {
            auto& ch = instance->channels_[i];
            if (ch.transmit_callback) {
                ch.transmit_callback(i, ch.status.symbols_processed, ch.transmit_user_data);
            }
            break;
        }
    }
    return false;
}
```

## Usage Examples

### Basic Channel Configuration
```cpp
EspPio pio;
pio.Initialize();

// Get appropriate channel for current ESP32 variant
uint8_t tx_channel = HfRmtGetTxChannel(0);  // First available TX channel

hf_pio_channel_config_t config;
config.gpio_pin = 8;                               // GPIO pin
config.direction = hf_pio_direction_t::Transmit;   // TX direction
config.resolution_hz = 8000000;                    // 8MHz for precise timing
config.polarity = hf_pio_polarity_t::Normal;
config.idle_state = hf_pio_idle_state_t::Low;

hf_pio_err_t result = pio.ConfigureChannel(tx_channel, config);
if (result != hf_pio_err_t::PIO_SUCCESS) {
    ESP_LOGE(TAG, "Channel configuration failed: %s", HfPioErrToString(result).data());
}
```

### Channel-Specific Callbacks
```cpp
// Define callback function
void OnWS2812TransmissionComplete(hf_u8_t channel_id, size_t symbols_sent, void* user_data) {
    const char* led_name = static_cast<const char*>(user_data);
    ESP_LOGI(TAG, "LED %s transmission complete on channel %d: %zu symbols", 
             led_name, channel_id, symbols_sent);
}

// Register callback with context
const char* led_context = "Built-in RGB LED";
pio.SetTransmitCallback(tx_channel, OnWS2812TransmissionComplete, 
                       const_cast<char*>(led_context));
```

### Multiple Channels with Different Purposes
```cpp
// Channel 0: WS2812 LED control (high precision)
uint8_t led_channel = HfRmtGetTxChannel(0);
hf_pio_channel_config_t led_config;
led_config.gpio_pin = 8;
led_config.direction = hf_pio_direction_t::Transmit;
led_config.resolution_hz = 8000000;  // 8MHz for 125ns precision
pio.ConfigureChannel(led_channel, led_config);
pio.SetTransmitCallback(led_channel, ws2812_callback, led_context);

// Channel 1: IR transmission (medium precision)
uint8_t ir_channel = HfRmtGetTxChannel(1);
if (ir_channel >= 0) {  // Check if second TX channel available
    hf_pio_channel_config_t ir_config;
    ir_config.gpio_pin = 9;
    ir_config.direction = hf_pio_direction_t::Transmit;
    ir_config.resolution_hz = 1000000;  // 1MHz for 1µs precision
    pio.ConfigureChannel(ir_channel, ir_config);
    pio.SetTransmitCallback(ir_channel, ir_callback, ir_context);
}

// Channel 2: Signal reception (if available)
uint8_t rx_channel = HfRmtGetRxChannel(0);
if (rx_channel >= 0) {
    hf_pio_channel_config_t rx_config;
    rx_config.gpio_pin = 18;
    rx_config.direction = hf_pio_direction_t::Receive;
    rx_config.resolution_hz = 1000000;  // 1MHz for 1µs precision
    pio.ConfigureChannel(rx_channel, rx_config);
    pio.SetReceiveCallback(rx_channel, rx_callback, rx_context);
}
```

### Error Handling and Validation
```cpp
// Validate channel before configuration
if (!HfRmtIsChannelValidForDirection(channel_id, hf_pio_direction_t::Transmit)) {
    ESP_LOGE(TAG, "Channel %d is not valid for TX on %s", channel_id, HfRmtGetVariantName());
    ESP_LOGE(TAG, "Valid TX channels: %d-%d", 
             HF_RMT_TX_CHANNEL_START, 
             HF_RMT_TX_CHANNEL_START + HF_RMT_MAX_TX_CHANNELS - 1);
    return;
}

// Configure with error callback
pio.SetErrorCallback(channel_id, [](hf_u8_t ch, hf_pio_err_t error, void* ctx) {
    ESP_LOGE(TAG, "Channel %d error: %s", ch, HfPioErrToString(error).data());
}, nullptr);
```

## Resolution and Timing Guidelines

### Common Timing Requirements

| Application | Resolution Hz | Equivalent Period | Use Case |
|-------------|---------------|-------------------|----------|
| WS2812 LEDs | 8,000,000     | 125 ns           | Precise color control |
| Standard Digital | 1,000,000 | 1 µs             | General purpose timing |
| IR Protocols | 38,000-56,000 | 18-26 µs         | Remote control |
| Servo Control | 50,000        | 20 µs            | PWM servo signals |
| Low Speed | 1,000         | 1 ms             | Slow control signals |

### Symbol Duration Calculation
```cpp
// For WS2812 protocol with 8MHz resolution (125ns per tick):
// T0H = 350ns -> 350/125 = 2.8 ≈ 3 ticks
// T0L = 900ns -> 900/125 = 7.2 ≈ 7 ticks
// T1H = 700ns -> 700/125 = 5.6 ≈ 6 ticks
// T1L = 600ns -> 600/125 = 4.8 ≈ 5 ticks

hf_pio_symbol_t ws2812_bit_0[] = {
    {3, true},   // T0H: 3 ticks high
    {7, false}   // T0L: 7 ticks low
};

hf_pio_symbol_t ws2812_bit_1[] = {
    {6, true},   // T1H: 6 ticks high
    {5, false}   // T1L: 5 ticks low
};
```

## Testing and Validation

### Base Tests
The implementation includes comprehensive base tests that validate:

- ESP32 variant detection and channel allocation
- Channel direction validation for each variant
- Resolution Hz configuration and boundary conditions
- Channel-specific callback system
- Configuration validation

### Running Tests
```bash
# Build and run PIO base tests
cd examples/esp32
idf.py set-target esp32c6  # or your target variant
idf.py build
idf.py flash monitor
```

### Test Coverage
- **Variant Detection**: Validates correct ESP32 variant identification
- **Channel Allocation**: Tests TX/RX channel helper functions
- **Configuration Validation**: Ensures proper error handling for invalid configurations
- **Resolution Testing**: Validates boundary conditions and clock calculations
- **Callback System**: Tests channel-specific callback registration and clearing

## Performance Characteristics

### Memory Usage
- **Zero Dynamic Allocation**: All storage uses `std::array` for predictable memory usage
- **Cache-Friendly**: Contiguous storage improves cache locality
- **Embedded-Optimized**: Fixed-size structures suitable for real-time systems

### Execution Performance
- **O(1) Callback Access**: Direct array indexing for callback lookup
- **Minimal ISR Overhead**: Static callbacks minimize interrupt processing time
- **Deterministic Timing**: Fixed-time operations for real-time applications

### Real-Time Characteristics
- **ISR-Safe**: Callbacks designed for interrupt context execution
- **No Heap Allocation**: Prevents garbage collection pauses
- **Predictable Latency**: Consistent timing for time-critical applications

## Migration Guide

### Breaking Changes
1. **Callback Registration**: Now requires `channel_id` parameter
2. **Configuration Structure**: Uses `resolution_hz` instead of `resolution_ns`
3. **New Methods**: `ClearChannelCallbacks()` added for per-channel management

### Migration Steps

#### 1. Update Callback Registration
```cpp
// Old API:
pio.SetTransmitCallback(my_callback, user_data);

// New API:
pio.SetTransmitCallback(channel_id, my_callback, user_data);
```

#### 2. Update Configuration
```cpp
// Old configuration:
config.resolution_ns = 1000;  // 1µs in nanoseconds

// New configuration:
config.resolution_hz = 1000000;  // 1µs as 1MHz frequency
```

#### 3. Add Channel Validation
```cpp
// Validate channel before use
uint8_t tx_channel = HfRmtGetTxChannel(0);
if (tx_channel < 0) {
    ESP_LOGE(TAG, "No TX channels available on %s", HfRmtGetVariantName());
    return;
}
```

### Backward Compatibility
- Core transmission/reception APIs remain unchanged
- Error codes and return types unchanged
- Symbol structure unchanged

## Troubleshooting

### Common Issues

#### Invalid Channel Configuration
```
Channel 2 is not valid for TX direction on ESP32-C6
Valid TX channels for ESP32-C6: 0-1
```
**Solution**: Use appropriate channels for your ESP32 variant. Check channel allocation table.

#### Resolution Out of Range
```
Invalid resolution 0 Hz for channel 0 (range: 1000-80000000 Hz)
```
**Solution**: Use valid resolution within supported range (1kHz to 80MHz).

#### Channel Busy Error
```
Channel already in use
```
**Solution**: Stop current operation or wait for completion before reconfiguring.

### Debug Logging
Enable detailed logging to diagnose issues:
```cpp
// Set log level for PIO component
esp_log_level_set("EspPio", ESP_LOG_DEBUG);
```

### Validation Helper
```cpp
// Quick validation function
bool validate_channel_config(uint8_t channel_id, const hf_pio_channel_config_t& config) {
    if (!HF_RMT_IS_VALID_CHANNEL(channel_id)) {
        ESP_LOGE(TAG, "Invalid channel %d for %s", channel_id, HfRmtGetVariantName());
        return false;
    }
    
    if (!HfRmtIsChannelValidForDirection(channel_id, config.direction)) {
        ESP_LOGE(TAG, "Channel %d invalid for direction %d", channel_id, static_cast<int>(config.direction));
        return false;
    }
    
    if (!HF_RMT_IS_VALID_RESOLUTION(config.resolution_hz)) {
        ESP_LOGE(TAG, "Invalid resolution %u Hz", config.resolution_hz);
        return false;
    }
    
    return true;
}
```

## Future Enhancements

### Planned Features
- DMA support for high-throughput applications
- Advanced carrier modulation for IR protocols
- Hardware digital filtering for noise reduction
- Multi-channel synchronization capabilities

### API Extensions
- Streaming symbol transmission for continuous signals
- Pattern matching for received signals
- Automatic timing calibration
- Power management integration

## References

- [ESP-IDF RMT Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html)
- [ESP32 Variants Comparison](https://www.espressif.com/en/products/socs)
- [WS2812 Protocol Specification](https://cdn-shop.adafruit.com/datasheets/WS2812.pdf)
- [ESP-IDF v5.5 Migration Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/migration-guides/release-5.x/index.html)