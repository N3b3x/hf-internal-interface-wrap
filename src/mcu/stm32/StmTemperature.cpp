/**
 * @file StmTemperature.cpp
 * @brief STM32 Temperature sensor implementation — ADC-based with internal/external modes.
 *
 * @author HardFOC
 * @date 2025
 * @copyright HardFOC — Licensed under GPL v3.0 or later.
 */

#include "StmTemperature.h"
#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
// STM32 HAL FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════════

extern "C" {
extern uint32_t HAL_ADC_Start(ADC_HandleTypeDef* hadc);
extern uint32_t HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout);
extern uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef* hadc);
extern uint32_t HAL_ADC_Stop(ADC_HandleTypeDef* hadc);
extern void     HAL_Delay(uint32_t Delay);
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTORS / DESTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

StmTemperature::StmTemperature(const hf_stm32_temp_sensor_config_t& config) noexcept
    : BaseTemperature()
    , config_(config)
    , calibration_offset_(0.0f)
{
}

StmTemperature::StmTemperature(ADC_HandleTypeDef* hadc,
                               hf_u8_t resolution_bits,
                               float vref_mv) noexcept
    : BaseTemperature()
    , calibration_offset_(0.0f)
{
    config_ = hf_stm32_temp_sensor_config_t{};
    config_.hadc = hadc;
    config_.resolution_bits = resolution_bits;
    config_.vref_mv = vref_mv;
}

StmTemperature::~StmTemperature() noexcept {
    if (IsInitialized()) Deinitialize();
}

// ═══════════════════════════════════════════════════════════════════════════════
// INITIALIZATION
// ═══════════════════════════════════════════════════════════════════════════════

bool StmTemperature::Initialize() noexcept {
    if (!config_.hadc) return false;
    // ADC peripheral is assumed to be initialized by CubeMX.
    // No additional init needed — just validate handle.
    return true;
}

