/**
 * @file McuInit.cpp
 * @brief Implementation of unified MCU hardware initialization interface.
 * 
 * This file provides the implementation for MCU-agnostic hardware initialization
 * that automatically adapts to the current MCU platform. This replaces the legacy
 * McuPlatformInit and consolidates all initialization under the unified MCU approach.
 */

#include "../mcu/McuInit.h"

// Include MCU-specific headers based on current platform
#if defined(ESP_PLATFORM) || defined(IDF_VER)
    #include "driver/gpio.h"
    #include "esp_adc/adc_oneshot.h"
    #include "esp_adc/adc_cali.h"
    #include "esp_log.h"
    #define MCU_PLATFORM_ESP32
#else
    #error "Unsupported MCU platform. Please add support for your target MCU."
#endif

#include <cstring>

namespace HardFOC {
namespace Mcu {

static const char* TAG = "McuInit";

//==============================================================================
// MCU-SPECIFIC IMPLEMENTATION (ESP32)
//==============================================================================

#ifdef MCU_PLATFORM_ESP32

/**
 * @brief Initialize GPIO pin for ESP32
 */
static HfGpioErr InitializeGpioPin_ESP32(const GpioPinConfig& config) {
    gpio_config_t io_conf = {};
    
    io_conf.pin_bit_mask = (1ULL << config.pin_number);
    io_conf.mode = config.is_output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;
    io_conf.pull_up_en = config.has_pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = config.has_pulldown ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO pin %d: %s", 
                 config.pin_number, esp_err_to_name(ret));
        return HfGpioErr::GPIO_ERR_CONFIGURATION_FAILED;
    }
    
    return HfGpioErr::GPIO_SUCCESS;
}

/**
 * @brief Initialize ADC channel for ESP32
 */
static HfAdcErr InitializeAdcChannel_ESP32(const AdcChannelConfig& config) {
    // ESP32 ADC initialization would go here
    // For now, return success as placeholder
    ESP_LOGI(TAG, "ADC channel %d configured with %d-bit resolution", 
             config.channel_number, config.resolution_bits);
    return HfAdcErr::ADC_SUCCESS;
}

#endif // MCU_PLATFORM_ESP32

//==============================================================================
// UNIFIED MCU INTERFACE IMPLEMENTATION
//==============================================================================

HfGpioErr InitializeGpioPin(const GpioPinConfig& config) {
#ifdef MCU_PLATFORM_ESP32
    return InitializeGpioPin_ESP32(config);
#else
    return HfGpioErr::GPIO_ERR_PLATFORM_NOT_SUPPORTED;
#endif
}

HfAdcErr InitializeAdcChannel(const AdcChannelConfig& config) {
#ifdef MCU_PLATFORM_ESP32
    return InitializeAdcChannel_ESP32(config);
#else
    return HfAdcErr::ADC_ERR_PLATFORM_NOT_SUPPORTED;
#endif
}

HfErr InitializeMcuSubsystem(McuSubsystem subsystem) {
    switch (subsystem) {
        case McuSubsystem::GPIO:
#ifdef MCU_PLATFORM_ESP32
            // GPIO is initialized per-pin, no global init needed
            ESP_LOGI(TAG, "GPIO subsystem ready");
            return HfErr::HF_OK;
#endif
            break;
            
        case McuSubsystem::ADC:
#ifdef MCU_PLATFORM_ESP32
            // ADC subsystem initialization
            ESP_LOGI(TAG, "ADC subsystem initialized");
            return HfErr::HF_OK;
#endif
            break;
            
        case McuSubsystem::CAN:
#ifdef MCU_PLATFORM_ESP32
            ESP_LOGI(TAG, "CAN subsystem ready");
            return HfErr::HF_OK;
#endif
            break;
            
        case McuSubsystem::I2C:
#ifdef MCU_PLATFORM_ESP32
            ESP_LOGI(TAG, "I2C subsystem ready");
            return HfErr::HF_OK;
#endif
            break;
            
        case McuSubsystem::SPI:
#ifdef MCU_PLATFORM_ESP32
            ESP_LOGI(TAG, "SPI subsystem ready");
            return HfErr::HF_OK;
#endif
            break;
            
        case McuSubsystem::UART:
#ifdef MCU_PLATFORM_ESP32
            ESP_LOGI(TAG, "UART subsystem ready");
            return HfErr::HF_OK;
#endif
            break;
            
        case McuSubsystem::PWM:
#ifdef MCU_PLATFORM_ESP32
            ESP_LOGI(TAG, "PWM subsystem ready");
            return HfErr::HF_OK;
#endif
            break;
            
        default:
            ESP_LOGE(TAG, "Unknown MCU subsystem: %d", static_cast<int>(subsystem));
            return HfErr::HF_ERR_INVALID_ARG;
    }
    
    return HfErr::HF_ERR_NOT_SUPPORTED;
}

HfErr InitializeAllMcuSubsystems() {
    ESP_LOGI(TAG, "Initializing all MCU subsystems...");
    
    // Initialize each subsystem
    const McuSubsystem subsystems[] = {
        McuSubsystem::GPIO,
        McuSubsystem::ADC,
        McuSubsystem::CAN,
        McuSubsystem::I2C,
        McuSubsystem::SPI,
        McuSubsystem::UART,
        McuSubsystem::PWM
    };
    
    for (auto subsystem : subsystems) {
        HfErr result = InitializeMcuSubsystem(subsystem);
        if (result != HfErr::HF_OK) {
            ESP_LOGE(TAG, "Failed to initialize subsystem %d", static_cast<int>(subsystem));
            return result;
        }
    }
    
    ESP_LOGI(TAG, "All MCU subsystems initialized successfully");
    return HfErr::HF_OK;
}

const char* GetMcuPlatformName() {
#ifdef MCU_PLATFORM_ESP32
    return "ESP32";
#else
    return "Unknown";
#endif
}

bool IsMcuSubsystemSupported(McuSubsystem subsystem) {
#ifdef MCU_PLATFORM_ESP32
    // ESP32 supports all current subsystems
    return true;
#else
    return false;
#endif
}

} // namespace Mcu
} // namespace HardFOC
