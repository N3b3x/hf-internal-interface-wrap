# 📶 BaseWifi API Reference

## 🌟 Overview

`BaseWifi` is the abstract base class for all WiFi implementations in the HardFOC system. It provides a unified interface for wireless networking operations including station mode (client), access point mode (hotspot), network scanning, security configuration, and comprehensive connection management.

## ✨ Features

- **📡 Station Mode** - Connect to existing WiFi networks as a client
- **🔥 Access Point Mode** - Create WiFi hotspots for device configuration
- **🔍 Network Scanning** - Discover available wireless networks
- **🔒 Security Support** - WPA/WPA2/WPA3 and enterprise authentication
- **📊 Connection Management** - Automatic reconnection and signal monitoring
- **⚡ Event System** - Comprehensive callback system for network events
- **🔧 Power Management** - Energy-efficient WiFi operation modes
- **📈 Signal Monitoring** - Real-time RSSI and connection quality tracking

## 📁 Header File

```cpp
#include "inc/base/BaseWifi.h"
```

## 🎯 Type Definitions

### 🚨 Error Codes

```cpp
enum class hf_wifi_err_t : hf_u8_t {
    WIFI_SUCCESS = 0,                   // ✅ Success
    WIFI_ERR_FAILURE = 1,               // ❌ General failure
    WIFI_ERR_INVALID_PARAM = 2,         // 🚫 Invalid parameter
    WIFI_ERR_NOT_INITIALIZED = 3,       // ⚠️ WiFi not initialized
    WIFI_ERR_ALREADY_INITIALIZED = 4,   // ⚠️ WiFi already initialized
    WIFI_ERR_NOT_CONNECTED = 5,         // 📶 WiFi not connected
    WIFI_ERR_ALREADY_CONNECTED = 6,     // 📶 WiFi already connected
    WIFI_ERR_CONNECTION_FAILED = 7,     // ❌ Connection failed
    WIFI_ERR_DISCONNECTION_FAILED = 8,  // ❌ Disconnection failed
    WIFI_ERR_SCAN_FAILED = 9,           // 🔍 Network scan failed
    WIFI_ERR_AP_START_FAILED = 10,      // 🔥 Access Point start failed
    WIFI_ERR_AP_STOP_FAILED = 11,       // 🔥 Access Point stop failed
    WIFI_ERR_TIMEOUT = 12,              // ⏰ Operation timeout
    WIFI_ERR_NO_MEMORY = 13,            // 💾 Insufficient memory
    WIFI_ERR_INVALID_SSID = 14,         // 📡 Invalid SSID
    WIFI_ERR_INVALID_PASSWORD = 15,     // 🔐 Invalid password
    WIFI_ERR_WEAK_SIGNAL = 16,          // 📉 Weak signal strength
    WIFI_ERR_AUTHENTICATION_FAILED = 17, // 🔐 Authentication failed
    WIFI_ERR_ASSOCIATION_FAILED = 18,   // 🔗 Association failed
    WIFI_ERR_HANDSHAKE_FAILED = 19,     // 🤝 4-way handshake failed
    WIFI_ERR_INIT_FAILED = 20,          // 🚀 WiFi initialization failed
    WIFI_ERR_CONFIG_INVALID = 21,       // ⚙️ Invalid configuration
    WIFI_ERR_ENTERPRISE_FAILED = 22,    // 🏢 Enterprise authentication failed
    WIFI_ERR_WPA3_NOT_SUPPORTED = 23,   // 🔒 WPA3 not supported
    WIFI_ERR_MESH_FAILED = 24           // 🕸️ Mesh operation failed
};
```

### 🌐 WiFi Modes

```cpp
enum class hf_wifi_mode_t : hf_u8_t {
    WIFI_MODE_NULL = 0,     // 🚫 WiFi disabled
    WIFI_MODE_STA = 1,      // 📱 Station mode (client)
    WIFI_MODE_AP = 2,       // 🔥 Access Point mode
    WIFI_MODE_APSTA = 3     // 🔄 Combined AP+STA mode
};
```

### 🔒 Security Types

