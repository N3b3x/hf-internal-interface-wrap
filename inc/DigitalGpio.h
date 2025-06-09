/**
 * @file DigitalGpio.h
 * @brief Abstract base class DigitalGpio for handling GPIO pins on ESP32-C6 (ESP-IDF).
 *
 * The DigitalGpio class provides common features for handling digital GPIO pins using ESP-IDF.
 * It is designed with lazy initialization; pins are initialized when first manipulated.
 *
 * @note These functions are not thread-safe. Use appropriate guards in ISR or multi-threaded contexts.
 */
#ifndef DIGITALGPIO_H
#define DIGITALGPIO_H

#include "BaseGpio.h"
#include "driver/gpio.h"

/**
 * @class DigitalGpio
 * @brief Abstract base class for digital GPIO operations on ESP32-C6.
 */
class DigitalGpio : public BaseGpio {
public:
    /**
     * @brief Enumeration for setting the mode of the GPIO pin.
     */
    enum class Mode : uint8_t {
        OpenDrain,   ///< Open-drain output
        PushPull     ///< Push-pull output
    };

    /**
     * @brief Enumeration for describing the state of the GPIO pin.
     */
    enum class State : uint8_t {
        Active,
        Inactive
    };

    /**
     * @brief Enumeration for describing the active state of the GPIO pin.
     */
    enum class ActiveState : uint8_t {
        High,   ///< Active when logic high
        Low     ///< Active when logic low
    };

    /**
     * @brief Enumeration for describing the resistance configuration of the GPIO pin.
     */
    enum class Resistance : uint8_t {
        Floating,   ///< No pull-up or pull-down
        PullUp,     ///< Pull-up enabled
        PullDown    ///< Pull-down enabled
    };

    /**
     * @brief Converts Mode enum to string.
     */
    static const char* ToString(Mode mode) noexcept;
    /**
     * @brief Converts State enum to string.
     */
    static const char* ToString(State state) noexcept;
    /**
     * @brief Converts ActiveState enum to string.
     */
    static const char* ToString(ActiveState activeState) noexcept;
    /**
     * @brief Converts Resistance enum to string.
     */
    static const char* ToString(Resistance resistance) noexcept;

protected:
    /**
     * @brief Constructor that initializes an instance with a pin and active state.
     * @param pinArg GPIO pin number (gpio_num_t)
     * @param activeStateArg Defines the active state of the pin.
     */
    DigitalGpio(gpio_num_t pinArg, ActiveState activeStateArg) noexcept :
        BaseGpio(pinArg),
        activeState(activeStateArg)
    {
        // No code at this time
    }

    /**
     * @brief Copy constructor is deleted to prevent instance copying.
     */
    DigitalGpio(const DigitalGpio& copy) = delete;
    /**
     * @brief Assignment operator is deleted to prevent instance copying.
     */
    DigitalGpio& operator=(const DigitalGpio& copy) = delete;
    /**
     * @brief Default destructor.
     */
    virtual ~DigitalGpio() = default;

    /**
     * @brief Checks if the pin is active high.
     * @return True if the pin is active high, false otherwise.
     */
    bool IsActiveHigh() const noexcept { return activeState == ActiveState::High; }
    /**
     * @brief Checks if the pin is active low.
     * @return True if the pin is active low, false otherwise.
     */
    bool IsActiveLow() const noexcept { return activeState == ActiveState::Low; }

    /**
     * @brief Fetches the resistance configuration of the pin (ESP-IDF style).
     * @return The resistance configuration of the pin.
     */
    Resistance GetResistance() const noexcept;

private:
    const ActiveState activeState; ///< Pin active state (high or low)
};

#endif // DIGITALGPIO_H
