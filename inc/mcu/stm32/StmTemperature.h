/**
 * @file StmTemperature.h
 * @brief STM32 Temperature sensor stub — placeholder for internal temp sensor integration.
 *
 * Copyright © 2025 HardFOC. Licensed under GPL v3.0 or later.
 */

#pragma once

#include "base/BaseTemperature.h"

/**
 * @brief STM32 Temperature sensor — stub implementation.
 */
class StmTemperature : public BaseTemperature {
public:
    StmTemperature() noexcept;
    ~StmTemperature() noexcept override;

    // Public pure virtuals
    hf_temp_err_t GetSensorInfo(hf_temp_sensor_info_t* info) const noexcept override;
    hf_u32_t GetCapabilities() const noexcept override;

protected:
    // Protected pure virtuals
    bool Initialize() noexcept override;
    bool Deinitialize() noexcept override;
    hf_temp_err_t ReadTemperatureCelsiusImpl(float* temperature_celsius) noexcept override;
};
