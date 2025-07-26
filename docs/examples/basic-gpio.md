# 🔌 Basic GPIO Example

<div align="center">

![GPIO Example](https://img.shields.io/badge/Example-Basic%20GPIO-blue?style=for-the-badge&logo=chip)

**📚 Simple GPIO control with LED and button**

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🔧 **Hardware Setup**](#-hardware-setup)
- [📋 **Code Example**](#-code-example)
- [🚀 **Running the Example**](#-running-the-example)
- [🔧 **Troubleshooting**](#-troubleshooting)

---

## 🎯 **Overview**

This example demonstrates basic GPIO operations using the HardFOC Internal Interface Wrapper. It shows how to:

- Configure GPIO pins as input and output
- Read button state
- Control LED based on button press
- Handle GPIO errors properly

### 🎯 **Learning Objectives**

- Understand GPIO initialization
- Learn error handling patterns
- Practice basic hardware control
- Implement simple user interaction

---

## 🔧 **Hardware Setup**

### 📋 **Required Components**

- ESP32-C6 development board
- LED (any color)
- Current limiting resistor (220Ω - 1kΩ)
- Push button
- Pull-up resistor (10kΩ)
- Breadboard and jumper wires

### 🔌 **Connections**

```
ESP32-C6 Pin | Component     | Notes
-------------|---------------|------------------
GPIO 2       | LED Anode     | Through resistor
GND          | LED Cathode   | Direct connection
GPIO 0       | Button        | One side to pin
3.3V         | Button        | Other side via 10kΩ resistor
GND          | Button        | Common ground
```

### 📊 **Circuit Diagram**

```
                    ESP32-C6
                 ┌─────────────┐
                 │             │
    3.3V ────────┤ 3.3V        │
                 │             │
      ┌─ Button ─┤ GPIO 0      │
      │          │             │
     ┌─┴─ 10kΩ ──┤ GND         │
     │           │             │
    GND          │ GPIO 2 ──┬──┤ 
                 │          │  │
                 └──────────┼──┘
                            │
                           220Ω
                            │
                           LED
                            │
                           GND
```

---

## 📋 **Code Example**

### 📄 **main.cpp**

```cpp
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspGpio.h"

// Pin definitions
constexpr hf_pin_num_t LED_PIN = 2;
constexpr hf_pin_num_t BUTTON_PIN = 0;

// Global GPIO objects
EspGpio led_pin(LED_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
EspGpio button_pin(BUTTON_PIN, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);

/**
 * @brief Initialize GPIO pins
 * @return true if successful, false otherwise
 */
bool InitializeGpio() {
    printf("🔧 Initializing GPIO pins...\n");
    
    // Initialize LED pin
    if (!led_pin.Initialize()) {
        printf("❌ Failed to initialize LED pin %d\n", LED_PIN);
        return false;
    }
    
    // Configure LED pin as output
    hf_gpio_err_t result = led_pin.SetAsOutput();
    if (IsError(result)) {
        printf("❌ Failed to configure LED pin as output: %d\n", 
               static_cast<int>(result));
        return false;
    }
    
    // Start with LED off
    result = led_pin.SetLow();
    if (IsError(result)) {
        printf("❌ Failed to set LED initial state: %d\n", 
               static_cast<int>(result));
        return false;
    }
    
    // Initialize button pin
    if (!button_pin.Initialize()) {
        printf("❌ Failed to initialize button pin %d\n", BUTTON_PIN);
        return false;
    }
    
    // Configure button pin as input with pull-up
    result = button_pin.SetAsInput();
    if (IsError(result)) {
        printf("❌ Failed to configure button pin as input: %d\n", 
               static_cast<int>(result));
        return false;
    }
    
    // Enable internal pull-up resistor
    result = button_pin.SetPullResistor(hf_gpio_pull_t::HF_GPIO_PULL_UP);
    if (IsError(result)) {
        printf("⚠️ Warning: Failed to set pull-up resistor: %d\n", 
               static_cast<int>(result));
        // Continue anyway - external pull-up should work
    }
    
    printf("✅ GPIO initialization complete\n");
    return true;
}

/**
 * @brief Read button state with debouncing
 * @return true if button is pressed, false otherwise
 */
bool IsButtonPressed() {
    static bool last_state = false;
    static uint32_t last_change_time = 0;
    constexpr uint32_t DEBOUNCE_MS = 50;
    
    bool current_state = !button_pin.Read();  // Invert due to pull-up
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Check if state changed and debounce time elapsed
    if (current_state != last_state) {
        if (current_time - last_change_time > DEBOUNCE_MS) {
            last_state = current_state;
            last_change_time = current_time;
            return current_state;
        }
    }
    
    return last_state;
}

/**
 * @brief Control LED based on button state
 */
void UpdateLed() {
    static bool led_state = false;
    static bool last_button_state = false;
    
    bool button_pressed = IsButtonPressed();
    
    // Detect button press (rising edge)
    if (button_pressed && !last_button_state) {
        led_state = !led_state;  // Toggle LED state
        
        hf_gpio_err_t result;
        if (led_state) {
            result = led_pin.SetHigh();
            printf("💡 LED ON\n");
        } else {
            result = led_pin.SetLow();
            printf("💡 LED OFF\n");
        }
        
        if (IsError(result)) {
            printf("❌ Failed to control LED: %d\n", 
                   static_cast<int>(result));
        }
    }
    
    last_button_state = button_pressed;
}

/**
 * @brief Main application task
 */
void AppTask(void* pvParameters) {
    printf("🚀 Starting GPIO example\n");
    
    // Initialize hardware
    if (!InitializeGpio()) {
        printf("❌ Hardware initialization failed - stopping\n");
        vTaskDelete(NULL);
        return;
    }
    
    printf("📋 Instructions:\n");
    printf("   - Press the button to toggle the LED\n");
    printf("   - LED will turn on/off with each button press\n");
    printf("   - Watch the console for status messages\n\n");
    
    // Main application loop
    while (true) {
        UpdateLed();
        
        // Small delay to prevent excessive CPU usage
        vTaskDelay(pdMS_TO_TICKS(10));  // 100Hz update rate
    }
}

/**
 * @brief Application entry point
 */
extern "C" void app_main() {
    printf("\n");
    printf("🔌 HardFOC GPIO Example\n");
    printf("========================\n");
    
    // Create main application task
    xTaskCreate(AppTask, "gpio_example", 4096, NULL, 5, NULL);
}
```

### 📄 **Advanced Example with Error Handling**

```cpp
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mcu/esp32/EspGpio.h"

class GpioExample {
private:
    EspGpio led_pin_;
    EspGpio button_pin_;
    bool initialized_;
    uint32_t button_press_count_;
    
public:
    GpioExample() 
        : led_pin_(2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT)
        , button_pin_(0, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT)
        , initialized_(false)
        , button_press_count_(0) {}
    
    bool Initialize() {
        printf("🔧 Initializing GPIO example...\n");
        
        // Initialize LED pin
        if (!InitializeLed()) {
            return false;
        }
        
        // Initialize button pin
        if (!InitializeButton()) {
            return false;
        }
        
        initialized_ = true;
        printf("✅ GPIO example initialized successfully\n");
        return true;
    }
    
    void Run() {
        if (!initialized_) {
            printf("❌ Example not initialized - call Initialize() first\n");
            return;
        }
        
        printf("🚀 GPIO example running\n");
        printf("Press button to see LED patterns...\n\n");
        
        while (true) {
            if (CheckButtonPress()) {
                button_press_count_++;
                printf("🔘 Button press #%u\n", button_press_count_);
                
                // Different patterns based on press count
                switch (button_press_count_ % 4) {
                    case 1:
                        BlinkPattern(3, 200);  // 3 fast blinks
                        break;
                    case 2:
                        BlinkPattern(2, 500);  // 2 slow blinks
                        break;
                    case 3:
                        BlinkPattern(5, 100);  // 5 very fast blinks
                        break;
                    case 0:
                        SolidOn(2000);         // Solid for 2 seconds
                        break;
                }
            }
            
            vTaskDelay(pdMS_TO_TICKS(50));  // 20Hz polling
        }
    }
    
private:
    bool InitializeLed() {
        if (!led_pin_.Initialize()) {
            printf("❌ Failed to initialize LED pin\n");
            return false;
        }
        
        hf_gpio_err_t result = led_pin_.SetAsOutput();
        if (IsError(result)) {
            printf("❌ Failed to configure LED as output: %d\n", 
                   static_cast<int>(result));
            return false;
        }
        
        // Set initial state
        result = led_pin_.SetLow();
        if (IsError(result)) {
            printf("❌ Failed to set LED initial state: %d\n", 
                   static_cast<int>(result));
            return false;
        }
        
        printf("✅ LED pin initialized\n");
        return true;
    }
    
    bool InitializeButton() {
        if (!button_pin_.Initialize()) {
            printf("❌ Failed to initialize button pin\n");
            return false;
        }
        
        hf_gpio_err_t result = button_pin_.SetAsInput();
        if (IsError(result)) {
            printf("❌ Failed to configure button as input: %d\n", 
                   static_cast<int>(result));
            return false;
        }
        
        // Enable pull-up resistor
        result = button_pin_.SetPullResistor(hf_gpio_pull_t::HF_GPIO_PULL_UP);
        if (IsError(result)) {
            printf("⚠️ Warning: Failed to set pull-up resistor\n");
        }
        
        printf("✅ Button pin initialized\n");
        return true;
    }
    
    bool CheckButtonPress() {
        static bool last_state = false;
        static uint32_t last_change = 0;
        constexpr uint32_t DEBOUNCE_MS = 50;
        
        bool current_state = !button_pin_.Read();  // Active low
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        if (current_state != last_state) {
            if (current_time - last_change > DEBOUNCE_MS) {
                last_state = current_state;
                last_change = current_time;
                
                // Return true on press (rising edge)
                return current_state;
            }
        }
        
        return false;
    }
    
    void BlinkPattern(int count, int delay_ms) {
        for (int i = 0; i < count; i++) {
            SetLedState(true);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
            SetLedState(false);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
    
    void SolidOn(int duration_ms) {
        SetLedState(true);
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        SetLedState(false);
    }
    
    void SetLedState(bool on) {
        hf_gpio_err_t result = on ? led_pin_.SetHigh() : led_pin_.SetLow();
        if (IsError(result)) {
            printf("❌ Failed to set LED state: %d\n", 
                   static_cast<int>(result));
        }
    }
};

extern "C" void app_main() {
    printf("\n");
    printf("🔌 Advanced GPIO Example\n");
    printf("=========================\n");
    
    GpioExample example;
    
    if (example.Initialize()) {
        example.Run();
    } else {
        printf("❌ Failed to initialize GPIO example\n");
    }
}
```

---

## 🚀 **Running the Example**

### 🔧 **Build and Flash**

```bash
# Set up ESP-IDF environment
source $IDF_PATH/export.sh

# Navigate to example directory
cd examples/esp32

# Configure project (if needed)
idf.py menuconfig

# Build the project
idf.py build

# Flash to ESP32-C6
idf.py -p /dev/ttyUSB0 flash monitor
```

### 📊 **Expected Output**

```
🔌 HardFOC GPIO Example
========================
🔧 Initializing GPIO pins...
✅ LED pin initialized
✅ Button pin initialized
✅ GPIO initialization complete
📋 Instructions:
   - Press the button to toggle the LED
   - LED will turn on/off with each button press
   - Watch the console for status messages

🚀 Starting GPIO example
🔘 Button press #1
💡 LED ON
🔘 Button press #2
💡 LED OFF
🔘 Button press #3
💡 LED ON
```

---

## 🔧 **Troubleshooting**

### ❌ **Common Issues**

#### **LED doesn't light up**
- Check LED polarity (anode to GPIO pin, cathode to GND)
- Verify resistor value (should be 220Ω - 1kΩ)
- Ensure GPIO pin number matches code

#### **Button doesn't respond**
- Check button wiring
- Verify pull-up resistor (10kΩ between button and 3.3V)
- Test button continuity with multimeter

#### **GPIO initialization fails**
- Verify pin numbers are valid for ESP32-C6
- Check if pins are reserved or already in use
- Ensure proper power supply to the board

### 🔍 **Debug Tips**

```cpp
// Add debug output to check pin states
void DebugGpioStates() {
    printf("Debug: LED pin state = %s\n", 
           led_pin.Read() ? "HIGH" : "LOW");
    printf("Debug: Button pin state = %s\n", 
           button_pin.Read() ? "HIGH" : "LOW");
}
```

### 📊 **Performance Notes**

- GPIO operations are very fast (~1μs)
- Button debouncing prevents false triggers
- 100Hz update rate provides responsive feel
- Minimal CPU usage with proper delays

---

<div align="center">

**🔌 This example demonstrates the basics of GPIO control with HardFOC**

*Build on these concepts to create more complex hardware interactions*

</div>