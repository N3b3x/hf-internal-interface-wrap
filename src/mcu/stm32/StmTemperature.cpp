/**
 * @file StmTemperature.cpp
 * @brief STM32 Temperature sensor stub implementation.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#include "StmTemperature.h"
#include <cstring>

StmTemperature::StmTemperature() noexcept = default;
StmTemperature::~StmTemperature() noexcept = default;

bool StmTemperature::Initialize() noexcept { return false; }
bool StmTemperature::Deinitialize() noexcept { return false; }

hf_temp_err_t StmTemperature::ReadTemperatureCelsiusImpl(float* temperature_celsius) noexcept {
    if (temperature_celsius) *temperature_celsius = 0.0f;
    return TEMP_ERR_UNSUPPORTED_OPERATION;
}

hf_temp_err_t StmTemperature::GetSensorInfo(hf_temp_sensor_info_t* info) const noexcept {
    if (info) memset(info, 0, sizeof(*info));
    return TEMP_ERR_UNSUPPORTED_OPERATION;
}

hf_u32_t StmTemperature::GetCapabilities() const noexcept { return 0; }
