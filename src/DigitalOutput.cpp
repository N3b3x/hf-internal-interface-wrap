/**
 * @file DigitalOutput.cpp
 * @brief Implementation of DigitalOutput for ESP32-C6 using ESP-IDF.
 *
 * This file provides the implementation for digital output pin abstraction on ESP32-C6.
 *
 * @note This implementation is intended for use with ESP-IDF. Pin configuration is handled via gpio_config().
 */
#include "DigitalOutput.h"
#include "driver/gpio.h"

DigitalOutput::DigitalOutput(gpio_num_t pinArg, ActiveState activeStateArg, State initialStateArg) noexcept :
    DigitalGpio(pinArg, activeStateArg),
    initialState(initialStateArg)
{
    // No code at this time.
}

DigitalOutput::~DigitalOutput() {
    // No dynamic resources to free for ESP32.
}

bool DigitalOutput::Initialize() noexcept {
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    bool ok = (gpio_config(&io_conf) == ESP_OK);
    if (ok) {
        // Set initial state
        if (IsActiveHigh()) {
            gpio_set_level(pin, initialState == State::Active ? 1 : 0);
        } else {
            gpio_set_level(pin, initialState == State::Active ? 0 : 1);
        }
    }
    return ok;
}

DigitalGpio::Mode DigitalOutput::OutputMode() const noexcept {
    // For ESP32, open-drain can be set in derived class if needed.
    // Here, always return PushPull for default implementation.
    return Mode::PushPull;
}

bool DigitalOutput::IsActive() noexcept {
    if (EnsureInitialized()) {
        int level = gpio_get_level(pin);
        if (IsActiveHigh()) {
            return level == 1;
        } else {
            return level == 0;
        }
    }
    return false;
}

bool DigitalOutput::SetActive() noexcept {
    if (EnsureInitialized()) {
        if (IsActiveHigh()) {
            return gpio_set_level(pin, 1) == ESP_OK;
        } else {
            return gpio_set_level(pin, 0) == ESP_OK;
        }
    }
    return false;
}

bool DigitalOutput::SetInactive() noexcept {
    if (EnsureInitialized()) {
        if (IsActiveHigh()) {
            return gpio_set_level(pin, 0) == ESP_OK;
        } else {
            return gpio_set_level(pin, 1) == ESP_OK;
        }
    }
    return false;
}

bool DigitalOutput::Toggle() noexcept {
    if (EnsureInitialized()) {
        int level = gpio_get_level(pin);
        if (IsActive()) {
            return SetInactive();
        } else {
            return SetActive();
        }
    }
    return false;
}

