/**
 * @file DigitalExternalIRQ.h
 * @brief External interrupt driver for ESP32-C6 using ESP-IDF.
 *
 * This driver provides a simple interface for enabling, disabling and waiting
 * for a GPIO interrupt.  It replaces the previous Synergy based implementation
 * and relies solely on ESP-IDF APIs.
 */

#ifndef DIGITALEXTERNALIRQ_H
#define DIGITALEXTERNALIRQ_H

#include <internal_interface_drivers/DigitalInput.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class DigitalExternalIRQ : public DigitalInput {
public:
    /**
     * @brief Construct an external interrupt on the given pin.
     *
     * @param pin           GPIO pin number.
     * @param interruptType ESP-IDF interrupt type (e.g. GPIO_INTR_POSEDGE).
     * @param activeState   Logical active level of the pin.
     */
    DigitalExternalIRQ(gpio_num_t pin,
                       gpio_int_type_t interruptType = GPIO_INTR_POSEDGE,
                       ActiveState activeState = ActiveState::High) noexcept;

    DigitalExternalIRQ(const DigitalExternalIRQ&) = delete;
    DigitalExternalIRQ& operator=(const DigitalExternalIRQ&) = delete;
    virtual ~DigitalExternalIRQ();

    /** Enable external interrupt handling. */
    bool Enable();
    /** Disable external interrupt handling. */
    bool Disable();

    /**
     * @brief Block until the interrupt occurs or the timeout expires.
     * @param timeoutMs Timeout in milliseconds (defaults to indefinite).
     * @return true if the interrupt was received, false on timeout or error.
     */
    bool Wait(uint32_t timeoutMs = portMAX_DELAY);

private:
    virtual bool Initialize() noexcept override;
    static void IRAM_ATTR IsrHandler(void* arg);

    gpio_int_type_t intrType;
    SemaphoreHandle_t binSem;
    bool enabled;
};

#endif // DIGITALEXTERNALIRQ_H
