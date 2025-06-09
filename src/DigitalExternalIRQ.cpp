#include "DigitalExternalIRQ.h"
#include <driver/gpio.h>
#include <esp_err.h>

DigitalExternalIRQ::DigitalExternalIRQ(gpio_num_t pinArg,
                                       gpio_int_type_t interruptType,
                                       ActiveState activeStateArg) noexcept
    : DigitalInput(pinArg, activeStateArg),
      intrType(interruptType),
      binSem(xSemaphoreCreateBinary()),
      enabled(false) {}

DigitalExternalIRQ::~DigitalExternalIRQ() {
    Disable();
    if (binSem) {
        vSemaphoreDelete(binSem);
    }
}

bool DigitalExternalIRQ::Initialize() noexcept {
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = intrType;
    return gpio_config(&io_conf) == ESP_OK;
}

void IRAM_ATTR DigitalExternalIRQ::IsrHandler(void* arg) {
    auto* self = static_cast<DigitalExternalIRQ*>(arg);
    BaseType_t higher_woken = pdFALSE;
    xSemaphoreGiveFromISR(self->binSem, &higher_woken);
    if (higher_woken) {
        portYIELD_FROM_ISR();
    }
}

bool DigitalExternalIRQ::Enable() {
    if (enabled) return true;
    if (!EnsureInitialized()) return false;
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return false;
    }
    err = gpio_isr_handler_add(pin, IsrHandler, this);
    if (err != ESP_OK) {
        return false;
    }
    enabled = true;
    return true;
}

bool DigitalExternalIRQ::Disable() {
    if (!enabled) return true;
    gpio_isr_handler_remove(pin);
    enabled = false;
    return true;
}

bool DigitalExternalIRQ::Wait(uint32_t timeoutMs) {
    if (!Enable()) return false;
    return xSemaphoreTake(binSem, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}