```cpp
enum class hf_wifi_security_t : hf_u8_t {
    WIFI_AUTH_OPEN = 0,         // 🔓 Open network (no security)
    WIFI_AUTH_WEP = 1,          // 🔐 WEP (deprecated)
    WIFI_AUTH_WPA_PSK = 2,      // 🔒 WPA-PSK
    WIFI_AUTH_WPA2_PSK = 3,     // 🔒 WPA2-PSK (most common)
    WIFI_AUTH_WPA_WPA2_PSK = 4, // 🔒 WPA/WPA2-PSK mixed
    WIFI_AUTH_WPA2_ENTERPRISE = 5, // 🏢 WPA2-Enterprise
    WIFI_AUTH_WPA3_PSK = 6,     // 🛡️ WPA3-PSK (latest)
    WIFI_AUTH_WPA2_WPA3_PSK = 7 // 🛡️ WPA2/WPA3-PSK mixed
};
```

### 📊 Event Types

```cpp
enum class hf_wifi_event_t : hf_u8_t {
    WIFI_EVENT_STA_START = 0,       // 🚀 Station started
    WIFI_EVENT_STA_STOP = 1,        // 🛑 Station stopped
    WIFI_EVENT_STA_CONNECTED = 2,   // ✅ Connected to AP
    WIFI_EVENT_STA_DISCONNECTED = 3, // ❌ Disconnected from AP
    WIFI_EVENT_STA_GOT_IP = 4,      // 🌐 Got IP address
    WIFI_EVENT_STA_LOST_IP = 5,     // 🌐 Lost IP address
    WIFI_EVENT_AP_START = 6,        // 🔥 AP started
    WIFI_EVENT_AP_STOP = 7,         // 🔥 AP stopped
    WIFI_EVENT_AP_STA_CONNECTED = 8, // 👤 Client connected to AP
    WIFI_EVENT_AP_STA_DISCONNECTED = 9, // 👤 Client disconnected from AP
    WIFI_EVENT_SCAN_DONE = 10       // 🔍 Network scan completed
};
```

### ⚙️ Configuration Structures

```cpp
struct hf_wifi_station_config_t {
    std::string ssid;                      // 📡 Network SSID
    std::string password;                  // 🔐 Network password
    uint8_t bssid[6];                     // 🆔 Target BSSID (optional)
    bool bssid_set;                       // 🎯 Use specific BSSID
    uint8_t channel;                      // 📻 WiFi channel (0 = auto)
    uint16_t listen_interval;             // ⏰ Listen interval
    hf_wifi_security_t threshold_authmode; // 🔒 Minimum security level
    int16_t threshold_rssi;               // 📶 Minimum signal strength
};

struct hf_wifi_ap_config_t {
    std::string ssid;            // 📡 AP SSID
    std::string password;        // 🔐 AP password
    uint8_t ssid_len;           // 📏 SSID length (0 for auto)
    uint8_t channel;            // 📻 WiFi channel
    hf_wifi_security_t authmode; // 🔒 Authentication mode
    uint8_t ssid_hidden;        // 👻 Hide SSID (0 = visible, 1 = hidden)
    uint8_t max_connection;     // 👥 Maximum concurrent connections
    uint16_t beacon_interval;   // 📡 Beacon interval (ms)
};

struct hf_wifi_scan_result_t {
    std::string ssid;              // 📡 Network SSID
    uint8_t bssid[6];             // 🆔 Network BSSID
    uint8_t primary_channel;       // 📻 Primary channel
    uint8_t secondary_channel;     // 📻 Secondary channel
    int8_t rssi;                  // 📶 Signal strength (dBm)
    hf_wifi_security_t authmode;   // 🔒 Authentication mode
    uint32_t phy_11b:1;           // 📊 802.11b support
    uint32_t phy_11g:1;           // 📊 802.11g support
    uint32_t phy_11n:1;           // 📊 802.11n support
    uint32_t wps:1;               // 🔧 WPS support
};
```

## 🏗️ Class Interface

