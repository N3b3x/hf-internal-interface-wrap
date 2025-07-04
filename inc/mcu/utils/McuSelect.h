/**
 * @file McuSelect.h
 * @brief Centralized MCU platform selection and configuration header.
 *
 * This header provides a SINGLE POINT OF CONTROL for MCU platform selection.
 * To select your target MCU, uncomment exactly ONE of the MCU selection defines below.
 * All platform-specific configuration is automatically handled based on your selection.
 * This system enables compile-time platform detection and feature configuration.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

//==============================================;================================
// USAGE INSTRUCTIONS - HOW TO SELECT YOUR MCU
//==============================================================================
/*
 * STEP 1: Select your target MCU by uncommenting exactly ONE define below
 * STEP 2: Comment out all other MCU defines (they should have // in front)
 * STEP 3: Build your project - all platform-specific code will be configured automatically
 *
 * EXAMPLES:
 * - For ESP32-C6:  uncomment "#define HF_TARGET_MCU_ESP32C6"
 * - For ESP32:      uncomment "#define HF_TARGET_MCU_ESP32"
 * - For STM32F4:    uncomment "#define HF_TARGET_MCU_STM32F4"
 *
 * NOTE: If you get "Multiple target MCUs" error, you have more than one uncommented.
 *       If you get "No target MCU" error, you need to uncomment exactly one.
 */

//==============================================================================
// CENTRAL MCU SELECTION - SELECT EXACTLY ONE MCU
//==============================================================================
// Uncomment exactly ONE of the following lines to select your target MCU:

#define HF_TARGET_MCU_ESP32C6 // ESP32-C6 RISC-V MCU (Primary target)
// #define HF_TARGET_MCU_ESP32       // ESP32 Classic Xtensa MCU
// #define HF_TARGET_MCU_STM32F4     // STM32F4 series ARM Cortex-M4
// #define HF_TARGET_MCU_STM32H7     // STM32H7 series ARM Cortex-M7

// #define HF_TARGET_MCU_RP2040      // Raspberry Pi Pico RP2040

// Optional thread safety support using RTOS-based mutexes
// Uncomment to enable mutex protection in MCU drivers
// #define HF_THREAD_SAFE

//==============================================================================
// AUTOMATIC PLATFORM CONFIGURATION BASED ON SELECTION
//==============================================================================

// ESP32-C6 Configuration
#ifdef HF_TARGET_MCU_ESP32C6

#define HF_MCU_ESP32C6
#define HF_MCU_FAMILY_ESP32
#define HF_MCU_NAME "ESP32-C6"
#define HF_MCU_ARCHITECTURE "RISC-V RV32IMAC"
#define HF_MCU_VARIANT_C6

// ESP32-C6 specific includes
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "driver/esp_twai.h"
#include "driver/esp_twai_onchip.h" // ESP-IDF v5.5+ modern TWAI node API
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

// ESP32 Classic Configuration
#elif defined(HF_TARGET_MCU_ESP32)
#define HF_MCU_ESP32
#define HF_MCU_FAMILY_ESP32
#define HF_MCU_NAME "ESP32"
#define HF_MCU_ARCHITECTURE "Xtensa LX6"
#define HF_MCU_VARIANT_CLASSIC

// ESP32 Classic specific includes
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/adc.h"
#include "driver/can.h" // Note: Different from ESP32-C6 TWAI
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifdef __cplusplus
}
#endif

// STM32F4 Configuration
#elif defined(HF_TARGET_MCU_STM32F4)
#define HF_MCU_STM32F4
#define HF_MCU_FAMILY_STM32
#define HF_MCU_NAME "STM32F4"
#define HF_MCU_ARCHITECTURE "ARM Cortex-M4"
#error "STM32F4 platform not yet implemented - please implement STM32F4 support"

// STM32H7 Configuration
#elif defined(HF_TARGET_MCU_STM32H7)
#define HF_MCU_STM32H7
#define HF_MCU_FAMILY_STM32
#define HF_MCU_NAME "STM32H7"
#define HF_MCU_ARCHITECTURE "ARM Cortex-M7"
#error "STM32H7 platform not yet implemented - please implement STM32H7 support"

// RP2040 Configuration
#elif defined(HF_TARGET_MCU_RP2040)
#define HF_MCU_RP2040
#define HF_MCU_FAMILY_RP2040
#define HF_MCU_NAME "RP2040"
#define HF_MCU_ARCHITECTURE "ARM Cortex-M0+"
#error "RP2040 platform not yet implemented - please implement RP2040 support"

