#include "DigitalOutput.h"
#include "DummyTransport.h"
#include "I2cBus.h"
#include "PeriodicTimer.h"

static void ToggleCb(void *arg) {
    auto led = static_cast<DigitalOutput *>(arg);
    led->Toggle();
}

extern "C" void app_main() {
    i2c_config_t cfg = {};
    cfg.mode = I2C_MODE_MASTER;
    cfg.sda_io_num = GPIO_NUM_8;
    cfg.scl_io_num = GPIO_NUM_9;
    cfg.master.clk_speed = 100000;

    I2cBus bus(I2C_NUM_0, cfg);
    bus.Open();

    DummyTransport transport(bus);
    uint8_t value = 0;
    transport.ReadReg(0x00, value);

    DigitalOutput led(GPIO_NUM_2, DigitalGpio::ActiveState::High);
    PeriodicTimer timer(&ToggleCb, &led);
    timer.Start(1000000);

    (void)value;
}
