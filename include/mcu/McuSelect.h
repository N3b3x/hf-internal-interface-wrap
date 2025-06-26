/**
 * @file McuSelect.h
 * @brief MCU platform selection and configuration header.
 * 
 * This header handles MCU platform detection and configuration,
 * providing unified access to MCU-specific implementations across
 * different hardware platforms.
 */

#ifndef MCU_SELECT_H
#define MCU_SELECT_H

//==============================================================================
// MCU PLATFORM DETECTION
//==============================================================================

//==============================================================================
// MCU PLATFORM DETECTION AND CONFIGURATION
//==============================================================================

// ESP32-C6 Platform Detection (Primary Target)
#if defined(CONFIG_IDF_TARGET_ESP32C6) || defined(ESP32C6)
    #define HF_MCU_ESP32C6
    #define HF_MCU_FAMILY_ESP32
    #define HF_MCU_NAME "ESP32-C6"
    #define HF_MCU_ARCHITECTURE "RISC-V RV32IMAC"
    #define HF_MCU_VARIANT_C6
    
    // ESP32-C6 specific includes
    #ifdef __cplusplus
    extern "C" {
    #endif
    
    #include "driver/gpio.h"
    #include "driver/adc.h"
    #include "driver/i2c.h"
    #include "driver/spi_master.h"
    #include "driver/uart.h"
    #include "driver/twai.h"
    #include "driver/ledc.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/semphr.h"
    
    #ifdef __cplusplus
    }
    #endif

// ESP32 Classic Platform Detection (Secondary Support)
#elif defined(CONFIG_IDF_TARGET_ESP32) || defined(ESP_PLATFORM) || defined(CONFIG_IDF_TARGET)
    #define HF_MCU_ESP32
    #define HF_MCU_FAMILY_ESP32
    #define HF_MCU_NAME "ESP32"
    #define HF_MCU_ARCHITECTURE "Xtensa LX6"
    #define HF_MCU_VARIANT_CLASSIC
    
    // ESP32 Classic specific includes
    #ifdef __cplusplus
    extern "C" {
    #endif
    
    #include "driver/gpio.h"
    #include "driver/adc.h"
    #include "driver/i2c.h"
    #include "driver/spi_master.h"
    #include "driver/uart.h"
    #include "driver/can.h"  // Note: Different from ESP32-C6 TWAI
    #include "driver/ledc.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "freertos/semphr.h"
    
    #ifdef __cplusplus
    }
    #endif

// STM32 Platform Detection (for future expansion)
#elif defined(STM32) || defined(STM32F4) || defined(STM32H7)
    #define HF_MCU_STM32
    #define HF_MCU_NAME "STM32"
    #define HF_MCU_ARCHITECTURE "ARM Cortex-M"
    #error "STM32 platform not yet implemented"

// RP2040 Platform Detection (for future expansion)
#elif defined(PICO_PLATFORM) || defined(PICO_SDK)
    #define HF_MCU_RP2040
    #define HF_MCU_NAME "RP2040"
    #define HF_MCU_ARCHITECTURE "ARM Cortex-M0+"
    #error "RP2040 platform not yet implemented"

// Unknown MCU - Default to ESP32-C6 with warning
#else
    #warning "Unknown MCU platform - defaulting to ESP32-C6"
    #define HF_MCU_ESP32C6
    #define HF_MCU_FAMILY_ESP32
    #define HF_MCU_NAME "ESP32-C6"
    #define HF_MCU_ARCHITECTURE "RISC-V RV32IMAC"
    #define HF_MCU_VARIANT_C6
#endif

//==============================================================================
// MCU CAPABILITY DEFINITIONS
//==============================================================================