// No MCU Selected - Error
#else
#error "No MCU selected! Please uncomment exactly one HF_TARGET_MCU_* define in McuSelect.h"
#endif

//==============================================================================
// PLATFORM VALIDATION - ENSURE ONLY ONE TARGET IS SELECTED
//==============================================================================

// Count the number of selected target MCUs
#define HF_TARGET_COUNT                                                                            \
  (defined(HF_TARGET_MCU_ESP32C6) + defined(HF_TARGET_MCU_ESP32) +                                 \
   defined(HF_TARGET_MCU_STM32F4) + defined(HF_TARGET_MCU_STM32H7) +                               \
   defined(HF_TARGET_MCU_RP2040))

#if HF_TARGET_COUNT > 1
#error                                                                                             \
    "Multiple target MCUs are selected. Please uncomment exactly ONE HF_TARGET_MCU_* define in McuSelect.h"
#elif HF_TARGET_COUNT == 0
#error                                                                                             \
    "No target MCU is selected. Please uncomment exactly ONE HF_TARGET_MCU_* define in McuSelect.h"
#endif

// Validate that the selected MCU has a corresponding platform family defined
#if !defined(HF_MCU_FAMILY_ESP32) && !defined(HF_MCU_FAMILY_STM32) && !defined(HF_MCU_FAMILY_RP2040)
#error "No MCU family is defined. This indicates an error in McuSelect.h configuration."
#endif

//==============================================================================
// MCU CAPABILITY DEFINITIONS
//==============================================================================

// ESP32-C6 Specific Capabilities (Primary Target)
#ifdef HF_MCU_ESP32C6
// GPIO capabilities
#define HF_MCU_HAS_GPIO 1
#define HF_MCU_GPIO_MAX_PINS 31 // ESP32-C6 has 31 GPIO pins (0-30)
#define HF_MCU_GPIO_HAS_PULLUP 1
#define HF_MCU_GPIO_HAS_PULLDOWN 1
#define HF_MCU_GPIO_HAS_INTERRUPTS 1

// ESP32-C6 Advanced GPIO Hardware Constants (ESP-IDF v5.5+)
#define HF_MCU_GPIO_PIN_COUNT 31                ///< Total GPIO pins (GPIO0-GPIO30)
#define HF_MCU_GPIO_MAX_PIN_NUMBER 30           ///< Maximum valid GPIO pin number
#define HF_MCU_GPIO_RTC_PIN_COUNT 8             ///< RTC GPIO pins (GPIO0-GPIO7)
#define HF_MCU_GPIO_LP_IO_PIN_COUNT 8           ///< LP_IO pins (LP_GPIO0-LP_GPIO7)
#define HF_MCU_GPIO_ADC_PIN_COUNT 7             ///< ADC capable pins (GPIO0-GPIO6)
#define HF_MCU_GPIO_FLEX_FILTER_COUNT 8         ///< Number of flexible glitch filters
#define HF_MCU_GPIO_DEDIC_BUNDLE_MAX_SIZE 8     ///< Maximum GPIOs per dedicated bundle
#define HF_MCU_GPIO_TOUCH_PIN_COUNT 0           ///< ESP32C6 has no touch pins
#define HF_MCU_GPIO_DAC_PIN_COUNT 0             ///< ESP32C6 has no DAC pins
#define HF_MCU_GPIO_ETM_CHANNEL_COUNT 50        ///< Number of ETM channels available
#define HF_MCU_GPIO_ETM_EVENT_COUNT_MAX 100     ///< Maximum ETM events
#define HF_MCU_GPIO_ETM_TASK_COUNT_MAX 100      ///< Maximum ETM tasks

// ESP32-C6 Clock frequencies for glitch filter timing calculations
#define HF_MCU_GPIO_APB_CLK_FREQ_HZ 80000000UL       ///< APB clock frequency (80MHz)
#define HF_MCU_GPIO_RC_FAST_CLK_FREQ_HZ 17500000UL   ///< RC_FAST clock frequency (~17.5MHz)
#define HF_MCU_GPIO_XTAL_CLK_FREQ_HZ 40000000UL      ///< XTAL clock frequency (40MHz)

// ESP32-C6 GPIO timing limits and characteristics
#define HF_MCU_GPIO_MAX_TOGGLE_FREQ_HZ 40000000UL    ///< Maximum GPIO toggle frequency (40MHz)
#define HF_MCU_GPIO_MIN_GLITCH_FILTER_NS 25UL        ///< Minimum glitch filter duration (25ns)
#define HF_MCU_GPIO_MAX_GLITCH_FILTER_NS 1000000UL   ///< Maximum glitch filter duration (1ms)

