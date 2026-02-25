/**
 * @file StmTemperature.h
 * @brief STM32 Temperature sensor — reads the internal temp sensor via ADC or
 *        an external sensor via a user-supplied ADC handle + conversion function.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#pragma once

#include "BaseTemperature.h"
#include "StmTypes.h"

struct ADC_HandleTypeDef;  // Forward declaration

/**
 * @brief User-supplied function to convert a raw ADC count to Celsius.
 *
 * For the STM32 internal sensor the typical formula is:
 *   T(°C) = ((V_SENSE – V_25) / Avg_Slope) + 25
 * but calibration constants differ per family (see RM).
 *
 * For external NTC / PTC / thermocouple channels the user provides their own
 * linearisation function.
 */
using hf_stm32_temp_convert_fn = float(*)(hf_u32_t raw_adc_count, hf_u8_t resolution_bits);

/**
 * @brief STM32 Temperature sensor configuration.
 */
struct hf_stm32_temp_sensor_config_t {
    ADC_HandleTypeDef*      hadc;                   ///< ADC handle (CubeMX)
    hf_u32_t                channel;                ///< ADC channel number
    hf_u8_t                 resolution_bits;        ///< ADC resolution (8/10/12/16)
    float                   vref_mv;                ///< ADC reference voltage in mV
    hf_stm32_temp_convert_fn convert_fn;            ///< Optional custom conversion
    hf_temp_sensor_type_t   sensor_type;            ///< Sensor type hint
    float                   range_min_celsius;      ///< Min measurable temp
    float                   range_max_celsius;      ///< Max measurable temp
    hf_u32_t                sample_count;           ///< Oversampling count (1 = no avg)
    hf_u32_t                timeout_ms;             ///< ADC poll timeout

    hf_stm32_temp_sensor_config_t() noexcept
        : hadc(nullptr), channel(0), resolution_bits(12), vref_mv(3300.0f)
        , convert_fn(nullptr), sensor_type(HF_TEMP_SENSOR_TYPE_INTERNAL)
        , range_min_celsius(-40.0f), range_max_celsius(125.0f)
        , sample_count(4), timeout_ms(100) {}
};

/**
 * @class StmTemperature
 * @brief STM32 Temperature sensor implementation.
 *
 * Two modes of operation:
 * 1. **Internal sensor** — reads the on-chip temperature channel.  The default
 *    conversion function uses the generic RM formula; users can override with
 *    family-specific calibration constants for better accuracy.
 * 2. **External sensor** — any NTC / PTC / thermocouple connected to an ADC
 *    channel.  The user provides a `convert_fn` that maps raw counts → °C.
 */
class StmTemperature : public BaseTemperature {
public:

    /**
     * @brief Construct from config.
     */
    explicit StmTemperature(const hf_stm32_temp_sensor_config_t& config) noexcept;

    /**
     * @brief Construct with ADC handle + basics (uses internal sensor defaults).
     */
    explicit StmTemperature(ADC_HandleTypeDef* hadc = nullptr,
                            hf_u8_t resolution_bits = 12,
                            float vref_mv = 3300.0f) noexcept;

    ~StmTemperature() noexcept override;

    // ── Pure virtuals from BaseTemperature ──────────────────────────────────
    hf_temp_err_t GetSensorInfo(hf_temp_sensor_info_t* info) const noexcept override;
    hf_u32_t GetCapabilities() const noexcept override;

    // ── Optional overrides ──────────────────────────────────────────────────
    hf_temp_err_t SetRange(float min_celsius, float max_celsius) noexcept override;
    hf_temp_err_t GetRange(float* min_celsius, float* max_celsius) const noexcept override;
    hf_temp_err_t SetCalibrationOffset(float offset_celsius) noexcept override;
    hf_temp_err_t GetCalibrationOffset(float* offset_celsius) const noexcept override;

protected:
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    hf_temp_err_t ReadTemperatureCelsiusImpl(float* temperature_celsius) noexcept override;

private:
    hf_stm32_temp_sensor_config_t config_;
    float calibration_offset_;   ///< User calibration offset (°C)

    /// Default internal-sensor conversion (generic RM formula).
    static float DefaultInternalConvert(hf_u32_t raw, hf_u8_t resolution_bits);
};
