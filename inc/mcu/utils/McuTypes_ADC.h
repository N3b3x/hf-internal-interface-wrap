/**
 * @file McuTypes_ADC.h
 * @brief MCU-agnostic ADC type definitions for hardware abstraction.
 *
 * This header defines all ADC-specific types and constants that are used
 * throughout the internal interface wrap layer for ADC operations.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include "HardwareTypes.h"  // For basic hardware types
#include "BaseAdc.h"       
#include <functional>
#include <cstdint>

//==============================================================================
// PLATFORM-AGNOSTIC ADC DRIVER ENUMS AND TYPES
//==============================================================================

/**
 * @brief ADC operation statistics.
 */
struct hf_adc_statistics_t {
    uint32_t totalConversions;        ///< Total conversions performed
    uint32_t successfulConversions;   ///< Successful conversions
    uint32_t failedConversions;       ///< Failed conversions
    uint32_t averageConversionTimeUs; ///< Average conversion time (microseconds)
    uint32_t maxConversionTimeUs;     ///< Maximum conversion time
    uint32_t minConversionTimeUs;     ///< Minimum conversion time
    uint32_t calibrationCount;        ///< Number of calibrations performed
    uint32_t thresholdViolations;     ///< Threshold monitor violations
    uint32_t calibration_errors;      ///< Calibration errors

    hf_adc_statistics_t()
        : totalConversions(0), successfulConversions(0), failedConversions(0),
          averageConversionTimeUs(0), maxConversionTimeUs(0), minConversionTimeUs(UINT32_MAX),
          calibrationCount(0), thresholdViolations(0), calibration_errors(0) {}
};

/**
 * @brief ADC diagnostic information.
 */
struct hf_adc_diagnostics_t {
    bool adcHealthy;                ///< Overall ADC health status
    hf_adc_err_t lastErrorCode;     ///< Last error code
    uint32_t lastErrorTimestamp;    ///< Last error timestamp
    uint32_t consecutiveErrors;     ///< Consecutive error count
    float temperatureC;             ///< ADC temperature (if available)
    float referenceVoltage;         ///< Reference voltage
    bool calibrationValid;          ///< Calibration validity
    uint32_t enabled_channels;      ///< Bit mask of enabled channels

    hf_adc_diagnostics_t()
        : adcHealthy(true), lastErrorCode(hf_adc_err_t::ADC_SUCCESS), lastErrorTimestamp(0), 
          consecutiveErrors(0), temperatureC(25.0f), referenceVoltage(3.3f), calibrationValid(false),
          enabled_channels(0) {}
};

//==============================================================================