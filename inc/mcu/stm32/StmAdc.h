/**
 * @file StmAdc.h
 * @brief STM32 ADC stub implementation — placeholder for future STM32 HAL integration.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseAdc.h"

/**
 * @brief STM32 ADC — stub implementation.
 */
class StmAdc : public BaseAdc {
public:
    StmAdc() noexcept;
    ~StmAdc() noexcept override;

    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    hf_u8_t GetMaxChannels() const noexcept override;
    bool IsChannelAvailable(hf_channel_id_t channel_id) const noexcept override;
    hf_adc_err_t ReadChannelV(hf_channel_id_t channel_id, float& channel_reading_v,
                              hf_u8_t numOfSamplesToAvg = 1,
                              hf_time_t timeBetweenSamples = 0) noexcept override;
    hf_adc_err_t ReadChannelCount(hf_channel_id_t channel_id, hf_u32_t& channel_reading_count,
                                  hf_u8_t numOfSamplesToAvg = 1,
                                  hf_time_t timeBetweenSamples = 0) noexcept override;
    hf_adc_err_t ReadChannel(hf_channel_id_t channel_id, hf_u32_t& channel_reading_count,
                             float& channel_reading_v, hf_u8_t numOfSamplesToAvg = 1,
                             hf_time_t timeBetweenSamples = 0) noexcept override;
};