```cpp
class BaseWifi {
public:
    // 🔧 Lifecycle management
    virtual ~BaseWifi() = default;
    virtual hf_wifi_err_t Initialize(hf_wifi_mode_t mode) = 0;
    virtual hf_wifi_err_t Deinitialize() = 0;
    virtual bool IsInitialized() const = 0;
    virtual hf_wifi_err_t SetMode(hf_wifi_mode_t mode) = 0;
    virtual hf_wifi_mode_t GetMode() const = 0;

    // 📱 Station mode operations
    virtual hf_wifi_err_t ConfigureStation(const hf_wifi_station_config_t& config) = 0;
    virtual hf_wifi_err_t ConnectStation(hf_timeout_ms_t timeout_ms = 0) = 0;
    virtual hf_wifi_err_t DisconnectStation() = 0;
    virtual bool IsStationConnected() const = 0;
    virtual hf_wifi_err_t GetStationInfo(hf_wifi_station_info_t& info) const = 0;

    // 🔥 Access Point operations
    virtual hf_wifi_err_t ConfigureAP(const hf_wifi_ap_config_t& config) = 0;
    virtual hf_wifi_err_t StartAP() = 0;
    virtual hf_wifi_err_t StopAP() = 0;
    virtual bool IsAPStarted() const = 0;
    virtual hf_wifi_err_t GetAPInfo(hf_wifi_ap_info_t& info) const = 0;

    // 🔍 Network scanning
    virtual hf_wifi_err_t StartScan(const hf_wifi_scan_config_t& config = {}) = 0;
    virtual hf_wifi_err_t GetScanResults(hf_wifi_scan_result_t* results, uint16_t& count) = 0;
    virtual bool IsScanInProgress() const = 0;

    // 📊 Network information
    virtual hf_wifi_err_t GetIPInfo(hf_wifi_ip_info_t& ip_info) const = 0;
    virtual int8_t GetRSSI() const = 0;
    virtual hf_wifi_err_t GetMACAddress(uint8_t mac[6]) const = 0;

    // 🎯 Event management
    virtual hf_wifi_err_t SetEventCallback(hf_wifi_event_callback_t callback) = 0;
    virtual hf_wifi_err_t ClearEventCallback() = 0;

    // 🔧 Power management
    virtual hf_wifi_err_t SetPowerSaveMode(hf_wifi_power_save_t mode) = 0;
    virtual hf_wifi_power_save_t GetPowerSaveMode() const = 0;
};
```

## 🎯 Core Methods

### 🔧 Initialization

```cpp
hf_wifi_err_t Initialize(hf_wifi_mode_t mode);
```
**Purpose:** 🚀 Initialize WiFi subsystem with specified mode  
**Parameters:** WiFi operating mode (STA, AP, or APSTA)  
**Returns:** Error code indicating success or failure

### 📱 Station Mode Operations

```cpp
hf_wifi_err_t ConfigureStation(const hf_wifi_station_config_t& config);
hf_wifi_err_t ConnectStation(hf_timeout_ms_t timeout_ms = 0);
hf_wifi_err_t DisconnectStation();
bool IsStationConnected() const;
```
**Purpose:** 📡 Connect to existing WiFi networks as a client  
**Parameters:** Network credentials, timeout values  
**Returns:** Connection status and error codes

### 🔥 Access Point Operations

```cpp
hf_wifi_err_t ConfigureAP(const hf_wifi_ap_config_t& config);
hf_wifi_err_t StartAP();
hf_wifi_err_t StopAP();
bool IsAPStarted() const;
```
**Purpose:** 🔥 Create and manage WiFi hotspots  
**Parameters:** AP configuration (SSID, password, security)  
**Returns:** AP status and error codes

### 🔍 Network Scanning

```cpp
hf_wifi_err_t StartScan(const hf_wifi_scan_config_t& config = {});
hf_wifi_err_t GetScanResults(hf_wifi_scan_result_t* results, uint16_t& count);
bool IsScanInProgress() const;
```
**Purpose:** 🔍 Discover and analyze available WiFi networks  
**Parameters:** Scan configuration and result buffers  
**Returns:** Available networks with signal strength and security info

## 💡 Usage Examples

### 📱 WiFi Station (Client) Mode

