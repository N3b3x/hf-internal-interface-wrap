/**
 * @file StmAdc.cpp
 * @brief STM32 ADC wrapper implementation — full STM32 HAL integration.
 *
 * Wraps HAL_ADC_Start, HAL_ADC_PollForConversion, HAL_ADC_GetValue for
 * single-shot ADC conversions with averaging support.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmAdc.h"

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
extern uint32_t HAL_ADC_Start(ADC_HandleTypeDef* hadc);
extern uint32_t HAL_ADC_Stop(ADC_HandleTypeDef* hadc);
extern uint32_t HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout);
extern uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef* hadc);
extern uint32_t HAL_GetTick(void);

// Delay helper
extern void HAL_Delay(uint32_t Delay);
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmAdc::StmAdc(const hf_stm32_adc_config_t& config) noexcept
    : config_(config) {}

StmAdc::StmAdc(ADC_HandleTypeDef* hal_handle, hf_u8_t num_channels) noexcept
    : config_(hf_stm32_adc_config_t(hal_handle, num_channels)) {}

StmAdc::~StmAdc() noexcept {
    if (initialized_) {
        Deinitialize();
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

bool StmAdc::Initialize() noexcept {
    if (initialized_) return true;
    if (!config_.hal_handle) return false;

    // ADC peripheral is assumed to be already initialized by CubeMX HAL_ADC_Init().
    // We just validate the handle and mark as ready.
    initialized_ = true;
    return true;
}

bool StmAdc::Deinitialize() noexcept {
    if (!initialized_) return true;
    HAL_ADC_Stop(config_.hal_handle);
    initialized_ = false;
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// CHANNEL QUERIES
// ═══════════════════════════════════════════════════════════════════════════════

hf_u8_t StmAdc::GetMaxChannels() const noexcept {
    return hf::stm32::kAdcMaxChannels;
}

bool StmAdc::IsChannelAvailable(hf_channel_id_t channel_id) const noexcept {
    return channel_id < config_.num_channels;
}

// ═══════════════════════════════════════════════════════════════════════════════
// READING
// ═══════════════════════════════════════════════════════════════════════════════

hf_adc_err_t StmAdc::ReadChannelV(hf_channel_id_t channel_id, float& channel_reading_v,
                                  hf_u8_t numOfSamplesToAvg,
                                  hf_time_t timeBetweenSamples) noexcept {
    if (!EnsureInitialized()) return hf_adc_err_t::ADC_ERR_NOT_INITIALIZED;
    if (!IsChannelAvailable(channel_id)) return hf_adc_err_t::ADC_ERR_INVALID_CHANNEL;
    if (numOfSamplesToAvg == 0) numOfSamplesToAvg = 1;

    hf_u64_t accumulator = 0;
    for (hf_u8_t i = 0; i < numOfSamplesToAvg; ++i) {
        hf_u32_t count = 0;
        auto err = ReadRawCount(count);
        if (err != hf_adc_err_t::ADC_SUCCESS) {
            statistics_.error_count++;
            return err;
        }
        accumulator += count;
        if (i < numOfSamplesToAvg - 1 && timeBetweenSamples > 0) {
            HAL_Delay(timeBetweenSamples);
        }
    }

    hf_u32_t avg_count = static_cast<hf_u32_t>(accumulator / numOfSamplesToAvg);
    channel_reading_v = CountToVoltage(avg_count);

    statistics_.total_reads++;
    return hf_adc_err_t::ADC_SUCCESS;
}

hf_adc_err_t StmAdc::ReadChannelCount(hf_channel_id_t channel_id, hf_u32_t& channel_reading_count,
                                      hf_u8_t numOfSamplesToAvg,
                                      hf_time_t timeBetweenSamples) noexcept {
    if (!EnsureInitialized()) return hf_adc_err_t::ADC_ERR_NOT_INITIALIZED;
    if (!IsChannelAvailable(channel_id)) return hf_adc_err_t::ADC_ERR_INVALID_CHANNEL;
    if (numOfSamplesToAvg == 0) numOfSamplesToAvg = 1;

    hf_u64_t accumulator = 0;
    for (hf_u8_t i = 0; i < numOfSamplesToAvg; ++i) {
        hf_u32_t count = 0;
        auto err = ReadRawCount(count);
        if (err != hf_adc_err_t::ADC_SUCCESS) {
            statistics_.error_count++;
            return err;
        }
        accumulator += count;
        if (i < numOfSamplesToAvg - 1 && timeBetweenSamples > 0) {
            HAL_Delay(timeBetweenSamples);
        }
    }

    channel_reading_count = static_cast<hf_u32_t>(accumulator / numOfSamplesToAvg);

    statistics_.total_reads++;
    return hf_adc_err_t::ADC_SUCCESS;
}

hf_adc_err_t StmAdc::ReadChannel(hf_channel_id_t channel_id,
                                 hf_u32_t& channel_reading_count,
                                 float& channel_reading_v,
                                 hf_u8_t numOfSamplesToAvg,
                                 hf_time_t timeBetweenSamples) noexcept {
    auto err = ReadChannelCount(channel_id, channel_reading_count,
                                numOfSamplesToAvg, timeBetweenSamples);
    if (err == hf_adc_err_t::ADC_SUCCESS) {
        channel_reading_v = CountToVoltage(channel_reading_count);
    }
    return err;
}

// ═══════════════════════════════════════════════════════════════════════════════
// RESOLUTION / CONVERSION HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

hf_u8_t StmAdc::GetResolutionBits() const noexcept {
    return static_cast<hf_u8_t>(config_.resolution);
}

hf_u32_t StmAdc::GetMaxCount() const noexcept {
    return (1U << GetResolutionBits()) - 1U;
}

float StmAdc::CountToVoltage(hf_u32_t count) const noexcept {
    hf_u32_t max_count = GetMaxCount();
    if (max_count == 0) return 0.0f;
    return (static_cast<float>(count) / static_cast<float>(max_count)) * config_.vref_v;
}

// ═══════════════════════════════════════════════════════════════════════════════
// PRIVATE HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

hf_adc_err_t StmAdc::ReadRawCount(hf_u32_t& count) noexcept {
    count = 0;
    if (!config_.hal_handle) return hf_adc_err_t::ADC_ERR_NOT_INITIALIZED;

    // Start conversion
    uint32_t status = HAL_ADC_Start(config_.hal_handle);
    if (!hf::stm32::IsHalOk(status)) {
        return ConvertHalStatus(status);
    }

    // Poll for completion (100ms timeout)
    status = HAL_ADC_PollForConversion(config_.hal_handle, 100);
    if (!hf::stm32::IsHalOk(status)) {
        HAL_ADC_Stop(config_.hal_handle);
        return ConvertHalStatus(status);
    }

    // Read result
    count = HAL_ADC_GetValue(config_.hal_handle);

    return hf_adc_err_t::ADC_SUCCESS;
}

hf_adc_err_t StmAdc::ConvertHalStatus(hf_u32_t hal_status) noexcept {
    auto status = hf::stm32::ToHalStatus(hal_status);
    switch (status) {
        case hf::stm32::HalStatus::OK:      return hf_adc_err_t::ADC_SUCCESS;
        case hf::stm32::HalStatus::BUSY:    return hf_adc_err_t::ADC_ERR_BUSY;
        case hf::stm32::HalStatus::TIMEOUT: return hf_adc_err_t::ADC_ERR_TIMEOUT;
        default:                             return hf_adc_err_t::ADC_ERR_READ_FAILED;
    }
}