// ESP32-C6 GPIO Feature Support Flags
#define HF_MCU_GPIO_HAS_GLITCH_FILTER 1         ///< Hardware glitch filtering support
#define HF_MCU_GPIO_HAS_DEDICATED_GPIO 1        ///< Dedicated GPIO bundle support
#define HF_MCU_GPIO_HAS_RTC_GPIO 1              ///< RTC GPIO domain support
#define HF_MCU_GPIO_HAS_LP_IO 1                 ///< Low-power IO domain support
#define HF_MCU_GPIO_HAS_ETM 1                   ///< Event Task Matrix support
#define HF_MCU_GPIO_HAS_HOLD_FUNCTION 1         ///< GPIO hold function support
#define HF_MCU_GPIO_HAS_SLEEP_CONFIG 1          ///< Sleep mode configuration support
#define HF_MCU_GPIO_HAS_WAKEUP_CONFIG 1         ///< Wake-up source configuration support
#define HF_MCU_GPIO_HAS_DRIVE_STRENGTH 1        ///< Drive strength control support

// ADC capabilities (ESP32-C6 specific)
#define HF_MCU_HAS_ADC 1
#define HF_MCU_ADC_MAX_CHANNELS 7 // ESP32-C6 has 7 ADC channels (0-6)
#define HF_MCU_ADC_MAX_RESOLUTION 12
#define HF_MCU_ADC_HAS_ATTENUATION 1
#define HF_MCU_ADC_NUM_UNITS 1 // ESP32-C6 has only ADC1

// I2C capabilities
#define HF_MCU_HAS_I2C 1
#define HF_MCU_I2C_MAX_PORTS 1 // ESP32-C6 has 1 I2C port
#define HF_MCU_I2C_MAX_FREQ_HZ 1000000
#define HF_MCU_I2C_HAS_SLAVE_MODE 1

// SPI capabilities
#define HF_MCU_HAS_SPI 1
#define HF_MCU_SPI_MAX_HOSTS 2 // ESP32-C6 has SPI2 and SPI3
#define HF_MCU_SPI_MAX_FREQ_HZ 60000000
#define HF_MCU_SPI_HAS_DMA 1

// UART capabilities
#define HF_MCU_HAS_UART 1
#define HF_MCU_UART_MAX_PORTS 2 // ESP32-C6 has UART0 and UART1
#define HF_MCU_UART_MAX_BAUDRATE 5000000
#define HF_MCU_UART_HAS_FLOW_CONTROL 1

// CAN capabilities (TWAI)
#define HF_MCU_HAS_CAN 1
#define HF_MCU_CAN_MAX_CONTROLLERS 2
#define HF_MCU_CAN_HAS_LISTEN_ONLY 1
#define HF_MCU_CAN_HAS_SELF_TEST 1
#define HF_MCU_CAN_PROTOCOL "TWAI"

// PWM capabilities (LEDC)
#define HF_MCU_HAS_PWM 1
#define HF_MCU_PWM_MAX_CHANNELS 6 // ESP32-C6 has 6 LEDC channels
#define HF_MCU_PWM_MAX_FREQ_HZ 40000000
#define HF_MCU_PWM_MAX_RESOLUTION 14 // ESP32-C6 max is 14-bit

// RMT capabilities (not PIO)
#define HF_MCU_HAS_PIO 0
#define HF_MCU_HAS_RMT 1
#define HF_MCU_RMT_MAX_CHANNELS 4 // ESP32-C6 has 4 RMT channels

// ESP32 Classic Capabilities (Secondary Support)
#elif defined(HF_MCU_ESP32)
// GPIO capabilities
#define HF_MCU_HAS_GPIO 1
#define HF_MCU_GPIO_MAX_PINS 40 // ESP32 has 40 GPIO pins
#define HF_MCU_GPIO_HAS_PULLUP 1
#define HF_MCU_GPIO_HAS_PULLDOWN 1
#define HF_MCU_GPIO_HAS_INTERRUPTS 1

// ADC capabilities
#define HF_MCU_HAS_ADC 1
#define HF_MCU_ADC_MAX_CHANNELS 18
#define HF_MCU_ADC_MAX_RESOLUTION 12
#define HF_MCU_ADC_HAS_ATTENUATION 1
#define HF_MCU_ADC_NUM_UNITS 2

