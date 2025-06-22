/**
 * @file Esp32C6Gpio.h
 * @brief ESP32-C6 specific GPIO implementations for the HardFOC system.
 *
 * This file provides concrete implementations for ESP32-C6 GPIO operations,
 * including input, output, and interrupt-capable pins.
 */

#ifndef ESP32C6_GPIO_H
#define ESP32C6_GPIO_H

#include "DigitalGpio.h"
#include "driver/gpio.h"
#include <array>
#include <bitset>

/**
 * @brief ESP32-C6 GPIO pin assignments and restrictions
 */
namespace Esp32C6GpioConfig {
    // ESP32-C6 has 31 GPIO pins (0-30), but some are restricted
    static constexpr uint8_t MAX_GPIO_PINS = 31;
    
    // Pins reserved for specific functions that should NOT be used as general GPIO
    static constexpr std::array<gpio_num_t, 8> RESERVED_PINS = {
        GPIO_NUM_18,  // SPI CS for TMC9660 (critical for motor control)
        GPIO_NUM_19,  // SPI CS EXT1 / CAN TX (shared, managed by protocol stack)
        GPIO_NUM_20,  // SPI CS for AS5047 encoder (critical for position feedback)
        GPIO_NUM_6,   // SPI CLK (critical for SPI bus)
        GPIO_NUM_7,   // SPI MOSI (critical for SPI bus)
        GPIO_NUM_2,   // SPI MISO / WS2812 (shared, but SPI priority)
        GPIO_NUM_22,  // I2C SDA (critical for PCAL95555 and other I2C devices)
        GPIO_NUM_23   // I2C SCL (critical for I2C bus)
    };
    
    // Pins that can be used as general GPIO but with restrictions
    static constexpr std::array<gpio_num_t, 8> CONDITIONAL_GPIO_PINS = {
        GPIO_NUM_8,   // SPI CS EXT2 (can be GPIO if not using external SPI device)
        GPIO_NUM_15,  // CAN RX (can be GPIO if not using CAN)
        GPIO_NUM_4,   // UART RX (can be GPIO if using different UART pins)
        GPIO_NUM_5,   // UART TX (can be GPIO if using different UART pins)
        GPIO_NUM_0,   // Debug UART TX (can be GPIO in production)
        GPIO_NUM_1,   // Debug UART RX (can be GPIO in production)
        GPIO_NUM_12,  // USB JTAG D- (can be GPIO if not debugging)
        GPIO_NUM_13   // USB JTAG D+ (can be GPIO if not debugging)
    };
    
    // Pins that are safe to use as general GPIO
    static constexpr std::array<gpio_num_t, 15> SAFE_GPIO_PINS = {
        GPIO_NUM_3, GPIO_NUM_9, GPIO_NUM_10, GPIO_NUM_11, GPIO_NUM_14,
        GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_21, GPIO_NUM_24, GPIO_NUM_25,
        GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_28, GPIO_NUM_29, GPIO_NUM_30
    };
}

/**
 * @class Esp32C6Output
 * @brief ESP32-C6 digital output implementation.
 */
class Esp32C6Output : public DigitalGpio {
public:
    /**
     * @brief Constructor for ESP32-C6 output pin.
     * @param pin_num GPIO pin number
     * @param active_state Active state configuration
     * @param initial_state Initial pin state
     * @param mode Output mode (push-pull or open-drain)
     */
    Esp32C6Output(gpio_num_t pin_num, 
                  ActiveState active_state = ActiveState::High,
                  State initial_state = State::Inactive,
                  Mode mode = Mode::PushPull) noexcept
        : DigitalGpio(pin_num, active_state), initial_state_(initial_state), mode_(mode) {}

    // BaseGpio interface implementation
    bool Initialize() noexcept override;
    bool IsPinAvailable() const noexcept override;
    uint8_t GetMaxPins() const noexcept override { return Esp32C6GpioConfig::MAX_GPIO_PINS; }
    std::string_view GetDescription() const noexcept override { return "ESP32-C6 Output"; }

protected:
    // DigitalGpio implementation
    HfGpioErr SetActiveImpl() noexcept override;
    HfGpioErr SetInactiveImpl() noexcept override;
    HfGpioErr ToggleImpl() noexcept override;
    HfGpioErr IsActiveImpl(bool& is_active) noexcept override;
    gpio_mode_t GetDirection() const noexcept override { return GPIO_MODE_OUTPUT; }

private:
    State initial_state_;
    Mode mode_;
    
    bool SetPinLevel(bool level) noexcept;
    bool GetPinLevel() noexcept;
};

/**
 * @class Esp32C6Input
 * @brief ESP32-C6 digital input implementation.
 */
