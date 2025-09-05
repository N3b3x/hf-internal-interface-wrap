# 📶 BaseWifi API Reference

<div align="center">

**📋 Navigation**

[← Previous: BaseCan](BaseCan.md) | [Back to API Index](README.md) | [Next: BaseBluetooth
→](BaseBluetooth.md)

</div>

---

## 🌟 Overview

`BaseWifi` is the abstract base class for all WiFi implementations in the HardFOC system.
It provides a unified interface for wireless networking operations including station mode (client),
access point mode (hotspot), network scanning, security configuration,
and comprehensive connection management.

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
```text

## 🎯 Type Definitions

### 🚨 Error Codes

```cpp
enum class hf*wifi*err*t : hf*u8*t {
    WIFI*SUCCESS = 0,                   // ✅ Success
    WIFI*ERR*FAILURE = 1,               // ❌ General failure
    WIFI*ERR*INVALID*PARAM = 2,         // 🚫 Invalid parameter
    WIFI*ERR*NOT*INITIALIZED = 3,       // ⚠️ WiFi not initialized
    WIFI*ERR*ALREADY*INITIALIZED = 4,   // ⚠️ WiFi already initialized
    WIFI*ERR*NOT*CONNECTED = 5,         // 📶 WiFi not connected
    WIFI*ERR*ALREADY*CONNECTED = 6,     // 📶 WiFi already connected
    WIFI*ERR*CONNECTION*FAILED = 7,     // ❌ Connection failed
    WIFI*ERR*DISCONNECTION*FAILED = 8,  // ❌ Disconnection failed
    WIFI*ERR*SCAN*FAILED = 9,           // 🔍 Network scan failed
    WIFI*ERR*AP*START*FAILED = 10,      // 🔥 Access Point start failed
    WIFI*ERR*AP*STOP*FAILED = 11,       // 🔥 Access Point stop failed
    WIFI*ERR*TIMEOUT = 12,              // ⏰ Operation timeout
    WIFI*ERR*NO*MEMORY = 13,            // 💾 Insufficient memory
    WIFI*ERR*INVALID*SSID = 14,         // 📡 Invalid SSID
    WIFI*ERR*INVALID*PASSWORD = 15,     // 🔐 Invalid password
    WIFI*ERR*WEAK*SIGNAL = 16,          // 📉 Weak signal strength
    WIFI*ERR*AUTHENTICATION*FAILED = 17, // 🔐 Authentication failed
    WIFI*ERR*ASSOCIATION*FAILED = 18,   // 🔗 Association failed
    WIFI*ERR*HANDSHAKE*FAILED = 19,     // 🤝 4-way handshake failed
    WIFI*ERR*INIT*FAILED = 20,          // 🚀 WiFi initialization failed
    WIFI*ERR*CONFIG*INVALID = 21,       // ⚙️ Invalid configuration
    WIFI*ERR*ENTERPRISE*FAILED = 22,    // 🏢 Enterprise authentication failed
    WIFI*ERR*WPA3*NOT*SUPPORTED = 23,   // 🔒 WPA3 not supported
    WIFI*ERR*MESH*FAILED = 24           // 🕸️ Mesh operation failed
};
```text

### 🌐 WiFi Modes

```cpp
enum class hf*wifi*mode*t : hf*u8*t {
    WIFI*MODE*NULL = 0,     // 🚫 WiFi disabled
    WIFI*MODE*STA = 1,      // 📱 Station mode (client)
    WIFI*MODE*AP = 2,       // 🔥 Access Point mode
    WIFI*MODE*APSTA = 3     // 🔄 Combined AP+STA mode
};
```text

### 🔒 Security Types

```cpp
enum class hf*wifi*security*t : hf*u8*t {
    WIFI*AUTH*OPEN = 0,         // 🔓 Open network (no security)
    WIFI*AUTH*WEP = 1,          // 🔐 WEP (deprecated)
    WIFI*AUTH*WPA*PSK = 2,      // 🔒 WPA-PSK
    WIFI*AUTH*WPA2*PSK = 3,     // 🔒 WPA2-PSK (most common)
    WIFI*AUTH*WPA*WPA2*PSK = 4, // 🔒 WPA/WPA2-PSK mixed
    WIFI*AUTH*WPA2*ENTERPRISE = 5, // 🏢 WPA2-Enterprise
    WIFI*AUTH*WPA3*PSK = 6,     // 🛡️ WPA3-PSK (latest)
    WIFI*AUTH*WPA2*WPA3*PSK = 7 // 🛡️ WPA2/WPA3-PSK mixed
};
```text

### 📊 Event Types

```cpp
enum class hf*wifi*event*t : hf*u8*t {
    WIFI*EVENT*STA*START = 0,       // 🚀 Station started
    WIFI*EVENT*STA*STOP = 1,        // 🛑 Station stopped
    WIFI*EVENT*STA*CONNECTED = 2,   // ✅ Connected to AP
    WIFI*EVENT*STA*DISCONNECTED = 3, // ❌ Disconnected from AP
    WIFI*EVENT*STA*GOT*IP = 4,      // 🌐 Got IP address
    WIFI*EVENT*STA*LOST*IP = 5,     // 🌐 Lost IP address
    WIFI*EVENT*AP*START = 6,        // 🔥 AP started
    WIFI*EVENT*AP*STOP = 7,         // 🔥 AP stopped
    WIFI*EVENT*AP*STA*CONNECTED = 8, // 👤 Client connected to AP
    WIFI*EVENT*AP*STA*DISCONNECTED = 9, // 👤 Client disconnected from AP
    WIFI*EVENT*SCAN*DONE = 10       // 🔍 Network scan completed
};
```text

### ⚙️ Configuration Structures

```cpp
struct hf*wifi*station*config*t {
    std::string ssid;                      // 📡 Network SSID
    std::string password;                  // 🔐 Network password
    uint8*t bssid[6];                     // 🆔 Target BSSID (optional)
    bool bssid*set;                       // 🎯 Use specific BSSID
    uint8*t channel;                      // 📻 WiFi channel (0 = auto)
    uint16*t listen*interval;             // ⏰ Listen interval
    hf*wifi*security*t threshold*authmode; // 🔒 Minimum security level
    int16*t threshold*rssi;               // 📶 Minimum signal strength
};

