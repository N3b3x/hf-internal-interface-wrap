/**
 * @file StmAdc.cpp
 * @brief STM32 ADC stub implementation.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#include "StmAdc.h"

StmAdc::StmAdc() noexcept = default;
StmAdc::~StmAdc() noexcept = default;

bool StmAdc::Initialize() noexcept { return false; }
bool StmAdc::Deinitialize() noexcept { return false; }
hf_u8_t StmAdc::GetMaxChannels() const noexcept { return 0; }
bool StmAdc::IsChannelAvailable(hf_channel_id_t) const noexcept { return false; }

hf_adc_err_t StmAdc::ReadChannelV(hf_channel_id_t, float&, hf_u8_t, hf_time_t) noexcept {
    return hf_adc_err_t::ADC_ERR_UNSUPPORTED_OPERATION;
}

hf_adc_err_t StmAdc::ReadChannelCount(hf_channel_id_t, hf_u32_t&, hf_u8_t, hf_time_t) noexcept {
    return hf_adc_err_t::ADC_ERR_UNSUPPORTED_OPERATION;
}

hf_adc_err_t StmAdc::ReadChannel(hf_channel_id_t, hf_u32_t&, float&, hf_u8_t, hf_time_t) noexcept {
    return hf_adc_err_t::ADC_ERR_UNSUPPORTED_OPERATION;
}
