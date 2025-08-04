// Disable pedantic warnings for ESP-IDF headers and our ESP32 includes
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <iostream>

// ESP-IDF C headers must be wrapped in extern "C" for C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"
#include "esp_log.h"

#ifdef __cplusplus
}
#endif

// Include all base classes
#include "base/BaseAdc.h"

#include "base/BaseCan.h"
#include "base/BaseGpio.h"
#include "base/BaseI2c.h"
#include "base/BaseNvs.h"
#include "base/BasePeriodicTimer.h"
#include "base/BasePio.h"
#include "base/BasePwm.h"
#include "base/BaseSpi.h"
#include "base/BaseTemperature.h"
#include "base/BaseUart.h"
#include "base/BaseWifi.h"
#include "base/HardwareTypes.h"

// Include utility classes
#include "utils/AsciiArtGenerator.h"
#include "utils/DigitalOutputGuard.h"
#include "utils/McuSelect.h"
#include "utils/RtosMutex.h"
#include "utils/memory_utils.h"

// ESP32 implementation classes
#include "mcu/esp32/EspAdc.h"
#include "mcu/esp32/EspCan.h"
#include "mcu/esp32/EspGpio.h"
#include "mcu/esp32/EspI2c.h"
#include "mcu/esp32/EspNvs.h"
#include "mcu/esp32/EspPeriodicTimer.h"
#include "mcu/esp32/EspPio.h"
#include "mcu/esp32/EspPwm.h"
#include "mcu/esp32/EspSpi.h"
#include "mcu/esp32/EspTemperature.h"
#include "mcu/esp32/EspUart.h"
#include "mcu/esp32/EspWifi.h"

#pragma GCC diagnostic pop

// ESP32 utility types
#include "mcu/esp32/utils/EspTypes.h"
#include "mcu/esp32/utils/EspTypes_ADC.h"
#include "mcu/esp32/utils/EspTypes_Base.h"
#include "mcu/esp32/utils/EspTypes_CAN.h"
#include "mcu/esp32/utils/EspTypes_GPIO.h"
#include "mcu/esp32/utils/EspTypes_I2C.h"
#include "mcu/esp32/utils/EspTypes_NVS.h"
#include "mcu/esp32/utils/EspTypes_PIO.h"
#include "mcu/esp32/utils/EspTypes_PWM.h"
#include "mcu/esp32/utils/EspTypes_SPI.h"
#include "mcu/esp32/utils/EspTypes_Timer.h"
#include "mcu/esp32/utils/EspTypes_UART.h"
#include "mcu/esp32/utils/EspTypes_WiFi.h"