struct hf*wifi*ap*config*t {
    std::string ssid;            // 📡 AP SSID
    std::string password;        // 🔐 AP password
    uint8*t ssid*len;           // 📏 SSID length (0 for auto)
    uint8*t channel;            // 📻 WiFi channel
    hf*wifi*security*t authmode; // 🔒 Authentication mode
    uint8*t ssid*hidden;        // 👻 Hide SSID (0 = visible, 1 = hidden)
    uint8*t max*connection;     // 👥 Maximum concurrent connections
    uint16*t beacon*interval;   // 📡 Beacon interval (ms)
};

struct hf*wifi*scan*result*t {
    std::string ssid;              // 📡 Network SSID
    uint8*t bssid[6];             // 🆔 Network BSSID
    uint8*t primary*channel;       // 📻 Primary channel
    uint8*t secondary*channel;     // 📻 Secondary channel
    int8*t rssi;                  // 📶 Signal strength (dBm)
    hf*wifi*security*t authmode;   // 🔒 Authentication mode
    uint32*t phy*11b:1;           // 📊 802.11b support
    uint32*t phy*11g:1;           // 📊 802.11g support
    uint32*t phy*11n:1;           // 📊 802.11n support
    uint32*t wps:1;               // 🔧 WPS support
};
```text

## 🏗️ Class Interface

```cpp
class BaseWifi {
public:
    // 🔧 Lifecycle management
    virtual ~BaseWifi() = default;
    virtual hf*wifi*err*t Initialize(hf*wifi*mode*t mode) = 0;
    virtual hf*wifi*err*t Deinitialize() = 0;
    virtual bool IsInitialized() const = 0;
    virtual hf*wifi*err*t SetMode(hf*wifi*mode*t mode) = 0;
    virtual hf*wifi*mode*t GetMode() const = 0;

