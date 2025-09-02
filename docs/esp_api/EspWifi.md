# EspWifi API Reference

## Overview

`EspWifi` is the ESP32-C6 implementation of the `BaseWifi` interface, providing comprehensive WiFi functionality specifically optimized for ESP32-C6 microcontrollers running ESP-IDF v5.5+. It offers both basic and advanced WiFi features with hardware-specific optimizations.

## Features

- **ESP32-C6 Optimized** - Full support for ESP32-C6 WiFi capabilities
- **802.11n Support** - High-speed WiFi with MIMO support
- **WPA3 Security** - Latest WiFi security standards
- **Mesh Networking** - ESP-WIFI-MESH support
- **Power Management** - Advanced power saving modes
- **Multiple Modes** - Station, Access Point, and Station+AP modes
- **Performance Optimized** - Hardware-accelerated operations

## Header File

```cpp
#include "inc/mcu/esp32/EspWifi.h"
```

## Class Definition

```cpp
class EspWifi : public BaseWifi {
public:
    // Constructor with full configuration
    explicit EspWifi(
        hf_wifi_mode_t mode = hf_wifi_mode_t::HF_WIFI_MODE_STATION,
        hf_wifi_band_t band = hf_wifi_band_t::HF_WIFI_BAND_2_4GHZ,
        hf_wifi_power_t tx_power = hf_wifi_power_t::HF_WIFI_POWER_19_5_DBM
    ) noexcept;

    // Destructor
    ~EspWifi() override;

    // BaseWifi implementation
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    bool IsInitialized() const noexcept override;
    const char* GetDescription() const noexcept override;

    // WiFi operations
    hf_wifi_err_t Connect(const hf_wifi_config_t& config) noexcept override;
    hf_wifi_err_t Disconnect() noexcept override;
    hf_wifi_err_t IsConnected(bool* connected) const noexcept override;
    hf_wifi_err_t GetConnectionInfo(hf_wifi_connection_info_t& info) noexcept override;

    // Access Point operations
    hf_wifi_err_t StartAccessPoint(const hf_wifi_ap_config_t& config) noexcept override;
    hf_wifi_err_t StopAccessPoint() noexcept override;
    hf_wifi_err_t GetAccessPointInfo(hf_wifi_ap_info_t& info) noexcept override;

    // Advanced features
    hf_wifi_err_t ScanNetworks(hf_wifi_scan_result_t* results, hf_size_t max_results, hf_size_t* found_count) noexcept override;
    hf_wifi_err_t SetPowerSaveMode(hf_wifi_power_save_mode_t mode) noexcept override;
    hf_wifi_err_t GetPowerSaveMode(hf_wifi_power_save_mode_t* mode) const noexcept override;
    hf_wifi_err_t SetTxPower(hf_wifi_power_t power) noexcept override;
    hf_wifi_err_t GetTxPower(hf_wifi_power_t* power) const noexcept override;
};
```

## Usage Examples

### Station Mode (Client)

```cpp
#include "inc/mcu/esp32/EspWifi.h"

// Create WiFi instance
EspWifi wifi(HF_WIFI_MODE_STATION);

// Initialize
if (!wifi.Initialize()) {
    printf("Failed to initialize WiFi\n");
    return;
}

// Configure and connect
hf_wifi_config_t config;
strcpy(config.ssid, "MyNetwork");
strcpy(config.password, "MyPassword");
config.security = HF_WIFI_SECURITY_WPA2_PSK;

hf_wifi_err_t err = wifi.Connect(config);
if (err != HF_WIFI_ERR_OK) {
    printf("Failed to connect: %d\n", err);
    return;
}

// Check connection status
bool connected;
err = wifi.IsConnected(&connected);
if (err == HF_WIFI_ERR_OK && connected) {
    printf("Connected to WiFi\n");
    
    // Get connection info
    hf_wifi_connection_info_t info;
    err = wifi.GetConnectionInfo(info);
    if (err == HF_WIFI_ERR_OK) {
        printf("SSID: %s\n", info.ssid);
        printf("RSSI: %d dBm\n", info.rssi);
        printf("Channel: %d\n", info.channel);
    }
}
```

### Access Point Mode

```cpp
// Create WiFi instance in AP mode
EspWifi wifi(HF_WIFI_MODE_ACCESS_POINT);

// Initialize
if (!wifi.Initialize()) {
    printf("Failed to initialize WiFi AP\n");
    return;
}

// Configure and start access point
hf_wifi_ap_config_t ap_config;
strcpy(ap_config.ssid, "MyESP32AP");
strcpy(ap_config.password, "MyPassword");
ap_config.security = HF_WIFI_SECURITY_WPA2_PSK;
ap_config.channel = 6;
ap_config.max_connections = 4;

hf_wifi_err_t err = wifi.StartAccessPoint(ap_config);
if (err != HF_WIFI_ERR_OK) {
    printf("Failed to start AP: %d\n", err);
    return;
}

printf("Access Point started: %s\n", ap_config.ssid);
```

### Network Scanning

```cpp
// Scan for available networks
hf_wifi_scan_result_t results[20];
hf_size_t found_count;

hf_wifi_err_t err = wifi.ScanNetworks(results, 20, &found_count);
if (err == HF_WIFI_ERR_OK) {
    printf("Found %zu networks:\n", found_count);
    for (hf_size_t i = 0; i < found_count; i++) {
        printf("  %s (RSSI: %d, Channel: %d, Security: %d)\n",
               results[i].ssid, results[i].rssi, results[i].channel, results[i].security);
    }
}
```

### Power Management

```cpp
// Set power save mode
hf_wifi_err_t err = wifi.SetPowerSaveMode(HF_WIFI_POWER_SAVE_MODEM);
if (err != HF_WIFI_ERR_OK) {
    printf("Failed to set power save mode: %d\n", err);
}

// Set transmit power
err = wifi.SetTxPower(HF_WIFI_POWER_19_5_DBM);
if (err != HF_WIFI_ERR_OK) {
    printf("Failed to set TX power: %d\n", err);
}
```

## ESP32-C6 Specific Features

### 802.11n Support

Full 802.11n support with MIMO capabilities for improved range and throughput.

### WPA3 Security

Support for the latest WPA3 security standard with enhanced protection.

### Mesh Networking

ESP-WIFI-MESH support for creating self-healing mesh networks.

### Advanced Power Management

Multiple power save modes optimized for different use cases.

## Error Handling

The `EspWifi` class provides comprehensive error handling with specific error codes:

- `HF_WIFI_ERR_OK` - Operation successful
- `HF_WIFI_ERR_INVALID_ARG` - Invalid parameter
- `HF_WIFI_ERR_NOT_INITIALIZED` - WiFi not initialized
- `HF_WIFI_ERR_TIMEOUT` - Operation timeout
- `HF_WIFI_ERR_CONNECTION_FAILED` - Connection failed
- `HF_WIFI_ERR_AUTH_FAILED` - Authentication failed
- `HF_WIFI_ERR_NOT_FOUND` - Network not found
- `HF_WIFI_ERR_ALREADY_CONNECTED` - Already connected

## Performance Considerations

- **Channel Selection**: Choose appropriate channel to avoid interference
- **Power Settings**: Balance range vs power consumption
- **Security**: Use WPA3 when possible for better security
- **Antenna**: Ensure proper antenna placement for optimal performance

## Related Documentation

- [BaseWifi API Reference](../api/BaseWifi.md) - Base class interface
- [HardwareTypes Reference](../api/HardwareTypes.md) - Platform-agnostic type definitions
- [ESP-IDF WiFi Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/api-reference/network/esp_wifi.html) - ESP-IDF documentation