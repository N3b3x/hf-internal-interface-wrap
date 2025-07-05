/**
 * @file LazyInitExamples.cpp
 * @brief Example usage of the refactored GPIO and CAN systems with lazy initialization
 * @author HardFOC Development Team
 * @date 2024
 * 
 * This file demonstrates the new lazy initialization pattern and ESP32C6
 * advanced features in the refactored GPIO and CAN systems.
 */

#include "McuGpio.h"
#include "McuCan.h"
#include "McuTypes.h"

namespace HardFOC {
namespace Examples {

/**
 * @brief Example demonstrating GPIO lazy initialization
 */
void GpioLazyInitExample() {
    // Create GPIO objects - NO hardware initialization occurs here
    McuGpio led_gpio(18);      // Built-in LED pin
    McuGpio button_gpio(0);    // Boot button pin
    McuGpio sensor_gpio(4);    // Example sensor pin
    
    // First operation triggers hardware initialization automatically
    auto led_result = led_gpio.SetMode(GpioMode::GPIO_MODE_OUTPUT);
    if (led_result != HfGpioErr::HF_GPIO_OK) {
        // Handle initialization error
        return;
    }
    
    auto button_result = button_gpio.SetMode(GpioMode::GPIO_MODE_INPUT);
    if (button_result != HfGpioErr::HF_GPIO_OK) {
        // Handle initialization error
        return;
    }
    
    // Subsequent operations use already-initialized hardware (fast)
    led_gpio.WriteLevel(1);     // Turn on LED
    auto button_state = button_gpio.ReadLevel();  // Read button
    
    // Objects can be created in bulk without performance penalty
    std::vector<McuGpio> gpio_array;
    for (int i = 10; i <= 20; ++i) {
        gpio_array.emplace_back(i);  // Fast - no hardware init
    }
    
    // Hardware initialization happens only when actually used
    gpio_array[0].SetMode(GpioMode::GPIO_MODE_OUTPUT);  // Only this pin gets initialized
}

/**
 * @brief Example demonstrating ESP32C6 advanced GPIO features
 */
#ifdef HF_MCU_ESP32C6
void Esp32C6AdvancedGpioExample() {
    McuGpio advanced_gpio(5);
    
    // Check pin capabilities before use
    if (HF_GPIO_SUPPORTS_GLITCH_FILTER(5)) {
        // Configure glitch filter for noise immunity
        GlitchFilterConfig filter_config = {
            .clk_src = GPIO_GLITCH_FILTER_CLK_SRC_APB,
            .filter_ns = 1000  // 1μs filter window
        };
        advanced_gpio.SetGlitchFilter(filter_config);
    }
    
    // Check if pin supports RTC operation (low power)
    if (HF_GPIO_IS_RTC_CAPABLE(5)) {
        advanced_gpio.EnableRtcMode();
        // GPIO can now operate in deep sleep
    }
    
    // Validate strapping pin usage
    if (HF_GPIO_IS_STRAPPING_PIN(5)) {
        // Handle carefully - affects boot behavior
        // Maybe warn user or use alternative pin
    }
    
    // Check ADC capability
    if (HF_GPIO_IS_ADC_CAPABLE(5)) {
        // Pin can be used for analog input
        auto adc_channel = HF_GPIO_TO_ADC_CHANNEL(5);
        // Configure ADC on this pin
    }
}
#endif // HF_MCU_ESP32C6

/**
 * @brief Example demonstrating CAN/TWAI lazy initialization
 */
void CanLazyInitExample() {
    // Create CAN objects - NO hardware initialization
    McuCan can0(TWAI_CONTROLLER_0);
    
#ifdef HF_MCU_ESP32C6
    McuCan can1(TWAI_CONTROLLER_1);  // ESP32C6 has dual TWAI
#endif
    
    // Configure CAN timing (triggers hardware initialization)
    TwaiTimingConfig timing = {
        .brp = 8,
        .tseg_1 = 15,
        .tseg_2 = 4,
        .sjw = 3,
        .triple_sampling = false
    };
    
    TwaiFilterConfig filter = {
        .acceptance_code = 0x00000000,
        .acceptance_mask = 0xFFFFFFFF,
        .single_filter = true
    };
    
    TwaiGeneralConfig general = {
        .mode = TWAI_MODE_NORMAL,
        .tx_io = static_cast<gpio_num_t>(21),
        .rx_io = static_cast<gpio_num_t>(22),
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 10,
        .rx_queue_len = 10,
        .alerts_enabled = TWAI_ALERT_ALL,
        .clkout_divider = 0
    };
    
    // First configuration call initializes hardware
    auto result = can0.Configure(timing, filter, general);
    if (result != HfCanErr::HF_CAN_OK) {
        // Handle initialization error
        return;
    }
    
    // Start CAN operation
    auto start_result = can0.Start();
    if (start_result == HfCanErr::HF_CAN_OK) {
        // CAN bus is ready for communication
        
        // Send a message
        TwaiMessage msg = {
            .identifier = 0x123,
            .data_length_code = 8,
            .data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
        };
        can0.Transmit(msg, pdMS_TO_TICKS(100));
        
        // Receive messages
        TwaiMessage received_msg;
        if (can0.Receive(received_msg, pdMS_TO_TICKS(100)) == HfCanErr::HF_CAN_OK) {
            // Process received message
        }
    }
}

/**
 * @brief Example demonstrating performance benefits of lazy initialization
 */
void PerformanceBenefitExample() {
    // Create many GPIO objects quickly (no hardware init)
    std::vector<McuGpio> gpio_bank;
    gpio_bank.reserve(20);
    
    auto start_time = esp_timer_get_time();
    
    // Fast creation - no hardware initialization
    for (int i = 0; i < 20; ++i) {
        gpio_bank.emplace_back(i);
    }
    
    auto creation_time = esp_timer_get_time() - start_time;
    
    // Initialize only the ones actually needed
    start_time = esp_timer_get_time();
    
    // Only these pins get hardware initialization
    gpio_bank[5].SetMode(GpioMode::GPIO_MODE_OUTPUT);   // Initialize pin 5
    gpio_bank[10].SetMode(GpioMode::GPIO_MODE_INPUT);   // Initialize pin 10
    gpio_bank[15].SetMode(GpioMode::GPIO_MODE_OUTPUT);  // Initialize pin 15
    
    auto init_time = esp_timer_get_time() - start_time;
    
    // Log performance metrics
    ESP_LOGI("Performance", "Created 20 GPIO objects in %lld μs", creation_time);
    ESP_LOGI("Performance", "Initialized 3 GPIOs in %lld μs", init_time);
    ESP_LOGI("Performance", "17 GPIOs remain uninitialized (zero overhead)");
}

/**
 * @brief Example demonstrating error handling with lazy initialization
 */
void ErrorHandlingExample() {
    // Invalid pin number - error detected during first operation
    McuGpio invalid_gpio(99);  // Pin 99 doesn't exist on ESP32C6
    
    auto result = invalid_gpio.SetMode(GpioMode::GPIO_MODE_OUTPUT);
    if (result == HfGpioErr::HF_GPIO_ERR_INVALID_PIN) {
        ESP_LOGE("GPIO", "Invalid pin number detected during initialization");
        // Handle error appropriately
        return;
    }
    
    // Valid pin but invalid configuration
    McuGpio valid_gpio(18);
    
    // Try to set invalid mode
    result = valid_gpio.SetMode(static_cast<GpioMode>(999));
    if (result == HfGpioErr::HF_GPIO_ERR_INVALID_CONFIG) {
        ESP_LOGE("GPIO", "Invalid configuration detected");
        // Handle error appropriately
    }
}

} // namespace Examples
} // namespace HardFOC

extern "C" void RunLazyInitExamples() {
    using namespace HardFOC::Examples;
    GpioLazyInitExample();
#ifdef HF_MCU_ESP32C6
    Esp32C6AdvancedGpioExample();
#endif
    CanLazyInitExample();
    PerformanceBenefitExample();
    ErrorHandlingExample();
}
