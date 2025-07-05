#define HF_TARGET_MCU_ESP32C6
// Mock ESP-IDF types for compilation testing
typedef int gpio_num_t;
typedef int i2c_port_t;
typedef struct {
    gpio_num_t sda_io_num;
    gpio_num_t scl_io_num;
    int mode;
} i2c_config_t;

#define GPIO_NUM_6 6
#define GPIO_NUM_7 7
#define GPIO_NUM_8 8
#define I2C_NUM_0 0

// Mock ESP error type
typedef int esp_err_t;
#define ESP_OK 0

// Now include our headers
#include "DigitalGpio.h"
#include "DigitalOutput.h"
#include "I2cBus.h"

// Just a simple compilation test
int main() {
    return 0;
}
