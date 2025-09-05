# ⏰ BasePeriodicTimer API Reference

<div align="center">

![BasePeriodicTimer](https://img.shields.io/badge/BasePeriodicTimer-Abstract%20Base%20Class-blue?style=for-the-badge&logo=clock)

**🎯 Unified periodic timer abstraction for all high-precision timing operations**

**📋 Navigation**

[← Previous: BaseTemperature](BaseTemperature.md) | [Back to API Index](README.md) | [Next: BasePio
→](BasePio.md)

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

The `BasePeriodicTimer` class provides a comprehensive periodic timer abstraction that serves as the
unified interface for all high-precision timing operations in the HardFOC system.
It supports microsecond resolution, callback-based notifications,
and works across different timer implementations.

### ✨ **Key Features**

- ⏰ **Microsecond Resolution** - High-precision timing down to microseconds
- 📞 **Callback Support** - Event-driven timer notifications
- 🔄 **Dynamic Period Control** - Change period during operation
- 🛡️ **Robust Error Handling** - Comprehensive validation and error reporting
- 🔌 **Platform Agnostic** - Works with hardware and software timers
- 📊 **Statistics & Diagnostics** - Built-in monitoring and health reporting
- 🧵 **Thread Safe** - Designed for multi-threaded applications
- ⚡ **Low Overhead** - Optimized for real-time applications

### 📊 **Supported Hardware**

| Implementation | Hardware Type | Resolution | Max Period | Features | Use Cases |

|----------------|---------------|------------|------------|----------|-----------|

| `EspPeriodicTimer` | ESP32-C6 Hardware | 1 μs | 8.5 hours | Multiple channels, DMA |
| Control loops, sampling |

---

## 🏗️ **Class Hierarchy**

```mermaid
classDiagram
    class BasePeriodicTimer {
        <<abstract>>
        +Initialize() hf*timer*err*t
        +Deinitialize() hf*timer*err*t
        +Start(period*us) hf*timer*err*t
        +Stop() hf*timer*err*t
        +SetPeriod(period*us) hf*timer*err*t
        +GetPeriod(period*us) hf*timer*err*t
        +SetCallback(callback, user*data) hf*timer*err*t
        +GetStats(callback*count, missed*callbacks, last*error) hf*timer*err*t
        +ResetStats() hf*timer*err*t
        +GetMinPeriod() uint64*t
        +GetMaxPeriod() uint64*t
        +GetResolution() uint64*t
    }
    
    class EspPeriodicTimer {
        +EspPeriodicTimer(timer*group, timer*num)
        +GetTimerGroup() timer*group*t
        +GetTimerNum() timer*idx*t
    }
    
    BasePeriodicTimer <|-- EspPeriodicTimer
```text

---

## 📋 **Error Codes**

The timer system uses comprehensive error codes for robust error handling:

### ✅ **Success Codes**

| Code | Value | Description |

|------|-------|-------------|

| `TIMER*SUCCESS` | 0 | ✅ Operation completed successfully |

### ❌ **General Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `TIMER*ERR*FAILURE` | 1 | ❌ General operation failure | Check hardware and configuration |

| `TIMER*ERR*NOT*INITIALIZED` | 2 | ⚠️ Timer not initialized | Call Initialize() first |

| `TIMER*ERR*ALREADY*INITIALIZED` | 3 | ⚠️ Timer already initialized | Check initialization state |

| `TIMER*ERR*INVALID*PARAMETER` | 4 | 🚫 Invalid parameter | Validate input parameters |

| `TIMER*ERR*NULL*POINTER` | 5 | 🚫 Null pointer provided | Check pointer validity |

| `TIMER*ERR*OUT*OF*MEMORY` | 6 | 💾 Memory allocation failed | Check system memory |

### ⏰ **Timer-Specific Error Codes**

| Code | Value | Description | Resolution |

|------|-------|-------------|------------|

| `TIMER*ERR*ALREADY*RUNNING` | 7 | 🔄 Timer already running | Stop timer first |

| `TIMER*ERR*NOT*RUNNING` | 8 | ⏸️ Timer not running | Start timer first |

| `TIMER*ERR*INVALID*PERIOD` | 9 | 📊 Invalid period | Use valid period range |

| `TIMER*ERR*RESOURCE*BUSY` | 10 | 🔄 Timer resource busy | Wait or use different timer |

| `TIMER*ERR*HARDWARE*FAULT` | 11 | 💥 Hardware fault | Check hardware connections |

| `TIMER*ERR*UNSUPPORTED*OPERATION` | 12 | 🚫 Unsupported operation | Check hardware capabilities |

---

## 🔧 **Core API**

### 🏗️ **Initialization Methods**

```cpp
/**
 * @brief Initialize the timer hardware/resources
 * @return hf*timer*err*t error code
 * 
 * 📝 Sets up timer hardware, configures callbacks, and prepares for operation.
 * Must be called before any timer operations.
 * 
 * @example
 * EspPeriodicTimer timer(TIMER*GROUP*0, TIMER*0);
 * if (timer.Initialize() == hf*timer*err*t::TIMER*SUCCESS) {
 *     // Timer ready for use
 * }
 */
virtual hf*timer*err*t Initialize() noexcept = 0;

/**
 * @brief Deinitialize the timer and free resources
 * @return hf*timer*err*t error code
 * 
 * 🧹 Cleanly shuts down timer hardware and releases resources.
 */
virtual hf*timer*err*t Deinitialize() noexcept = 0;

/**
 * @brief Check if timer is initialized
 * @return true if initialized, false otherwise
 * 
 * ❓ Query initialization status without side effects.
 */
bool IsInitialized() const noexcept;

/**
 * @brief Check if timer is currently running
 * @return true if running, false otherwise
 * 
 * ❓ Query running status without side effects.
 */
bool IsRunning() const noexcept;
```text

### ⏰ **Timer Control Methods**

```cpp
/**
 * @brief Start the periodic timer with specified period
 * @param period*us Timer period in microseconds
 * @return hf*timer*err*t error code
 * 
 * ⏰ Starts the timer with the specified period.
 * Callback will be invoked at each period.
 * 
 * @example
 * hf*timer*err*t result = timer.Start(1000000);  // 1 second period
 * if (result != hf*timer*err*t::TIMER*SUCCESS) {
 *     printf("Timer start failed: %s\n", HfTimerErrToString(result));
 * }
 */
virtual hf*timer*err*t Start(uint64*t period*us) noexcept = 0;

/**
 * @brief Stop the periodic timer
 * @return hf*timer*err*t error code
 * 
 * ⏸️ Stops the timer and cancels all pending callbacks.
 * 
 * @example
 * hf*timer*err*t result = timer.Stop();
 * if (result == hf*timer*err*t::TIMER*SUCCESS) {
 *     printf("Timer stopped successfully\n");
 * }
 */
virtual hf*timer*err*t Stop() noexcept = 0;

/**
 * @brief Change the timer period while running
 * @param period*us New timer period in microseconds
 * @return hf*timer*err*t error code
 * 
 * 🔄 Changes the timer period without stopping and restarting.
 * 
 * @example
 * // Change from 1 second to 500ms while running
 * timer.SetPeriod(500000);
 */
virtual hf*timer*err*t SetPeriod(uint64*t period*us) noexcept = 0;

/**
 * @brief Get the current timer period
 * @param period*us Reference to store the current period
 * @return hf*timer*err*t error code
 * 
 * 📊 Retrieves the current timer period.
 * 
 * @example
 * uint64*t current*period;
 * if (timer.GetPeriod(current*period) == hf*timer*err*t::TIMER*SUCCESS) {
 *     printf("Current period: %llu μs\n", current*period);
 * }
 */
virtual hf*timer*err*t GetPeriod(uint64*t &period*us) noexcept = 0;
```text

### 📞 **Callback Management**

```cpp
/**
 * @brief Set callback function for timer events
 * @param callback Callback function to invoke
 * @param user*data User data to pass to callback (optional)
 * @return hf*timer*err*t error code
 * 
 * 📞 Sets the callback function that will be invoked at each timer period.
 * 
 * @example
 * void on*timer*tick(void* user*data) {
 *     printf("Timer tick! User data: %p\n", user*data);
 *     // Handle timer event
 * }
 * 
 * timer.SetCallback(on*timer*tick, nullptr);
 */
hf*timer*err*t SetCallback(hf*timer*callback*t callback, void *user*data = nullptr) noexcept;

/**
 * @brief Get current user data pointer
 * @return User data pointer
 * 
 * 📊 Returns the user data associated with the timer callback.
 */
void *GetUserData() const noexcept;

/**
 * @brief Check if timer has a valid callback
 * @return true if callback is set, false otherwise
 * 
 * ✅ Checks if a callback function has been set.
 */
bool HasValidCallback() const noexcept;
```text

### 📊 **Information Methods**

```cpp
/**
 * @brief Get description of this timer implementation
 * @return Description string
 * 
 * 📝 Returns a human-readable description of this timer implementation.
 */
virtual const char *GetDescription() const noexcept = 0;

/**
 * @brief Get minimum supported timer period
 * @return Minimum period in microseconds
 * 
 * 📊 Returns the minimum supported timer period for this hardware.
 */
virtual uint64*t GetMinPeriod() const noexcept = 0;

/**
 * @brief Get maximum supported timer period
 * @return Maximum period in microseconds
 * 
 * 📊 Returns the maximum supported timer period for this hardware.
 */
virtual uint64*t GetMaxPeriod() const noexcept = 0;

/**
 * @brief Get timer resolution
 * @return Timer resolution in microseconds
 * 
 * 📊 Returns the timer resolution (minimum time increment).
 */
virtual uint64*t GetResolution() const noexcept = 0;
```text

### 📈 **Statistics and Diagnostics**

```cpp
/**
 * @brief Get timer statistics and status information
 * @param callback*count Number of callbacks executed
 * @param missed*callbacks Number of missed callbacks (if supported)
 * @param last*error Last error that occurred
 * @return hf*timer*err*t error code
 * 
 * 📊 Retrieves comprehensive statistics about timer operation.
 * 
 * @example
 * uint64*t callback*count, missed*callbacks;
 * hf*timer*err*t last*error;
 * if (timer.GetStats(callback*count, missed*callbacks, last*error) == hf*timer*err*t::TIMER*SUCCESS) {
 *     printf("Callbacks: %llu, Missed: %llu, Last error: %s\n", 
 *            callback*count, missed*callbacks, HfTimerErrToString(last*error));
 * }
 */
virtual hf*timer*err*t GetStats(uint64*t &callback*count, uint64*t &missed*callbacks,
                                hf*timer*err*t &last*error) noexcept = 0;

/**
 * @brief Reset timer statistics
 * @return hf*timer*err*t error code
 * 
 * 🔄 Clears all accumulated statistics counters.
 */
virtual hf*timer*err*t ResetStats() noexcept = 0;

/**
 * @brief Reset timer operation statistics
 * @return hf*timer*err*t error code
 * 
 * 🔄 Clears operation statistics.
 */
virtual hf*timer*err*t ResetStatistics() noexcept;

/**
 * @brief Reset timer diagnostic information
 * @return hf*timer*err*t error code
 * 
 * 🔄 Clears diagnostic information and error counters.
 */
virtual hf*timer*err*t ResetDiagnostics() noexcept;

/**
 * @brief Get timer operation statistics
 * @param statistics Reference to store statistics data
 * @return hf*timer*err*t error code
 * 
 * 📊 Retrieves comprehensive statistics about timer operations.
 */
virtual hf*timer*err*t GetStatistics(hf*timer*statistics*t &statistics) const noexcept;

/**
 * @brief Get timer diagnostic information
 * @param diagnostics Reference to store diagnostics data
 * @return hf*timer*err*t error code
 * 
 * 🔍 Retrieves diagnostic information about timer health and status.
 */
virtual hf*timer*err*t GetDiagnostics(hf*timer*diagnostics*t &diagnostics) const noexcept;
```text

---

## 📊 **Data Structures**

### 📞 **Timer Callback Type**

```cpp
using hf*timer*callback*t = std::function<void(void *user*data)>;
```text

### 📈 **Timer Statistics Structure**

```cpp
struct hf*timer*statistics*t {
    uint32*t totalStarts;          ///< Total timer starts
    uint32*t totalStops;           ///< Total timer stops
    uint32*t callbackExecutions;   ///< Number of callback executions
    uint32*t missedCallbacks;      ///< Number of missed callbacks
    uint32*t averageCallbackTimeUs; ///< Average callback execution time (microseconds)
    uint32*t maxCallbackTimeUs;    ///< Maximum callback execution time
    uint32*t minCallbackTimeUs;    ///< Minimum callback execution time
    uint64*t totalRunningTimeUs;   ///< Total running time in microseconds
};
```text

### 🔍 **Timer Diagnostics Structure**

```cpp
struct hf*timer*diagnostics*t {
    bool timerHealthy;             ///< Overall timer health status
    hf*timer*err*t lastErrorCode;  ///< Last error code
    uint32*t lastErrorTimestamp;   ///< Last error timestamp
    uint32*t consecutiveErrors;    ///< Consecutive error count
    bool timerInitialized;         ///< Timer initialization status
    bool timerRunning;             ///< Timer running status
    uint64*t currentPeriodUs;      ///< Current timer period in microseconds
    uint64*t timerResolutionUs;    ///< Timer resolution in microseconds
};
```text

### 📊 **Timer Stats Structure**

```cpp
struct hf*timer*stats*t {
    uint64*t start*count;          ///< Number of timer starts
    uint64*t stop*count;           ///< Number of timer stops
    uint64*t callback*count;       ///< Number of callback executions
    uint64*t missed*callbacks;     ///< Number of missed callbacks
    hf*timer*err*t last*error;     ///< Last error encountered
    hf*timestamp*us*t last*start*us; ///< Timestamp of last start
};
```text

---

## 📊 **Usage Examples**

### ⏰ **Basic Periodic Timer**

```cpp
#include "mcu/esp32/EspPeriodicTimer.h"

// Create timer instance
EspPeriodicTimer timer(TIMER*GROUP*0, TIMER*0);

// Timer callback function
void on*timer*tick(void* user*data) {
    static uint32*t tick*count = 0;
    tick*count++;
    
    printf("⏰ Timer tick %u\n", tick*count);
    
    // Perform periodic task
    // e.g., read sensors, update control loops, etc.
}

void setup*timer() {
    // Initialize timer
    if (timer.Initialize() != hf*timer*err*t::TIMER*SUCCESS) {
        printf("❌ Timer initialization failed\n");
        return;
    }
    
    // Set callback function
    timer.SetCallback(on*timer*tick, nullptr);
    
    // Start timer with 1 second period
    hf*timer*err*t result = timer.Start(1000000);  // 1,000,000 μs = 1 second
    if (result == hf*timer*err*t::TIMER*SUCCESS) {
        printf("✅ Timer started successfully\n");
    } else {
        printf("❌ Timer start failed: %s\n", HfTimerErrToString(result));
    }
}

void stop*timer() {
    hf*timer*err*t result = timer.Stop();
    if (result == hf*timer*err*t::TIMER*SUCCESS) {
        printf("✅ Timer stopped successfully\n");
    }
}

void print*timer*info() {
    printf("📊 Timer Information:\n");
    printf("  Description: %s\n", timer.GetDescription());
    printf("  Min period: %llu μs\n", timer.GetMinPeriod());
    printf("  Max period: %llu μs\n", timer.GetMaxPeriod());
    printf("  Resolution: %llu μs\n", timer.GetResolution());
    printf("  Initialized: %s\n", timer.IsInitialized() ? "Yes" : "No");
    printf("  Running: %s\n", timer.IsRunning() ? "Yes" : "No");
}
```text

### 🔄 **Control Loop Timer**

```cpp
#include "mcu/esp32/EspPeriodicTimer.h"

class ControlLoop {
private:
    EspPeriodicTimer timer*;
    float setpoint*;
    float current*value*;
    float kp*, ki*, kd*;
    float integral*;
    float last*error*;
    
public:
    ControlLoop() : timer*(TIMER*GROUP*0, TIMER*0), setpoint*(0.0f), current*value*(0.0f),
                    kp*(1.0f), ki*(0.1f), kd*(0.01f), integral*(0.0f), last*error*(0.0f) {}
    
    bool initialize() {
        // Initialize timer
        if (timer*.Initialize() != hf*timer*err*t::TIMER*SUCCESS) {
            printf("❌ Control loop timer initialization failed\n");
            return false;
        }
        
        // Set callback
        timer*.SetCallback([](void* user*data) {
            static*cast<ControlLoop*>(user*data)->control*step();
        }, this);
        
        return true;
    }
    
    void start(float frequency*hz) {
        uint64*t period*us = static*cast<uint64*t>(1000000.0f / frequency*hz);
        
        hf*timer*err*t result = timer*.Start(period*us);
        if (result == hf*timer*err*t::TIMER*SUCCESS) {
            printf("✅ Control loop started at %.1f Hz\n", frequency*hz);
        } else {
            printf("❌ Control loop start failed: %s\n", HfTimerErrToString(result));
        }
    }
    
    void stop() {
        timer*.Stop();
        printf("⏸️ Control loop stopped\n");
    }
    
    void set*setpoint(float setpoint) {
        setpoint* = setpoint;
    }
    
    void set*current*value(float value) {
        current*value* = value;
    }
    
    void set*gains(float kp, float ki, float kd) {
        kp* = kp;
        ki* = ki;
        kd* = kd;
        integral* = 0.0f;  // Reset integral on gain change
    }
    
private:
    void control*step() {
        // Calculate error
        float error = setpoint* - current*value*;
        
        // PID control
        float proportional = kp* * error;
        integral* += ki* * error;
        float derivative = kd* * (error - last*error*);
        
        float output = proportional + integral* + derivative;
        
        // Apply output (example: motor speed)
        apply*control*output(output);
        
        // Update for next iteration
        last*error* = error;
        
        // Optional: print control info
        static uint32*t step*count = 0;
        if (++step*count % 100 == 0) {  // Print every 100 steps
            printf("🎯 Control - Setpoint: %.2f, Current: %.2f, Output: %.2f\n",
                   setpoint*, current*value*, output);
        }
    }
    
    void apply*control*output(float output) {
        // Apply control output to actuator
        // This is just an example - implement based on your hardware
        printf("⚡ Control output: %.2f\n", output);
    }
};

void control*loop*example() {
    ControlLoop controller;
    
    if (!controller.initialize()) {
        printf("❌ Controller initialization failed\n");
        return;
    }
    
    // Configure control parameters
    controller.set*gains(2.0f, 0.5f, 0.1f);
    controller.set*setpoint(100.0f);
    
    // Start control loop at 100 Hz
    controller.start(100.0f);
    
    // Simulate changing setpoint
    vTaskDelay(pdMS*TO*TICKS(5000));  // Wait 5 seconds
    controller.set*setpoint(200.0f);
    
    vTaskDelay(pdMS*TO*TICKS(5000));  // Wait 5 seconds
    controller.stop();
}
```text

### 📊 **High-Frequency Sampling Timer**

```cpp
#include "mcu/esp32/EspPeriodicTimer.h"
#include <vector>

class HighFrequencySampler {
private:
    EspPeriodicTimer timer*;
    std::vector<float> samples*;
    size*t max*samples*;
    bool sampling*active*;
    
public:
    HighFrequencySampler(size*t max*samples = 1000) 
        : timer*(TIMER*GROUP*0, TIMER*0), max*samples*(max*samples), sampling*active*(false) {
        samples*.reserve(max*samples);
    }
    
    bool initialize() {
        if (timer*.Initialize() != hf*timer*err*t::TIMER*SUCCESS) {
            printf("❌ Sampler timer initialization failed\n");
            return false;
        }
        
        timer*.SetCallback([](void* user*data) {
            static*cast<HighFrequencySampler*>(user*data)->sample*data();
        }, this);
        
        return true;
    }
    
    void start*sampling(float frequency*hz) {
        samples*.clear();
        sampling*active* = true;
        
        uint64*t period*us = static*cast<uint64*t>(1000000.0f / frequency*hz);
        
        hf*timer*err*t result = timer*.Start(period*us);
        if (result == hf*timer*err*t::TIMER*SUCCESS) {
            printf("✅ Sampling started at %.1f Hz\n", frequency*hz);
        } else {
            printf("❌ Sampling start failed: %s\n", HfTimerErrToString(result));
        }
    }
    
    void stop*sampling() {
        sampling*active* = false;
        timer*.Stop();
        printf("⏸️ Sampling stopped\n");
    }
    
    const std::vector<float>& get*samples() const {
        return samples*;
    }
    
    void print*statistics() {
        if (samples*.empty()) {
            printf("❌ No samples collected\n");
            return;
        }
        
        float sum = 0.0f;
        float min*val = samples*[0];
        float max*val = samples*[0];
        
        for (float sample : samples*) {
            sum += sample;
            min*val = std::min(min*val, sample);
            max*val = std::max(max*val, sample);
        }
        
        float average = sum / samples*.size();
        
        printf("📊 Sampling Statistics:\n");
        printf("  Samples collected: %zu\n", samples*.size());
        printf("  Average: %.3f\n", average);
        printf("  Min: %.3f\n", min*val);
        printf("  Max: %.3f\n", max*val);
        printf("  Range: %.3f\n", max*val - min*val);
    }
    
private:
    void sample*data() {
        if (!sampling*active*) {
            return;
        }
        
        // Simulate reading sensor data
        float sensor*value = read*sensor*value();
        
        if (samples*.size() < max*samples*) {
            samples*.push*back(sensor*value);
        } else {
            // Buffer full, stop sampling
            sampling*active* = false;
            timer*.Stop();
            printf("📦 Sample buffer full (%zu samples)\n", samples*.size());
        }
    }
    
    float read*sensor*value() {
        // Simulate sensor reading
        // Replace with actual sensor reading code
        static float value = 0.0f;
        value += 0.1f;  // Simulate changing value
        if (value > 10.0f) value = 0.0f;
        return value;
    }
};

void sampling*example() {
    HighFrequencySampler sampler(1000);
    
    if (!sampler.initialize()) {
        printf("❌ Sampler initialization failed\n");
        return;
    }
    
    // Start high-frequency sampling (1 kHz)
    sampler.start*sampling(1000.0f);
    
    // Wait for sampling to complete
    vTaskDelay(pdMS*TO*TICKS(2000));  // Wait 2 seconds
    
    // Stop sampling
    sampler.stop*sampling();
    
    // Print results
    sampler.print*statistics();
}
```text

### 🔄 **Dynamic Period Timer**

```cpp
#include "mcu/esp32/EspPeriodicTimer.h"

class AdaptiveTimer {
private:
    EspPeriodicTimer timer*;
    uint64*t current*period*;
    uint64*t min*period*;
    uint64*t max*period*;
    uint32*t load*factor*;
    
public:
    AdaptiveTimer(uint64*t min*period*us = 1000, uint64*t max*period*us = 1000000)
        : timer*(TIMER*GROUP*0, TIMER*0), current*period*(100000), 
          min*period*(min*period*us), max*period*(max*period*us), load*factor*(50) {}
    
    bool initialize() {
        if (timer*.Initialize() != hf*timer*err*t::TIMER*SUCCESS) {
            return false;
        }
        
        timer*.SetCallback([](void* user*data) {
            static*cast<AdaptiveTimer*>(user*data)->adaptive*step();
        }, this);
        
        return true;
    }
    
    void start() {
        hf*timer*err*t result = timer*.Start(current*period*);
        if (result == hf*timer*err*t::TIMER*SUCCESS) {
            printf("✅ Adaptive timer started with period %llu μs\n", current*period*);
        }
    }
    
    void stop() {
        timer*.Stop();
    }
    
    void set*load*factor(uint32*t factor) {
        load*factor* = std::min(factor, 100u);  // Clamp to 0-100
        adjust*period();
    }
    
    uint64*t get*current*period() const {
        return current*period*;
    }
    
private:
    void adaptive*step() {
        // Simulate system load measurement
        uint32*t current*load = measure*system*load();
        
        // Adjust load factor based on current load
        if (current*load > 80) {
            load*factor* = std::min(load*factor* + 5, 100u);
        } else if (current*load < 20) {
            load*factor* = std::max(load*factor* - 5, 0u);
        }
        
        // Adjust period based on load factor
        adjust*period();
        
        // Perform periodic task
        perform*task();
    }
    
    void adjust*period() {
        // Calculate new period based on load factor
        // Higher load = longer period (slower execution)
        uint64*t new*period = min*period* + 
            ((max*period* - min*period*) * load*factor*) / 100;
        
        if (new*period != current*period*) {
            current*period* = new*period;
            timer*.SetPeriod(current*period*);
            printf("🔄 Period adjusted to %llu μs (load: %u%%)\n", current*period*, load*factor*);
        }
    }
    
    uint32*t measure*system*load() {
        // Simulate system load measurement
        // Replace with actual load measurement
        static uint32*t load = 50;
        load += (rand() % 21) - 10;  // Random change ±10
        if (load > 100) load = 100;
        if (load < 0) load = 0;
        return load;
    }
    
    void perform*task() {
        // Simulate periodic task execution
        static uint32*t task*count = 0;
        task*count++;
        
        if (task*count % 100 == 0) {
            printf("⚡ Task executed %u times (period: %llu μs)\n", task*count, current*period*);
        }
    }
};

void adaptive*timer*example() {
    AdaptiveTimer timer(1000, 100000);  // 1ms to 100ms range
    
    if (!timer.initialize()) {
        printf("❌ Adaptive timer initialization failed\n");
        return;
    }
    
    timer.start();
    
    // Simulate changing system load
    for (int i = 0; i < 10; i++) {
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // Simulate high load
        timer.set*load*factor(80);
        
        vTaskDelay(pdMS*TO*TICKS(1000));
        
        // Simulate low load
        timer.set*load*factor(20);
    }
    
    timer.stop();
}
```text

---

## 🧪 **Best Practices**

### ✅ **Recommended Patterns**

```cpp
// ✅ Always check initialization
if (timer.Initialize() != hf*timer*err*t::TIMER*SUCCESS) {
    printf("❌ Timer initialization failed\n");
    return false;
}

// ✅ Use appropriate period ranges
uint64*t min*period = timer.GetMinPeriod();
uint64*t max*period = timer.GetMaxPeriod();
uint64*t period = std::clamp(desired*period, min*period, max*period);

// ✅ Handle all error codes
hf*timer*err*t result = timer.Start(period);
if (result != hf*timer*err*t::TIMER*SUCCESS) {
    printf("⚠️ Timer Error: %s\n", HfTimerErrToString(result));
    // Handle specific error types
    if (result == hf*timer*err*t::TIMER*ERR*INVALID*PERIOD) {
        // Period out of range
    } else if (result == hf*timer*err*t::TIMER*ERR*ALREADY*RUNNING) {
        // Timer already running
    }
}

// ✅ Set callback before starting timer
timer.SetCallback(on*timer*tick, user*data);
timer.Start(period);

// ✅ Keep callbacks short and efficient
void on*timer*tick(void* user*data) {
    // Quick operations only
    // Avoid blocking operations
    // Use queues for longer tasks
}

// ✅ Monitor timer statistics
uint64*t callback*count, missed*callbacks;
hf*timer*err*t last*error;
if (timer.GetStats(callback*count, missed*callbacks, last*error) == hf*timer*err*t::TIMER*SUCCESS) {
    if (missed*callbacks > 0) {
        printf("⚠️ Missed callbacks detected: %llu\n", missed*callbacks);
    }
}
```text

### ❌ **Common Pitfalls**

```cpp
// ❌ Don't ignore initialization
timer.Start(period);  // May fail silently

// ❌ Don't use periods outside valid range
timer.Start(0);  // Invalid period

// ❌ Don't ignore error codes
timer.Start(period);  // Error handling missing

// ❌ Don't perform blocking operations in callbacks
void on*timer*tick(void* user*data) {
    vTaskDelay(100);  // ❌ Blocking in callback
    // Use queues instead
}

// ❌ Don't start timer without callback
timer.Start(period);  // No callback set

// ❌ Don't forget to stop timer
// Always stop timer when done
```text

### 🎯 **Performance Optimization**

```cpp
// 🚀 Use appropriate period for application
// Too short: may cause missed callbacks
// Too long: may not meet timing requirements

// 🚀 Keep callbacks lightweight
// Use queues for longer operations
// Avoid memory allocation in callbacks

// 🚀 Use hardware timers when available
// Hardware timers are more precise than software timers

// 🚀 Monitor missed callbacks
// High missed callback count indicates system overload

// 🚀 Use appropriate timer resolution
// Don't use 1μs resolution for 1-second periods

// 🚀 Consider timer priority
// High-priority timers for critical operations
// Lower priority for non-critical operations
```text

---

## 🔗 **Related Documentation**

- [⚙️ **EspPeriodicTimer**](../esp_api/EspPeriodicTimer.md) - ESP32-C6 implementation
- [🎯 **Hardware Types**](HardwareTypes.md) - Platform-agnostic types

---

<div align="center">

**📋 Navigation**

[← Previous: BaseTemperature](BaseTemperature.md) | [Back to API Index](README.md) | [Next: BasePio
→](BasePio.md)

</div>

---

<div align="center">

**⏰ BasePeriodicTimer - The Foundation of High-Precision Timing in HardFOC**

*Part of the HardFOC Internal Interface Wrapper Documentation*

</div> 