```cpp
#include "inc/mcu/esp32/EspWifi.h"

class WiFiClient {
private:
    EspWifi wifi_;
    bool connected_;
    
public:
    WiFiClient() : connected_(false) {}
    
    bool initialize() {
        // 🚀 Initialize WiFi in station mode
        hf_wifi_err_t result = wifi_.Initialize(hf_wifi_mode_t::WIFI_MODE_STA);
        if (result != hf_wifi_err_t::WIFI_SUCCESS) {
            printf("❌ Failed to initialize WiFi: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 📡 Set up event callback for connection monitoring
        wifi_.SetEventCallback([this](hf_wifi_event_t event, void* data) {
            handle_wifi_event(event, data);
        });
        
        printf("✅ WiFi initialized in station mode\n");
        return true;
    }
    
    bool connect_to_network(const std::string& ssid, const std::string& password) {
        // ⚙️ Configure station parameters
        hf_wifi_station_config_t config;
        config.ssid = ssid;
        config.password = password;
        config.bssid_set = false;  // Don't target specific BSSID
        config.channel = 0;        // Auto-select channel
        config.threshold_authmode = hf_wifi_security_t::WIFI_AUTH_WPA2_PSK;
        config.threshold_rssi = -80; // Minimum -80dBm signal strength
        
        hf_wifi_err_t result = wifi_.ConfigureStation(config);
        if (result != hf_wifi_err_t::WIFI_SUCCESS) {
            printf("❌ Failed to configure station: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 🔗 Attempt connection with 30 second timeout
        printf("🔗 Connecting to network '%s'...\n", ssid.c_str());
        result = wifi_.ConnectStation(30000);
        
        if (result == hf_wifi_err_t::WIFI_SUCCESS) {
            printf("✅ Connected to WiFi network\n");
            return true;
        } else {
            printf("❌ Connection failed: %s\n", HfWifiErrToString(result).data());
            return false;
        }
    }
    
    void disconnect() {
        if (connected_) {
            hf_wifi_err_t result = wifi_.DisconnectStation();
            if (result == hf_wifi_err_t::WIFI_SUCCESS) {
                printf("✅ Disconnected from WiFi\n");
            } else {
                printf("❌ Disconnect failed: %s\n", HfWifiErrToString(result).data());
            }
        }
    }
    
    void print_connection_info() {
        if (!connected_) {
            printf("❌ Not connected to WiFi\n");
            return;
        }
        
        // 📊 Get IP information
        hf_wifi_ip_info_t ip_info;
        if (wifi_.GetIPInfo(ip_info) == hf_wifi_err_t::WIFI_SUCCESS) {
            printf("🌐 IP Address: %d.%d.%d.%d\n",
                   (ip_info.ip >> 0) & 0xFF,
                   (ip_info.ip >> 8) & 0xFF,
                   (ip_info.ip >> 16) & 0xFF,
                   (ip_info.ip >> 24) & 0xFF);
            printf("🌐 Gateway: %d.%d.%d.%d\n",
                   (ip_info.gateway >> 0) & 0xFF,
                   (ip_info.gateway >> 8) & 0xFF,
                   (ip_info.gateway >> 16) & 0xFF,
                   (ip_info.gateway >> 24) & 0xFF);
        }
        
        // 📶 Get signal strength
        int8_t rssi = wifi_.GetRSSI();
        printf("📶 Signal Strength: %d dBm", rssi);
        if (rssi > -50) {
            printf(" (Excellent)\n");
        } else if (rssi > -60) {
            printf(" (Good)\n");
        } else if (rssi > -70) {
            printf(" (Fair)\n");
        } else {
            printf(" (Poor)\n");
        }
        
        // 🆔 Get MAC address
        uint8_t mac[6];
        if (wifi_.GetMACAddress(mac) == hf_wifi_err_t::WIFI_SUCCESS) {
            printf("🆔 MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
    }
    
    void handle_wifi_event(hf_wifi_event_t event, void* data) {
        switch (event) {
            case hf_wifi_event_t::WIFI_EVENT_STA_START:
                printf("📡 WiFi station started\n");
                break;
                
            case hf_wifi_event_t::WIFI_EVENT_STA_CONNECTED:
                printf("✅ Connected to access point\n");
                break;
                
            case hf_wifi_event_t::WIFI_EVENT_STA_GOT_IP:
                printf("🌐 Got IP address\n");
                connected_ = true;
                print_connection_info();
                break;
                
            case hf_wifi_event_t::WIFI_EVENT_STA_DISCONNECTED:
                printf("❌ Disconnected from access point\n");
                connected_ = false;
                break;
                
            case hf_wifi_event_t::WIFI_EVENT_STA_LOST_IP:
                printf("🌐 Lost IP address\n");
                connected_ = false;
                break;
                
            default:
                printf("📡 WiFi event: %d\n", static_cast<int>(event));
                break;
        }
    }
    
    bool is_connected() const {
        return connected_ && wifi_.IsStationConnected();
    }
};

void wifi_client_demo() {
    WiFiClient client;
    
    if (!client.initialize()) {
        printf("❌ WiFi client initialization failed\n");
        return;
    }
    
    // 🔗 Connect to your WiFi network
    if (client.connect_to_network("YourWiFiSSID", "YourPassword")) {
        printf("🎉 Successfully connected to WiFi!\n");
        
        // 📊 Monitor connection
        for (int i = 0; i < 60; i++) {  // Monitor for 1 minute
            if (client.is_connected()) {
                printf("📶 WiFi connected (check %d/60)\n", i + 1);
            } else {
                printf("❌ WiFi disconnected\n");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        client.disconnect();
    }
}
```

