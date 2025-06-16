#include "DigitalOutput.h"
#include "I2cBus.h"
#include "DummyDevice.hpp"

extern "C" void app_main(void) {
    i2c_config_t cfg{};
    cfg.sda_io_num = GPIO_NUM_21;
    cfg.scl_io_num = GPIO_NUM_22;
    I2cBus bus(I2C_NUM_0, cfg);
    bus.Open();
    DummyDevice dev(bus, 0x50);
    dev.Init();
    DigitalOutput led(GPIO_NUM_2, DigitalGpio::ActiveState::High);
    led.SetActive();
}