class Esp32C6Input : public DigitalGpio {
public:
    /**
     * @brief Constructor for ESP32-C6 input pin.
     * @param pin_num GPIO pin number
     * @param active_state Active state configuration
     * @param pull_resistance Pull resistor configuration
     */
    Esp32C6Input(gpio_num_t pin_num,
                 ActiveState active_state = ActiveState::High,
                 Resistance pull_resistance = Resistance::Floating) noexcept
        : DigitalGpio(pin_num, active_state), pull_resistance_(pull_resistance) {}

    // BaseGpio interface implementation
    bool Initialize() noexcept override;
    bool IsPinAvailable() const noexcept override;
    uint8_t GetMaxPins() const noexcept override { return Esp32C6GpioConfig::MAX_GPIO_PINS; }
    std::string_view GetDescription() const noexcept override { return "ESP32-C6 Input"; }

    // Input-specific methods
    Resistance GetResistance() const noexcept override { return pull_resistance_; }

protected:
    // DigitalGpio implementation
    HfGpioErr SetActiveImpl() noexcept override { return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH; }
    HfGpioErr SetInactiveImpl() noexcept override { return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH; }
    HfGpioErr ToggleImpl() noexcept override { return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH; }
    HfGpioErr IsActiveImpl(bool& is_active) noexcept override;
    gpio_mode_t GetDirection() const noexcept override { return GPIO_MODE_INPUT; }

private:
    Resistance pull_resistance_;
};

/**
 * @class Esp32C6InterruptInput
 * @brief ESP32-C6 interrupt-capable input implementation.
 */
class Esp32C6InterruptInput : public Esp32C6Input {
public:
    /**
     * @brief Interrupt callback function type.
     */
    using InterruptCallback = void(*)(void* user_data);

    /**
     * @brief Constructor for ESP32-C6 interrupt input pin.
     * @param pin_num GPIO pin number
     * @param active_state Active state configuration
     * @param interrupt_type Interrupt trigger type
     * @param pull_resistance Pull resistor configuration
     */
    Esp32C6InterruptInput(gpio_num_t pin_num,
                          ActiveState active_state = ActiveState::High,
                          gpio_int_type_t interrupt_type = GPIO_INTR_POSEDGE,
                          Resistance pull_resistance = Resistance::Floating) noexcept
        : Esp32C6Input(pin_num, active_state, pull_resistance), 
          interrupt_type_(interrupt_type), callback_(nullptr), user_data_(nullptr),
          interrupt_enabled_(false) {}

    ~Esp32C6InterruptInput() noexcept override;

    // BaseGpio interface implementation
    bool Initialize() noexcept override;
    std::string_view GetDescription() const noexcept override { return "ESP32-C6 Interrupt Input"; }
    bool SupportsInterrupts() const noexcept override { return true; }

    // Interrupt-specific methods
    HfGpioErr EnableInterrupt(InterruptCallback callback, void* user_data) noexcept;
    HfGpioErr DisableInterrupt() noexcept;
    bool IsInterruptEnabled() const noexcept { return interrupt_enabled_; }

private:
    gpio_int_type_t interrupt_type_;
    InterruptCallback callback_;
    void* user_data_;
    bool interrupt_enabled_;

    static void IRAM_ATTR InterruptHandler(void* arg);
};

/**
 * @class Esp32C6GpioManager
 * @brief Central manager for ESP32-C6 GPIO pin allocation and validation.
 */
class Esp32C6GpioManager {
public:
    /**
     * @brief Get the singleton instance.
     */
    static Esp32C6GpioManager& GetInstance() noexcept;

    /**
     * @brief Check if a pin is available for GPIO use.
     * @param pin_num GPIO pin number to check
     * @return true if pin is available, false if reserved
     */
    bool IsPinAvailable(gpio_num_t pin_num) const noexcept;

    /**
     * @brief Check if a pin is reserved for specific functions.
     * @param pin_num GPIO pin number to check
     * @return true if pin is reserved, false if available
     */
    bool IsPinReserved(gpio_num_t pin_num) const noexcept;

    /**
     * @brief Reserve a pin for GPIO use.
     * @param pin_num GPIO pin number to reserve
     * @return HfGpioErr::GPIO_SUCCESS if successful
     */
    HfGpioErr ReservePin(gpio_num_t pin_num) noexcept;

    /**
     * @brief Release a reserved pin.
     * @param pin_num GPIO pin number to release
     * @return HfGpioErr::GPIO_SUCCESS if successful
     */
    HfGpioErr ReleasePin(gpio_num_t pin_num) noexcept;

    /**
     * @brief Get list of available GPIO pins.
     * @return Array of available pin numbers
     */
    std::array<gpio_num_t, 15> GetAvailablePins() const noexcept;

    /**
     * @brief Print pin allocation status for debugging.
     */
    void PrintPinStatus() const noexcept;

private:
    Esp32C6GpioManager() = default;
    std::bitset<Esp32C6GpioConfig::MAX_GPIO_PINS + 1> reserved_pins_;
    mutable std::mutex mutex_;
};

#endif // ESP32C6_GPIO_H
