#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char* TAG = "SimpleMain";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "=== ESP32-C6 Simple Test Start ===");
    
    // Configure GPIO for LED
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_8);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "GPIO configured successfully");
    
    int count = 0;
    while (true) {
        ESP_LOGI(TAG, "Blink count: %d", count++);
        
        // Toggle LED
        gpio_set_level(GPIO_NUM_8, count % 2);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