    // 📱 Station mode operations
    virtual hf*wifi*err*t ConfigureStation(const hf*wifi*station*config*t& config) = 0;
    virtual hf*wifi*err*t ConnectStation(hf*timeout*ms*t timeout*ms = 0) = 0;
    virtual hf*wifi*err*t DisconnectStation() = 0;
    virtual bool IsStationConnected() const = 0;
    virtual hf*wifi*err*t GetStationInfo(hf*wifi*station*info*t& info) const = 0;

    // 🔥 Access Point operations
    virtual hf*wifi*err*t ConfigureAP(const hf*wifi*ap*config*t& config) = 0;
    virtual hf*wifi*err*t StartAP() = 0;
    virtual hf*wifi*err*t StopAP() = 0;
    virtual bool IsAPStarted() const = 0;
    virtual hf*wifi*err*t GetAPInfo(hf*wifi*ap*info*t& info) const = 0;

    // 🔍 Network scanning
    virtual hf*wifi*err*t StartScan(const hf*wifi*scan*config*t& config = {}) = 0;
    virtual hf*wifi*err*t GetScanResults(hf*wifi*scan*result*t* results, uint16*t& count) = 0;
    virtual bool IsScanInProgress() const = 0;

    // 📊 Network information
    virtual hf*wifi*err*t GetIPInfo(hf*wifi*ip*info*t& ip*info) const = 0;
    virtual int8*t GetRSSI() const = 0;
    virtual hf*wifi*err*t GetMACAddress(uint8*t mac[6]) const = 0;

    // 🎯 Event management
    virtual hf*wifi*err*t SetEventCallback(hf*wifi*event*callback*t callback) = 0;
    virtual hf*wifi*err*t ClearEventCallback() = 0;

    // 🔧 Power management
    virtual hf*wifi*err*t SetPowerSaveMode(hf*wifi*power*save*t mode) = 0;
    virtual hf*wifi*power*save*t GetPowerSaveMode() const = 0;
};
```text

## 🎯 Core Methods

### 🔧 Initialization

```cpp
hf*wifi*err*t Initialize(hf*wifi*mode*t mode);
```text
**Purpose:** 🚀 Initialize WiFi subsystem with specified mode  
**Parameters:** WiFi operating mode (STA, AP, or APSTA)  
**Returns:** Error code indicating success or failure

### 📱 Station Mode Operations

```cpp
hf*wifi*err*t ConfigureStation(const hf*wifi*station*config*t& config);
hf*wifi*err*t ConnectStation(hf*timeout*ms*t timeout*ms = 0);
hf*wifi*err*t DisconnectStation();
bool IsStationConnected() const;
```text
**Purpose:** 📡 Connect to existing WiFi networks as a client  
**Parameters:** Network credentials, timeout values  
**Returns:** Connection status and error codes

### 🔥 Access Point Operations

```cpp
hf*wifi*err*t ConfigureAP(const hf*wifi*ap*config*t& config);
hf*wifi*err*t StartAP();
hf*wifi*err*t StopAP();
bool IsAPStarted() const;
```text
**Purpose:** 🔥 Create and manage WiFi hotspots  
**Parameters:** AP configuration (SSID, password, security)  
**Returns:** AP status and error codes

### 🔍 Network Scanning

```cpp
hf*wifi*err*t StartScan(const hf*wifi*scan*config*t& config = {});
hf*wifi*err*t GetScanResults(hf*wifi*scan*result*t* results, uint16*t& count);
bool IsScanInProgress() const;
```text
**Purpose:** 🔍 Discover and analyze available WiFi networks  
**Parameters:** Scan configuration and result buffers  
**Returns:** Available networks with signal strength and security info

## 💡 Usage Examples

### 📱 WiFi Station (Client) Mode

```cpp
#include "inc/mcu/esp32/EspWifi.h"

class WiFiClient {
private:
    EspWifi wifi*;
    bool connected*;
    
public:
    WiFiClient() : connected*(false) {}
    