### 🔥 WiFi Access Point (Hotspot) Mode

```cpp
class WiFiHotspot {
private:
    EspWifi wifi_;
    bool ap_started_;
    uint8_t connected_clients_;
    
public:
    WiFiHotspot() : ap_started_(false), connected_clients_(0) {}
    
    bool initialize() {
        // 🚀 Initialize WiFi in AP mode
        hf_wifi_err_t result = wifi_.Initialize(hf_wifi_mode_t::WIFI_MODE_AP);
        if (result != hf_wifi_err_t::WIFI_SUCCESS) {
            printf("❌ Failed to initialize WiFi AP: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 📡 Set up event callback for client monitoring
        wifi_.SetEventCallback([this](hf_wifi_event_t event, void* data) {
            handle_ap_event(event, data);
        });
        
        printf("✅ WiFi initialized in AP mode\n");
        return true;
    }
    
    bool start_hotspot(const std::string& ssid, const std::string& password, 
                      uint8_t max_clients = 4) {
        // ⚙️ Configure access point
        hf_wifi_ap_config_t config;
        config.ssid = ssid;
        config.password = password;
        config.ssid_len = 0;  // Auto-calculate length
        config.channel = 1;   // Channel 1 (2.4GHz)
        config.authmode = password.empty() ? 
            hf_wifi_security_t::WIFI_AUTH_OPEN : 
            hf_wifi_security_t::WIFI_AUTH_WPA2_PSK;
        config.ssid_hidden = 0;  // Broadcast SSID
        config.max_connection = max_clients;
        config.beacon_interval = 100;  // 100ms beacon interval
        
        hf_wifi_err_t result = wifi_.ConfigureAP(config);
        if (result != hf_wifi_err_t::WIFI_SUCCESS) {
            printf("❌ Failed to configure AP: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 🔥 Start the access point
        result = wifi_.StartAP();
        if (result == hf_wifi_err_t::WIFI_SUCCESS) {
            printf("🔥 WiFi hotspot '%s' started successfully\n", ssid.c_str());
            printf("👥 Maximum clients: %u\n", max_clients);
            if (!password.empty()) {
                printf("🔒 Security: WPA2-PSK\n");
            } else {
                printf("🔓 Security: Open (no password)\n");
            }
            return true;
        } else {
            printf("❌ Failed to start AP: %s\n", HfWifiErrToString(result).data());
            return false;
        }
    }
    
    void stop_hotspot() {
        if (ap_started_) {
            hf_wifi_err_t result = wifi_.StopAP();
            if (result == hf_wifi_err_t::WIFI_SUCCESS) {
                printf("🔥 WiFi hotspot stopped\n");
            } else {
                printf("❌ Failed to stop hotspot: %s\n", HfWifiErrToString(result).data());
            }
        }
    }
    
    void print_ap_info() {
        if (!ap_started_) {
            printf("❌ Access point not started\n");
            return;
        }
        
        // 📊 Get AP information
        hf_wifi_ap_info_t ap_info;
        if (wifi_.GetAPInfo(ap_info) == hf_wifi_err_t::WIFI_SUCCESS) {
            printf("📡 AP SSID: %s\n", ap_info.ssid.c_str());
            printf("📻 Channel: %u\n", ap_info.channel);
            printf("👥 Connected Clients: %u/%u\n", 
                   connected_clients_, ap_info.max_connection);
        }
        
        // 🌐 Get IP information
        hf_wifi_ip_info_t ip_info;
        if (wifi_.GetIPInfo(ip_info) == hf_wifi_err_t::WIFI_SUCCESS) {
            printf("🌐 AP IP Address: %d.%d.%d.%d\n",
                   (ip_info.ip >> 0) & 0xFF,
                   (ip_info.ip >> 8) & 0xFF,
                   (ip_info.ip >> 16) & 0xFF,
                   (ip_info.ip >> 24) & 0xFF);
        }
        
        // 🆔 Get MAC address
        uint8_t mac[6];
        if (wifi_.GetMACAddress(mac) == hf_wifi_err_t::WIFI_SUCCESS) {
            printf("🆔 AP MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
    }
    
    void handle_ap_event(hf_wifi_event_t event, void* data) {
        switch (event) {
            case hf_wifi_event_t::WIFI_EVENT_AP_START:
                printf("🔥 Access point started\n");
                ap_started_ = true;
                print_ap_info();
                break;
                
            case hf_wifi_event_t::WIFI_EVENT_AP_STOP:
                printf("🔥 Access point stopped\n");
                ap_started_ = false;
                connected_clients_ = 0;
                break;
                
            case hf_wifi_event_t::WIFI_EVENT_AP_STA_CONNECTED:
                connected_clients_++;
                printf("👤 Client connected (total: %u)\n", connected_clients_);
                break;
                
            case hf_wifi_event_t::WIFI_EVENT_AP_STA_DISCONNECTED:
                if (connected_clients_ > 0) connected_clients_--;
                printf("👤 Client disconnected (total: %u)\n", connected_clients_);
                break;
                
            default:
                printf("📡 AP event: %d\n", static_cast<int>(event));
                break;
        }
    }
    
    bool is_running() const {
        return ap_started_ && wifi_.IsAPStarted();
    }
    
    uint8_t get_client_count() const {
        return connected_clients_;
    }
};

void wifi_hotspot_demo() {
    WiFiHotspot hotspot;
    
    if (!hotspot.initialize()) {
        printf("❌ WiFi hotspot initialization failed\n");
        return;
    }
    
    // 🔥 Start hotspot with custom settings
    if (hotspot.start_hotspot("HardFOC-Config", "hardfoc123", 8)) {
        printf("🎉 WiFi hotspot started successfully!\n");
        
        // 📊 Monitor hotspot for 5 minutes
        for (int i = 0; i < 300; i++) {  // 5 minutes = 300 seconds
            if (hotspot.is_running()) {
                printf("🔥 Hotspot running - %u clients connected (time: %ds)\n", 
                       hotspot.get_client_count(), i + 1);
            } else {
                printf("❌ Hotspot stopped unexpectedly\n");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        hotspot.stop_hotspot();
    }
}
```

