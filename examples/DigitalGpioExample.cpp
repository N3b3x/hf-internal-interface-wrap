/**
 * @file DigitalGpioExample.cpp 
 * @brief Example usage of the unified DigitalGpio interface with dynamic mode switching.
 *
 * This file demonstrates how to use the new unified DigitalGpio class that supports
 * dynamic switching between input and output modes at runtime. It shows examples
 * with both MCU pins and PCAL95555 expander pins using the same interface.
 */

#include "McuDigitalGpio.h"
#include "Pcal95555DigitalGpio.h"
#include "SfI2cBus.h"
#include "mcu/McuTypes.h"
#include <memory>
#include <vector>

// Example: Using MCU GPIO with dynamic mode switching
void ExampleMcuGpioDynamicMode() {
    // Create an MCU GPIO pin initially configured as input
    McuDigitalGpio gpio_pin(GPIO_NUM_2, 
                           DigitalGpio::Direction::Input,
                           DigitalGpio::ActiveState::High,
                           DigitalGpio::OutputMode::PushPull,
                           DigitalGpio::PullMode::PullUp);
    
    // Initialize the pin
    if (!gpio_pin.Initialize()) {
        // Handle initialization error
        return;
    }
    
    // Read input state
    DigitalGpio::State input_state;
    if (gpio_pin.ReadState(input_state) == HfGpioErr::GPIO_SUCCESS) {
        // Process input state
        bool is_active = (input_state == DigitalGpio::State::Active);
        // ... use input value
    }
    
    // Now dynamically switch to output mode
    if (gpio_pin.SetDirection(DigitalGpio::Direction::Output) == HfGpioErr::GPIO_SUCCESS) {
        // Pin is now configured as output
        
        // Set to active state
        gpio_pin.SetState(DigitalGpio::State::Active);
        
        // Toggle the pin
        gpio_pin.Toggle();
        
        // Set to inactive state
        gpio_pin.SetState(DigitalGpio::State::Inactive);
    }
    
    // Switch back to input if needed
    if (gpio_pin.SetDirection(DigitalGpio::Direction::Input) == HfGpioErr::GPIO_SUCCESS) {
        // Pin is now input again
        // Change pull mode
        gpio_pin.SetPullMode(DigitalGpio::PullMode::PullDown);
        
        // Read new state
        gpio_pin.ReadState(input_state);
    }
}