// ESP32-C6 Specific Capabilities (Primary Target)
#ifdef HF_MCU_ESP32C6
    // GPIO capabilities
    #define HF_MCU_HAS_GPIO                 1
    #define HF_MCU_GPIO_MAX_PINS            31      // ESP32-C6 has 31 GPIO pins (0-30)
    #define HF_MCU_GPIO_HAS_PULLUP          1
    #define HF_MCU_GPIO_HAS_PULLDOWN        1
    #define HF_MCU_GPIO_HAS_INTERRUPTS      1
    
    // ADC capabilities (ESP32-C6 specific)
    #define HF_MCU_HAS_ADC                  1
    #define HF_MCU_ADC_MAX_CHANNELS         7       // ESP32-C6 has 7 ADC channels
    #define HF_MCU_ADC_MAX_RESOLUTION       12
    #define HF_MCU_ADC_HAS_ATTENUATION      1
    #define HF_MCU_ADC_NUM_UNITS            1       // ESP32-C6 has only ADC1
    
    // I2C capabilities
    #define HF_MCU_HAS_I2C                  1
    #define HF_MCU_I2C_MAX_PORTS            1       // ESP32-C6 has 1 I2C port
    #define HF_MCU_I2C_MAX_FREQ_HZ          1000000
    #define HF_MCU_I2C_HAS_SLAVE_MODE       1
    
    // SPI capabilities
    #define HF_MCU_HAS_SPI                  1
    #define HF_MCU_SPI_MAX_HOSTS            2       // ESP32-C6 has SPI2 and SPI3
    #define HF_MCU_SPI_MAX_FREQ_HZ          60000000
    #define HF_MCU_SPI_HAS_DMA              1
    
    // UART capabilities
    #define HF_MCU_HAS_UART                 1
    #define HF_MCU_UART_MAX_PORTS           2       // ESP32-C6 has UART0 and UART1
    #define HF_MCU_UART_MAX_BAUDRATE        5000000
    #define HF_MCU_UART_HAS_FLOW_CONTROL    1
    
    // CAN capabilities (TWAI)
    #define HF_MCU_HAS_CAN                  1
    #define HF_MCU_CAN_MAX_CONTROLLERS      1
    #define HF_MCU_CAN_HAS_LISTEN_ONLY      1
    #define HF_MCU_CAN_HAS_SELF_TEST        1
    #define HF_MCU_CAN_PROTOCOL             "TWAI"
    
    // PWM capabilities (LEDC)
    #define HF_MCU_HAS_PWM                  1
    #define HF_MCU_PWM_MAX_CHANNELS         8       // ESP32-C6 has 8 LEDC channels
    #define HF_MCU_PWM_MAX_FREQ_HZ          40000000
    #define HF_MCU_PWM_MAX_RESOLUTION       14      // ESP32-C6 max is 14-bit
    
    // RMT capabilities (not PIO)
    #define HF_MCU_HAS_PIO                  0
    #define HF_MCU_HAS_RMT                  1
    #define HF_MCU_RMT_MAX_CHANNELS         4       // ESP32-C6 has 4 RMT channels

// ESP32 Classic Capabilities (Secondary Support)
#elif defined(HF_MCU_ESP32)
    // GPIO capabilities
    #define HF_MCU_HAS_GPIO                 1
    #define HF_MCU_GPIO_MAX_PINS            40      // ESP32 has 40 GPIO pins
    #define HF_MCU_GPIO_HAS_PULLUP          1
    #define HF_MCU_GPIO_HAS_PULLDOWN        1
    #define HF_MCU_GPIO_HAS_INTERRUPTS      1
    
    // ADC capabilities
    #define HF_MCU_HAS_ADC                  1
    #define HF_MCU_ADC_MAX_CHANNELS         18
    #define HF_MCU_ADC_MAX_RESOLUTION       12
    #define HF_MCU_ADC_HAS_ATTENUATION      1
    #define HF_MCU_ADC_NUM_UNITS            2
    
    // I2C capabilities
    #define HF_MCU_HAS_I2C                  1
    #define HF_MCU_I2C_MAX_PORTS            2
    #define HF_MCU_I2C_MAX_FREQ_HZ          1000000
    #define HF_MCU_I2C_HAS_SLAVE_MODE       1
    
    // SPI capabilities
    #define HF_MCU_HAS_SPI                  1
    #define HF_MCU_SPI_MAX_HOSTS            3
    #define HF_MCU_SPI_MAX_FREQ_HZ          80000000
    #define HF_MCU_SPI_HAS_DMA              1
    
    // UART capabilities
    #define HF_MCU_HAS_UART                 1
    #define HF_MCU_UART_MAX_PORTS           3
    #define HF_MCU_UART_MAX_BAUDRATE        5000000
    #define HF_MCU_UART_HAS_FLOW_CONTROL    1
    
    // CAN capabilities
    #define HF_MCU_HAS_CAN                  1
    #define HF_MCU_CAN_MAX_CONTROLLERS      1
    #define HF_MCU_CAN_HAS_LISTEN_ONLY      1
    #define HF_MCU_CAN_HAS_SELF_TEST        1
    #define HF_MCU_CAN_PROTOCOL             "CAN"
    
    // PWM capabilities
    #define HF_MCU_HAS_PWM                  1
    #define HF_MCU_PWM_MAX_CHANNELS         16
    #define HF_MCU_PWM_MAX_FREQ_HZ          40000000
    #define HF_MCU_PWM_MAX_RESOLUTION       20
    
    // RMT capabilities
    #define HF_MCU_HAS_PIO                  0
    #define HF_MCU_HAS_RMT                  1
    #define HF_MCU_RMT_MAX_CHANNELS         8