bool StmTemperature::Deinitialize() noexcept {
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// CORE TEMPERATURE READING
// ═══════════════════════════════════════════════════════════════════════════════

hf_temp_err_t StmTemperature::ReadTemperatureCelsiusImpl(float* temperature_celsius) noexcept {
    if (!temperature_celsius) return TEMP_ERR_NULL_POINTER;
    if (!config_.hadc) return TEMP_ERR_SENSOR_NOT_AVAILABLE;

    hf_u32_t sample_count = (config_.sample_count > 0) ? config_.sample_count : 1;
    hf_u64_t accumulator = 0;

    for (hf_u32_t i = 0; i < sample_count; ++i) {
        uint32_t status = HAL_ADC_Start(config_.hadc);
        if (!hf::stm32::IsHalOk(status)) return TEMP_ERR_READ_FAILED;

        status = HAL_ADC_PollForConversion(config_.hadc, config_.timeout_ms);
        if (!hf::stm32::IsHalOk(status)) {
            HAL_ADC_Stop(config_.hadc);
            return TEMP_ERR_TIMEOUT;
        }

        accumulator += HAL_ADC_GetValue(config_.hadc);
        HAL_ADC_Stop(config_.hadc);

        if (i + 1 < sample_count) HAL_Delay(1);
    }

    hf_u32_t avg_raw = static_cast<hf_u32_t>(accumulator / sample_count);

    // Convert raw → °C
    float temp_c;
    if (config_.convert_fn) {
        temp_c = config_.convert_fn(avg_raw, config_.resolution_bits);
    } else {
        temp_c = DefaultInternalConvert(avg_raw, config_.resolution_bits);
    }

    // Apply calibration offset
    temp_c += calibration_offset_;

    // Range check
    if (temp_c < config_.range_min_celsius || temp_c > config_.range_max_celsius) {
        *temperature_celsius = temp_c;  // Still provide reading
        return TEMP_ERR_OUT_OF_RANGE;
    }

    *temperature_celsius = temp_c;
    return TEMP_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// INFO & CAPABILITIES
// ═══════════════════════════════════════════════════════════════════════════════

hf_temp_err_t StmTemperature::GetSensorInfo(hf_temp_sensor_info_t* info) const noexcept {
    if (!info) return TEMP_ERR_NULL_POINTER;

    std::memset(info, 0, sizeof(*info));
    info->sensor_type = config_.sensor_type;
    info->min_temp_celsius = config_.range_min_celsius;
    info->max_temp_celsius = config_.range_max_celsius;

    // Resolution depends on ADC bits and sensor slope
    float max_count = static_cast<float>(1U << config_.resolution_bits);
    info->resolution_celsius = (config_.range_max_celsius - config_.range_min_celsius) / max_count;
    info->accuracy_celsius = 1.5f;  // Typical for STM32 internal sensor
    info->response_time_ms = config_.timeout_ms * config_.sample_count;
    info->capabilities = GetCapabilities();
    info->manufacturer = "STMicroelectronics";
    info->model = (config_.sensor_type == HF_TEMP_SENSOR_TYPE_INTERNAL)
                  ? "STM32 Internal" : "External";
    info->version = "1.0.0";

    return TEMP_SUCCESS;
}

hf_u32_t StmTemperature::GetCapabilities() const noexcept {
    return HF_TEMP_CAP_CALIBRATION | HF_TEMP_CAP_CONTINUOUS_READING;
}

// ═══════════════════════════════════════════════════════════════════════════════
// OPTIONAL OVERRIDES
// ═══════════════════════════════════════════════════════════════════════════════

hf_temp_err_t StmTemperature::SetRange(float min_celsius, float max_celsius) noexcept {
    if (min_celsius >= max_celsius) return TEMP_ERR_INVALID_RANGE;
    config_.range_min_celsius = min_celsius;
    config_.range_max_celsius = max_celsius;
    return TEMP_SUCCESS;
}

hf_temp_err_t StmTemperature::GetRange(float* min_celsius, float* max_celsius) const noexcept {
    if (!min_celsius || !max_celsius) return TEMP_ERR_NULL_POINTER;
    *min_celsius = config_.range_min_celsius;
    *max_celsius = config_.range_max_celsius;
    return TEMP_SUCCESS;
}

hf_temp_err_t StmTemperature::SetCalibrationOffset(float offset_celsius) noexcept {
    calibration_offset_ = offset_celsius;
    return TEMP_SUCCESS;
}

hf_temp_err_t StmTemperature::GetCalibrationOffset(float* offset_celsius) const noexcept {
    if (!offset_celsius) return TEMP_ERR_NULL_POINTER;
    *offset_celsius = calibration_offset_;
    return TEMP_SUCCESS;
}

// ═══════════════════════════════════════════════════════════════════════════════
// DEFAULT INTERNAL SENSOR CONVERSION
// ═══════════════════════════════════════════════════════════════════════════════

float StmTemperature::DefaultInternalConvert(hf_u32_t raw, hf_u8_t resolution_bits) {
    /*
     * Generic STM32 internal temperature sensor formula (from RM):
     *   T(°C) = ((V_SENSE - V_25) / Avg_Slope) + 25
     *
     * Typical values (STM32F4):
     *   V_25 = 0.76 V (voltage at 25°C)
     *   Avg_Slope = 2.5 mV/°C
     *   VDDA = 3.3 V
     *
     * For better accuracy, use the per-chip calibration stored in system memory:
     *   TS_CAL1 = ADC reading at 30°C / VDDA=3.3V
     *   TS_CAL2 = ADC reading at 110°C / VDDA=3.3V
     *
     * Users needing factory calibration should provide a custom convert_fn.
     */
    float max_count = static_cast<float>(1U << resolution_bits);
    float vdda_v = 3.3f;
    float v_sense = (static_cast<float>(raw) / max_count) * vdda_v;

    constexpr float v_25   = 0.76f;      // V at 25°C (typical F4)
    constexpr float slope  = 0.0025f;    // 2.5 mV/°C

    return ((v_sense - v_25) / slope) + 25.0f;
}
