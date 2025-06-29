/**
 * @file McuGpio.cpp
 * @brief Implementation of advanced MCU-specific unified BaseGpio class.
 *
 * This file contains the implementation of MCU-specific GPIO operations
 * for the unified BaseGpio class with advanced ESP32C6/ESP-IDF v5.5+ features.
 * It handles dynamic mode switching, pull resistor configuration, platform-specific
 * GPIO management, glitch filtering, power management, and RTC GPIO support.
 * The implementation provides interrupt handling, debouncing, and hardware acceleration.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#include "McuGpio.h"
#include <algorithm>

// Platform-specific includes via centralized McuSelect.h (included in McuGpio.h)
#ifdef HF_MCU_ESP32C6
// ESP32-C6 specific includes with ESP-IDF v5.5+ features
#include "driver/gpio_filter.h"
#include "driver/lp_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "hal/gpio_types.h"
#include "hal/rtc_io_types.h"
#include "soc/clk_tree_defs.h"
static const char *TAG = "McuGpio";
#elif defined(HF_MCU_FAMILY_ESP32)
// ESP32 family includes for advanced GPIO features
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "soc/rtc.h"
static const char *TAG = "McuGpio";
#else
#error "Unsupported MCU platform. Please add support for your target MCU."
#endif

//==============================================================================
// Constructor
//==============================================================================

McuGpio::McuGpio(HfPinNumber pin_num, Direction direction, ActiveState active_state,
                 OutputMode output_mode, PullMode pull_mode,
                 GpioDriveCapability drive_capability) noexcept
    : BaseGpio(pin_num, direction, active_state, output_mode, pull_mode),
      interrupt_trigger_(InterruptTrigger::None), interrupt_callback_(nullptr),
      interrupt_user_data_(nullptr), interrupt_enabled_(false), interrupt_count_(0),
      platform_semaphore_(nullptr), drive_capability_(drive_capability),
      glitch_filter_type_(GpioGlitchFilterType::None), pin_glitch_filter_enabled_(false),
      flex_glitch_filter_enabled_(false), flex_filter_config_{}, sleep_config_{},
      hold_enabled_(false), rtc_gpio_enabled_(false), wakeup_config_{},
      glitch_filter_handle_(nullptr), rtc_gpio_handle_(nullptr) {
  // Constructor delegates to BaseGpio base class
  // Initialize interrupt state and advanced features

  // Initialize sleep configuration to safe defaults
  sleep_config_.sleep_direction = Direction::Input;
  sleep_config_.sleep_pull_mode = PullMode::Floating;
  sleep_config_.sleep_output_enable = false;
  sleep_config_.sleep_input_enable = true;
  sleep_config_.hold_during_sleep = false;

  // Initialize wake-up configuration
  wakeup_config_.wake_trigger = InterruptTrigger::None;
  wakeup_config_.enable_rtc_wake = false;
  wakeup_config_.enable_ext1_wake = false;
  wakeup_config_.wake_level = 0;
}

//==============================================================================
// Destructor
//==============================================================================

McuGpio::~McuGpio() {
  // Disable interrupts before cleanup
  if (interrupt_enabled_) {
    DisableInterrupt();
  }

  // Clean up glitch filter resources
#ifdef HF_MCU_FAMILY_ESP32
  if (glitch_filter_handle_ != nullptr) {
    gpio_glitch_filter_disable(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
    gpio_del_glitch_filter(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
    glitch_filter_handle_ = nullptr;
  }
  
  if (rtc_gpio_handle_ != nullptr) {
    gpio_glitch_filter_disable(static_cast<gpio_glitch_filter_handle_t>(rtc_gpio_handle_));
    gpio_del_glitch_filter(static_cast<gpio_glitch_filter_handle_t>(rtc_gpio_handle_));
    rtc_gpio_handle_ = nullptr;
  }
#endif

  CleanupInterruptSemaphore();
  
  ESP_LOGD(TAG, "McuGpio destroyed for pin %d", pin_);
}

//==============================================================================
// BaseGpio Implementation
//==============================================================================

bool McuGpio::Initialize() noexcept {
  if (IsInitialized()) {
    ESP_LOGW(TAG, "Pin %d already initialized", pin_);
    return true;
  }

  InitializeInterruptSemaphore();

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
  if (result != HfGpioErr::GPIO_SUCCESS) {
    ESP_LOGE(TAG, "Failed to configure pin %d: %s", pin_, HfGpioErrToString(result));
    return false;
  }

  ESP_LOGI(TAG, "Initialized pin %d as %s, %s, %s", pin_, ToString(current_direction_),
           ToString(active_state_), ToString(pull_mode_));

  return true;
}

bool McuGpio::Deinitialize() noexcept {
  if (!IsInitialized()) {
    return true;
  }

#ifdef HF_MCU_FAMILY_ESP32
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

bool McuGpio::IsPinAvailable() const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
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
  case 12:
  case 13:
    return false;

  // Boot strapping pins - use with caution
  case 8:
  case 9:
    ESP_LOGW(TAG, "Pin %d is a bootstrap pin - use with caution", pin_);
    return true;

  // Valid GPIO pins for ESP32-C6
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 10:
  case 11:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30:
    return true;

  default:
    return false;
  }

#else
  // ESP32 Classic pin availability checks
  switch (pin_) {
  // Input-only pins on ESP32 Classic
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
    return (current_direction_ == Direction::Input);

  // Flash pins - typically reserved
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
    return false;

  // Valid GPIO pins for ESP32 Classic
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 21:
  case 22:
  case 23:
  case 25:
  case 26:
  case 27:
  case 32:
  case 33:
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

uint8_t McuGpio::GetMaxPins() const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  return GPIO_NUM_MAX;
#else
  return 32; // Default fallback
#endif
}

const char *McuGpio::GetDescription() const noexcept {
  return "McuGpio - Unified MCU GPIO with dynamic mode switching";
}

bool McuGpio::SupportsInterrupts() const noexcept {
  return true; // Most MCU GPIOs support interrupts
}

//==============================================================================
// BaseGpio Pure Virtual Implementations
//==============================================================================

HfGpioErr McuGpio::SetDirectionImpl(Direction direction) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  gpio_config_t io_conf = {};
  io_conf.pin_bit_mask = (1ULL << pin_);

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
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set direction for pin %d: %s", pin_, esp_err_to_name(result));
    return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
  }

  ESP_LOGD(TAG, "Set pin %d direction to %s", pin_, ToString(direction));
  return HfGpioErr::GPIO_SUCCESS;
#else
  return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuGpio::SetOutputModeImpl(OutputMode mode) noexcept {
  // For MCUs, output mode change typically requires reconfiguring the pin
  // if it's currently an output
  if (current_direction_ == Direction::Output) {
    return SetDirectionImpl(Direction::Output); // Reconfigure with new mode
  }
  ESP_LOGD(TAG, "Set pin %d output mode to %s (will apply on next output config)", pin_,
           ToString(mode));
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::SetActiveImpl() noexcept {
  if (current_direction_ != Direction::Output) {
    return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
  }

#ifdef HF_MCU_FAMILY_ESP32
  bool level = StateToLevel(State::Active);
  esp_err_t result = gpio_set_level(static_cast<gpio_num_t>(pin_), level ? 1 : 0);

  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set pin %d active: %s", pin_, esp_err_to_name(result));
    return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
  }

  ESP_LOGV(TAG, "Set pin %d to active (%s)", pin_, level ? "HIGH" : "LOW");
  return HfGpioErr::GPIO_SUCCESS;
#else
  return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuGpio::SetInactiveImpl() noexcept {
  if (current_direction_ != Direction::Output) {
    return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
  }

#ifdef HF_MCU_FAMILY_ESP32
  bool level = StateToLevel(State::Inactive);
  esp_err_t result = gpio_set_level(static_cast<gpio_num_t>(pin_), level ? 1 : 0);

  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set pin %d inactive: %s", pin_, esp_err_to_name(result));
    return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
  }

  ESP_LOGV(TAG, "Set pin %d to inactive (%s)", pin_, level ? "HIGH" : "LOW");
  return HfGpioErr::GPIO_SUCCESS;
#else
  return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuGpio::IsActiveImpl(bool &is_active) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  int level = gpio_get_level(static_cast<gpio_num_t>(pin_));
  if (level < 0) {
    ESP_LOGE(TAG, "Failed to read pin %d level", pin_);
    return HfGpioErr::GPIO_ERR_READ_FAILURE;
  }

  State current_state = LevelToState(level != 0);
  is_active = (current_state == State::Active);
  ESP_LOGV(TAG, "Read pin %d: %s (%s)", pin_, is_active ? "ACTIVE" : "INACTIVE",
           level ? "HIGH" : "LOW");

  return HfGpioErr::GPIO_SUCCESS;
#else
  return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuGpio::ToggleImpl() noexcept {
  if (current_direction_ != Direction::Output) {
    return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Read current level and invert it
  int current_level = gpio_get_level(static_cast<gpio_num_t>(pin_));
  if (current_level < 0) {
    ESP_LOGE(TAG, "Failed to read pin %d level for toggle", pin_);
    return HfGpioErr::GPIO_ERR_READ_FAILURE;
  }

  int new_level = current_level ? 0 : 1;
  esp_err_t result = gpio_set_level(static_cast<gpio_num_t>(pin_), new_level);

  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to toggle pin %d: %s", pin_, esp_err_to_name(result));
    return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
  }

  ESP_LOGV(TAG, "Toggled pin %d from %s to %s", pin_, current_level ? "HIGH" : "LOW",
           new_level ? "HIGH" : "LOW");
  return HfGpioErr::GPIO_SUCCESS;
#else
  return HfGpioErr::GPIO_ERR_UNSUPPORTED_OPERATION;
#endif
}

HfGpioErr McuGpio::SetPullModeImpl(PullMode mode) noexcept {
  // For MCUs, changing pull mode typically requires reconfiguring the pin
  return SetDirectionImpl(current_direction_); // Reconfigure with new pull mode
}

BaseGpio::PullMode McuGpio::GetPullModeImpl() const noexcept {
  // Return cached pull mode - querying from hardware registers is complex
  // and not always reliable across different MCU platforms
  return pull_mode_;
}

//==============================================================================
// Interrupt Functionality Implementation
//==============================================================================

HfGpioErr McuGpio::ConfigureInterrupt(InterruptTrigger trigger, InterruptCallback callback,
                                      void *user_data) noexcept {
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

#ifdef HF_MCU_FAMILY_ESP32
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
  io_conf.pull_down_en =
      (pull_mode_ == PullMode::PullDown) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
  io_conf.intr_type = static_cast<gpio_int_type_t>(platform_trigger);

  esp_err_t result = gpio_config(&io_conf);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure interrupt for pin %d: %s", pin_, esp_err_to_name(result));
    return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
  }
#endif

  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::EnableInterrupt() noexcept {
  if (interrupt_enabled_) {
    return HfGpioErr::GPIO_ERR_INTERRUPT_ALREADY_ENABLED;
  }

  if (interrupt_trigger_ == InterruptTrigger::None) {
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_FAMILY_ESP32
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

HfGpioErr McuGpio::DisableInterrupt() noexcept {
  if (!interrupt_enabled_) {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_ENABLED;
  }

#ifdef HF_MCU_FAMILY_ESP32
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

HfGpioErr McuGpio::WaitForInterrupt(uint32_t timeout_ms) noexcept {
  if (!interrupt_enabled_) {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_ENABLED;
  }

  if (!platform_semaphore_) {
    return HfGpioErr::GPIO_ERR_SYSTEM_ERROR;
  }

#ifdef HF_MCU_FAMILY_ESP32
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

HfGpioErr McuGpio::GetInterruptStatus(InterruptStatus &status) noexcept {
  status.is_enabled = interrupt_enabled_;
  status.trigger_type = interrupt_trigger_;
  status.interrupt_count = interrupt_count_;
  status.has_callback = (interrupt_callback_ != nullptr);

  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::ClearInterruptStats() noexcept {
  interrupt_count_ = 0;
  return HfGpioErr::GPIO_SUCCESS;
}

//==============================================================================
// Interrupt Helper Methods
//==============================================================================

uint32_t McuGpio::ConvertInterruptTrigger(InterruptTrigger trigger) const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  switch (trigger) {
  case InterruptTrigger::RisingEdge:
    return gpio_int_type_t::GPIO_INTR_POSEDGE;
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

void IRAM_ATTR McuGpio::InterruptHandler(void *arg) noexcept {
  auto *self = static_cast<McuGpio *>(arg);
  if (!self)
    return;

  // Increment interrupt counter
  self->interrupt_count_++;

#ifdef HF_MCU_FAMILY_ESP32
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

void McuGpio::InitializeInterruptSemaphore() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  platform_semaphore_ = xSemaphoreCreateBinary();
  if (!platform_semaphore_) {
    ESP_LOGE(TAG, "Failed to create interrupt semaphore for pin %d", pin_);
  }
#endif
}

void McuGpio::CleanupInterruptSemaphore() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  if (platform_semaphore_) {
    vSemaphoreDelete(platform_semaphore_);
    platform_semaphore_ = nullptr;
  }
#endif
}

//==============================================================================
// ADVANCED GPIO FEATURES IMPLEMENTATION
//==============================================================================

HfGpioErr McuGpio::SetDriveCapability(GpioDriveCapability capability) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  if (!IsInitialized()) {
    drive_capability_ = capability; // Store for later application
    return HfGpioErr::GPIO_SUCCESS;
  }

  gpio_drive_cap_t esp_drive_cap;
  switch (capability) {
  case GpioDriveCapability::Weak:
    esp_drive_cap = GPIO_DRIVE_CAP_0; // ~5mA
    break;
  case GpioDriveCapability::Stronger:
    esp_drive_cap = GPIO_DRIVE_CAP_1; // ~10mA
    break;
  case GpioDriveCapability::Medium:
    esp_drive_cap = GPIO_DRIVE_CAP_2; // ~20mA
    break;
  case GpioDriveCapability::Strongest:
    esp_drive_cap = GPIO_DRIVE_CAP_3; // ~40mA
    break;
  default:
    return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
  }

  esp_err_t err = gpio_set_drive_capability(static_cast<gpio_num_t>(pin_), esp_drive_cap);
  if (err == ESP_OK) {
    drive_capability_ = capability;
    ESP_LOGD(TAG, "Set GPIO %d drive capability to %dmA", pin_, 
             capability == GpioDriveCapability::Weak ? 5 :
             capability == GpioDriveCapability::Stronger ? 10 :
             capability == GpioDriveCapability::Medium ? 20 : 40);
    return HfGpioErr::GPIO_SUCCESS;
  } else {
    ESP_LOGE(TAG, "Failed to set drive capability for GPIO %d: %s", pin_, esp_err_to_name(err));
    return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
  }
#else
  // Store capability for non-ESP32 platforms
  drive_capability_ = capability;
  return HfGpioErr::GPIO_SUCCESS;
#endif
}

bool McuGpio::SupportsGlitchFilter() const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  return true; // ESP32C6 supports glitch filters
#else
  return false;
#endif
}

HfGpioErr McuGpio::ConfigurePinGlitchFilter(bool enable) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  if (!IsInitialized()) {
    pin_glitch_filter_enabled_ = enable;
    return HfGpioErr::GPIO_SUCCESS;
  }

  gpio_num_t pin = static_cast<gpio_num_t>(pin_);

  if (enable) {
    // Create pin glitch filter using ESP-IDF v5.5+ API
    gpio_pin_glitch_filter_config_t filter_config = {
        .clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT,
        .gpio_num = pin,
    };

    gpio_glitch_filter_handle_t filter_handle = nullptr;
    esp_err_t err = gpio_new_pin_glitch_filter(&filter_config, &filter_handle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create pin glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    }

    // Enable the filter
    err = gpio_glitch_filter_enable(filter_handle);
    if (err != ESP_OK) {
      gpio_del_glitch_filter(filter_handle);
      ESP_LOGE(TAG, "Failed to enable pin glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    }

    // Clean up old filter if exists
    if (glitch_filter_handle_ != nullptr) {
      gpio_glitch_filter_disable(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
      gpio_del_glitch_filter(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
    }

    glitch_filter_handle_ = filter_handle;
    pin_glitch_filter_enabled_ = true;

    // Update filter type state
    if (glitch_filter_type_ == GpioGlitchFilterType::None) {
      glitch_filter_type_ = GpioGlitchFilterType::Pin;
    } else if (glitch_filter_type_ == GpioGlitchFilterType::Flex) {
      glitch_filter_type_ = GpioGlitchFilterType::Both;
    }

    ESP_LOGI(TAG, "Pin glitch filter enabled for GPIO %d", pin_);
  } else {
    // Disable and clean up pin glitch filter
    if (glitch_filter_handle_ != nullptr) {
      esp_err_t err = gpio_glitch_filter_disable(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to disable pin glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      }

      err = gpio_del_glitch_filter(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
      if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to delete pin glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      }

      glitch_filter_handle_ = nullptr;
    }

    pin_glitch_filter_enabled_ = false;

    // Update filter type state
    if (glitch_filter_type_ == GpioGlitchFilterType::Pin) {
      glitch_filter_type_ = GpioGlitchFilterType::None;
    } else if (glitch_filter_type_ == GpioGlitchFilterType::Both) {
      glitch_filter_type_ = GpioGlitchFilterType::Flex;
    }

    ESP_LOGI(TAG, "Pin glitch filter disabled for GPIO %d", pin_);
  }

  return HfGpioErr::GPIO_SUCCESS;
#else
  pin_glitch_filter_enabled_ = enable;
  return HfGpioErr::GPIO_SUCCESS;
#endif
}

HfGpioErr McuGpio::ConfigureFlexGlitchFilter(const FlexGlitchFilterConfig &config) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  if (!IsInitialized()) {
    flex_filter_config_ = config;
    flex_glitch_filter_enabled_ = true;
    return HfGpioErr::GPIO_SUCCESS;
  }

  gpio_num_t pin = static_cast<gpio_num_t>(pin_);

  // Create flexible glitch filter using ESP-IDF v5.5+ API
  gpio_flex_glitch_filter_config_t filter_config = {
      .clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT,
      .gpio_num = pin,
      .window_width_ns = config.window_width_ns,
      .window_thres_ns = config.window_threshold_ns,
  };

  gpio_glitch_filter_handle_t filter_handle = nullptr;
  esp_err_t err = gpio_new_flex_glitch_filter(&filter_config, &filter_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create flex glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
    if (err == ESP_ERR_NOT_FOUND) {
      return HfGpioErr::GPIO_ERR_RESOURCE_UNAVAILABLE;
    } else if (err == ESP_ERR_INVALID_ARG) {
      return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
    } else {
      return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    }
  }

  // Enable the filter if requested
  if (config.enable_on_init) {
    err = gpio_glitch_filter_enable(filter_handle);
    if (err != ESP_OK) {
      gpio_del_glitch_filter(filter_handle);
      ESP_LOGE(TAG, "Failed to enable flex glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    }
  }

  // Clean up old filter if exists and store new one
  // Note: We use a separate handle for flex filter vs pin filter
  if (rtc_gpio_handle_ != nullptr) {  // Reusing this handle for flex filter
    gpio_glitch_filter_disable(static_cast<gpio_glitch_filter_handle_t>(rtc_gpio_handle_));
    gpio_del_glitch_filter(static_cast<gpio_glitch_filter_handle_t>(rtc_gpio_handle_));
  }

  rtc_gpio_handle_ = filter_handle;
  flex_filter_config_ = config;
  flex_glitch_filter_enabled_ = true;

  // Update filter type state
  if (glitch_filter_type_ == GpioGlitchFilterType::None) {
    glitch_filter_type_ = GpioGlitchFilterType::Flex;
  } else if (glitch_filter_type_ == GpioGlitchFilterType::Pin) {
    glitch_filter_type_ = GpioGlitchFilterType::Both;
  }

  ESP_LOGI(TAG, "Flex glitch filter configured for GPIO %d (window: %lu ns, threshold: %lu ns)", 
           pin_, config.window_width_ns, config.window_threshold_ns);

  return HfGpioErr::GPIO_SUCCESS;
#else
  flex_filter_config_ = config;
  flex_glitch_filter_enabled_ = true;
  return HfGpioErr::GPIO_SUCCESS;
#endif
}

HfGpioErr McuGpio::EnableGlitchFilters() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  HfGpioErr result = HfGpioErr::GPIO_SUCCESS;

  // Enable pin glitch filter if configured
  if (pin_glitch_filter_enabled_ && glitch_filter_handle_ != nullptr) {
    esp_err_t err = gpio_glitch_filter_enable(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable pin glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      result = HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    } else {
      ESP_LOGD(TAG, "Pin glitch filter enabled for GPIO %d", pin_);
    }
  }

  // Enable flex glitch filter if configured
  if (flex_glitch_filter_enabled_ && rtc_gpio_handle_ != nullptr) {
    esp_err_t err = gpio_glitch_filter_enable(static_cast<gpio_glitch_filter_handle_t>(rtc_gpio_handle_));
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable flex glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      result = HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    } else {
      ESP_LOGD(TAG, "Flex glitch filter enabled for GPIO %d", pin_);
    }
  }

  if (result == HfGpioErr::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "All glitch filters enabled for GPIO %d", pin_);
  }

  return result;
#else
  return HfGpioErr::GPIO_SUCCESS;
#endif
}

HfGpioErr McuGpio::DisableGlitchFilters() noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  HfGpioErr result = HfGpioErr::GPIO_SUCCESS;

  // Disable pin glitch filter if active
  if (pin_glitch_filter_enabled_ && glitch_filter_handle_ != nullptr) {
    esp_err_t err = gpio_glitch_filter_disable(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
    if (err != ESP_OK) {
      ESP_LOGW(TAG, "Failed to disable pin glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      result = HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    } else {
      ESP_LOGD(TAG, "Pin glitch filter disabled for GPIO %d", pin_);
    }
  }

  // Disable flex glitch filter if active
  if (flex_glitch_filter_enabled_ && rtc_gpio_handle_ != nullptr) {
    esp_err_t err = gpio_glitch_filter_disable(static_cast<gpio_glitch_filter_handle_t>(rtc_gpio_handle_));
    if (err != ESP_OK) {
      ESP_LOGW(TAG, "Failed to disable flex glitch filter for GPIO %d: %s", pin_, esp_err_to_name(err));
      result = HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    } else {
      ESP_LOGD(TAG, "Flex glitch filter disabled for GPIO %d", pin_);
    }
  }

  if (result == HfGpioErr::GPIO_SUCCESS) {
    ESP_LOGI(TAG, "All glitch filters disabled for GPIO %d", pin_);
  }

  return result;
#else
  return HfGpioErr::GPIO_SUCCESS;
#endif
}

bool McuGpio::SupportsRtcGpio() const noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  // Check if pin supports RTC GPIO functionality
  return rtc_gpio_is_valid_gpio(static_cast<gpio_num_t>(pin_));
#else
  return false;
#endif
}

HfGpioErr McuGpio::ConfigureSleep(const GpioSleepConfig &config) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  sleep_config_ = config;

  if (SupportsRtcGpio()) {
    gpio_num_t pin = static_cast<gpio_num_t>(pin_);

    // Configure sleep direction
    if (config.sleep_direction == Direction::Output) {
      esp_err_t err = rtc_gpio_set_direction(pin, RTC_GPIO_MODE_OUTPUT_ONLY);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set RTC GPIO direction for pin %d: %s", pin_, esp_err_to_name(err));
        return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
      }
    } else {
      esp_err_t err = rtc_gpio_set_direction(pin, RTC_GPIO_MODE_INPUT_ONLY);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set RTC GPIO direction for pin %d: %s", pin_, esp_err_to_name(err));
        return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
      }
    }

    // Configure pull resistors
    rtc_gpio_pullup_dis(pin);
    rtc_gpio_pulldown_dis(pin);
    if (config.sleep_pull_mode == PullMode::PullUp) {
      rtc_gpio_pullup_en(pin);
    } else if (config.sleep_pull_mode == PullMode::PullDown) {
      rtc_gpio_pulldown_en(pin);
    }

    // Configure hold
    if (config.hold_during_sleep) {
      rtc_gpio_hold_en(pin);
    } else {
      rtc_gpio_hold_dis(pin);
    }

    ESP_LOGI(TAG, "Sleep configuration applied for RTC GPIO %d", pin_);
  } else {
    ESP_LOGW(TAG, "Pin %d does not support RTC GPIO, sleep config stored but not applied", pin_);
  }

  return HfGpioErr::GPIO_SUCCESS;
#else
  sleep_config_ = config;
  return HfGpioErr::GPIO_SUCCESS;
#endif
}

HfGpioErr McuGpio::ConfigureHold(bool enable) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  hold_enabled_ = enable;

  if (SupportsRtcGpio()) {
    gpio_num_t pin = static_cast<gpio_num_t>(pin_);
    if (enable) {
      rtc_gpio_hold_en(pin);
      ESP_LOGD(TAG, "RTC GPIO hold enabled for pin %d", pin_);
    } else {
      rtc_gpio_hold_dis(pin);
      ESP_LOGD(TAG, "RTC GPIO hold disabled for pin %d", pin_);
    }
  } else {
    // Use digital GPIO hold for non-RTC pins
    gpio_num_t pin = static_cast<gpio_num_t>(pin_);
    if (enable) {
      gpio_hold_en(pin);
      ESP_LOGD(TAG, "Digital GPIO hold enabled for pin %d", pin_);
    } else {
      gpio_hold_dis(pin);
      ESP_LOGD(TAG, "Digital GPIO hold disabled for pin %d", pin_);
    }
  }

  return HfGpioErr::GPIO_SUCCESS;
#else
  hold_enabled_ = enable;
  return HfGpioErr::GPIO_SUCCESS;
#endif
}

HfGpioErr McuGpio::ConfigureWakeUp(const GpioWakeUpConfig &config) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  wakeup_config_ = config;

  if (config.enable_rtc_wake && SupportsRtcGpio()) {
    gpio_num_t pin = static_cast<gpio_num_t>(pin_);

    // Configure as RTC GPIO for wake-up
    esp_err_t err = rtc_gpio_init(pin);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize RTC GPIO %d: %s", pin_, esp_err_to_name(err));
      return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    }

    // Set direction for wake-up
    err = rtc_gpio_set_direction(pin, RTC_GPIO_MODE_INPUT_ONLY);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set RTC GPIO direction for pin %d: %s", pin_, esp_err_to_name(err));
      return HfGpioErr::GPIO_ERR_HARDWARE_FAULT;
    }

    // Configure wake-up trigger
    if (config.wake_trigger == InterruptTrigger::RisingEdge ||
        config.wake_trigger == InterruptTrigger::BothEdges) {
      esp_sleep_enable_gpio_wakeup();
      ESP_LOGI(TAG, "GPIO wake-up enabled for pin %d", pin_);
    }

    ESP_LOGI(TAG, "Wake-up configuration applied for RTC GPIO %d", pin_);
  } else if (config.enable_rtc_wake) {
    ESP_LOGW(TAG, "Pin %d does not support RTC GPIO, wake-up config stored but not applied", pin_);
  }

  return HfGpioErr::GPIO_SUCCESS;
#else
  wakeup_config_ = config;
  return HfGpioErr::GPIO_SUCCESS;
#endif
}

GpioConfigDump McuGpio::GetConfigurationDump() const noexcept {
  GpioConfigDump dump;

  dump.pin_number = static_cast<uint8_t>(pin_);
  dump.direction = GetDirection();
  dump.pull_mode = GetPullMode();
  dump.output_mode = GetOutputMode();
  dump.drive_capability = drive_capability_;
  dump.input_enabled = (GetDirection() == Direction::Input);
  dump.output_enabled = (GetDirection() == Direction::Output);
  dump.open_drain = (GetOutputMode() == OutputMode::OpenDrain);
  dump.sleep_sel_enabled = false; // Would be read from hardware
  dump.function_select = 0;       // Would be read from IOMUX
  dump.is_rtc_gpio = SupportsRtcGpio();
  dump.glitch_filter_enabled = (glitch_filter_type_ != GpioGlitchFilterType::None);
  dump.filter_type = glitch_filter_type_;

  return dump;
}

bool McuGpio::IsHeld() const noexcept {
  return hold_enabled_;
}
