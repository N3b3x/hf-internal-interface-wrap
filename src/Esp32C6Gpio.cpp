/**
 * @file Esp32C6Gpio.cpp
 * @brief Implementation of ESP32-C6 GPIO classes.
 *
 * This file contains the implementation of ESP32-C6 specific GPIO operations
 * for the HardFOC system.
 */

#include "Esp32C6Gpio.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <algorithm>
#include <mutex>

static const char* TAG = "Esp32C6Gpio";

//==============================================================================
// Esp32C6Output Implementation
//==============================================================================

bool Esp32C6Output::Initialize() noexcept {
    if (IsInitialized()) {
        return true;
    }

    if (!IsPinAvailable()) {
        ESP_LOGE(TAG, "Pin %d not available for GPIO output", pin);
        return false;
    }

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    // Configure open-drain mode if specified
    if (mode_ == Mode::OpenDrain) {
        io_conf.mode = GPIO_MODE_OUTPUT_OD;
    }

    esp_err_t result = gpio_config(&io_conf);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure pin %d: %s", pin, esp_err_to_name(result));
        return false;
    }

    // Set initial state
    bool initial_level;
    if (initial_state_ == State::Active) {
        initial_level = IsActiveHigh();
    } else {
        initial_level = !IsActiveHigh();
    }

    result = gpio_set_level(pin, initial_level ? 1 : 0);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set initial level for pin %d", pin);
        return false;
    }

    ESP_LOGI(TAG, "Initialized output pin %d (active %s, initial %s)", 
             pin, IsActiveHigh() ? "high" : "low",
             initial_state_ == State::Active ? "active" : "inactive");

    return true;
}

bool Esp32C6Output::IsPinAvailable() const noexcept {
    return Esp32C6GpioManager::GetInstance().IsPinAvailable(pin);
}

HfGpioErr Esp32C6Output::SetActiveImpl() noexcept {
    if (!SetPinLevel(IsActiveHigh())) {
        return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
    }
    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr Esp32C6Output::SetInactiveImpl() noexcept {
    if (!SetPinLevel(!IsActiveHigh())) {
        return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
    }
    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr Esp32C6Output::ToggleImpl() noexcept {
    bool current_level = GetPinLevel();
    if (!SetPinLevel(!current_level)) {
        return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
    }
    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr Esp32C6Output::IsActiveImpl(bool& is_active) noexcept {
    bool level = GetPinLevel();
    is_active = IsActiveHigh() ? level : !level;
    return HfGpioErr::GPIO_SUCCESS;
}

bool Esp32C6Output::SetPinLevel(bool level) noexcept {
    esp_err_t result = gpio_set_level(pin, level ? 1 : 0);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set pin %d level: %s", pin, esp_err_to_name(result));
        return false;
    }
    return true;
}

bool Esp32C6Output::GetPinLevel() noexcept {
    return gpio_get_level(pin) != 0;
}

//==============================================================================
// Esp32C6Input Implementation
//==============================================================================

bool Esp32C6Input::Initialize() noexcept {
    if (IsInitialized()) {
        return true;
    }

    if (!IsPinAvailable()) {
        ESP_LOGE(TAG, "Pin %d not available for GPIO input", pin);
        return false;
    }

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    // Configure pull resistors
    switch (pull_resistance_) {
        case Resistance::PullUp:
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case Resistance::PullDown:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        case Resistance::Floating:
        default:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
    }

    esp_err_t result = gpio_config(&io_conf);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure input pin %d: %s", pin, esp_err_to_name(result));
        return false;
    }

    ESP_LOGI(TAG, "Initialized input pin %d (active %s, pull %s)", 
             pin, IsActiveHigh() ? "high" : "low",
             DigitalGpio::ToString(pull_resistance_));

    return true;
}

bool Esp32C6Input::IsPinAvailable() const noexcept {
    return Esp32C6GpioManager::GetInstance().IsPinAvailable(pin);
}

HfGpioErr Esp32C6Input::IsActiveImpl(bool& is_active) noexcept {
    int level = gpio_get_level(pin);
    is_active = IsActiveHigh() ? (level != 0) : (level == 0);
    return HfGpioErr::GPIO_SUCCESS;
}

//==============================================================================
// Esp32C6InterruptInput Implementation
//==============================================================================

Esp32C6InterruptInput::~Esp32C6InterruptInput() noexcept {
    if (interrupt_enabled_) {
        DisableInterrupt();
    }
}

bool Esp32C6InterruptInput::Initialize() noexcept {
    if (IsInitialized()) {
        return true;
    }

    if (!IsPinAvailable()) {
        ESP_LOGE(TAG, "Pin %d not available for GPIO interrupt input", pin);
        return false;
    }

    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.intr_type = interrupt_type_;

    // Configure pull resistors
    switch (pull_resistance_) {
        case Resistance::PullUp:
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case Resistance::PullDown:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        case Resistance::Floating:
        default:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
    }

    esp_err_t result = gpio_config(&io_conf);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure interrupt input pin %d: %s", pin, esp_err_to_name(result));
        return false;
    }

    ESP_LOGI(TAG, "Initialized interrupt input pin %d (active %s, interrupt type %d)", 
             pin, IsActiveHigh() ? "high" : "low", interrupt_type_);

    return true;
}

