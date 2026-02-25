/**
 * @file StmAdc.h
 * @brief STM32 ADC wrapper — wraps STM32 HAL ADC via CubeMX-generated handle.
 *
 * Provides the full BaseAdc interface for STM32 ADC peripherals. Users configure
 * ADC channels in STM32CubeMX and pass the ADC_HandleTypeDef* to this wrapper.
 *
 * @section Usage
 * @code
 * extern ADC_HandleTypeDef hadc1;
 *
 * hf_stm32_adc_config_t config(&hadc1, 4, 3.3f);  // 4 channels, 3.3V Vref
 * StmAdc adc(config);
 * adc.Initialize();
 *
 * float voltage = 0.0f;
 * adc.ReadChannelV(0, voltage);
 * @endcode
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseAdc.h"
#include "StmTypes.h"

/**
 * @brief STM32 ADC wrapper — wraps STM32 HAL ADC with full channel management.
 *
 * Features:
 * - Single-conversion and multi-sample averaging via HAL_ADC_Start/PollForConversion
 * - Voltage conversion using configurable Vref and resolution
 * - Raw count reading for custom calibration workflows
 * - Combined count + voltage reads in a single call
 * - Statistics tracking (total reads, errors, timing)
 */
class StmAdc : public BaseAdc {
public:
    /**
     * @brief Construct with full configuration.
     * @param config  ADC config including CubeMX HAL handle, resolution, Vref, channel count
     */
    explicit StmAdc(const hf_stm32_adc_config_t& config) noexcept;

    /// @brief Construct from raw HAL handle with defaults.
    explicit StmAdc(ADC_HandleTypeDef* hal_handle, hf_u8_t num_channels = 1) noexcept;

    ~StmAdc() noexcept override;

    // ── BaseAdc overrides ────────────────────────────────────────────────

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

    // ── STM32-specific ───────────────────────────────────────────────────

    /// @brief Get the underlying HAL ADC handle
    ADC_HandleTypeDef* GetHalHandle() const noexcept { return config_.hal_handle; }

    /// @brief Get the configured reference voltage
    float GetVref() const noexcept { return config_.vref_v; }

    /// @brief Get the current ADC resolution in bits
    hf_u8_t GetResolutionBits() const noexcept;

    /// @brief Get the maximum ADC count for current resolution
    hf_u32_t GetMaxCount() const noexcept;

    /// @brief Convert raw ADC count to voltage
    float CountToVoltage(hf_u32_t count) const noexcept;

private:
    /// @brief Perform a single ADC conversion and return the raw count
    hf_adc_err_t ReadRawCount(hf_u32_t& count) noexcept;

    /// @brief Convert HAL status to ADC error code
    static hf_adc_err_t ConvertHalStatus(hf_u32_t hal_status) noexcept;

    hf_stm32_adc_config_t config_;   ///< ADC configuration
};