// Example: Using PCAL95555 expander GPIO with the same interface
void ExamplePcal95555GpioDynamicMode() {
    // Set up I2C bus for PCAL95555 communication
    hf_i2c_config_t i2c_config = {};
    i2c_config.mode = I2C_MODE_MASTER;
    i2c_config.sda_io_num = GPIO_NUM_21;
    i2c_config.scl_io_num = GPIO_NUM_22;
    i2c_config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.master.clk_speed = 100000; // 100kHz
    
    SfI2cBus i2c_bus(I2C_NUM_0, i2c_config, nullptr);
    if (!i2c_bus.Open()) {
        // Handle I2C initialization error
        return;
    }
    
    // Create shared PCAL95555 driver
    auto pcal95555_driver = CreatePcal95555Driver(i2c_bus, 0x20);
    
    // Create PCAL95555 GPIO pin initially as output
    auto expander_pin = CreatePcal95555Pin(
        0,  // Pin 0 on PCAL95555
        pcal95555_driver,
        0x20,
        DigitalGpio::Direction::Output,
        DigitalGpio::ActiveState::Low,  // Active-low LED
        DigitalGpio::OutputMode::PushPull,
        DigitalGpio::PullMode::Floating
    );
    
    // Initialize the expander pin
    if (!expander_pin->Initialize()) {
        // Handle initialization error
        return;
    }
    
    // Use as output - blink an LED
    for (int i = 0; i < 10; ++i) {
        expander_pin->SetState(DigitalGpio::State::Active);   // LED on
        vTaskDelay(pdMS_TO_TICKS(500));
        expander_pin->SetState(DigitalGpio::State::Inactive); // LED off
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // Now switch to input mode to read a button
    if (expander_pin->SetDirection(DigitalGpio::Direction::Input) == HfGpioErr::GPIO_SUCCESS) {
        // Configure pull-up for button input
        expander_pin->SetPullMode(DigitalGpio::PullMode::PullUp);
        
        // Read button state
        DigitalGpio::State button_state;
        if (expander_pin->ReadState(button_state) == HfGpioErr::GPIO_SUCCESS) {
            bool button_pressed = (button_state == DigitalGpio::State::Active);
            // ... handle button state
        }
    }
}

// Example: Mixed MCU and expander GPIO usage with unified interface
void ExampleMixedGpioUsage() {
    // Create a collection of GPIO pins from different sources
    std::vector<std::unique_ptr<DigitalGpio>> gpio_pins;
    
    // Add MCU pins
    gpio_pins.emplace_back(std::make_unique<McuDigitalGpio>(
        GPIO_NUM_2, DigitalGpio::Direction::Output));
    gpio_pins.emplace_back(std::make_unique<McuDigitalGpio>(
        GPIO_NUM_4, DigitalGpio::Direction::Input, 
        DigitalGpio::ActiveState::High, 
        DigitalGpio::OutputMode::PushPull,
        DigitalGpio::PullMode::PullUp));
    
    // Set up I2C and add expander pins
    hf_i2c_config_t i2c_config = {};
    // ... configure I2C
    SfI2cBus i2c_bus(I2C_NUM_0, i2c_config, nullptr);
    if (i2c_bus.Open()) {
        auto pcal95555_driver = CreatePcal95555Driver(i2c_bus, 0x20);
        
        gpio_pins.push_back(CreatePcal95555Pin(0, pcal95555_driver));
        gpio_pins.push_back(CreatePcal95555Pin(1, pcal95555_driver));
    }
    
    // Initialize all pins
    for (auto& pin : gpio_pins) {
        if (!pin->Initialize()) {
            // Handle error
            continue;
        }
    }
    
    // Use all pins with the same interface regardless of source
    for (auto& pin : gpio_pins) {
        // Check current configuration
        if (pin->IsOutput()) {
            // Set output pins to active
            pin->SetState(DigitalGpio::State::Active);
        } else if (pin->IsInput()) {
            // Read input pins
            DigitalGpio::State state;
            pin->ReadState(state);
            // ... process input
        }
        
        // Example: Switch all pins to opposite mode
        DigitalGpio::Direction new_direction = pin->IsInput() ? 
            DigitalGpio::Direction::Output : DigitalGpio::Direction::Input;
        
        pin->SetDirection(new_direction);
        
        if (pin->IsOutput()) {
            pin->SetState(DigitalGpio::State::Inactive);
        }
    }
}

// Example: Configuration change scenarios
void ExampleConfigurationChanges() {
    McuDigitalGpio gpio_pin(GPIO_NUM_5);
    
    if (!gpio_pin.Initialize()) {
        return;
    }
    
    // Scenario 1: Change from input to output
    gpio_pin.SetDirection(DigitalGpio::Direction::Input);
    gpio_pin.SetPullMode(DigitalGpio::PullMode::PullUp);
    
    // Read some input values...
    
    // Now need to drive the same pin as output
    gpio_pin.SetDirection(DigitalGpio::Direction::Output);
    gpio_pin.SetOutputMode(DigitalGpio::OutputMode::OpenDrain);
    gpio_pin.SetState(DigitalGpio::State::Active);
    
    // Scenario 2: Change polarity at runtime
    gpio_pin.SetActiveState(DigitalGpio::ActiveState::Low);
    gpio_pin.SetState(DigitalGpio::State::Active); // Now drives LOW
    
    // Scenario 3: Change output drive mode
    gpio_pin.SetOutputMode(DigitalGpio::OutputMode::PushPull);
    gpio_pin.SetState(DigitalGpio::State::Inactive); // Now drives HIGH
    
    // All changes are applied immediately and work seamlessly
}

// Example: Error handling with the unified interface
void ExampleErrorHandling() {
    McuDigitalGpio gpio_pin(GPIO_NUM_6, DigitalGpio::Direction::Input);
    
    if (!gpio_pin.Initialize()) {
        // Initialization failed
        return;
    }
    
    // Try to write to an input pin (should fail)
    HfGpioErr result = gpio_pin.SetState(DigitalGpio::State::Active);
    if (result != HfGpioErr::GPIO_SUCCESS) {
        // Handle direction mismatch error
        if (result == HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH) {
            // Either switch to output or handle as error
            gpio_pin.SetDirection(DigitalGpio::Direction::Output);
            gpio_pin.SetState(DigitalGpio::State::Active); // Now works
        }
    }
    
    // Try to read from an uninitialized pin
    DigitalGpio::State state;
    result = gpio_pin.ReadState(state);
    if (result != HfGpioErr::GPIO_SUCCESS) {
        // Handle read error
        switch (result) {
            case HfGpioErr::GPIO_ERR_NOT_INITIALIZED:
                // Reinitialize
                gpio_pin.Initialize();
                break;
            case HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH:
                // Switch to input mode
                gpio_pin.SetDirection(DigitalGpio::Direction::Input);
                break;
            default:
                // Other error
                break;
        }
    }
}