    bool initialize() {
        // 🚀 Initialize WiFi in station mode
        hf*wifi*err*t result = wifi*.Initialize(hf*wifi*mode*t::WIFI*MODE*STA);
        if (result != hf*wifi*err*t::WIFI*SUCCESS) {
            printf("❌ Failed to initialize WiFi: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 📡 Set up event callback for connection monitoring
        wifi*.SetEventCallback([this](hf*wifi*event*t event, void* data) {
            handle*wifi*event(event, data);
        });
        
        printf("✅ WiFi initialized in station mode\n");
        return true;
    }
    
    bool connect*to*network(const std::string& ssid, const std::string& password) {
        // ⚙️ Configure station parameters
        hf*wifi*station*config*t config;
        config.ssid = ssid;
        config.password = password;
        config.bssid*set = false;  // Don't target specific BSSID
        config.channel = 0;        // Auto-select channel
        config.threshold*authmode = hf*wifi*security*t::WIFI*AUTH*WPA2*PSK;
        config.threshold*rssi = -80; // Minimum -80dBm signal strength
        
        hf*wifi*err*t result = wifi*.ConfigureStation(config);
        if (result != hf*wifi*err*t::WIFI*SUCCESS) {
            printf("❌ Failed to configure station: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 🔗 Attempt connection with 30 second timeout
        printf("🔗 Connecting to network '%s'...\n", ssid.c*str());
        result = wifi*.ConnectStation(30000);
        
        if (result == hf*wifi*err*t::WIFI*SUCCESS) {
            printf("✅ Connected to WiFi network\n");
            return true;
        } else {
            printf("❌ Connection failed: %s\n", HfWifiErrToString(result).data());
            return false;
        }
    }
    
    void disconnect() {
        if (connected*) {
            hf*wifi*err*t result = wifi*.DisconnectStation();
            if (result == hf*wifi*err*t::WIFI*SUCCESS) {
                printf("✅ Disconnected from WiFi\n");
            } else {
                printf("❌ Disconnect failed: %s\n", HfWifiErrToString(result).data());
            }
        }
    }
    
    void print*connection*info() {
        if (!connected*) {
            printf("❌ Not connected to WiFi\n");
            return;
        }
        
        // 📊 Get IP information
        hf*wifi*ip*info*t ip*info;
        if (wifi*.GetIPInfo(ip*info) == hf*wifi*err*t::WIFI*SUCCESS) {
            printf("🌐 IP Address: %d.%d.%d.%d\n",
                   (ip*info.ip >> 0) & 0xFF,
                   (ip*info.ip >> 8) & 0xFF,
                   (ip*info.ip >> 16) & 0xFF,
                   (ip*info.ip >> 24) & 0xFF);
            printf("🌐 Gateway: %d.%d.%d.%d\n",
                   (ip*info.gateway >> 0) & 0xFF,
                   (ip*info.gateway >> 8) & 0xFF,
                   (ip*info.gateway >> 16) & 0xFF,
                   (ip*info.gateway >> 24) & 0xFF);
        }
        
        // 📶 Get signal strength
        int8*t rssi = wifi*.GetRSSI();
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
        uint8*t mac[6];
        if (wifi*.GetMACAddress(mac) == hf*wifi*err*t::WIFI*SUCCESS) {
            printf("🆔 MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
    }
    
    void handle*wifi*event(hf*wifi*event*t event, void* data) {
        switch (event) {
            case hf*wifi*event*t::WIFI*EVENT*STA*START:
                printf("📡 WiFi station started\n");
                break;
                
            case hf*wifi*event*t::WIFI*EVENT*STA*CONNECTED:
                printf("✅ Connected to access point\n");
                break;
                
            case hf*wifi*event*t::WIFI*EVENT*STA*GOT*IP:
                printf("🌐 Got IP address\n");
                connected* = true;
                print*connection*info();
                break;
                
            case hf*wifi*event*t::WIFI*EVENT*STA*DISCONNECTED:
                printf("❌ Disconnected from access point\n");
                connected* = false;
                break;
                
            case hf*wifi*event*t::WIFI*EVENT*STA*LOST*IP:
                printf("🌐 Lost IP address\n");
                connected* = false;
                break;
                
            default:
                printf("📡 WiFi event: %d\n", static*cast<int>(event));
                break;
        }
    }
    
    bool is*connected() const {
        return connected* && wifi*.IsStationConnected();
    }
};

void wifi*client*demo() {
    WiFiClient client;
    
    if (!client.initialize()) {
        printf("❌ WiFi client initialization failed\n");
        return;
    }
    
    // 🔗 Connect to your WiFi network
    if (client.connect*to*network("YourWiFiSSID", "YourPassword")) {
        printf("🎉 Successfully connected to WiFi!\n");
        
        // 📊 Monitor connection
        for (int i = 0; i < 60; i++) {  // Monitor for 1 minute
            if (client.is*connected()) {
                printf("📶 WiFi connected (check %d/60)\n", i + 1);
            } else {
                printf("❌ WiFi disconnected\n");
                break;
            }
            vTaskDelay(pdMS*TO*TICKS(1000));
        }
        
        client.disconnect();
    }
}
```text

### 🔥 WiFi Access Point (Hotspot) Mode

```cpp
class WiFiHotspot {
private:
    EspWifi wifi*;
    bool ap*started*;
    uint8*t connected*clients*;
    
public:
    WiFiHotspot() : ap*started*(false), connected*clients*(0) {}
    
    bool initialize() {
        // 🚀 Initialize WiFi in AP mode
        hf*wifi*err*t result = wifi*.Initialize(hf*wifi*mode*t::WIFI*MODE*AP);
        if (result != hf*wifi*err*t::WIFI*SUCCESS) {
            printf("❌ Failed to initialize WiFi AP: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 📡 Set up event callback for client monitoring
        wifi*.SetEventCallback([this](hf*wifi*event*t event, void* data) {
            handle*ap*event(event, data);
        });
        
        printf("✅ WiFi initialized in AP mode\n");
        return true;
    }
    
    bool start*hotspot(const std::string& ssid, const std::string& password, 
                      uint8*t max*clients = 4) {
        // ⚙️ Configure access point
        hf*wifi*ap*config*t config;
        config.ssid = ssid;
        config.password = password;
        config.ssid*len = 0;  // Auto-calculate length
        config.channel = 1;   // Channel 1 (2.4GHz)
        config.authmode = password.empty() ? 
            hf*wifi*security*t::WIFI*AUTH*OPEN : 
            hf*wifi*security*t::WIFI*AUTH*WPA2*PSK;
        config.ssid*hidden = 0;  // Broadcast SSID
        config.max*connection = max*clients;
        config.beacon*interval = 100;  // 100ms beacon interval
        
        hf*wifi*err*t result = wifi*.ConfigureAP(config);
        if (result != hf*wifi*err*t::WIFI*SUCCESS) {
            printf("❌ Failed to configure AP: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 🔥 Start the access point
        result = wifi*.StartAP();
        if (result == hf*wifi*err*t::WIFI*SUCCESS) {
            printf("🔥 WiFi hotspot '%s' started successfully\n", ssid.c*str());
            printf("👥 Maximum clients: %u\n", max*clients);
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
    
    void stop*hotspot() {
        if (ap*started*) {
            hf*wifi*err*t result = wifi*.StopAP();
            if (result == hf*wifi*err*t::WIFI*SUCCESS) {
                printf("🔥 WiFi hotspot stopped\n");
            } else {
                printf("❌ Failed to stop hotspot: %s\n", HfWifiErrToString(result).data());
            }
        }
    }
    
    void print*ap*info() {
        if (!ap*started*) {
            printf("❌ Access point not started\n");
            return;
        }
        
        // 📊 Get AP information
        hf*wifi*ap*info*t ap*info;
        if (wifi*.GetAPInfo(ap*info) == hf*wifi*err*t::WIFI*SUCCESS) {
            printf("📡 AP SSID: %s\n", ap*info.ssid.c*str());
            printf("📻 Channel: %u\n", ap*info.channel);
            printf("👥 Connected Clients: %u/%u\n", 
                   connected*clients*, ap*info.max*connection);
        }
        
        // 🌐 Get IP information
        hf*wifi*ip*info*t ip*info;
        if (wifi*.GetIPInfo(ip*info) == hf*wifi*err*t::WIFI*SUCCESS) {
            printf("🌐 AP IP Address: %d.%d.%d.%d\n",
                   (ip*info.ip >> 0) & 0xFF,
                   (ip*info.ip >> 8) & 0xFF,
                   (ip*info.ip >> 16) & 0xFF,
                   (ip*info.ip >> 24) & 0xFF);
        }
        
        // 🆔 Get MAC address
        uint8*t mac[6];
        if (wifi*.GetMACAddress(mac) == hf*wifi*err*t::WIFI*SUCCESS) {
            printf("🆔 AP MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
    }
    
    void handle*ap*event(hf*wifi*event*t event, void* data) {
        switch (event) {
            case hf*wifi*event*t::WIFI*EVENT*AP*START:
                printf("🔥 Access point started\n");
                ap*started* = true;
                print*ap*info();
                break;
                
            case hf*wifi*event*t::WIFI*EVENT*AP*STOP:
                printf("🔥 Access point stopped\n");
                ap*started* = false;
                connected*clients* = 0;
                break;
                
            case hf*wifi*event*t::WIFI*EVENT*AP*STA*CONNECTED:
                connected*clients*++;
                printf("👤 Client connected (total: %u)\n", connected*clients*);
                break;
                
            case hf*wifi*event*t::WIFI*EVENT*AP*STA*DISCONNECTED:
                if (connected*clients* > 0) connected*clients*--;
                printf("👤 Client disconnected (total: %u)\n", connected*clients*);
                break;
                
            default:
                printf("📡 AP event: %d\n", static*cast<int>(event));
                break;
        }
    }
    
    bool is*running() const {
        return ap*started* && wifi*.IsAPStarted();
    }
    
    uint8*t get*client*count() const {
        return connected*clients*;
    }
};

void wifi*hotspot*demo() {
    WiFiHotspot hotspot;
    
    if (!hotspot.initialize()) {
        printf("❌ WiFi hotspot initialization failed\n");
        return;
    }
    
    // 🔥 Start hotspot with custom settings
    if (hotspot.start*hotspot("HardFOC-Config", "hardfoc123", 8)) {
        printf("🎉 WiFi hotspot started successfully!\n");
        
        // 📊 Monitor hotspot for 5 minutes
        for (int i = 0; i < 300; i++) {  // 5 minutes = 300 seconds
            if (hotspot.is*running()) {
                printf("🔥 Hotspot running - %u clients connected (time: %ds)\n", 
                       hotspot.get*client*count(), i + 1);
            } else {
                printf("❌ Hotspot stopped unexpectedly\n");
                break;
            }
            vTaskDelay(pdMS*TO*TICKS(1000));
        }
        
        hotspot.stop*hotspot();
    }
}
```text

### 🔍 WiFi Network Scanner

```cpp
class WiFiScanner {
private:
    EspWifi wifi*;
    std::vector<hf*wifi*scan*result*t> scan*results*;
    
public:
    bool initialize() {
        // 🚀 Initialize WiFi for scanning (station mode)
        hf*wifi*err*t result = wifi*.Initialize(hf*wifi*mode*t::WIFI*MODE*STA);
        if (result != hf*wifi*err*t::WIFI*SUCCESS) {
            printf("❌ Failed to initialize WiFi scanner: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        printf("✅ WiFi scanner initialized\n");
        return true;
    }
    
    bool scan*networks(bool show*hidden = false) {
        printf("🔍 Scanning for WiFi networks...\n");
        
        // ⚙️ Configure scan parameters
        hf*wifi*scan*config*t config;
        config.ssid = "";           // Scan all SSIDs
        config.bssid = nullptr;     // Scan all BSSIDs
        config.channel = 0;         // Scan all channels
        config.show*hidden = show*hidden;
        config.scan*type = hf*wifi*scan*type*t::WIFI*SCAN*TYPE*ACTIVE;
        config.scan*time.active.min = 120;  // Min scan time per channel (ms)
        config.scan*time.active.max = 150;  // Max scan time per channel (ms)
        
        // 🔍 Start the scan
        hf*wifi*err*t result = wifi*.StartScan(config);
        if (result != hf*wifi*err*t::WIFI*SUCCESS) {
            printf("❌ Failed to start scan: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // ⏰ Wait for scan to complete (with timeout)
        int timeout*count = 0;
        while (wifi*.IsScanInProgress() && timeout*count < 60) {  // 6 second timeout
            vTaskDelay(pdMS*TO*TICKS(100));
            timeout*count++;
        }
        
        if (wifi*.IsScanInProgress()) {
            printf("⏰ Scan timeout - may be incomplete\n");
            return false;
        }
        
        // 📊 Get scan results
        scan*results*.clear();
        scan*results*.resize(50);  // Prepare for up to 50 networks
        uint16*t count = scan*results*.size();
        
        result = wifi*.GetScanResults(scan*results*.data(), count);
        if (result != hf*wifi*err*t::WIFI*SUCCESS) {
            printf("❌ Failed to get scan results: %s\n", HfWifiErrToString(result).data());
            return false;
        }
        
        // 📏 Resize to actual count
        scan*results*.resize(count);
        
        printf("✅ Scan completed - found %u networks\n", count);
        return true;
    }
    
    void print*scan*results() {
        if (scan*results*.empty()) {
            printf("❌ No scan results available\n");
            return;
        }
        
        printf("\n📊 WiFi Network Scan Results:\n");
        printf("═══════════════════════════════════════════════════════════════════════\n");
        printf("│ %-32s │ 📶 RSSI │ 📻 Ch │ 🔒 Security         │\n", "SSID");
        printf("├──────────────────────────────────────┼─────────┼───────┼─────────────────────┤\n");
        
        // 📊 Sort by signal strength (strongest first)
        std::sort(scan*results*.begin(), scan*results*.end(),
                  [](const hf*wifi*scan*result*t& a, const hf*wifi*scan*result*t& b) {
                      return a.rssi > b.rssi;
                  });
        
        for (const auto& result : scan*results*) {
            std::string ssid = result.ssid.empty() ? "<Hidden Network>" : result.ssid;
            if (ssid.length() > 32) {
                ssid = ssid.substr(0, 29) + "...";
            }
            
            std::string signal*bar = get*signal*bars(result.rssi);
            std::string security = get*security*string(result.authmode);
            
            printf("│ %-32s │ %3d dBm │  %2u   │ %-19s │\n",
                   ssid.c*str(), result.rssi, result.primary*channel, security.c*str());
        }
        
        printf("└──────────────────────────────────────┴─────────┴───────┴─────────────────────┘\n");
        
        print*scan*statistics();
    }
    
    void print*scan*statistics() {
        if (scan*results*.empty()) return;
        
        printf("\n📈 Scan Statistics:\n");
        
        // 📊 Count by security type
        std::map<hf*wifi*security*t, int> security*counts;
        std::map<uint8*t, int> channel*counts;
        int strong*signals = 0, weak*signals = 0;
        
        for (const auto& result : scan*results*) {
            security*counts[result.authmode]++;
            channel*counts[result.primary*channel]++;
            
            if (result.rssi > -60) {
                strong*signals++;
            } else if (result.rssi < -80) {
                weak*signals++;
            }
        }
        
        printf("   🔒 Security Distribution:\n");
        for (const auto& [auth, count] : security*counts) {
            printf("      %s: %d networks\n", get*security*string(auth).c*str(), count);
        }
        
        printf("   📻 Popular Channels:\n");
        auto top*channels = get*top*channels(channel*counts, 3);
        for (const auto& [channel, count] : top*channels) {
            printf("      Channel %u: %d networks\n", channel, count);
        }
        
        printf("   📶 Signal Quality:\n");
        printf("      Strong (>-60dBm): %d networks\n", strong*signals);
        printf("      Weak (<-80dBm): %d networks\n", weak*signals);
    }
    
private:
    std::string get*signal*bars(int8*t rssi) {
        if (rssi > -50) return "████";      // Excellent
        else if (rssi > -60) return "███ ";  // Good
        else if (rssi > -70) return "██  ";  // Fair
        else if (rssi > -80) return "█   ";  // Poor
        else return "    ";                  // Very poor
    }
    
    std::string get*security*string(hf*wifi*security*t auth) {
        switch (auth) {
            case hf*wifi*security*t::WIFI*AUTH*OPEN: return "🔓 Open";
            case hf*wifi*security*t::WIFI*AUTH*WEP: return "🔐 WEP";
            case hf*wifi*security*t::WIFI*AUTH*WPA*PSK: return "🔒 WPA";
            case hf*wifi*security*t::WIFI*AUTH*WPA2*PSK: return "🔒 WPA2";
            case hf*wifi*security*t::WIFI*AUTH*WPA*WPA2*PSK: return "🔒 WPA/WPA2";
            case hf*wifi*security*t::WIFI*AUTH*WPA2*ENTERPRISE: return "🏢 WPA2-Enterprise";
            case hf*wifi*security*t::WIFI*AUTH*WPA3*PSK: return "🛡️ WPA3";
            case hf*wifi*security*t::WIFI*AUTH*WPA2*WPA3*PSK: return "🛡️ WPA2/WPA3";
            default: return "❓ Unknown";
        }
    }
    
    std::vector<std::pair<uint8*t, int>> get*top*channels(
        const std::map<uint8*t, int>& channel*counts, int limit) {
        std::vector<std::pair<uint8*t, int>> sorted*channels(
            channel*counts.begin(), channel*counts.end());
        
        std::sort(sorted*channels.begin(), sorted*channels.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        if (sorted*channels.size() > limit) {
            sorted*channels.resize(limit);
        }
        
        return sorted*channels;
    }
};

void wifi*scanner*demo() {
    WiFiScanner scanner;
    
    if (!scanner.initialize()) {
        printf("❌ WiFi scanner initialization failed\n");
        return;
    }
    
    // 🔍 Perform network scan
    if (scanner.scan*networks(true)) {  // Include hidden networks
        scanner.print*scan*results();
    } else {
        printf("❌ Network scan failed\n");
    }
}
```text

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
config.authmode = hf*wifi*security*t::WIFI*AUTH*WPA3*PSK;  // Prefer WPA3

// ✅ Set minimum security thresholds
config.threshold*authmode = hf*wifi*security*t::WIFI*AUTH*WPA2*PSK;

// ✅ Use strong passwords
config.password = "YourSecurePassword123!@#";  // Strong password example

// ✅ Monitor security events
wifi.SetEventCallback([](hf*wifi*event*t event, void* data) {
    if (event == hf*wifi*event*t::WIFI*EVENT*STA*DISCONNECTED) {
        // Log security events
        printf("🔒 Security: Connection lost - investigating...\n");
    }
});
```text

## 🧵 Thread Safety

The `BaseWifi` class is **not inherently thread-safe**.
For concurrent access from multiple tasks, use appropriate synchronization mechanisms.

## 🔗 Related Documentation

- **[EspWifi API Reference](../esp_api/EspWifi.md)** - ESP32-C6 WiFi implementation
- **[BaseLogger API Reference](BaseLogger.md)** - Logging WiFi events and diagnostics
- **[HardwareTypes Reference](HardwareTypes.md)** - Platform-agnostic type definitions

---

<div align="center">

**📋 Navigation**

[← Previous: BaseCan](BaseCan.md) | [Back to API Index](README.md) | [Next: BaseBluetooth
→](BaseBluetooth.md)

</div>

**📶 BaseWifi - Connecting HardFOC to the World** 🌐

*From IoT connectivity to device configuration - BaseWifi enables seamless wireless communication* 🚀

</div>