### 🔍 WiFi Network Scanner

```cpp
class WiFiScanner {
private:
    EspWifi wifi_;
    std::vector<hf_wifi_scan_result_t> scan_results_;
    
public:
    bool initialize() {
        // 🚀 Initialize WiFi for scanning (station mode)
        hf_wifi_err_t result = wifi_.Initialize(hf_wifi_mode_t::WIFI_MODE_STA);
        if (result != hf_wifi_err_t::WIFI_SUCCESS) {
            printf("❌ Failed to initialize WiFi scanner: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        printf("✅ WiFi scanner initialized\n");
        return true;
    }
    
    bool scan_networks(bool show_hidden = false) {
        printf("🔍 Scanning for WiFi networks...\n");
        
        // ⚙️ Configure scan parameters
        hf_wifi_scan_config_t config;
        config.ssid = "";           // Scan all SSIDs
        config.bssid = nullptr;     // Scan all BSSIDs
        config.channel = 0;         // Scan all channels
        config.show_hidden = show_hidden;
        config.scan_type = hf_wifi_scan_type_t::WIFI_SCAN_TYPE_ACTIVE;
        config.scan_time.active.min = 120;  // Min scan time per channel (ms)
        config.scan_time.active.max = 150;  // Max scan time per channel (ms)
        
        // 🔍 Start the scan
        hf_wifi_err_t result = wifi_.StartScan(config);
        if (result != hf_wifi_err_t::WIFI_SUCCESS) {
            printf("❌ Failed to start scan: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // ⏰ Wait for scan to complete (with timeout)
        int timeout_count = 0;
        while (wifi_.IsScanInProgress() && timeout_count < 60) {  // 6 second timeout
            vTaskDelay(pdMS_TO_TICKS(100));
            timeout_count++;
        }
        
        if (wifi_.IsScanInProgress()) {
            printf("⏰ Scan timeout - may be incomplete\n");
            return false;
        }
        
        // 📊 Get scan results
        scan_results_.clear();
        scan_results_.resize(50);  // Prepare for up to 50 networks
        uint16_t count = scan_results_.size();
        
        result = wifi_.GetScanResults(scan_results_.data(), count);
        if (result != hf_wifi_err_t::WIFI_SUCCESS) {
            printf("❌ Failed to get scan results: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 📏 Resize to actual count
        scan_results_.resize(count);
        
        printf("✅ Scan completed - found %u networks\n", count);
        return true;
    }
    
    void print_scan_results() {
        if (scan_results_.empty()) {
            printf("❌ No scan results available\n");
            return;
        }
        
        printf("\n📊 WiFi Network Scan Results:\n");
        printf("═══════════════════════════════════════════════════════════════════════\n");
        printf("│ %-32s │ 📶 RSSI │ 📻 Ch │ 🔒 Security         │\n", "SSID");
        printf("├──────────────────────────────────────┼─────────┼───────┼─────────────────────┤\n");
        
        // 📊 Sort by signal strength (strongest first)
        std::sort(scan_results_.begin(), scan_results_.end(),
                  [](const hf_wifi_scan_result_t& a, const hf_wifi_scan_result_t& b) {
                      return a.rssi > b.rssi;
                  });
        
        for (const auto& result : scan_results_) {
            std::string ssid = result.ssid.empty() ? "<Hidden Network>" : result.ssid;
            if (ssid.length() > 32) {
                ssid = ssid.substr(0, 29) + "...";
            }
            
            std::string signal_bar = get_signal_bars(result.rssi);
            std::string security = get_security_string(result.authmode);
            
            printf("│ %-32s │ %3d dBm │  %2u   │ %-19s │\n",
                   ssid.c_str(), result.rssi, result.primary_channel, security.c_str());
        }
        
        printf("└──────────────────────────────────────┴─────────┴───────┴─────────────────────┘\n");
        
        print_scan_statistics();
    }
    
    void print_scan_statistics() {
        if (scan_results_.empty()) return;
        
        printf("\n📈 Scan Statistics:\n");
        
        // 📊 Count by security type
        std::map<hf_wifi_security_t, int> security_counts;
        std::map<uint8_t, int> channel_counts;
        int strong_signals = 0, weak_signals = 0;
        
        for (const auto& result : scan_results_) {
            security_counts[result.authmode]++;
            channel_counts[result.primary_channel]++;
            
            if (result.rssi > -60) {
                strong_signals++;
            } else if (result.rssi < -80) {
                weak_signals++;
            }
        }
        
        printf("   🔒 Security Distribution:\n");
        for (const auto& [auth, count] : security_counts) {
            printf("      %s: %d networks\n", get_security_string(auth).c_str(), count);
        }
        
        printf("   📻 Popular Channels:\n");
        auto top_channels = get_top_channels(channel_counts, 3);
        for (const auto& [channel, count] : top_channels) {
            printf("      Channel %u: %d networks\n", channel, count);
        }
        
        printf("   📶 Signal Quality:\n");
        printf("      Strong (>-60dBm): %d networks\n", strong_signals);
        printf("      Weak (<-80dBm): %d networks\n", weak_signals);
    }
    
private:
    std::string get_signal_bars(int8_t rssi) {
        if (rssi > -50) return "████";      // Excellent
        else if (rssi > -60) return "███ ";  // Good
        else if (rssi > -70) return "██  ";  // Fair
        else if (rssi > -80) return "█   ";  // Poor
        else return "    ";                  // Very poor
    }
    
    std::string get_security_string(hf_wifi_security_t auth) {
        switch (auth) {
            case hf_wifi_security_t::WIFI_AUTH_OPEN: return "🔓 Open";
            case hf_wifi_security_t::WIFI_AUTH_WEP: return "🔐 WEP";
            case hf_wifi_security_t::WIFI_AUTH_WPA_PSK: return "🔒 WPA";
            case hf_wifi_security_t::WIFI_AUTH_WPA2_PSK: return "🔒 WPA2";
            case hf_wifi_security_t::WIFI_AUTH_WPA_WPA2_PSK: return "🔒 WPA/WPA2";
            case hf_wifi_security_t::WIFI_AUTH_WPA2_ENTERPRISE: return "🏢 WPA2-Enterprise";
            case hf_wifi_security_t::WIFI_AUTH_WPA3_PSK: return "🛡️ WPA3";
            case hf_wifi_security_t::WIFI_AUTH_WPA2_WPA3_PSK: return "🛡️ WPA2/WPA3";
            default: return "❓ Unknown";
        }
    }
    
    std::vector<std::pair<uint8_t, int>> get_top_channels(
        const std::map<uint8_t, int>& channel_counts, int limit) {
        std::vector<std::pair<uint8_t, int>> sorted_channels(
            channel_counts.begin(), channel_counts.end());
        
        std::sort(sorted_channels.begin(), sorted_channels.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        if (sorted_channels.size() > limit) {
            sorted_channels.resize(limit);
        }
        
        return sorted_channels;
    }
};

void wifi_scanner_demo() {
    WiFiScanner scanner;
    
    if (!scanner.initialize()) {
        printf("❌ WiFi scanner initialization failed\n");
        return;
    }
    
    // 🔍 Perform network scan
    if (scanner.scan_networks(true)) {  // Include hidden networks
        scanner.print_scan_results();
    } else {
        printf("❌ Network scan failed\n");
    }
}
```

