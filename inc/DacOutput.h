#ifndef DACOUTPUT_H
#define DACOUTPUT_H

#include <driver/dac_common.h>
#include <cstdint>

/**
 * @file DacOutput.h
 * @brief Thin wrapper for the ESP-IDF DAC driver.
 */
class DacOutput {
public:
    explicit DacOutput(dac_channel_t channel) noexcept;
    ~DacOutput() noexcept;
    DacOutput(const DacOutput&) = delete;
    DacOutput& operator=(const DacOutput&) = delete;

    bool Enable() noexcept;   ///< Enable the channel
    bool Disable() noexcept;  ///< Disable the channel
    bool SetValue(uint8_t value) noexcept; ///< Output 8-bit value

    bool IsEnabled() const noexcept { return enabled; }

private:
    dac_channel_t channel;
    bool enabled;
};

#endif // DACOUTPUT_H
