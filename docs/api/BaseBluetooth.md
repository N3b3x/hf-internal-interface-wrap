# 📲 BaseBluetooth API Reference

## 🎯 Unified Bluetooth abstraction for Classic and BLE wireless communication

## 📋 Navigation

[← Previous: BaseWifi](BaseWifi.md) | [Back to API Index](README.md) |
[Next: BaseNvs →](BaseNvs.md)

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Class Hierarchy**](#-class-hierarchy)
- [📋 **Error Codes**](#-error-codes)
- [🔧 **Core API**](#-core-api)
- [📊 **Data Structures**](#-data-structures)
- [📲 **Bluetooth Modes**](#-bluetooth-modes)
- [📊 **Usage Examples**](#-usage-examples)
- [🧪 **Best Practices**](#-best-practices)

---

## 🎯 **Overview**

The `BaseBluetooth` class provides a comprehensive Bluetooth abstraction
that serves as the unified interface for all Bluetooth operations in the
HardFOC system. It supports both Bluetooth Classic and Bluetooth Low Energy
(BLE), device discovery, pairing, connection management, and data transfer
across different hardware implementations.

### ✨ **Key Features**

- 📲 **Dual Mode Support** - Both Bluetooth Classic and BLE in a single interface
- 🔍 **Device Discovery** - Scan for and discover nearby Bluetooth devices
- 🤝 **Pairing & Bonding** - Secure device pairing and credential management
- 📡 **Connection Management** - Robust connection handling with reconnection
- 📊 **Service Discovery** - BLE GATT service and characteristic discovery
- 🔐 **Security Support** - Encryption, authentication, and authorization
- 🛡️ **Robust Error Handling** - Comprehensive validation and error reporting
- 🏎️ **Performance Optimized** - Minimal overhead for real-time applications
- 🔌 **Platform Agnostic** - Works across different MCU platforms

### 📊 **Supported Hardware**

| Implementation | Classic BT | BLE | Max Connections | Range | Power |

|----------------|------------|-----|-----------------|-------|-------|

| `Esp32C6 Bluetooth` | ✅ | ✅ | 7 connections | 10-100m | Configurable |

| `NrfBluetooth` | ✅ | ✅ | 20 connections | 10-240m | Ultra-low power |

| `Ti2640Bluetooth` | ❌ | ✅ | 3 connections | 10-50m | Low power |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BaseBluetooth {
        <<abstract>>
        +EnsureInitialized() hf*bt*err*t
        +StartDiscovery() hf*bt*err*t
        +StopDiscovery() hf*bt*err*t
        +ConnectToDevice(bt*addr*t) hf*bt*err*t
        +DisconnectDevice(hf*u32*t) hf*bt*err*t
        +SendData(hf*u32*t, data*, size) hf*bt*err*t
        +ReceiveData(hf*u32*t, data*, size&) hf*bt*err*t
        +PairDevice(bt*addr*t) hf*bt*err*t
        +SetMode(hf*bt*mode*t) hf*bt*err*t
        +RegisterEventCallback(callback) hf*bt*err*t
        +IsInitialized() bool
        +GetStatistics(hf*bt*statistics*t&) hf*bt*err*t
        #DoInitialize() hf*bt*err*t*
        #DoStartDiscovery() hf*bt*err*t*
        #DoConnect(bt*addr*t) hf*bt*err*t*
    }

    class Esp32C6Bluetooth {
+Esp32C6Bluetooth()
        +SetClassicConfig(hf*bt*classic*config*t) hf*bt*err*t
        +SetBleConfig(hf*ble*config*t) hf*bt*err*t
        +StartAdvertising() hf*bt*err*t
        +StopAdvertising() hf*bt*err*t
        +CreateGattService(hf*ble*service*t&) hf*bt*err*t
    }

    class NrfBluetooth {
        +NrfBluetooth()
        +SetTxPower(hf*i8*t) hf*bt*err*t
        +EnterLowPowerMode() hf*bt*err*t
        +SetBondingMode(bool) hf*bt*err*t
    }

    BaseBluetooth <|-- Esp32C6Bluetooth
    BaseBluetooth <|-- NrfBluetooth
```text

---

## 📋 **Error Codes**

### 🚨 **Bluetooth Error Enumeration**

```cpp
enum class hf*bt*err*t : hf*u32*t {
    // Success codes
    BT*SUCCESS = 0,
    
    // General errors
    BT*ERR*FAILURE = 1,
    BT*ERR*NOT*INITIALIZED = 2,
    BT*ERR*ALREADY*INITIALIZED = 3,
    BT*ERR*INVALID*PARAMETER = 4,
    BT*ERR*NULL*POINTER = 5,
    BT*ERR*OUT*OF*MEMORY = 6,
    
    // Connection errors
    BT*ERR*CONNECTION*FAILED = 7,
    BT*ERR*CONNECTION*TIMEOUT = 8,
    BT*ERR*CONNECTION*LOST = 9,
    BT*ERR*DEVICE*NOT*FOUND = 10,
    BT*ERR*DEVICE*UNREACHABLE = 11,
    BT*ERR*MAX*CONNECTIONS*REACHED = 12,
    
    // Pairing errors
    BT*ERR*PAIRING*FAILED = 13,
    BT*ERR*PAIRING*REJECTED = 14,
    BT*ERR*AUTHENTICATION*FAILED = 15,
    BT*ERR*AUTHORIZATION*FAILED = 16,
    BT*ERR*ENCRYPTION*FAILED = 17,
    
    // Discovery errors
    BT*ERR*DISCOVERY*FAILED = 18,
    BT*ERR*DISCOVERY*TIMEOUT = 19,
    BT*ERR*SERVICE*NOT*FOUND = 20,
    BT*ERR*CHARACTERISTIC*NOT*FOUND = 21,
    
    // Data transfer errors
    BT*ERR*SEND*FAILED = 22,
    BT*ERR*RECEIVE*FAILED = 23,
    BT*ERR*BUFFER*OVERFLOW = 24,
    BT*ERR*INVALID*DATA*SIZE = 25,
    
    // BLE specific errors
    BLE*ERR*ADVERTISING*FAILED = 26,
    BLE*ERR*GATT*ERROR = 27,
    BLE*ERR*INVALID*ATT*SIZE = 28,
    BLE*ERR*INVALID*HANDLE = 29,
    
    // Classic specific errors
    BT*CLASSIC*ERR*SPP*FAILED = 30,
    BT*CLASSIC*ERR*PROFILE*ERROR = 31,
    BT*CLASSIC*ERR*SDP*FAILED = 32,
    
    // System errors
    BT*ERR*SYSTEM*ERROR = 33,
    BT*ERR*PERMISSION*DENIED = 34,
    BT*ERR*OPERATION*ABORTED = 35
};
```text

### 📊 **Error Code Categories**

| Category | Range | Description |

|----------|-------|-------------|

| **Success** | 0 | Successful operation |

| **General** | 1-6 | Basic initialization and parameter errors |

| **Connection** | 7-12 | Device connection and management errors |

| **Pairing** | 13-17 | Security and pairing errors |

| **Discovery** | 18-21 | Device and service discovery errors |

| **Data Transfer** | 22-25 | Data transmission errors |

| **BLE Specific** | 26-29 | BLE protocol specific errors |

| **Classic Specific** | 30-32 | Classic Bluetooth errors |

| **System** | 33-35 | System-level errors |

---

## 🔧 **Core API**

### 🎯 **Essential Methods**

#### **Initialization & Configuration**
```cpp
/**
 * @brief Ensure the Bluetooth controller is initialized
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t EnsureInitialized() = 0;

/**
 * @brief Set Bluetooth operating mode
 * @param mode Bluetooth mode (Classic, BLE, or Dual)
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t SetMode(hf*bt*mode*t mode) = 0;

/**
 * @brief Check if Bluetooth is initialized
 * @return bool True if initialized
 */
virtual bool IsInitialized() const = 0;
```text

#### **Device Discovery**
```cpp
/**
 * @brief Start device discovery/scanning
 * @param discovery*time*s Discovery duration in seconds
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t StartDiscovery(hf*u32*t discovery*time*s = 10) = 0;

/**
 * @brief Stop device discovery/scanning
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t StopDiscovery() = 0;

/**
 * @brief Get discovered devices list
 * @param devices Output array of discovered devices
 * @param max*devices Maximum devices to return
 * @param actual*count Actual number of devices found
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t GetDiscoveredDevices(hf*bt*device*t* devices,
                                       hf*u32*t max*devices,
                                       hf*u32*t& actual*count) = 0;
```text

#### **Connection Management**
```cpp
/**
 * @brief Connect to a Bluetooth device
 * @param device*addr Target device address
 * @param connection*id Output connection ID
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t ConnectToDevice(const bt*addr*t& device*addr,
                                  hf*u32*t& connection*id) = 0;

/**
 * @brief Disconnect from a device
 * @param connection*id Connection ID to disconnect
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t DisconnectDevice(hf*u32*t connection*id) = 0;

/**
 * @brief Check if device is connected
 * @param connection*id Connection ID to check
 * @param is*connected Output connection status
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t IsDeviceConnected(hf*u32*t connection*id,
                                    bool& is*connected) = 0;
```text

#### **Data Transfer**
```cpp
/**
 * @brief Send data to connected device
 * @param connection*id Target connection ID
 * @param data Data buffer to send
 * @param data*size Size of data to send
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t SendData(hf*u32*t connection*id,
                           const hf*u8*t* data,
                           hf*u32*t data*size) = 0;

/**
 * @brief Receive data from connected device
 * @param connection*id Source connection ID
 * @param data Buffer to store received data
 * @param buffer*size Size of receive buffer
 * @param received*size Actual bytes received
 * @return hf*bt*err*t Error code
 */
virtual hf*bt*err*t ReceiveData(hf*u32*t connection*id,
                              hf*u8*t* data,
                              hf*u32*t buffer*size,
                              hf*u32*t& received*size) = 0;
```text

---

## 📊 **Data Structures**

### 📲 **Bluetooth Mode Types**

```cpp
enum class hf*bt*mode*t : hf*u8*t {
    BT*MODE*DISABLED = 0,           ///< Bluetooth disabled
    BT*MODE*CLASSIC = 1,            ///< Classic Bluetooth only
    BT*MODE*BLE = 2,                ///< BLE only
    BT*MODE*DUAL = 3                ///< Both Classic and BLE
};
```text

### 📱 **Device Information**

```cpp
struct hf*bt*device*t {
    bt*addr*t address;                      ///< Device MAC address
    char name[32];                          ///< Device name
    hf*u32*t class*of*device;               ///< Class of device (Classic BT)
    hf*i8*t rssi;                           ///< Signal strength (dBm)
    hf*bt*device*type*t device*type;        ///< Device type (Classic/BLE/Dual)
    bool is*paired;                         ///< Pairing status
    bool is*bonded;                         ///< Bonding status
    hf*u32*t last*seen*time*ms;             ///< Last discovery time
};
```text

### 🔐 **Security Configuration**

```cpp
struct hf*bt*security*config*t {
    bool require*pairing;                   ///< Require pairing for connections
    bool require*bonding;                   ///< Require bonding (stored keys)
    bool require*mitm*protection;           ///< Man-in-the-middle protection
    bool use*secure*connections;            ///< Use secure connections (if available)
    hf*u32*t passkey;                       ///< Static passkey (if used)
    hf*bt*io*capabilities*t io*cap;         ///< I/O capabilities for pairing
};
```text

### 📈 **Bluetooth Statistics**

```cpp
struct hf*bt*statistics*t {
    hf*u32*t total*connections;             ///< Total connection attempts
    hf*u32*t successful*connections;        ///< Successful connections
    hf*u32*t failed*connections;            ///< Failed connections
    hf*u32*t total*bytes*sent;              ///< Total bytes transmitted
    hf*u32*t total*bytes*received;          ///< Total bytes received
    hf*u32*t pairing*attempts;              ///< Total pairing attempts
    hf*u32*t successful*pairings;           ///< Successful pairings
    hf*u32*t discovery*scans;               ///< Total discovery scans
    hf*u32*t devices*discovered;            ///< Total devices discovered
    hf*u32*t active*connections;            ///< Currently active connections
};
```text

---

## 📲 **Bluetooth Modes**

### 📻 **Classic Bluetooth**

Classic Bluetooth is ideal for:
- **Audio streaming** (A2DP profile)
- **File transfers** (OBEX/FTP profiles)
- **Serial data** (SPP profile)
- **Human interface devices** (HID profile)

```cpp
// Configure for Classic Bluetooth
hf*bt*classic*config*t classic*config = {
    .device*name = "HardFOC Controller",
    .discoverable = true,
    .connectable = true,
    .profiles = BT*PROFILE*SPP | BT*PROFILE*A2DP
};
bluetooth.SetClassicConfig(classic*config);
```text

### 📡 **Bluetooth Low Energy (BLE)**

BLE is optimal for:
- **Sensor data collection** (low power, periodic data)
- **IoT applications** (battery-powered devices)
- **Beacon applications** (advertising-only mode)
- **Mobile app integration** (smartphones/tablets)

```cpp
// Configure for BLE
hf*ble*config*t ble*config = {
    .device*name = "HardFOC BLE",
    .advertising*interval*ms = 100,
    .connection*interval*ms = 20,
    .slave*latency = 0,
    .supervision*timeout*ms = 4000
};
bluetooth.SetBleConfig(ble*config);
```text

### 🔄 **Dual Mode**

Dual mode enables both Classic and BLE simultaneously:

```cpp
// Enable dual mode
bluetooth.SetMode(hf*bt*mode*t::BT*MODE*DUAL);
```text

---

## 📊 **Usage Examples**

### 📱 **Basic BLE Sensor**

```cpp
#include "inc/base/BaseBluetooth.h"

class BleSensorBeacon {
private:
    // ESP32C6 implementation would inherit from BaseBluetooth
    // class Esp32C6Bluetooth : public BaseBluetooth { ... };
    BaseBluetooth* bluetooth*;
    hf*u32*t service*handle*;
    hf*u32*t characteristic*handle*;
    
public:
    bool initialize() {
        // Initialize Bluetooth in BLE mode
        if (bluetooth*.EnsureInitialized() != hf*bt*err*t::BT*SUCCESS) {
            return false;
        }
        
        if (bluetooth*.SetMode(hf*bt*mode*t::BT*MODE*BLE) != hf*bt*err*t::BT*SUCCESS) {
            return false;
        }
        
        // Create GATT service for sensor data
        hf*ble*service*t sensor*service = {
            .uuid = {0x12, 0x34, 0x56, 0x78}, // Custom UUID
            .primary = true
        };
        
        if (bluetooth*.CreateGattService(sensor*service) != hf*bt*err*t::BT*SUCCESS) {
            return false;
        }
        
        // Start advertising
        return bluetooth*.StartAdvertising() == hf*bt*err*t::BT*SUCCESS;
    }
    
    void advertise*sensor*data(float temperature, float humidity) {
        // Pack sensor data
        struct {
            float temperature;
            float humidity;
            hf*u32*t timestamp;
        } sensor*data = {
            .temperature = temperature,
            .humidity = humidity,
            .timestamp = esp*timer*get*time() / 1000
        };
        
        // Update characteristic value
        bluetooth*.UpdateCharacteristic(characteristic*handle*,
                                      (hf*u8*t*)&sensor*data,
                                      sizeof(sensor*data));
        
        printf("📡 BLE: Broadcasting T=%.1f°C, H=%.1f%%\n", 
               temperature, humidity);
    }
};
```text

### 🔗 **Classic Bluetooth Serial Bridge**

```cpp
#include "inc/base/BaseBluetooth.h"

class BluetoothSerialBridge {
private:
    // ESP32C6 implementation would inherit from BaseBluetooth
    // class Esp32C6Bluetooth : public BaseBluetooth { ... };
    BaseBluetooth* bluetooth*;
    hf*u32*t spp*connection*id*;
    bool is*connected*;
    
public:
    BluetoothSerialBridge() : spp*connection*id*(0), is*connected*(false) {}
    
    bool initialize() {
        // Initialize Classic Bluetooth with SPP profile
        if (bluetooth*.EnsureInitialized() != hf*bt*err*t::BT*SUCCESS) {
            return false;
        }
        
        if (bluetooth*.SetMode(hf*bt*mode*t::BT*MODE*CLASSIC) != hf*bt*err*t::BT*SUCCESS) {
            return false;
        }
        
        // Configure Classic Bluetooth
        hf*bt*classic*config*t config = {
            .device*name = "HardFOC Serial",
            .discoverable = true,
            .connectable = true,
            .profiles = BT*PROFILE*SPP
        };
        
        if (bluetooth*.SetClassicConfig(config) != hf*bt*err*t::BT*SUCCESS) {
            return false;
        }
        
        // Register connection event callback
        bluetooth*.RegisterEventCallback([this](hf*bt*event*t& event) {
            this->handle*bluetooth*event(event);
        });
        
        printf("📻 Classic Bluetooth SPP ready for connections\n");
        return true;
    }
    
    void send*message(const std::string& message) {
        if (!is*connected*) {
            printf("❌ No Bluetooth connection active\n");
            return;
        }
        
        hf*bt*err*t result = bluetooth*.SendData(spp*connection*id*,
                                                (hf*u8*t*)message.c*str(),
                                                message.length());
        
        if (result == hf*bt*err*t::BT*SUCCESS) {
            printf("📤 BT Sent: %s\n", message.c*str());
        } else {
            printf("❌ BT Send failed: %d\n", static*cast<int>(result));
        }
    }
    
    void check*for*messages() {
        if (!is*connected*) return;
        
        hf*u8*t buffer[256];
        hf*u32*t received*size;
        
        hf*bt*err*t result = bluetooth*.ReceiveData(spp*connection*id*,
                                                   buffer,
                                                   sizeof(buffer) - 1,
                                                   received*size);
        
        if (result == hf*bt*err*t::BT*SUCCESS && received*size > 0) {
            buffer[received*size] = '\0'; // Null terminate
            printf("📥 BT Received: %s\n", (char*)buffer);
            
            // Echo the message back
            send*message("Echo: " + std::string((char*)buffer));
        }
    }
    
private:
    void handle*bluetooth*event(hf*bt*event*t& event) {
        switch (event.type) {
            case BT*EVENT*CONNECTION*ESTABLISHED:
                spp*connection*id* = event.connection*id;
                is*connected* = true;
                printf("✅ Bluetooth device connected (ID: %lu)\n", spp*connection*id*);
                break;
                
            case BT*EVENT*CONNECTION*LOST:
                if (event.connection*id == spp*connection*id*) {
                    is*connected* = false;
                    printf("❌ Bluetooth device disconnected\n");
                }
                break;
                
            case BT*EVENT*PAIRING*REQUEST:
                printf("🔐 Pairing request from device\n");
                bluetooth*.AcceptPairing(event.device*addr);
                break;
        }
    }
};
```text

### 🔍 **Device Scanner & Manager**

```cpp
#include "inc/mcu/esp32/EspBluetooth.h"

class BluetoothDeviceManager {
private:
    EspBluetooth bluetooth*;
    std::vector<hf*bt*device*t> discovered*devices*;
    
public:
    bool initialize() {
        if (bluetooth*.EnsureInitialized() != hf*bt*err*t::BT*SUCCESS) {
            return false;
        }
        
        // Enable dual mode for maximum compatibility
        return bluetooth*.SetMode(hf*bt*mode*t::BT*MODE*DUAL) == hf*bt*err*t::BT*SUCCESS;
    }
    
    void scan*for*devices(hf*u32*t scan*duration*s = 15) {
        printf("🔍 Starting Bluetooth device scan (%lu seconds)...\n", scan*duration*s);
        
        // Clear previous results
        discovered*devices*.clear();
        
        // Start discovery
        if (bluetooth*.StartDiscovery(scan*duration*s) != hf*bt*err*t::BT*SUCCESS) {
            printf("❌ Failed to start device discovery\n");
            return;
        }
        
        // Wait for scan to complete
        vTaskDelay(pdMS*TO*TICKS(scan*duration*s * 1000));
        
        // Get discovered devices
        hf*bt*device*t devices[20];
        hf*u32*t device*count;
        
        if (bluetooth*.GetDiscoveredDevices(devices, 20, device*count) == hf*bt*err*t::BT*SUCCESS) {
            printf("📱 Found %lu Bluetooth devices:\n", device*count);
            
            for (hf*u32*t i = 0; i < device*count; i++) {
                discovered*devices*.push*back(devices[i]);
                print*device*info(devices[i]);
            }
        }
        
        bluetooth*.StopDiscovery();
    }
    
    bool connect*to*device(const std::string& device*name) {
        // Find device by name
        auto device*it = std::find*if(discovered*devices*.begin(),
                                    discovered*devices*.end(),
                                    [&device*name](const hf*bt*device*t& dev) {
                                        return std::string(dev.name) == device*name;
                                    });
        
        if (device*it == discovered*devices*.end()) {
            printf("❌ Device '%s' not found in scan results\n", device*name.c*str());
            return false;
        }
        
        printf("🔗 Connecting to device: %s\n", device*name.c*str());
        
        hf*u32*t connection*id;
        hf*bt*err*t result = bluetooth*.ConnectToDevice(device*it->address, connection*id);
        
        if (result == hf*bt*err*t::BT*SUCCESS) {
            printf("✅ Successfully connected to %s (ID: %lu)\n", 
                   device*name.c*str(), connection*id);
            return true;
        } else {
            printf("❌ Failed to connect to %s: %d\n", 
                   device*name.c*str(), static*cast<int>(result));
            return false;
        }
    }
    
    void show*statistics() {
        hf*bt*statistics*t stats;
        if (bluetooth*.GetStatistics(stats) == hf*bt*err*t::BT*SUCCESS) {
            printf("📊 Bluetooth Statistics:\n");
            printf("   Total Connections: %lu\n", stats.total*connections);
            printf("   Success Rate: %.1f%%\n", 
                   (float)stats.successful*connections / stats.total*connections * 100.0f);
            printf("   Data Sent: %lu bytes\n", stats.total*bytes*sent);
            printf("   Data Received: %lu bytes\n", stats.total*bytes*received);
            printf("   Active Connections: %lu\n", stats.active*connections);
            printf("   Devices Discovered: %lu\n", stats.devices*discovered);
        }
    }
    
private:
    void print*device*info(const hf*bt*device*t& device) {
        const char* type*str = (device.device*type == BT*DEVICE*TYPE*CLASSIC) ? "Classic" :
                              (device.device*type == BT*DEVICE*TYPE*BLE) ? "BLE" : "Dual";
        
        printf("   📱 %s [%s] RSSI: %ddBm %s%s\n",
               device.name,
               type*str,
               device.rssi,
               device.is*paired ? "(Paired)" : "",
               device.is*bonded ? "(Bonded)" : "");
    }
};
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Practices**

1. **🎯 Choose Appropriate Mode**
   ```cpp
   // For low-power sensors
   bluetooth.SetMode(hf*bt*mode*t::BT*MODE*BLE);
   
   // For audio/data streaming
   bluetooth.SetMode(hf*bt*mode*t::BT*MODE*CLASSIC);
   
   // For maximum compatibility
   bluetooth.SetMode(hf*bt*mode*t::BT*MODE*DUAL);
   ```

1. **🔐 Implement Proper Security**
   ```cpp
   hf*bt*security*config*t security = {
       .require*pairing = true,
       .require*bonding = true,
       .require*mitm*protection = true,
       .use*secure*connections = true
   };
   bluetooth.SetSecurityConfig(security);
   ```

1. **📡 Handle Connection Events**
   ```cpp
   bluetooth.RegisterEventCallback([](hf*bt*event*t& event) {
       switch (event.type) {
           case BT*EVENT*CONNECTION*ESTABLISHED:
               printf("✅ Device connected\n");
               break;
           case BT*EVENT*CONNECTION*LOST:
               printf("❌ Device disconnected\n");
               // Implement reconnection logic
               break;
       }
   });
   ```

1. **📊 Monitor Performance**
   ```cpp
   // Regular statistics monitoring
   hf*bt*statistics*t stats;
   bluetooth.GetStatistics(stats);
   if (stats.successful*connections < stats.total*connections * 0.9f) {
       printf("⚠️ Low Bluetooth connection success rate\n");
   }
   ```

### ❌ **Common Pitfalls**

1. **🚫 Not Checking Connection Status**
   ```cpp
   // BAD: Sending without checking connection
   bluetooth.SendData(connection*id, data, size);
   
   // GOOD: Always verify connection
   bool connected;
   if (bluetooth.IsDeviceConnected(connection*id, connected) == BT*SUCCESS && connected) {
       bluetooth.SendData(connection*id, data, size);
   }
   ```

1. **🚫 Ignoring Power Management**
   ```cpp
   // BAD: Always on, drains battery
   bluetooth.SetMode(hf*bt*mode*t::BT*MODE*DUAL);
   
   // GOOD: Use BLE for battery-powered applications
   bluetooth.SetMode(hf*bt*mode*t::BT*MODE*BLE);
   bluetooth.SetLowPowerMode(true);
   ```

1. **🚫 Poor Error Handling**
   ```cpp
   // BAD: Ignoring connection failures
   bluetooth.ConnectToDevice(addr, connection*id);
   
   // GOOD: Handle connection failures gracefully
   if (bluetooth.ConnectToDevice(addr, connection*id) != BT*SUCCESS) {
       printf("Connection failed, will retry in 5 seconds\n");
       // Implement retry logic
   }
   ```

### 🎯 **Performance Tips**

1. **⚡ Optimize BLE Connection Parameters**
   ```cpp
   hf*ble*config*t ble*config = {
       .connection*interval*ms = 7.5f,    // Faster updates
       .slave*latency = 0,                // No latency
       .supervision*timeout*ms = 4000     // 4 second timeout
   };
   ```

1. **📱 Use Appropriate Advertising Intervals**
   ```cpp
   // Fast connection establishment
   bluetooth.SetAdvertisingInterval(20);  // 20ms for quick discovery
   
   // Battery conservation
   bluetooth.SetAdvertisingInterval(1000); // 1s for low power
   ```

1. **🔄 Implement Connection Pooling**
   ```cpp
   // Manage multiple connections efficiently
   std::vector<hf*u32*t> active*connections;
   // Reuse connections instead of creating new ones
   ```

---

<div align="center">

**📋 Navigation**

[← Previous: BaseWifi](BaseWifi.md) | [Back to API Index](README.md) | [Next: BaseNvs →](BaseNvs.md)

</div>

---

<div align="center">

**📲 Professional Bluetooth Communication for Modern Applications**

*Enabling seamless wireless connectivity with robust security and optimal performance*

</div>