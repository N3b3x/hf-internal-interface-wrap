/**
 * @file McuDigitalGpio.cpp * @brief Implementation of MCU-specific unified BaseGpio class.
 *
 * This file contains the implementation of MCU-specific GPIO operations
 * for the unified BaseGpio class. It handles dynamic mode switching,
 * pull resistor configuration, and platform-specific GPIO management.
 */

#include "../mcu/McuDigitalGpio.h"
#include <algorithm>

// Platform detection - automatic detection based on build environment
#if defined(ESP_PLATFORM) || defined(IDF_VER)
    #define MCU_PLATFORM_ESP32
    #include "driver/gpio.h"
    #include "esp_log.h"
    #include "esp_err.h"
    #include "soc/gpio_reg.h"
    #include "hal/gpio_hal.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/semphr.h"
    #include "freertos/task.h"
#else
    #error "Unsupported MCU platform. Please add support for your target MCU."
#endif

static const char* TAG = "McuDigitalGpio";

//==============================================================================
// Constructor
//==============================================================================

McuDigitalGpio::McuDigitalGpio(hf_gpio_num_t pin_num, 
                               Direction direction,
                               ActiveState active_state,
                               OutputMode output_mode,
                               PullMode pull_mode) noexcept
    : BaseGpio(pin_num, direction, active_state, output_mode, pull_mode)
    , interrupt_trigger_(InterruptTrigger::None)
    , interrupt_callback_(nullptr)
    , interrupt_user_data_(nullptr)
    , interrupt_enabled_(false)
    , interrupt_count_(0)
    , platform_semaphore_(nullptr)
{    // Constructor delegates to BaseGpio base class
    // Initialize interrupt state
    InitializeInterruptSemaphore();
}

//==============================================================================
// Destructor
//==============================================================================

McuDigitalGpio::~McuDigitalGpio() {
    // Disable interrupts before cleanup
    if (interrupt_enabled_) {
        DisableInterrupt();
    }
    CleanupInterruptSemaphore();
}

//==============================================================================
// BaseGpio Implementation
//==============================================================================

bool McuDigitalGpio::Initialize() noexcept {
    if (IsInitialized()) {
        ESP_LOGW(TAG, "Pin %d already initialized", pin_);
        return true;
    }

    if (!ValidatePinNumber()) {
        ESP_LOGE(TAG, "Invalid pin number %d", pin_);
        return false;
    }

    if (!IsPinAvailable()) {
        ESP_LOGE(TAG, "Pin %d not available for GPIO", pin_);
        return false;
    }

    // Apply initial configuration
    HfGpioErr result = ApplyConfiguration();
    if (result != HfGpioErr::GPIO_SUCCESS) {        ESP_LOGE(TAG, "Failed to configure pin %d: %s", pin_, 
                 HfGpioErrToString(result));
        return false;
    }

    ESP_LOGI(TAG, "Initialized pin %d as %s, %s, %s", 
             pin_,
             ToString(current_direction_),
             ToString(active_state_),
             ToString(pull_mode_));

    return true;
}

bool McuDigitalGpio::Deinitialize() noexcept {
    if (!IsInitialized()) {
        return true;
    }

#if defined(MCU_PLATFORM_ESP32)
    // Reset pin to safe default state
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin_);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    
    esp_err_t result = gpio_config(&io_conf);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize pin %d: %s", pin_, esp_err_to_name(result));
        return false;
    }
#endif

    ESP_LOGI(TAG, "Deinitialized pin %d", pin_);
    return BaseGpio::Deinitialize();
}

bool McuDigitalGpio::IsPinAvailable() const noexcept {
#if defined(MCU_PLATFORM_ESP32)
    // Check if pin is valid for GPIO operations
    if (pin_ < 0 || pin_ >= GPIO_NUM_MAX) {
        return false;
    }
    
#ifdef HF_MCU_ESP32C6
    // ESP32-C6 specific pin availability (0-30 available)
    if (pin_ > 30) {
        return false;
    }
    
    // ESP32-C6 pin restrictions
    switch (pin_) {
        // USB pins - reserved for USB functionality
        case 12: case 13:
            return false;
            
        // Boot strapping pins - use with caution
        case 8: case 9:
            ESP_LOGW(TAG, "Pin %d is a bootstrap pin - use with caution", pin_);
            return true;
            
        // Valid GPIO pins for ESP32-C6
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
        case 10: case 11: case 14: case 15: case 16: case 17: case 18: case 19:
        case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27:
        case 28: case 29: case 30:
            return true;
            
        default:
            return false;
    }
    
#else
    // ESP32 Classic pin availability checks
    switch (pin_) {
        // Input-only pins on ESP32 Classic
        case 34: case 35: case 36: case 37: case 38: case 39:
            return (current_direction_ == Direction::Input);
        
        // Flash pins - typically reserved
        case 6: case 7: case 8: case 9: case 10: case 11:
            return false;
            
        // Valid GPIO pins for ESP32 Classic
        case 0: case 1: case 2: case 3: case 4: case 5:
        case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19:
        case 21: case 22: case 23: case 25: case 26: case 27:
        case 32: case 33:
            return true;
            
        default:
            return false;
    }
#endif

#else
    // Add other MCU platform checks here
    return (pin_ >= 0);
#endif
}

