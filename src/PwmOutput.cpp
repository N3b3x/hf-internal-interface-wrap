#include "PwmOutput.h"
#include "driver/ledc.h"

PwmOutput::PwmOutput(gpio_num_t pinArg, ledc_channel_t channelArg,
                     ledc_timer_t timerArg, uint32_t freqHz,
                     ledc_timer_bit_t resolutionArg,
                     ActiveState activeStateArg) noexcept
    : DigitalGpio(pinArg, activeStateArg),
      channel(channelArg),
      timer(timerArg),
      frequency(freqHz),
      resolution(resolutionArg) {}

PwmOutput::~PwmOutput() noexcept {
    Stop();
}

bool PwmOutput::Initialize() noexcept {
    ledc_timer_config_t timer_conf = {};
    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_conf.timer_num = timer;
    timer_conf.freq_hz = frequency;
    timer_conf.duty_resolution = resolution;
    if (ledc_timer_config(&timer_conf) != ESP_OK) {
        return false;
    }

    ledc_channel_config_t ch_conf = {};
    ch_conf.channel = channel;
    ch_conf.gpio_num = pin;
    ch_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    ch_conf.hpoint = 0;
    ch_conf.timer_sel = timer;
    ch_conf.duty = 0;
    if (ledc_channel_config(&ch_conf) != ESP_OK) {
        return false;
    }
    return true;
}

bool PwmOutput::Start() noexcept {
    if (!EnsureInitialized()) return false;
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, channel) == ESP_OK;
}

bool PwmOutput::Stop() noexcept {
    if (!initialized) return true;
    return ledc_stop(LEDC_LOW_SPEED_MODE, channel, IsActiveHigh() ? 0 : 1) == ESP_OK;
}

bool PwmOutput::SetDuty(float duty) noexcept {
    if (!EnsureInitialized()) return false;
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;
    uint32_t max_duty = (1u << resolution) - 1u;
    uint32_t duty_val = static_cast<uint32_t>(duty * max_duty);
    if (ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty_val) != ESP_OK) {
        return false;
    }
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, channel) == ESP_OK;
}

bool PwmOutput::SetFrequency(uint32_t freqHz) noexcept {
    if (!EnsureInitialized()) return false;
    frequency = freqHz;
    return ledc_set_freq(LEDC_LOW_SPEED_MODE, timer, frequency) > 0;
}