## 🏎️ Performance Considerations

### ⚡ Optimization Tips

- **📶 Signal Strength** - Maintain RSSI above -70dBm for reliable operation
- **📻 Channel Selection** - Use channels 1, 6, or 11 for 2.4GHz to avoid interference
- **🔋 Power Management** - Use power save modes for battery-powered applications
- **🔄 Reconnection Logic** - Implement automatic reconnection for critical applications
- **📊 Connection Monitoring** - Monitor signal quality and implement roaming logic

### 📊 Typical Performance Ranges

| **WiFi Standard** | **Max Speed** | **Range** | **Power Consumption** |
|-------------------|---------------|-----------|----------------------|
| **802.11b** | 11 Mbps | ~150m outdoor | Low |
| **802.11g** | 54 Mbps | ~150m outdoor | Medium |
| **802.11n** | 300 Mbps | ~250m outdoor | Medium-High |

## 🛡️ Security Best Practices

### 🔒 Secure WiFi Implementation

```cpp
// ✅ Use strong security protocols
config.authmode = hf_wifi_security_t::WIFI_AUTH_WPA3_PSK;  // Prefer WPA3

// ✅ Set minimum security thresholds
config.threshold_authmode = hf_wifi_security_t::WIFI_AUTH_WPA2_PSK;

// ✅ Use strong passwords
config.password = "YourSecurePassword123!@#";  // Strong password example

// ✅ Monitor security events
wifi.SetEventCallback([](hf_wifi_event_t event, void* data) {
    if (event == hf_wifi_event_t::WIFI_EVENT_STA_DISCONNECTED) {
        // Log security events
        printf("🔒 Security: Connection lost - investigating...\n");
    }
});
```

## 🧵 Thread Safety

The `BaseWifi` class is **not inherently thread-safe**. For concurrent access from multiple tasks, use appropriate synchronization mechanisms.

## 🔗 Related Documentation

- **[EspWifi API Reference](../esp_api/EspWifi.md)** - ESP32-C6 WiFi implementation
- **[BaseLogger API Reference](BaseLogger.md)** - Logging WiFi events and diagnostics
- **[HardwareTypes Reference](HardwareTypes.md)** - Platform-agnostic type definitions

---

<div align="center">

**📶 BaseWifi - Connecting HardFOC to the World** 🌐

*From IoT connectivity to device configuration - BaseWifi enables seamless wireless communication* 🚀

</div>