HfGpioErr Esp32C6InterruptInput::EnableInterrupt(InterruptCallback callback, void* user_data) noexcept {
    if (!EnsureInitialized()) {
        return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
    }

    if (callback == nullptr) {
        return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
    }

    if (interrupt_enabled_) {
        return HfGpioErr::GPIO_ERR_ALREADY_INITIALIZED;
    }

    // Install GPIO ISR service (safe to call multiple times)
    esp_err_t result = gpio_install_isr_service(0);
    if (result != ESP_OK && result != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(result));
        return HfGpioErr::GPIO_ERR_SYSTEM_ERROR;
    }

    // Store callback and user data
    callback_ = callback;
    user_data_ = user_data;

    // Add ISR handler
    result = gpio_isr_handler_add(pin, InterruptHandler, this);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler for pin %d: %s", pin, esp_err_to_name(result));
        callback_ = nullptr;
        user_data_ = nullptr;
        return HfGpioErr::GPIO_ERR_SYSTEM_ERROR;
    }

    interrupt_enabled_ = true;
    ESP_LOGI(TAG, "Enabled interrupt for pin %d", pin);

    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr Esp32C6InterruptInput::DisableInterrupt() noexcept {
    if (!interrupt_enabled_) {
        return HfGpioErr::GPIO_SUCCESS;
    }

    esp_err_t result = gpio_isr_handler_remove(pin);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to remove ISR handler for pin %d: %s", pin, esp_err_to_name(result));
        return HfGpioErr::GPIO_ERR_SYSTEM_ERROR;
    }

    callback_ = nullptr;
    user_data_ = nullptr;
    interrupt_enabled_ = false;

    ESP_LOGI(TAG, "Disabled interrupt for pin %d", pin);
    return HfGpioErr::GPIO_SUCCESS;
}

void IRAM_ATTR Esp32C6InterruptInput::InterruptHandler(void* arg) {
    auto* self = static_cast<Esp32C6InterruptInput*>(arg);
    if (self && self->callback_) {
        self->callback_(self->user_data_);
    }
}

//==============================================================================
// Esp32C6GpioManager Implementation
//==============================================================================

Esp32C6GpioManager& Esp32C6GpioManager::GetInstance() noexcept {
    static Esp32C6GpioManager instance;
    return instance;
}

bool Esp32C6GpioManager::IsPinAvailable(gpio_num_t pin_num) const noexcept {
    if (pin_num < 0 || pin_num > Esp32C6GpioConfig::MAX_GPIO_PINS) {
        return false;
    }

    // Check if pin is in reserved list
    auto& reserved = Esp32C6GpioConfig::RESERVED_PINS;
    if (std::find(reserved.begin(), reserved.end(), pin_num) != reserved.end()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    return !reserved_pins_[pin_num];
}

bool Esp32C6GpioManager::IsPinReserved(gpio_num_t pin_num) const noexcept {
    if (pin_num < 0 || pin_num > Esp32C6GpioConfig::MAX_GPIO_PINS) {
        return true; // Invalid pins are considered reserved
    }

    // Check if pin is in reserved list
    auto& reserved = Esp32C6GpioConfig::RESERVED_PINS;
    if (std::find(reserved.begin(), reserved.end(), pin_num) != reserved.end()) {
        return true;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    return reserved_pins_[pin_num];
}

HfGpioErr Esp32C6GpioManager::ReservePin(gpio_num_t pin_num) noexcept {
    if (pin_num < 0 || pin_num > Esp32C6GpioConfig::MAX_GPIO_PINS) {
        return HfGpioErr::GPIO_ERR_INVALID_PIN;
    }

    if (!IsPinAvailable(pin_num)) {
        return HfGpioErr::GPIO_ERR_PIN_ALREADY_REGISTERED;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    reserved_pins_[pin_num] = true;

    ESP_LOGI(TAG, "Reserved GPIO pin %d", pin_num);
    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr Esp32C6GpioManager::ReleasePin(gpio_num_t pin_num) noexcept {
    if (pin_num < 0 || pin_num > Esp32C6GpioConfig::MAX_GPIO_PINS) {
        return HfGpioErr::GPIO_ERR_INVALID_PIN;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    reserved_pins_[pin_num] = false;

    ESP_LOGI(TAG, "Released GPIO pin %d", pin_num);
    return HfGpioErr::GPIO_SUCCESS;
}

std::array<gpio_num_t, 15> Esp32C6GpioManager::GetAvailablePins() const noexcept {
    return Esp32C6GpioConfig::SAFE_GPIO_PINS;
}

void Esp32C6GpioManager::PrintPinStatus() const noexcept {
    ESP_LOGI(TAG, "=== ESP32-C6 GPIO Pin Status ===");
    
    ESP_LOGI(TAG, "Reserved pins (hardware functions):");
    for (auto pin : Esp32C6GpioConfig::RESERVED_PINS) {
        ESP_LOGI(TAG, "  GPIO%d - Reserved for hardware function", pin);
    }
    
    ESP_LOGI(TAG, "Conditional pins (can be used as GPIO with care):");
    for (auto pin : Esp32C6GpioConfig::CONDITIONAL_GPIO_PINS) {
        ESP_LOGI(TAG, "  GPIO%d - Conditional use", pin);
    }
    
    ESP_LOGI(TAG, "Safe GPIO pins:");
    for (auto pin : Esp32C6GpioConfig::SAFE_GPIO_PINS) {
        std::lock_guard<std::mutex> lock(mutex_);
        const char* status = reserved_pins_[pin] ? "ALLOCATED" : "AVAILABLE";
        ESP_LOGI(TAG, "  GPIO%d - %s", pin, status);
    }
    
    ESP_LOGI(TAG, "================================");
}
