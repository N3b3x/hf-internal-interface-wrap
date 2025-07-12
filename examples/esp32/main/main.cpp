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
#include "base/BaseUart.h"
#include "base/HardwareTypes.h"

// Include utility classes
#include "utils/DigitalOutputGuard.h"
#include "utils/McuSelect.h"
#include "utils/RtosMutex.h"

// ESP32 implementation classes (temporarily commented out due to compilation errors)
#include "mcu/esp32/EspAdc.h"
#include "mcu/esp32/EspCan.h"
#include "mcu/esp32/EspGpio.h"
#include "mcu/esp32/EspI2c.h"
#include "mcu/esp32/EspNvs.h"
#include "mcu/esp32/EspPeriodicTimer.h"
#include "mcu/esp32/EspPio.h"
#include "mcu/esp32/EspPwm.h"
#include "mcu/esp32/EspSpi.h"
#include "mcu/esp32/EspUart.h"

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

static const char* TAG = "IID_Test";

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "=== ESP32 IID Test Start ===");

  // Minimal integration of all ESP32 libraries:
  // 1. EspGpio
  EspGpio test_gpio(8, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                    hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
  bool gpio_init = test_gpio.Initialize();
  ESP_LOGI(TAG, "EspGpio initialized: %s", gpio_init ? "true" : "false");

  // 2. EspAdc
  hf_adc_unit_config_t adc_cfg = {};
  adc_cfg.unit_id = 0;
  EspAdc test_adc(adc_cfg);
  bool adc_init = test_adc.EnsureInitialized();
  ESP_LOGI(TAG, "EspAdc initialized: %s", adc_init ? "true" : "false");

  // 3. EspUart
  hf_uart_config_t uart_cfg = {};
  uart_cfg.port_number = 0;
  uart_cfg.baud_rate = 115200;
  uart_cfg.tx_pin = 21;
  uart_cfg.rx_pin = 20;
  EspUart test_uart(uart_cfg);
  bool uart_init = test_uart.EnsureInitialized();
  ESP_LOGI(TAG, "EspUart initialized: %s", uart_init ? "true" : "false");

  // 4. EspCan
  hf_esp_can_config_t can_cfg = {};
  can_cfg.controller_id = hf_can_controller_id_t::HF_CAN_CONTROLLER_0;
  can_cfg.tx_pin = 7;
  can_cfg.rx_pin = 6;
  can_cfg.tx_queue_len = 8;
  EspCan test_can(can_cfg);
  auto can_init = test_can.Initialize();
  ESP_LOGI(TAG, "EspCan initialized: %s", can_init == hf_can_err_t::CAN_SUCCESS ? "true" : "false");

  // 5. EspSpi
  hf_spi_bus_config_t spi_cfg = {};
  spi_cfg.mosi_pin = 10;
  spi_cfg.miso_pin = 9;
  spi_cfg.sclk_pin = 11;
  spi_cfg.clock_speed_hz = 1000000;
  spi_cfg.mode = 0;
  EspSpi test_spi(spi_cfg);
  bool spi_init = test_spi.EnsureInitialized();
  ESP_LOGI(TAG, "EspSpi initialized: %s", spi_init ? "true" : "false");

  // 6. EspI2c
  hf_i2c_master_bus_config_t i2c_cfg = {};
  i2c_cfg.i2c_port = I2C_NUM_0;
  i2c_cfg.sda_io_num = 21;
  i2c_cfg.scl_io_num = 22;
  i2c_cfg.enable_internal_pullup = true;
  EspI2c test_i2c(i2c_cfg);
  bool i2c_init = test_i2c.Initialize();
  ESP_LOGI(TAG, "EspI2c initialized: %s", i2c_init ? "true" : "false");

  // 7. EspPwm
  hf_pwm_unit_config_t pwm_cfg = {};
  pwm_cfg.unit_id = 0;
  pwm_cfg.mode = hf_pwm_mode_t::HF_PWM_MODE_FADE;
  pwm_cfg.base_clock_hz = HF_PWM_APB_CLOCK_HZ;
  pwm_cfg.clock_source = hf_pwm_clock_source_t::HF_PWM_CLK_SRC_DEFAULT;
  pwm_cfg.enable_fade = true;
  pwm_cfg.enable_interrupts = true;
  EspPwm test_pwm(pwm_cfg);
  auto pwm_init = test_pwm.EnsureInitialized();
  ESP_LOGI(TAG, "EspPwm initialized: %s", pwm_init ? "true" : "false");

  // Test hardware types
  ESP_LOGI(TAG, "Testing HardwareTypes...");
  hf_pin_num_t test_pin = 5;
  hf_port_num_t test_port = 0;
  hf_frequency_hz_t test_freq = 1000000;
  ESP_LOGI(TAG, "Pin: %ld, Port: %lu, Freq: %lu Hz", test_pin, test_port, test_freq);

  // Test GPIO error codes
  ESP_LOGI(TAG, "Testing GPIO error codes...");
  hf_gpio_err_t test_error = hf_gpio_err_t::GPIO_SUCCESS;
  ESP_LOGI(TAG, "GPIO Error: %s", HfGpioErrToString(test_error));

  // Test GPIO states
  ESP_LOGI(TAG, "Testing GPIO states...");
  hf_gpio_state_t test_state = hf_gpio_state_t::HF_GPIO_STATE_ACTIVE;
  ESP_LOGI(TAG, "GPIO State: %s", BaseGpio::ToString(test_state));

  // Test GPIO directions
  ESP_LOGI(TAG, "Testing GPIO directions...");
  hf_gpio_direction_t test_dir = hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT;
  ESP_LOGI(TAG, "GPIO Direction: %s", BaseGpio::ToString(test_dir));

  // Test GPIO active states
  ESP_LOGI(TAG, "Testing GPIO active states...");
  hf_gpio_active_state_t test_active = hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH;
  ESP_LOGI(TAG, "GPIO Active State: %s", BaseGpio::ToString(test_active));

  // Test GPIO output modes
  ESP_LOGI(TAG, "Testing GPIO output modes...");
  hf_gpio_output_mode_t test_output = hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL;
  ESP_LOGI(TAG, "GPIO Output Mode: %s", BaseGpio::ToString(test_output));

  // Test GPIO pull modes
  ESP_LOGI(TAG, "Testing GPIO pull modes...");
  hf_gpio_pull_mode_t test_pull = hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING;
  ESP_LOGI(TAG, "GPIO Pull Mode: %s", BaseGpio::ToString(test_pull));

  // Test GPIO interrupt triggers
  ESP_LOGI(TAG, "Testing GPIO interrupt triggers...");
  hf_gpio_interrupt_trigger_t test_trigger =
      hf_gpio_interrupt_trigger_t::HF_GPIO_INTERRUPT_TRIGGER_RISING_EDGE;
  ESP_LOGI(TAG, "GPIO Interrupt Trigger: %s", BaseGpio::ToString(test_trigger));

  // Test validation functions
  ESP_LOGI(TAG, "Testing validation functions...");
  ESP_LOGI(TAG, "Valid pin: %s", IsValidPin(test_pin) ? "true" : "false");
  ESP_LOGI(TAG, "Valid port: %s", IsValidPort(test_port) ? "true" : "false");

  // Configure GPIO for LED
  ESP_LOGI(TAG, "Configuring GPIO for LED...");
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << GPIO_NUM_8);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  ESP_LOGI(TAG, "GPIO configured successfully");
  ESP_LOGI(TAG, "All base classes and utility classes compiled successfully!");

  int count = 0;
  while (true) {
    ESP_LOGI(TAG, "Blink count: %d", count++);

    // Toggle LED
    gpio_set_level(GPIO_NUM_8, count % 2);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
