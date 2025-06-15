#ifndef PWMOUTPUT_H
#define PWMOUTPUT_H

#include "DigitalGpio.h"
#include "driver/ledc.h"
#include <cstdint>

/**
 * @file PwmOutput.h
 * @brief PWM output abstraction using the ESP-IDF LEDC driver.
 *
 * Provides a lightweight wrapper around the LEDC driver to allow
 * simple PWM generation with configurable frequency and duty cycle.
 * This class follows the lazy initialization design of other drivers
 * in this component.
 *
 * @note Not thread-safe. Guard calls if used from multiple contexts.
 */
class PwmOutput : public DigitalGpio {
public:
  /**
   * @brief Construct a PwmOutput.
   * @param pin GPIO pin to output PWM
   * @param channel LEDC channel (LEDC_CHANNEL_0..)
   * @param timer LEDC timer index (LEDC_TIMER_0..)
   * @param freqHz PWM frequency in Hz
   * @param resolution Timer resolution (e.g. LEDC_TIMER_13_BIT)
   * @param activeState Signal active state
   */
  PwmOutput(gpio_num_t pin, ledc_channel_t channel, ledc_timer_t timer, uint32_t freqHz,
            ledc_timer_bit_t resolution, ActiveState activeState = ActiveState::High) noexcept;

  PwmOutput(const PwmOutput &) = delete;
  PwmOutput &operator=(const PwmOutput &) = delete;
  virtual ~PwmOutput() override;

  /**
   * @brief Start PWM generation.
   * @return true if started
   */
  bool Start() noexcept;

  /**
   * @brief Stop PWM output and set pin inactive.
   * @return true if stopped
   */
  bool Stop() noexcept;

  /**
   * @brief Set duty cycle as fraction from 0.0 to 1.0.
   */
  bool SetDuty(float duty) noexcept;

  /**
   * @brief Update the output frequency.
   */
  bool SetFrequency(uint32_t freqHz) noexcept;

  /**
   * @brief Get the configured frequency in Hz.
   */
  uint32_t GetFrequency() const noexcept {
    return frequency;
  }

private:
  virtual bool Initialize() noexcept override;
  ledc_channel_t channel;
  ledc_timer_t timer;
  uint32_t frequency;
  ledc_timer_bit_t resolution;
};

#endif // PWMOUTPUT_H
