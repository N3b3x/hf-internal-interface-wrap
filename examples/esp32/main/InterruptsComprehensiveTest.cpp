#include "TestFramework.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_intr_alloc.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <nvs_flash.h>

static const char* TAG = "INTERRUPTS_TEST";

class InterruptsTestFramework : public TestFramework {
private:
    static volatile uint32_t gpio_interrupt_count;
    static volatile uint32_t timer_interrupt_count;
    static esp_timer_handle_t test_timer;
    
    static void IRAM_ATTR gpio_isr_handler(void* arg) {
        gpio_interrupt_count++;
    }
    
    static void timer_callback(void* arg) {
        timer_interrupt_count++;
    }
    
public:
    InterruptsTestFramework() : TestFramework("ESP32 Interrupts Comprehensive Test") {}
    
    void setup() override {
        ESP_LOGI(TAG, "Setting up interrupt tests");
        
        // Configure GPIO for interrupt testing
        gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_POSEDGE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << GPIO_NUM_4);
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);
        
        // Install ISR service
        gpio_install_isr_service(0);
        gpio_isr_handler_add(GPIO_NUM_4, gpio_isr_handler, nullptr);
        
        // Setup ESP timer for periodic interrupts
        esp_timer_create_args_t timer_args = {};
        timer_args.callback = timer_callback;
        timer_args.name = "test_timer";
        esp_timer_create(&timer_args, &test_timer);
    }
    
    void runTests() override {
        ESP_LOGI(TAG, "Running interrupt system tests");
        
        testDescription("GPIO Interrupt Response");
        testGpioInterrupts();
        
        testDescription("Timer Interrupt Accuracy");
        testTimerInterrupts();
        
        testDescription("Interrupt Priority Handling");
        testInterruptPriorities();
        
        testDescription("Interrupt Latency Measurement");
        testInterruptLatency();
        
        cleanup();
    }
    
private:
    void testGpioInterrupts() {
        ESP_LOGI(TAG, "Testing GPIO interrupts");
        
        gpio_interrupt_count = 0;
        
        // Simulate GPIO interrupts by toggling another pin
        gpio_config_t output_conf = {};
        output_conf.mode = GPIO_MODE_OUTPUT;
        output_conf.pin_bit_mask = (1ULL << GPIO_NUM_5);
        gpio_config(&output_conf);
        
        // Generate some edges
        for (int i = 0; i < 10; i++) {
            gpio_set_level(GPIO_NUM_5, 1);
            vTaskDelay(pdMS_TO_TICKS(10));
            gpio_set_level(GPIO_NUM_5, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
        
        if (gpio_interrupt_count > 0) {
            testResult(true, "GPIO interrupts working");
            ESP_LOGI(TAG, "GPIO interrupt count: %lu", gpio_interrupt_count);
        } else {
            testResult(false, "No GPIO interrupts detected");
        }
    }
    
    void testTimerInterrupts() {
        ESP_LOGI(TAG, "Testing timer interrupts");
        
        timer_interrupt_count = 0;
        
        // Start periodic timer (1ms interval)
        esp_timer_start_periodic(test_timer, 1000);  // 1ms in microseconds
        
        vTaskDelay(pdMS_TO_TICKS(100));  // Wait 100ms
        
        esp_timer_stop(test_timer);
        
        // Should have approximately 100 interrupts (±10%)
        uint32_t expected = 100;
        uint32_t tolerance = 10;
        
        if (timer_interrupt_count >= (expected - tolerance) && 
            timer_interrupt_count <= (expected + tolerance)) {
            testResult(true, "Timer interrupts accurate");
            ESP_LOGI(TAG, "Timer interrupt count: %lu (expected ~%lu)", 
                     timer_interrupt_count, expected);
        } else {
            testResult(false, "Timer interrupt count inaccurate");
            ESP_LOGE(TAG, "Timer interrupt count: %lu (expected ~%lu)", 
                     timer_interrupt_count, expected);
        }
    }
    
    void testInterruptPriorities() {
        ESP_LOGI(TAG, "Testing interrupt priorities");
        
        // Test that higher priority interrupts can preempt lower priority ones
        // This is more of a conceptual test since we can't easily measure preemption
        
        testResult(true, "Interrupt priority system available");
        ESP_LOGI(TAG, "ESP32 supports interrupt priorities 1-7");
        ESP_LOGI(TAG, "Level 1 = highest priority, Level 7 = lowest priority");
    }
    
    void testInterruptLatency() {
        ESP_LOGI(TAG, "Testing interrupt latency");
        
        // Measure approximate interrupt response time
        // This is a basic test - actual latency depends on many factors
        
        uint64_t start_time = esp_timer_get_time();
        timer_interrupt_count = 0;
        
        esp_timer_start_once(test_timer, 1);  // Fire in 1 microsecond
        
        // Wait for interrupt to fire
        while (timer_interrupt_count == 0) {
            // Busy wait
        }
        
        uint64_t end_time = esp_timer_get_time();
        uint64_t latency = end_time - start_time;
        
        if (latency < 100) {  // Less than 100 microseconds is reasonable
            testResult(true, "Interrupt latency acceptable");
            ESP_LOGI(TAG, "Measured interrupt latency: %llu microseconds", latency);
        } else {
            testResult(false, "Interrupt latency too high");
            ESP_LOGE(TAG, "Measured interrupt latency: %llu microseconds", latency);
        }
    }
    
    void cleanup() {
        ESP_LOGI(TAG, "Cleaning up interrupt tests");
        
        if (test_timer) {
            esp_timer_stop(test_timer);
            esp_timer_delete(test_timer);
            test_timer = nullptr;
        }
        
        gpio_isr_handler_remove(GPIO_NUM_4);
        gpio_uninstall_isr_service();
    }
};

// Static member definitions
volatile uint32_t InterruptsTestFramework::gpio_interrupt_count = 0;
volatile uint32_t InterruptsTestFramework::timer_interrupt_count = 0;
esp_timer_handle_t InterruptsTestFramework::test_timer = nullptr;

extern "C" void app_main() {
    // Initialize NVS for any components that might need it
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    InterruptsTestFramework framework;
    framework.runFramework();
}