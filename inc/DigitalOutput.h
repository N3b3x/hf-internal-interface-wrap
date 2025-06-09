/**
 * @file DigitalOutput.h
 * @brief Contains the definition of the DigitalOutput class for ESP32-C6 (ESP-IDF).
 *
 * Provides facilities to describe and control GPIO output pins using ESP-IDF. Supports push-pull and open-drain modes,
 * as well as lazy initialization and thread safety (if Mutex is implemented for ESP32).
 *
 * @note These functions are not thread-safe unless protected externally. Use appropriate guards in ISR or multi-threaded contexts.
 */
#ifndef DIGITALOUTPUT_H
#define DIGITALOUTPUT_H

#include <internal_interface_drivers/DigitalGpio.h>
#include "driver/gpio.h"
#include <string>

/**
 * @class DigitalOutput
 * @brief Provides facilities to describe and control GPIO output pins on ESP32-C6.
 */
class DigitalOutput : public DigitalGpio {
public:
    /**
     * @brief Constructor allocates an instance of the class with a pin, active state, and initial state.
     * @param pinArg GPIO pin number (gpio_num_t)
     * @param activeStateArg Indicates whether the set bit is active.
     * @param initialStateArg The initial state of the pin (default: State::Inactive).
     */
    DigitalOutput(gpio_num_t pinArg, ActiveState activeStateArg, State initialStateArg = State::Inactive) noexcept;

    /**
     * @brief Copy constructor is deleted to avoid copying instances.
     */
    DigitalOutput(const DigitalOutput& copy) = delete;
    /**
     * @brief Assignment operator is deleted to avoid copying instances.
     */
    DigitalOutput& operator=(const DigitalOutput& copy) = delete;
    /**
     * @brief Destructor.
     */
    virtual ~DigitalOutput() override;

    /**
     * @brief Helper function to return the output mode (PushPull/OpenDrain).
     * @returns The output mode.
     */
    Mode OutputMode() const noexcept;

    /**
     * @brief Helper function to return the logical state of the pin.
     * @returns True if the pin is logically active, false otherwise.
     */
    bool IsActive() noexcept;

    /**
     * @brief Function to set the logical active state.
     * @returns True if the operation was successful, false otherwise.
     */
    bool SetActive() noexcept;

    /**
     * @brief Function to reset the pin to the logical inactive state.
     * @returns True if the operation was successful, false otherwise.
     */
    bool SetInactive() noexcept;

    /**
     * @brief Function to toggle the pin state.
     * @returns True if the operation was successful, false otherwise.
     */
    bool Toggle() noexcept;

private:
    const State initialState; ///< The initial state of the pin.

    /**
     * @brief Virtual function to initialize the peripheral (configures pin as output).
     * @returns True if able to initialize, false otherwise.
     */
    virtual bool Initialize() noexcept override;
};

#endif // DIGITALOUTPUT_H