#endif

//==============================================================================
// MCU TYPE MAPPING
//==============================================================================

// ESP32-C6 Type Mappings
#ifdef HF_MCU_ESP32C6
    // GPIO type mappings
    #define HF_MCU_GPIO_NUM_TYPE            gpio_num_t
    #define HF_MCU_GPIO_MODE_TYPE           gpio_mode_t
    #define HF_MCU_GPIO_PULL_TYPE           gpio_pull_mode_t
    #define HF_MCU_GPIO_INTR_TYPE           gpio_int_type_t
    #define HF_MCU_GPIO_INVALID             GPIO_NUM_NC
    
    // ADC type mappings
    #define HF_MCU_ADC_CHANNEL_TYPE         adc_channel_t
    #define HF_MCU_ADC_UNIT_TYPE            adc_unit_t
    #define HF_MCU_ADC_ATTEN_TYPE           adc_atten_t
    #define HF_MCU_ADC_BITS_TYPE            adc_bits_width_t
    
    // I2C type mappings
    #define HF_MCU_I2C_PORT_TYPE            i2c_port_t
    #define HF_MCU_I2C_MODE_TYPE            i2c_mode_t
    
    // SPI type mappings
    #define HF_MCU_SPI_HOST_TYPE            spi_host_device_t
    
    // UART type mappings
    #define HF_MCU_UART_PORT_TYPE           uart_port_t
    
    // CAN type mappings (TWAI for ESP32-C6)
    #define HF_MCU_CAN_MODE_TYPE            twai_mode_t
    #define HF_MCU_CAN_TIMING_TYPE          twai_timing_config_t
    #define HF_MCU_CAN_FILTER_TYPE          twai_filter_config_t
    #define HF_MCU_CAN_MSG_TYPE             twai_message_t
    
    // PWM type mappings
    #define HF_MCU_PWM_CHANNEL_TYPE         ledc_channel_t
    #define HF_MCU_PWM_TIMER_TYPE           ledc_timer_t
    #define HF_MCU_PWM_MODE_TYPE            ledc_mode_t