// I2C capabilities
#define HF_MCU_HAS_I2C 1
#define HF_MCU_I2C_MAX_PORTS 2
#define HF_MCU_I2C_MAX_FREQ_HZ 1000000
#define HF_MCU_I2C_HAS_SLAVE_MODE 1

// SPI capabilities
#define HF_MCU_HAS_SPI 1
#define HF_MCU_SPI_MAX_HOSTS 3
#define HF_MCU_SPI_MAX_FREQ_HZ 80000000
#define HF_MCU_SPI_HAS_DMA 1

// UART capabilities
#define HF_MCU_HAS_UART 1
#define HF_MCU_UART_MAX_PORTS 3
#define HF_MCU_UART_MAX_BAUDRATE 5000000
#define HF_MCU_UART_HAS_FLOW_CONTROL 1

// CAN capabilities
#define HF_MCU_HAS_CAN 1
#define HF_MCU_CAN_MAX_CONTROLLERS 1
#define HF_MCU_CAN_HAS_LISTEN_ONLY 1
#define HF_MCU_CAN_HAS_SELF_TEST 1
#define HF_MCU_CAN_PROTOCOL "CAN"

// PWM capabilities
#define HF_MCU_HAS_PWM 1
#define HF_MCU_PWM_MAX_CHANNELS 16
#define HF_MCU_PWM_MAX_FREQ_HZ 40000000
#define HF_MCU_PWM_MAX_RESOLUTION 20

// RMT capabilities
#define HF_MCU_HAS_PIO 0
#define HF_MCU_HAS_RMT 1
#define HF_MCU_RMT_MAX_CHANNELS 8

#endif

//==============================================================================
// MCU CONFIGURATION CONSTANTS
//==============================================================================

// ESP32-C6 Configuration Constants
#ifdef HF_MCU_ESP32C6
// Default timeout values
#define HF_MCU_DEFAULT_TIMEOUT_MS 1000
#define HF_MCU_I2C_TIMEOUT_MS 500 // Reduced for faster response
#define HF_MCU_SPI_TIMEOUT_MS 1000
#define HF_MCU_UART_TIMEOUT_MS 1000
#define HF_MCU_CAN_TIMEOUT_MS 500

// Buffer sizes (optimized for ESP32-C6)
#define HF_MCU_UART_RX_BUFFER_SIZE 512 // Increased for motor control
#define HF_MCU_UART_TX_BUFFER_SIZE 256
#define HF_MCU_I2C_BUFFER_SIZE 64 // Reduced for ESP32-C6
#define HF_MCU_SPI_BUFFER_SIZE 256
#define HF_MCU_CAN_RX_QUEUE_SIZE 16 // Optimized for ESP32-C6
#define HF_MCU_CAN_TX_QUEUE_SIZE 16

// Stack sizes for tasks (optimized for RISC-V)
#define HF_MCU_TASK_STACK_SIZE 3072 // Reduced for RISC-V efficiency
#define HF_MCU_TASK_PRIORITY 5

// ADC specific constants
#define HF_MCU_ADC_DEFAULT_VREF 1100 // mV, ESP32-C6 default
#define HF_MCU_ADC_MAX_VOLTAGE 3300  // mV, with 11dB attenuation

// ESP32 Classic Configuration Constants
#elif defined(HF_MCU_ESP32)
// Default timeout values
#define HF_MCU_DEFAULT_TIMEOUT_MS 1000
#define HF_MCU_I2C_TIMEOUT_MS 1000
#define HF_MCU_SPI_TIMEOUT_MS 1000
#define HF_MCU_UART_TIMEOUT_MS 1000
#define HF_MCU_CAN_TIMEOUT_MS 1000

// Buffer sizes
#define HF_MCU_UART_RX_BUFFER_SIZE 256
#define HF_MCU_UART_TX_BUFFER_SIZE 256
#define HF_MCU_I2C_BUFFER_SIZE 128
#define HF_MCU_SPI_BUFFER_SIZE 256
#define HF_MCU_CAN_RX_QUEUE_SIZE 32
#define HF_MCU_CAN_TX_QUEUE_SIZE 32

// Stack sizes for tasks
#define HF_MCU_TASK_STACK_SIZE 4096
#define HF_MCU_TASK_PRIORITY 5

// ADC specific constants
#define HF_MCU_ADC_DEFAULT_VREF 1100 // mV
#define HF_MCU_ADC_MAX_VOLTAGE 3900  // mV, with 11dB attenuation
#endif

//==============================================================================
// END OF MCU SELECT HEADER
//==============================================================================