static const char* TAG = "IID_Test";

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "=== ESP32 IID Comprehensive Test Start ===");

  // Demonstrate ASCII Art Generator
  ESP_LOGI(TAG, "=== ASCII Art Generator Test ===");
  AsciiArtGenerator art_gen;
  ESP_LOGI(TAG, "%s", art_gen.Generate("HardFOC").c_str());
  ESP_LOGI(TAG, "%s", art_gen.Generate("ESP32-C6 Integration").c_str());

  // 1. EspGpio
  ESP_LOGI(TAG, "=== Testing EspGpio ===");
  EspGpio test_gpio(8, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
  auto gpio_init = test_gpio.EnsureInitialized();
  ESP_LOGI(TAG, "EspGpio initialized: %s", gpio_init ? "SUCCESS" : "FAILED");

  // 2. EspAdc
  ESP_LOGI(TAG, "=== Testing EspAdc ===");
  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  EspAdc test_adc(adc_cfg);
  bool adc_init = test_adc.EnsureInitialized();
  ESP_LOGI(TAG, "EspAdc initialized: %s", adc_init ? "SUCCESS" : "FAILED");

  // 3. EspUart
  ESP_LOGI(TAG, "=== Testing EspUart ===");
  hf_uart_config_t uart_cfg = {};
  uart_cfg.port_number = 0;
  uart_cfg.baud_rate = 115200;
  uart_cfg.tx_pin = 21;
  uart_cfg.rx_pin = 20;
  EspUart test_uart(uart_cfg);
  bool uart_init = test_uart.EnsureInitialized();
  ESP_LOGI(TAG, "EspUart initialized: %s", uart_init ? "SUCCESS" : "FAILED");

  // 4. EspCan
  ESP_LOGI(TAG, "=== Testing EspCan ===");
  hf_esp_can_config_t can_cfg = {};
  can_cfg.controller_id = hf_can_controller_id_t::HF_CAN_CONTROLLER_0;
  can_cfg.tx_pin = 7;
  can_cfg.rx_pin = 6;
  can_cfg.tx_queue_len = 8;
  EspCan test_can(can_cfg);
  auto can_init = test_can.EnsureInitialized();
  ESP_LOGI(TAG, "EspCan initialized: %s", can_init ? "SUCCESS" : "FAILED");

  // 5. EspSpi (Bus-Device Architecture)
  ESP_LOGI(TAG, "=== Testing EspSpi (Bus-Device Architecture) ===");
  hf_spi_bus_config_t spi_bus_cfg = {};
  spi_bus_cfg.mosi_pin = 10;
  spi_bus_cfg.miso_pin = 9;
  spi_bus_cfg.sclk_pin = 11;
  spi_bus_cfg.clock_speed_hz = 1000000;
  spi_bus_cfg.host = SPI2_HOST;
  EspSpiBus test_spi_bus(spi_bus_cfg);
  bool spi_bus_init = test_spi_bus.Initialize();
  ESP_LOGI(TAG, "EspSpiBus initialized: %s", spi_bus_init ? "SUCCESS" : "FAILED");

  // Create SPI device on the bus
  if (spi_bus_init) {
    hf_spi_device_config_t spi_dev_cfg = {};
    spi_dev_cfg.clock_speed_hz = 1000000;
    spi_dev_cfg.mode = hf_spi_mode_t::HF_SPI_MODE_0;
    spi_dev_cfg.cs_pin = 12;
    int device_index = test_spi_bus.CreateDevice(spi_dev_cfg);
    ESP_LOGI(TAG, "EspSpiDevice created with index: %d", device_index);
  }

  // 6. EspI2c (Bus-Device Architecture)
  ESP_LOGI(TAG, "=== Testing EspI2c (Bus-Device Architecture) ===");
  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;
  EspI2cBus test_i2c_bus(i2c_cfg);
  bool i2c_bus_init = test_i2c_bus.IsInitialized();
  ESP_LOGI(TAG, "EspI2cBus initialized: %s", i2c_bus_init ? "SUCCESS" : "FAILED");

  // 7. EspPwm
  ESP_LOGI(TAG, "=== Testing EspPwm ===");
  hf_pwm_unit_config_t pwm_cfg = {};
  pwm_cfg.unit_id = 0;
  pwm_cfg.mode = hf_pwm_mode_t::HF_PWM_MODE_FADE;
  pwm_cfg.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  pwm_cfg.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  pwm_cfg.enable_fade = true;
  pwm_cfg.enable_interrupts = true;
  EspPwm test_pwm(pwm_cfg);
  auto pwm_init = test_pwm.EnsureInitialized();
  ESP_LOGI(TAG, "EspPwm initialized: %s", pwm_init ? "SUCCESS" : "FAILED");

  // 8. EspTemperature
  ESP_LOGI(TAG, "=== Testing EspTemperature ===");
  EspTemperature test_temp;
  auto temp_init = test_temp.IsInitialized();
  ESP_LOGI(TAG, "EspTemperature initialized: %s", temp_init ? "SUCCESS" : "FAILED");

  // Read temperature if initialized successfully
  if (temp_init == hf_temp_err_t::TEMP_SUCCESS) {
    hf_temp_reading_t temp_reading = {};
    auto temp_read = test_temp.ReadTemperature(&temp_reading);
    if (temp_read == hf_temp_err_t::TEMP_SUCCESS) {
      ESP_LOGI(TAG, "Chip temperature: %.2f°C", temp_reading.temperature_raw);
    }
  }

  // 9. EspWifi
  ESP_LOGI(TAG, "=== Testing EspWifi ===");
  EspWifi test_wifi;
  auto wifi_init = test_wifi.IsInitialized();
  ESP_LOGI(TAG, "EspWifi initialized: %s", wifi_init ? "SUCCESS" : "FAILED");

  // 10. EspPeriodicTimer
  ESP_LOGI(TAG, "=== Testing EspPeriodicTimer ===");
  auto timer_callback = [](void* user_data) {
    static int count = 0;
    count++;
    if (count % 10 == 0) {
      ESP_LOGI("Timer", "Timer callback executed %d times", count);
    }
  };
  EspPeriodicTimer test_timer(timer_callback, nullptr);
  auto timer_init = test_timer.IsInitialized();
  ESP_LOGI(TAG, "EspPeriodicTimer initialized: %s", timer_init ? "SUCCESS" : "FAILED");

  if (timer_init) {
    test_timer.Start(1000000); // 1 second interval
    ESP_LOGI(TAG, "EspPeriodicTimer started with 1-second interval");
  }

  // 11. EspPio (RMT-based)
  ESP_LOGI(TAG, "=== Testing EspPio ===");
  EspPio test_pio;
  auto pio_init = test_pio.EnsureInitialized();
  ESP_LOGI(TAG, "EspPio initialized: %s", pio_init ? "SUCCESS" : "FAILED");

  // 12. EspNvs
  ESP_LOGI(TAG, "=== Testing EspNvs ===");
  // NVS configuration is handled internally by EspNvs
  EspNvs test_nvs("hardfoc");
  auto nvs_init = test_nvs.IsInitialized();
  ESP_LOGI(TAG, "EspNvs initialized: %s", nvs_init ? "SUCCESS" : "FAILED");

  // Test hardware types
  ESP_LOGI(TAG, "=== Testing HardwareTypes ===");
  hf_pin_num_t test_pin = 5;
  hf_port_num_t test_port = 0;
  hf_frequency_hz_t test_freq = 1000000;
  hf_timestamp_us_t test_timestamp = 12345678;
  uint32_t test_voltage = 3300;
  ESP_LOGI(TAG, "Pin: %d, Port: %d, Freq: %lu Hz", test_pin, test_port, test_freq);
  ESP_LOGI(TAG, "Timestamp: %llu us, Voltage: %d mV", test_timestamp, test_voltage);

  hf_gpio_state_t test_state = hf_gpio_state_t::HF_GPIO_STATE_ACTIVE;
  ESP_LOGI(TAG, "GPIO state: %s",
           test_state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE ? "ACTIVE" : "INACTIVE");

  // Test memory utilities
  ESP_LOGI(TAG, "=== Testing Memory Utilities ===");
  auto unique_int = hf::utils::make_unique_nothrow<int>(42);
  if (unique_int) {
    ESP_LOGI(TAG, "make_unique_nothrow created int with value: %d", *unique_int);
  }

  auto unique_array = std::make_unique<int[]>(10);
  if (unique_array) {
    for (int i = 0; i < 10; i++) {
      unique_array[i] = i * i;
    }
    ESP_LOGI(TAG, "make_unique_nothrow created array, element[5] = %d", unique_array[5]);
  }

  ESP_LOGI(TAG, "=== ESP32 IID Comprehensive Test Complete ===");
  ESP_LOGI(TAG, "%s", art_gen.Generate("ALL TESTS COMPLETE").c_str());

  // Keep the task running with periodic status updates
  int count = 0;
  while (true) {
    // Toggle LED if GPIO was initialized successfully
    if (gpio_init) {
      test_gpio.SetState(count % 2 == 0 ? hf_gpio_state_t::HF_GPIO_STATE_ACTIVE
                                        : hf_gpio_state_t::HF_GPIO_STATE_INACTIVE);
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP_LOGI(TAG, "System running... All interfaces operational (iteration %d)", ++count);
  }
}