uint8_t McuDigitalGpio::GetMaxPins() const noexcept {
#if defined(MCU_PLATFORM_ESP32)
    return GPIO_NUM_MAX;
#else
    return 32; // Default fallback
#endif
}

const char* McuDigitalGpio::GetDescription() const noexcept {
    return "McuDigitalGpio - Unified MCU GPIO with dynamic mode switching";
}

bool McuDigitalGpio::SupportsInterrupts() const noexcept {
    return true; // Most MCU GPIOs support interrupts
}

//==============================================================================
// BaseGpio Pure Virtual Implementations
//==============================================================================

HfGpioErr McuDigitalGpio::SetDirectionImpl(Direction direction) noexcept {
#if defined(MCU_PLATFORM_ESP32)
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    
    if (direction == Direction::Input) {
        io_conf.mode = GPIO_MODE_INPUT;
    } else {
        // Configure as output with current output mode
        if (output_mode_ == OutputMode::OpenDrain) {
            io_conf.mode = GPIO_MODE_OUTPUT_OD;
        } else {
            io_conf.mode = GPIO_MODE_OUTPUT;
        }
    }
    
    // Apply current pull mode
    switch (pull_mode_) {
        case PullMode::PullUp:
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case PullMode::PullDown:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        case PullMode::Floating:
        default:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
    }
    
    io_conf.intr_type = GPIO_INTR_DISABLE;
    
    esp_err_t result = gpio_config(&io_conf);
    if (result != ESP_OK) {        ESP_LOGE(TAG, "Failed to set direction for pin %d: %s", pin_, esp_err_to_name(result));
        return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    }
    
    ESP_LOGD(TAG, "Set pin %d direction to %s", pin_, ToString(direction));
    return HfGpioErr::GPIO_SUCCESS;
#else
    return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuDigitalGpio::SetOutputModeImpl(OutputMode mode) noexcept {
    // For MCUs, output mode change typically requires reconfiguring the pin
    // if it's currently an output
    if (current_direction_ == Direction::Output) {
        return SetDirectionImpl(Direction::Output); // Reconfigure with new mode
    }
      ESP_LOGD(TAG, "Set pin %d output mode to %s (will apply on next output config)", 
             pin_, ToString(mode));
    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuDigitalGpio::SetActiveImpl() noexcept {
    if (current_direction_ != Direction::Output) {
        return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
    }

#if defined(MCU_PLATFORM_ESP32)
    bool level = StateToLevel(State::Active);
    esp_err_t result = gpio_set_level(static_cast<gpio_num_t>(pin), level ? 1 : 0);
    
    if (result != ESP_OK) {        ESP_LOGE(TAG, "Failed to set pin %d active: %s", pin_, esp_err_to_name(result));
        return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
    }
    
    ESP_LOGV(TAG, "Set pin %d to active (%s)", pin_, level ? "HIGH" : "LOW");
    return HfGpioErr::GPIO_SUCCESS;
#else
    return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuDigitalGpio::SetInactiveImpl() noexcept {
    if (current_direction_ != Direction::Output) {
        return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
    }

#if defined(MCU_PLATFORM_ESP32)
    bool level = StateToLevel(State::Inactive);
    esp_err_t result = gpio_set_level(static_cast<gpio_num_t>(pin), level ? 1 : 0);
    
    if (result != ESP_OK) {        ESP_LOGE(TAG, "Failed to set pin %d inactive: %s", pin_, esp_err_to_name(result));
        return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
    }
    
    ESP_LOGV(TAG, "Set pin %d to inactive (%s)", pin_, level ? "HIGH" : "LOW");
    return HfGpioErr::GPIO_SUCCESS;
#else
    return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuDigitalGpio::IsActiveImpl(bool& is_active) noexcept {
#if defined(MCU_PLATFORM_ESP32)
    int level = gpio_get_level(static_cast<gpio_num_t>(pin));
    if (level < 0) {
        ESP_LOGE(TAG, "Failed to read pin %d level", pin);
        return HfGpioErr::GPIO_ERR_READ_FAILURE;
    }
    
    State current_state = LevelToState(level != 0);
    is_active = (current_state == State::Active);
      ESP_LOGV(TAG, "Read pin %d: %s (%s)", pin_, 
             is_active ? "ACTIVE" : "INACTIVE", 
             level ? "HIGH" : "LOW");
    
    return HfGpioErr::GPIO_SUCCESS;
#else
    return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuDigitalGpio::ToggleImpl() noexcept {
    if (current_direction_ != Direction::Output) {
        return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
    }

#if defined(MCU_PLATFORM_ESP32)
    // Read current level and invert it
    int current_level = gpio_get_level(static_cast<gpio_num_t>(pin));
    if (current_level < 0) {
        ESP_LOGE(TAG, "Failed to read pin %d level for toggle", pin);
        return HfGpioErr::GPIO_ERR_READ_FAILURE;
    }
    
    int new_level = current_level ? 0 : 1;
    esp_err_t result = gpio_set_level(static_cast<gpio_num_t>(pin), new_level);
    
    if (result != ESP_OK) {        ESP_LOGE(TAG, "Failed to toggle pin %d: %s", pin_, esp_err_to_name(result));
        return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
    }
    
    ESP_LOGV(TAG, "Toggled pin %d from %s to %s", pin_, 
             current_level ? "HIGH" : "LOW", new_level ? "HIGH" : "LOW");
    return HfGpioErr::GPIO_SUCCESS;
#else
    return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuDigitalGpio::SetPullModeImpl(PullMode mode) noexcept {
    // For MCUs, changing pull mode typically requires reconfiguring the pin
    return SetDirectionImpl(current_direction_); // Reconfigure with new pull mode
}

BaseGpio::PullMode McuDigitalGpio::GetPullModeImpl() const noexcept {
    // Return cached pull mode - querying from hardware registers is complex
    // and not always reliable across different MCU platforms
    return pull_mode_;
}

//==============================================================================
// Interrupt Functionality Implementation
//==============================================================================

HfGpioErr McuDigitalGpio::ConfigureInterrupt(InterruptTrigger trigger, 
                                             InterruptCallback callback, 
                                             void* user_data) noexcept {
    if (!EnsureInitialized()) {
        return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
    }

    // Disable interrupt first if it's currently enabled
    if (interrupt_enabled_) {
        DisableInterrupt();
    }

    // Store configuration
    interrupt_trigger_ = trigger;
    interrupt_callback_ = callback;
    interrupt_user_data_ = user_data;
    interrupt_count_ = 0;

    // If trigger is None, just clear configuration
    if (trigger == InterruptTrigger::None) {
        return HfGpioErr::GPIO_SUCCESS;
    }

#if defined(MCU_PLATFORM_ESP32)
    // Configure interrupt type
    uint32_t platform_trigger = ConvertInterruptTrigger(trigger);
    if (platform_trigger == GPIO_INTR_DISABLE) {
        return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
    }

    // Configure GPIO for interrupt
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin_);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = (pull_mode_ == PullMode::PullUp) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = (pull_mode_ == PullMode::PullDown) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = static_cast<gpio_int_type_t>(platform_trigger);

    esp_err_t result = gpio_config(&io_conf);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure interrupt for pin %d: %s", pin_, esp_err_to_name(result));
        return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    }
#endif

    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuDigitalGpio::EnableInterrupt() noexcept {
    if (interrupt_enabled_) {
        return HfGpioErr::GPIO_ERR_INTERRUPT_ALREADY_ENABLED;
    }

    if (interrupt_trigger_ == InterruptTrigger::None) {
        return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
    }

    if (!EnsureInitialized()) {
        return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
    }

#if defined(MCU_PLATFORM_ESP32)
    // Install ISR service if not already installed
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(err));
        return HfGpioErr::GPIO_ERR_INTERRUPT_HANDLER_FAILED;
    }

    // Add ISR handler for this pin
    err = gpio_isr_handler_add(static_cast<gpio_num_t>(pin_), InterruptHandler, this);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler for pin %d: %s", pin_, esp_err_to_name(err));
        return HfGpioErr::GPIO_ERR_INTERRUPT_HANDLER_FAILED;
    }
#endif

    interrupt_enabled_ = true;
    ESP_LOGI(TAG, "Interrupt enabled for pin %d with trigger %s", pin_, ToString(interrupt_trigger_));
    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuDigitalGpio::DisableInterrupt() noexcept {
    if (!interrupt_enabled_) {
        return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_ENABLED;
    }

#if defined(MCU_PLATFORM_ESP32)
    // Remove ISR handler
    esp_err_t err = gpio_isr_handler_remove(static_cast<gpio_num_t>(pin_));
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to remove ISR handler for pin %d: %s", pin_, esp_err_to_name(err));
        // Continue anyway since we're disabling
    }
#endif

    interrupt_enabled_ = false;
    ESP_LOGI(TAG, "Interrupt disabled for pin %d", pin_);
    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuDigitalGpio::WaitForInterrupt(uint32_t timeout_ms) noexcept {
    if (!interrupt_enabled_) {
        return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_ENABLED;
    }

    if (!platform_semaphore_) {
        return HfGpioErr::GPIO_ERR_SYSTEM_ERROR;
    }

#if defined(MCU_PLATFORM_ESP32)
    TickType_t timeout_ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    BaseType_t result = xSemaphoreTake(platform_semaphore_, timeout_ticks);
    
    if (result == pdTRUE) {
        return HfGpioErr::GPIO_SUCCESS;
    } else {
        return HfGpioErr::GPIO_ERR_TIMEOUT;
    }
#else
    return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuDigitalGpio::GetInterruptStatus(InterruptStatus& status) noexcept {
    status.is_enabled = interrupt_enabled_;
    status.trigger_type = interrupt_trigger_;
    status.interrupt_count = interrupt_count_;
    status.has_callback = (interrupt_callback_ != nullptr);
    
    return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuDigitalGpio::ClearInterruptStats() noexcept {
    interrupt_count_ = 0;
    return HfGpioErr::GPIO_SUCCESS;
}

//==============================================================================
// Interrupt Helper Methods
//==============================================================================

uint32_t McuDigitalGpio::ConvertInterruptTrigger(InterruptTrigger trigger) const noexcept {
#if defined(MCU_PLATFORM_ESP32)
    switch (trigger) {
        case InterruptTrigger::RisingEdge:
            return GPIO_INTR_POSEDGE;
        case InterruptTrigger::FallingEdge:
            return GPIO_INTR_NEGEDGE;
        case InterruptTrigger::BothEdges:
            return GPIO_INTR_ANYEDGE;
        case InterruptTrigger::LowLevel:
            return GPIO_INTR_LOW_LEVEL;
        case InterruptTrigger::HighLevel:
            return GPIO_INTR_HIGH_LEVEL;
        case InterruptTrigger::None:
        default:
            return GPIO_INTR_DISABLE;
    }
#else
    return 0;
#endif
}

void IRAM_ATTR McuDigitalGpio::InterruptHandler(void* arg) noexcept {
    auto* self = static_cast<McuDigitalGpio*>(arg);
    if (!self) return;

    // Increment interrupt counter
    self->interrupt_count_++;

#if defined(MCU_PLATFORM_ESP32)
    // Signal semaphore for WaitForInterrupt
    if (self->platform_semaphore_) {
        BaseType_t higher_priority_task_woken = pdFALSE;
        xSemaphoreGiveFromISR(self->platform_semaphore_, &higher_priority_task_woken);
        if (higher_priority_task_woken) {
            portYIELD_FROM_ISR();
        }
    }
#endif

    // Call user callback if registered
    if (self->interrupt_callback_) {
        self->interrupt_callback_(self, self->interrupt_trigger_, self->interrupt_user_data_);
    }
}

void McuDigitalGpio::InitializeInterruptSemaphore() noexcept {
#if defined(MCU_PLATFORM_ESP32)
    platform_semaphore_ = xSemaphoreCreateBinary();
    if (!platform_semaphore_) {
        ESP_LOGE(TAG, "Failed to create interrupt semaphore for pin %d", pin_);
    }
#endif
}

void McuDigitalGpio::CleanupInterruptSemaphore() noexcept {
#if defined(MCU_PLATFORM_ESP32)
    if (platform_semaphore_) {
        vSemaphoreDelete(platform_semaphore_);
        platform_semaphore_ = nullptr;
    }
#endif
}

//==============================================================================
// Private Helper Methods
//==============================================================================

uint32_t McuDigitalGpio::ConvertPullMode(PullMode pull_mode) const noexcept {
#if defined(MCU_PLATFORM_ESP32)
    switch (pull_mode) {
        case PullMode::PullUp:
            return GPIO_PULLUP_ENABLE;
        case PullMode::PullDown:
            return GPIO_PULLDOWN_ENABLE;
        case PullMode::Floating:
        default:
            return GPIO_PULLUP_DISABLE;
    }
#else
    return 0;
#endif
}

uint32_t McuDigitalGpio::ConvertOutputMode(OutputMode output_mode) const noexcept {
#if defined(MCU_PLATFORM_ESP32)
    switch (output_mode) {
        case OutputMode::OpenDrain:
            return GPIO_MODE_OUTPUT_OD;
        case OutputMode::PushPull:
        default:
            return GPIO_MODE_OUTPUT;
    }
#else
    return 0;
#endif
}

bool McuDigitalGpio::ValidatePinNumber() const noexcept {
#if defined(MCU_PLATFORM_ESP32)
    return (pin >= 0 && pin < GPIO_NUM_MAX);
#else
    return (pin >= 0);
#endif
}

HfGpioErr McuDigitalGpio::ApplyConfiguration() noexcept {
    return SetDirectionImpl(current_direction_);
}
