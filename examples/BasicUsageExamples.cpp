/*
 * @file BasicUsageExamples.cpp
 * @brief Collection of small examples demonstrating the HAL interfaces.
 *
 * These examples are minimal so they can be compiled as part of the
 * ESP-IDF test project. Each function exercises one part of the API
 * and prints a short message so the behavior can be observed when
 * run from main.cpp.
 */

#include "McuGpio.h"
#include "McuAdc.h"
#include "McuI2c.h"
#include "McuPwm.h"
#include "McuCan.h"
#include "McuUart.h"
#include "McuNvsStorage.h"
#include "McuPeriodicTimer.h"
#include "SfCan.h"
#include "SfUartDriver.h"
#include <memory>
#include <cstring>
#include <iostream>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace std;

static void TimerCb(void *) { std::cout << "Timer fired" << std::endl; }

static void ExampleBasicGpio() {
    McuGpio led(GPIO_NUM_2);
    McuGpio button(GPIO_NUM_0);

    led.Initialize();
    led.SetDirection(BaseGpio::Direction::Output);

    button.Initialize();
    button.SetDirection(BaseGpio::Direction::Input);
    button.SetPullMode(BaseGpio::PullMode::PullUp);

    bool pressed = false;
    button.IsActive(pressed);
    if (pressed) {
        led.SetActive();
    } else {
        led.SetInactive();
    }
    std::cout << "GPIO example done" << std::endl;
}

static void ExampleBasicAdc() {
    McuAdc adc;
    adc.Initialize();
    float voltage = 0.0f;
    adc.ReadChannelV(HF_ADC_CHANNEL_0, voltage);
    std::cout << "ADC Voltage: " << voltage << " V" << std::endl;
}

static void ExampleBasicI2c() {
    I2cMasterBusConfig cfg;
    cfg.i2c_port = I2C_NUM_0;
    cfg.sda_io_num = GPIO_NUM_21;
    cfg.scl_io_num = GPIO_NUM_22;

    McuI2c bus(cfg);
    bus.Initialize();

    uint8_t value = 0;
    bus.ReadRegister(0x48, 0x00, value);
    std::cout << "I2C read value: " << static_cast<int>(value) << std::endl;
}

static void ExampleBasicUart() {
    UartConfig cfg;
    cfg.baud_rate = 115200;
    cfg.tx_pin = GPIO_NUM_1;
    cfg.rx_pin = GPIO_NUM_3;

    auto impl = std::make_unique<McuUart>(1, cfg);
    SfUartDriver uart(std::move(impl));
    uart.Open();

    const char msg[] = "Hello from UART\n";
    uart.Write(reinterpret_cast<const uint8_t *>(msg), strlen(msg));
    uart.Close();
}

static void ExampleBasicCan() {
    CanBusConfig cfg;
    cfg.tx_pin = GPIO_NUM_5;
    cfg.rx_pin = GPIO_NUM_4;
    cfg.baudrate = 500000;

    auto impl = std::make_unique<McuCan>(cfg);
    SfCan can(std::move(impl));
    can.Initialize();
    can.Start();

    CanMessage msg{};
    msg.id = 0x123;
    msg.dlc = 1;
    msg.data[0] = 0x42;

    can.SendMessage(msg);
}

static void ExampleBasicPwm() {
    McuPwm pwm;
    pwm.Initialize();

    PwmChannelConfig ch{};
    ch.output_pin = GPIO_NUM_4;
    ch.frequency_hz = 5000;
    ch.resolution_bits = 13;
    ch.initial_duty_cycle = 0.5f;

    pwm.ConfigureChannel(0, ch);
    pwm.EnableChannel(0);
    pwm.StartAll();
    std::cout << "PWM running" << std::endl;
}

static void ExampleTimer() {
    McuPeriodicTimer timer(&TimerCb);
    timer.Initialize();
    timer.Start(500000);
    vTaskDelay(pdMS_TO_TICKS(1200));
    timer.Stop();
}

static void ExampleNvs() {
    McuNvsStorage store("app");
    if (store.Initialize() == HfNvsErr::NVS_SUCCESS) {
        store.SetU32("count", 42);
        uint32_t val = 0;
        store.GetU32("count", val);
        std::cout << "NVS value: " << val << std::endl;
        store.Deinitialize();
    }
}

extern "C" void RunBasicExamples() {
    std::cout << "\n=== Basic Usage Examples ===" << std::endl;
    ExampleBasicGpio();
    ExampleBasicAdc();
    ExampleBasicI2c();
    ExampleBasicUart();
    ExampleBasicCan();
    ExampleBasicPwm();
    ExampleTimer();
    ExampleNvs();
    std::cout << "=== Examples Complete ===" << std::endl;
}