// ESP32 Classic Type Mappings
#elif defined(HF_MCU_ESP32)
    // GPIO type mappings
    #define HF_MCU_GPIO_NUM_TYPE            gpio_num_t
    #define HF_MCU_GPIO_MODE_TYPE           gpio_mode_t
    #define HF_MCU_GPIO_PULL_TYPE           gpio_pull_mode_t
    #define HF_MCU_GPIO_INTR_TYPE           gpio_int_type_t
    #define HF_MCU_GPIO_INVALID             GPIO_NUM_NC
    
    // ADC type mappings
    #define HF_MCU_ADC_CHANNEL_TYPE         adc_channel_t
    #define HF_MCU_ADC_UNIT_TYPE            adc_unit_t
    #define HF_MCU_ADC_ATTEN_TYPE           adc_atten_t
    #define HF_MCU_ADC_BITS_TYPE            adc_bits_width_t
    
    // I2C type mappings
    #define HF_MCU_I2C_PORT_TYPE            i2c_port_t
    #define HF_MCU_I2C_MODE_TYPE            i2c_mode_t
    
    // SPI type mappings
    #define HF_MCU_SPI_HOST_TYPE            spi_host_device_t
    
    // UART type mappings
    #define HF_MCU_UART_PORT_TYPE           uart_port_t
    
    // CAN type mappings (Classic CAN for ESP32)
    #define HF_MCU_CAN_MODE_TYPE            can_mode_t
    #define HF_MCU_CAN_TIMING_TYPE          can_timing_config_t
    #define HF_MCU_CAN_FILTER_TYPE          can_filter_config_t
    #define HF_MCU_CAN_MSG_TYPE             can_message_t
    
    // PWM type mappings
    #define HF_MCU_PWM_CHANNEL_TYPE         ledc_channel_t
    #define HF_MCU_PWM_TIMER_TYPE           ledc_timer_t
    #define HF_MCU_PWM_MODE_TYPE            ledc_mode_t
#endif

//==============================================================================
// MCU CONFIGURATION CONSTANTS
//==============================================================================

// ESP32-C6 Configuration Constants
#ifdef HF_MCU_ESP32C6
    // Default timeout values
    #define HF_MCU_DEFAULT_TIMEOUT_MS       1000
    #define HF_MCU_I2C_TIMEOUT_MS           500     // Reduced for faster response
    #define HF_MCU_SPI_TIMEOUT_MS           1000
    #define HF_MCU_UART_TIMEOUT_MS          1000
    #define HF_MCU_CAN_TIMEOUT_MS           500
    
    // Buffer sizes (optimized for ESP32-C6)
    #define HF_MCU_UART_RX_BUFFER_SIZE      512     // Increased for motor control
    #define HF_MCU_UART_TX_BUFFER_SIZE      256
    #define HF_MCU_I2C_BUFFER_SIZE          64      // Reduced for ESP32-C6
    #define HF_MCU_SPI_BUFFER_SIZE          256
    #define HF_MCU_CAN_RX_QUEUE_SIZE        16      // Optimized for ESP32-C6
    #define HF_MCU_CAN_TX_QUEUE_SIZE        16
    
    // Stack sizes for tasks (optimized for RISC-V)
    #define HF_MCU_TASK_STACK_SIZE          3072    // Reduced for RISC-V efficiency
    #define HF_MCU_TASK_PRIORITY            5
    
    // ADC specific constants
    #define HF_MCU_ADC_DEFAULT_VREF         1100    // mV, ESP32-C6 default
    #define HF_MCU_ADC_MAX_VOLTAGE          3300    // mV, with 11dB attenuation

// ESP32 Classic Configuration Constants
#elif defined(HF_MCU_ESP32)
    // Default timeout values
    #define HF_MCU_DEFAULT_TIMEOUT_MS       1000
    #define HF_MCU_I2C_TIMEOUT_MS           1000
    #define HF_MCU_SPI_TIMEOUT_MS           1000
    #define HF_MCU_UART_TIMEOUT_MS          1000
    #define HF_MCU_CAN_TIMEOUT_MS           1000
    
    // Buffer sizes
    #define HF_MCU_UART_RX_BUFFER_SIZE      256
    #define HF_MCU_UART_TX_BUFFER_SIZE      256
    #define HF_MCU_I2C_BUFFER_SIZE          128
    #define HF_MCU_SPI_BUFFER_SIZE          256
    #define HF_MCU_CAN_RX_QUEUE_SIZE        32
    #define HF_MCU_CAN_TX_QUEUE_SIZE        32
    
    // Stack sizes for tasks
    #define HF_MCU_TASK_STACK_SIZE          4096
    #define HF_MCU_TASK_PRIORITY            5
    
    // ADC specific constants
    #define HF_MCU_ADC_DEFAULT_VREF         1100    // mV
    #define HF_MCU_ADC_MAX_VOLTAGE          3900    // mV, with 11dB attenuation
#endif

#endif // MCU_SELECT_H
