/**
 * @file DigitalOutput.cpp
 * @brief Implementation of DigitalOutput for ESP32-C6 using ESP-IDF.
 *
 * This file provides the implementation for digital output pin abstraction on
 * ESP32-C6.
 *
 * @note This implementation is intended for use with ESP-IDF. Pin configuration
 * is handled via gpio_config().
 */
#include "DigitalOutput.h"
#include "driver/gpio.h"

DigitalOutput::DigitalOutput(gpio_num_t pinArg, ActiveState activeStateArg,
                             State initialStateArg) noexcept
    : DigitalGpio(pinArg, activeStateArg), initialState(initialStateArg) {
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

bool DigitalOutput::IsPinAvailable() const noexcept {
  // For legacy compatibility, assume all pins are available
  // In practice, this should check against reserved pins
  return pin >= 0 && pin <= 30;
}

uint8_t DigitalOutput::GetMaxPins() const noexcept {
  return 31; // ESP32-C6 has 31 GPIO pins (0-30)
}

DigitalGpio::Mode DigitalOutput::OutputMode() const noexcept {
  // For ESP32, open-drain can be set in derived class if needed.
  // Here, always return PushPull for default implementation.
  return Mode::PushPull;
}

HfGpioErr DigitalOutput::SetActiveImpl() noexcept {
  if (IsActiveHigh()) {
    return gpio_set_level(pin, 1) == ESP_OK ? HfGpioErr::GPIO_SUCCESS : HfGpioErr::GPIO_ERR_WRITE_FAILURE;
  } else {
    return gpio_set_level(pin, 0) == ESP_OK ? HfGpioErr::GPIO_SUCCESS : HfGpioErr::GPIO_ERR_WRITE_FAILURE;
  }
}

HfGpioErr DigitalOutput::SetInactiveImpl() noexcept {
  if (IsActiveHigh()) {
    return gpio_set_level(pin, 0) == ESP_OK ? HfGpioErr::GPIO_SUCCESS : HfGpioErr::GPIO_ERR_WRITE_FAILURE;
  } else {
    return gpio_set_level(pin, 1) == ESP_OK ? HfGpioErr::GPIO_SUCCESS : HfGpioErr::GPIO_ERR_WRITE_FAILURE;
  }
}

HfGpioErr DigitalOutput::ToggleImpl() noexcept {
  bool is_active;
  HfGpioErr result = IsActiveImpl(is_active);
  if (result != HfGpioErr::GPIO_SUCCESS) {
    return result;
  }
  
  return is_active ? SetInactiveImpl() : SetActiveImpl();
}

HfGpioErr DigitalOutput::IsActiveImpl(bool& is_active) noexcept {
  int level = gpio_get_level(pin);
  is_active = IsActiveHigh() ? (level != 0) : (level == 0);
  return HfGpioErr::GPIO_SUCCESS;
}


