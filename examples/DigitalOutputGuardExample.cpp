/**
 * @file DigitalOutputGuardExample.cpp
 * @brief Examples demonstrating the updated DigitalOutputGuard with unified DigitalGpio.
 *
 * This file shows how to use the updated DigitalOutputGuard class with the new
 * unified DigitalGpio interface that supports dynamic mode switching.
 */

#include "DigitalOutputGuard.h"
#include "McuDigitalGpio.h"
#include "mcu/McuTypes.h"

// Example 1: Basic RAII usage with automatic output mode configuration
void ExampleBasicRAAI() {
    // Create a GPIO pin initially configured as input
    McuDigitalGpio led_pin(GPIO_NUM_2, DigitalGpio::Direction::Input);
    
    if (!led_pin.Initialize()) {
        // Handle initialization error
        return;
    }
    
    // Read the pin as input first
    DigitalGpio::State input_state;
    led_pin.ReadState(input_state);
    
    {
        // Create guard - this will automatically switch to output mode and set active
        DigitalOutputGuard led_guard(led_pin, true); // true = ensure output mode
        
        if (led_guard.IsValid()) {
            // LED is now ON (active) and pin is in output mode
            // Do some work...
            vTaskDelay(pdMS_TO_TICKS(1000));
            
            // Can manually control the LED while guard is active
            led_guard.SetInactive(); // LED OFF
            vTaskDelay(pdMS_TO_TICKS(500));
            led_guard.SetActive();   // LED ON
            vTaskDelay(pdMS_TO_TICKS(500));
            
        } // LED automatically turns OFF when guard goes out of scope
    }
    
    // Pin is still in output mode (guard doesn't change it back)
    // Can switch back to input if needed
    led_pin.SetDirection(DigitalGpio::Direction::Input);
}

// Example 2: Error handling with guard validation
void ExampleErrorHandling() {
    McuDigitalGpio control_pin(GPIO_NUM_4);
    
    if (!control_pin.Initialize()) {
        return;
    }
    
    // Try to use with a pin that might not support output
    DigitalOutputGuard control_guard(control_pin, true);
    
    if (!control_guard.IsValid()) {
        // Check what went wrong
        HfGpioErr error = control_guard.GetLastError();
        
        switch (error) {
            case HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH:
                // Pin might be input-only
                break;
            case HfGpioErr::GPIO_ERR_NOT_INITIALIZED:
                // Pin not properly initialized
                break;
            case HfGpioErr::GPIO_ERR_INVALID_PIN:
                // Invalid pin number
                break;
            default:
                // Other error
                break;
        }
        return;
    }
    
    // Guard is valid, can use safely
    control_guard.SetActive();
    // ... do work
}

// Example 3: Using with pre-configured output pin
void ExamplePreConfiguredOutput() {
    // Create GPIO already configured as output
    McuDigitalGpio motor_enable(GPIO_NUM_5, 
                               DigitalGpio::Direction::Output,
                               DigitalGpio::ActiveState::High,
                               DigitalGpio::OutputMode::PushPull);
    
    if (!motor_enable.Initialize()) {
        return;
    }
    
    {
        // Create guard without forcing output mode (pin is already output)
        DigitalOutputGuard motor_guard(motor_enable, false); // false = don't force output mode
        
        if (motor_guard.IsValid()) {
            // Motor is now enabled (active)
            // Perform motor operations...
            
            // Can check current state
            DigitalGpio::State current = motor_guard.GetCurrentState();
            if (current == DigitalGpio::State::Active) {
                // Motor is enabled
            }
            
        } // Motor automatically disabled when guard goes out of scope
    }
}

// Example 4: Exception safety demonstration
void ExampleExceptionSafety() {
    McuDigitalGpio safety_relay(GPIO_NUM_6);
    safety_relay.Initialize();
    
    try {
        // Create guard - relay activates
        DigitalOutputGuard relay_guard(safety_relay);
        
        if (!relay_guard.IsValid()) {
            throw std::runtime_error("Failed to activate safety relay");
        }
        
        // Do potentially risky operations
        PerformRiskyOperation(); // This might throw an exception
        
        // More operations...
        
    } catch (const std::exception& e) {
        // Even if an exception occurs, the relay guard destructor
        // will automatically deactivate the relay when the guard
        // goes out of scope, ensuring safety
    }
    // Relay is guaranteed to be deactivated here
}

// Example 5: Multiple guards for coordinated control
void ExampleCoordinatedControl() {
    McuDigitalGpio power_enable(GPIO_NUM_7);
    McuDigitalGpio motor_enable(GPIO_NUM_8);
    McuDigitalGpio brake_release(GPIO_NUM_9);
    
    // Initialize all pins
    power_enable.Initialize();
    motor_enable.Initialize();
    brake_release.Initialize();
    
    {
        // Create guards in specific order for safe startup sequence
        DigitalOutputGuard power_guard(power_enable);    // 1. Power on
        
        if (!power_guard.IsValid()) return;
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Power stabilization delay
        
        DigitalOutputGuard brake_guard(brake_release);   // 2. Release brake
        
        if (!brake_guard.IsValid()) return;
        
        vTaskDelay(pdMS_TO_TICKS(50));  // Brake release delay
        
        DigitalOutputGuard motor_guard(motor_enable);    // 3. Enable motor
        
        if (!motor_guard.IsValid()) return;
        
        // All systems are active and coordinated
        // Perform operations...
        
        // Manual control during operation
        motor_guard.SetInactive(); // Stop motor
        vTaskDelay(pdMS_TO_TICKS(500));
        motor_guard.SetActive();   // Restart motor
        
    } // Automatic shutdown in reverse order:
      // 3. Motor disabled (motor_guard destructor)
      // 2. Brake engaged (brake_guard destructor)  
      // 1. Power off (power_guard destructor)
}

// Example 6: Using with pointer interface
void ExamplePointerInterface() {
    auto led_pin = std::make_unique<McuDigitalGpio>(GPIO_NUM_10);
    
    if (!led_pin->Initialize()) {
        return;
    }
    
    {
        // Use pointer interface
        DigitalOutputGuard led_guard(led_pin.get());
        
        if (led_guard.IsValid()) {
            // LED is active
            // Can still use the original pin object
            if (led_pin->IsOutput()) {
                // Pin is properly configured as output
            }
        }
    } // LED turns off automatically
}

// Example 7: Integration with existing code that uses legacy methods
void ExampleLegacyCompatibility() {
    McuDigitalGpio status_led(GPIO_NUM_11);
    status_led.Initialize();
    
    {
        DigitalOutputGuard led_guard(status_led);
        
        if (led_guard.IsValid()) {
            // Can still use legacy BaseGpio methods
            bool is_active;
            HfGpioErr result = status_led.IsActive(is_active);
            
            if (result == HfGpioErr::GPIO_SUCCESS && is_active) {
                // LED is on as expected
            }
            
            // Legacy toggle method still works
            status_led.Toggle(); // LED off
            status_led.Toggle(); // LED on
        }
    }
}

// Dummy function for example
void PerformRiskyOperation() {
    // Simulate some operation that might throw
    // ...
}
