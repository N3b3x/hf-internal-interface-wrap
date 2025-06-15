/**
 * @file DigitalGpio.cpp
 * @brief Implementation of DigitalGpio for ESP32-C6 using ESP-IDF.
 *
 * This file provides the implementation for digital GPIO pin abstraction on
 * ESP32-C6.
 *
 * @note This implementation is intended for use with ESP-IDF. Pin configuration
 * is expected to be handled by derived classes or via the
 * PinCfg/gpio_config_esp32c6.hpp utilities.
 */
#include "DigitalGpio.h"
#include "driver/gpio.h"

const char *DigitalGpio::ToString(Mode mode) noexcept {
  switch (mode) {
  case Mode::OpenDrain:
    return "Open-Drain";
  case Mode::PushPull:
    return "Push-Pull";
  }
  return "Unknown";
}
const char *DigitalGpio::ToString(State state) noexcept {
  switch (state) {
  case State::Active:
    return "Active";
  case State::Inactive:
    return "Inactive";
  }
  return "Unknown";
}
const char *DigitalGpio::ToString(ActiveState activeState) noexcept {
  switch (activeState) {
  case ActiveState::High:
    return "High";
  case ActiveState::Low:
    return "Low";
  }
  return "Unknown";
}
const char *DigitalGpio::ToString(Resistance resistance) noexcept {
  switch (resistance) {
  case Resistance::Floating:
    return "Floating";
  case Resistance::PullUp:
    return "PullUp";
  case Resistance::PullDown:
    return "PullDown";
  }
  return "Unknown";
}

DigitalGpio::Resistance DigitalGpio::GetResistance() const noexcept {
  // For ESP32, use gpio_get_pull_mode if needed, or store config in derived
  // class. Here, we assume derived classes set the pull mode and can override
  // this if needed. Default: Floating (no pull-up/down)
  return Resistance::Floating;
}
