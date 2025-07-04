/**
 * @file McuGpio.cpp
 * @brief Production-quality ESP32C6 GPIO implementation with ESP-IDF v5.5+ advanced features.
 *
 * This file contains a world-class implementation of MCU-specific GPIO operations
 * for the BaseGpio class with comprehensive ESP32C6/ESP-IDF v5.5+ feature support.
 * Features include dynamic mode switching, advanced glitch filtering, RTC GPIO,
 * power management, sleep configuration, wake-up support, interrupt handling,
 * and comprehensive diagnostics for industrial-grade applications.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 * 
 * @note This implementation represents production-ready, industrial-grade GPIO
 *       control suitable for mission-critical automotive and industrial applications.
 */

#include "McuGpio.h"
#include <algorithm>
#include <atomic>
#include <cstring>

// Platform-specific includes via centralized McuSelect.h
#ifdef HF_MCU_ESP32C6
// ESP32-C6 specific includes with ESP-IDF v5.5+ features
#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "driver/lp_io.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "hal/gpio_types.h"
#include "hal/rtc_io_types.h"
#include "soc/clk_tree_defs.h"
#include "soc/gpio_sig_map.h"
static const char *TAG = "McuGpio";
#elif defined(HF_MCU_FAMILY_ESP32)
// ESP32 family includes for advanced GPIO features
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "soc/rtc.h"
static const char *TAG = "McuGpio";
#else
// Provide stub implementations for non-ESP32 platforms
static const char *TAG = "McuGpio";
#define ESP_LOGE(tag, format, ...)
#define ESP_LOGW(tag, format, ...)
#define ESP_LOGI(tag, format, ...)
#define ESP_LOGD(tag, format, ...)
#define ESP_LOGV(tag, format, ...)
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_ARG -2
#define ESP_ERR_NO_MEM -3
#define ESP_ERR_TIMEOUT -4
#endif

namespace {
  // Thread-safe interrupt statistics tracking
  std::atomic<uint32_t> g_total_gpio_interrupts{0};
  std::atomic<uint32_t> g_active_gpio_count{0};
  
  // GPIO pin capabilities lookup table for ESP32C6
  #ifdef HF_MCU_ESP32C6
  constexpr hf_gpio_pin_capabilities_t GPIO_PIN_CAPABILITIES[HF_MCU_GPIO_PIN_COUNT] = {
    // GPIO0-GPIO7: ADC and RTC capable
    {true, true, true, false, false, false, false, 0, 1, 0},  // GPIO0
    {true, true, true, false, false, false, false, 1, 1, 1},  // GPIO1
    {true, true, true, false, false, false, false, 2, 1, 2},  // GPIO2
    {true, true, true, false, false, false, false, 3, 1, 3},  // GPIO3
    {true, true, true, false, true, false, false, 4, 1, 4},   // GPIO4 (strapping)
    {true, true, true, false, true, false, false, 5, 1, 5},   // GPIO5 (strapping)
    {true, true, true, false, false, false, false, 6, 1, 6},  // GPIO6
    {true, false, true, false, false, false, false, 7, 0, 0}, // GPIO7
    // GPIO8-GPIO11: Regular GPIOs
    {true, false, false, false, true, false, false, 255, 0, 0}, // GPIO8 (strapping)
    {true, false, false, false, true, false, false, 255, 0, 0}, // GPIO9 (strapping)
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO10
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO11
    // GPIO12-GPIO13: USB-JTAG pins
    {true, false, false, false, false, false, true, 255, 0, 0}, // GPIO12 (USB-JTAG)
    {true, false, false, false, false, false, true, 255, 0, 0}, // GPIO13 (USB-JTAG)
    // GPIO14: Not available on some variants
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO14
    // GPIO15: Strapping pin
    {true, false, false, false, true, false, false, 255, 0, 0}, // GPIO15 (strapping)
    // GPIO16-GPIO23: Regular GPIOs
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO16
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO17
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO18
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO19
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO20
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO21
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO22
    {true, false, false, false, false, false, false, 255, 0, 0}, // GPIO23
    // GPIO24-GPIO30: SPI flash pins (not recommended for GPIO)
    {true, false, false, false, false, true, false, 255, 0, 0}, // GPIO24 (SPI flash)
    {true, false, false, false, false, true, false, 255, 0, 0}, // GPIO25 (SPI flash)
    {true, false, false, false, false, true, false, 255, 0, 0}, // GPIO26 (SPI flash)
    {true, false, false, false, false, true, false, 255, 0, 0}, // GPIO27 (SPI flash)
    {true, false, false, false, false, true, false, 255, 0, 0}, // GPIO28 (SPI flash)
    {true, false, false, false, false, true, false, 255, 0, 0}, // GPIO29 (SPI flash)
    {true, false, false, false, false, true, false, 255, 0, 0}, // GPIO30 (SPI flash)
  };
  #endif
} // anonymous namespace

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
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
      glitch_filter_handle_(nullptr), rtc_gpio_handle_(nullptr), initialized_(false) {

  // Validate pin number for target platform
  if (!HF_GPIO_IS_VALID_GPIO(pin_num)) {
    ESP_LOGE(TAG, "Invalid GPIO pin number: %d", static_cast<int>(pin_num));
    return;
  }

  // Check for special pins and log warnings (informational only, no hardware access)
  #ifdef HF_MCU_ESP32C6
  if (HF_GPIO_IS_STRAPPING_PIN(pin_num)) {
    ESP_LOGW(TAG, "GPIO%d is a strapping pin - use with caution", static_cast<int>(pin_num));
  }
  
  if (HF_GPIO_IS_SPI_FLASH_PIN(pin_num)) {
    ESP_LOGW(TAG, "GPIO%d is typically used for SPI flash - not recommended for general GPIO", 
             static_cast<int>(pin_num));
  }
  
  if (HF_GPIO_IS_USB_JTAG_PIN(pin_num)) {
    ESP_LOGW(TAG, "GPIO%d is used for USB-JTAG - JTAG will be disabled if reconfigured", 
             static_cast<int>(pin_num));
  }
  #endif

  // Initialize advanced feature configurations to safe defaults
  sleep_config_.sleep_direction = Direction::Input;
  sleep_config_.sleep_pull_mode = PullMode::Floating;
  sleep_config_.sleep_output_enable = false;
  sleep_config_.sleep_input_enable = true;
  sleep_config_.hold_during_sleep = false;

  wakeup_config_.wake_trigger = InterruptTrigger::None;
  wakeup_config_.enable_rtc_wake = false;
  wakeup_config_.enable_ext1_wake = false;
  wakeup_config_.wake_level = 0;

  flex_filter_config_.window_width_ns = 0;
  flex_filter_config_.window_threshold_ns = 0;
  flex_filter_config_.enable_on_init = false;
  // Increment active GPIO count for statistics
  g_active_gpio_count.fetch_add(1, std::memory_order_relaxed);
  
  ESP_LOGD(TAG, "Created McuGpio instance for pin %d (LAZY INIT) with drive capability %d", 
           static_cast<int>(pin_num), static_cast<int>(drive_capability));
}

McuGpio::~McuGpio() {
  ESP_LOGD(TAG, "Destroying McuGpio instance for pin %d", static_cast<int>(pin_));
  
  // Disable interrupts before cleanup
  if (interrupt_enabled_) {
    DisableInterrupt();
  }

  // Clean up advanced feature resources
  CleanupAdvancedFeatures();
  
  // Clean up interrupt semaphore
  CleanupInterruptSemaphore();
  
  // Decrement active GPIO count
  g_active_gpio_count.fetch_sub(1, std::memory_order_relaxed);
  
  ESP_LOGD(TAG, "McuGpio instance destroyed for pin %d", static_cast<int>(pin_));
}

//==============================================================================
// BASEGPIO INTERFACE IMPLEMENTATION
//==============================================================================

bool McuGpio::Initialize() noexcept {
  if (initialized_) {
    ESP_LOGW(TAG, "GPIO%d already initialized", static_cast<int>(pin_));
    return true;
  }

  ESP_LOGI(TAG, "Initializing GPIO%d with advanced ESP32C6 features", static_cast<int>(pin_));

  #ifdef HF_MCU_FAMILY_ESP32
  // Configure GPIO using ESP-IDF v5.5+ advanced configuration
  gpio_config_t io_conf = {};
  
  // Set GPIO direction
  switch (current_direction_) {
    case Direction::Input:
      io_conf.mode = GPIO_MODE_INPUT;
      break;
    case Direction::Output:
      if (output_mode_ == OutputMode::OpenDrain) {
        io_conf.mode = GPIO_MODE_OUTPUT_OD;
      } else {
        io_conf.mode = GPIO_MODE_OUTPUT;
      }
      break;
  }

  // Configure pull resistors
  switch (pull_mode_) {
    case PullMode::Floating:
      io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      break;
    case PullMode::PullUp:
      io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
      break;
    case PullMode::PullDown:
      io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
      io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
      break;
  }

  // Set interrupt type
  io_conf.intr_type = MapInterruptTrigger(interrupt_trigger_);
  
  // Set pin bit mask
  io_conf.pin_bit_mask = (1ULL << static_cast<gpio_num_t>(pin_));

  // Apply configuration
  esp_err_t ret = gpio_config(&io_conf);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure GPIO%d: %s", static_cast<int>(pin_), esp_err_to_name(ret));
    return false;
  }

  // Set drive capability
  if (current_direction_ == Direction::Output) {
    ret = SetDriveCapability(drive_capability_);
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "Failed to set drive capability for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
    }
  }

  // Initialize advanced features if requested
  if (!InitializeAdvancedFeatures()) {
    ESP_LOGW(TAG, "Some advanced features failed to initialize for GPIO%d", static_cast<int>(pin_));
    // Continue initialization - basic GPIO functionality should still work
  }

  #else
  // Stub implementation for non-ESP32 platforms
  ESP_LOGW(TAG, "GPIO initialization stubbed for non-ESP32 platform");
  #endif

  initialized_ = true;
  ESP_LOGI(TAG, "GPIO%d initialized successfully", static_cast<int>(pin_));
  return true;
}

bool McuGpio::Deinitialize() noexcept {
  if (!initialized_) {
    return true;
  }

  ESP_LOGI(TAG, "Deinitializing GPIO%d", static_cast<int>(pin_));

  // Disable interrupts first
  if (interrupt_enabled_) {
    DisableInterrupt();
  }

  // Clean up advanced features
  CleanupAdvancedFeatures();

  #ifdef HF_MCU_FAMILY_ESP32
  // Reset GPIO to default state
  esp_err_t ret = gpio_reset_pin(static_cast<gpio_num_t>(pin_));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to reset GPIO%d: %s", static_cast<int>(pin_), esp_err_to_name(ret));
    return false;
  }
  #endif

  initialized_ = false;
  ESP_LOGI(TAG, "GPIO%d deinitialized successfully", static_cast<int>(pin_));
  return true;
}

bool McuGpio::IsPinAvailable() const noexcept {
  #ifdef HF_MCU_ESP32C6
  return pin_ >= 0 && pin_ <= HF_MCU_GPIO_MAX_PIN_NUMBER && 
         GPIO_PIN_CAPABILITIES[pin_].is_valid_gpio;
  #else
  return pin_ >= 0 && pin_ < 32; // Generic validation
  #endif
}

uint8_t McuGpio::GetMaxPins() const noexcept {
  #ifdef HF_MCU_ESP32C6
  return HF_MCU_GPIO_PIN_COUNT;
  #else
  return 32; // Generic default
  #endif
}

const char *McuGpio::GetDescription() const noexcept {
  static char desc_buffer[128];
  #ifdef HF_MCU_ESP32C6
  const auto& caps = GPIO_PIN_CAPABILITIES[pin_];
  snprintf(desc_buffer, sizeof(desc_buffer), 
           "ESP32C6 GPIO%d (ADC:%s, RTC:%s, Strapping:%s)", 
           static_cast<int>(pin_),
           caps.supports_adc ? "Yes" : "No",
           caps.supports_rtc ? "Yes" : "No", 
           caps.is_strapping_pin ? "Yes" : "No");
  #else
  snprintf(desc_buffer, sizeof(desc_buffer), "MCU GPIO%d", static_cast<int>(pin_));
  #endif
  return desc_buffer;
}
  
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
  initialized_ = true;
  return initialized_;
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
  return true; // All ESP32C6 GPIOs support interrupts
}

//==============================================================================
// INTERRUPT FUNCTIONALITY IMPLEMENTATION
//==============================================================================

HfGpioErr McuGpio::ConfigureInterrupt(InterruptTrigger trigger, InterruptCallback callback,
                                      void *user_data) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }

  ESP_LOGD(TAG, "Configuring interrupt for GPIO%d with trigger type %d", 
           static_cast<int>(pin_), static_cast<int>(trigger));

  interrupt_trigger_ = trigger;
  interrupt_callback_ = callback;
  interrupt_user_data_ = user_data;

  #ifdef HF_MCU_FAMILY_ESP32
  // Update GPIO interrupt configuration
  esp_err_t ret = gpio_set_intr_type(static_cast<gpio_num_t>(pin_), MapInterruptTrigger(trigger));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set interrupt type for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_INTERRUPT_HANDLER_FAILED;
  }

  // Install ISR handler if not already done
  if (interrupt_callback_ && !gpio_isr_handler_installed_) {
    ret = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
      ESP_LOGE(TAG, "Failed to install GPIO ISR service: %s", esp_err_to_name(ret));
      return HfGpioErr::GPIO_ERR_INTERRUPT_HANDLER_FAILED;
    }
    gpio_isr_handler_installed_ = true;
  }

  // Add ISR handler for this pin
  if (interrupt_callback_) {
    ret = gpio_isr_handler_add(static_cast<gpio_num_t>(pin_), StaticInterruptHandler, this);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
      ESP_LOGE(TAG, "Failed to add ISR handler for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      return HfGpioErr::GPIO_ERR_INTERRUPT_HANDLER_FAILED;
    }
  }
  #endif

  ESP_LOGI(TAG, "Interrupt configured successfully for GPIO%d", static_cast<int>(pin_));
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::EnableInterrupt() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }

  if (interrupt_trigger_ == InterruptTrigger::None) {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_ENABLED;
  }

  #ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret = gpio_intr_enable(static_cast<gpio_num_t>(pin_));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable interrupt for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_INTERRUPT_HANDLER_FAILED;
  }
  #endif

  interrupt_enabled_ = true;
  ESP_LOGD(TAG, "Interrupt enabled for GPIO%d", static_cast<int>(pin_));
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::DisableInterrupt() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  if (!interrupt_enabled_) {
    return HfGpioErr::GPIO_SUCCESS;
  }

  #ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret = gpio_intr_disable(static_cast<gpio_num_t>(pin_));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disable interrupt for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_INTERRUPT_HANDLER_FAILED;
  }

  // Remove ISR handler
  gpio_isr_handler_remove(static_cast<gpio_num_t>(pin_));
  #endif

  interrupt_enabled_ = false;
  ESP_LOGD(TAG, "Interrupt disabled for GPIO%d", static_cast<int>(pin_));
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::WaitForInterrupt(uint32_t timeout_ms) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  if (!interrupt_enabled_) {
    return HfGpioErr::GPIO_ERR_INTERRUPT_NOT_ENABLED;
  }

  // Create semaphore if not exists
  if (!platform_semaphore_) {
    #ifdef HF_MCU_FAMILY_ESP32
    platform_semaphore_ = xSemaphoreCreateBinary();
    if (!platform_semaphore_) {
      ESP_LOGE(TAG, "Failed to create interrupt semaphore for GPIO%d", static_cast<int>(pin_));
      return HfGpioErr::GPIO_ERR_OUT_OF_MEMORY;
    }
    #endif
  }

  #ifdef HF_MCU_FAMILY_ESP32
  TickType_t ticks_to_wait = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  BaseType_t result = xSemaphoreTake(static_cast<SemaphoreHandle_t>(platform_semaphore_), ticks_to_wait);
  
  if (result == pdTRUE) {
    return HfGpioErr::GPIO_SUCCESS;
  } else {
    return HfGpioErr::GPIO_ERR_TIMEOUT;
  }
  #else
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
  #endif
}

HfGpioErr McuGpio::GetInterruptStatus(InterruptStatus &status) noexcept {
  status.is_enabled = interrupt_enabled_;
  status.trigger_type = interrupt_trigger_;
  status.interrupt_count = interrupt_count_.load(std::memory_order_relaxed);
  status.has_callback = (interrupt_callback_ != nullptr);
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::ClearInterruptStats() noexcept {
  interrupt_count_.store(0, std::memory_order_relaxed);
  return HfGpioErr::GPIO_SUCCESS;
}

//==============================================================================
// BASEGPIO PURE VIRTUAL IMPLEMENTATIONS
//==============================================================================

HfGpioErr McuGpio::SetDirectionImpl(Direction direction) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  #ifdef HF_MCU_FAMILY_ESP32
  gpio_mode_t mode;
  switch (direction) {
    case Direction::Input:
      mode = GPIO_MODE_INPUT;
      break;
    case Direction::Output:
      if (output_mode_ == OutputMode::OpenDrain) {
        mode = GPIO_MODE_OUTPUT_OD;
      } else {
        mode = GPIO_MODE_OUTPUT;
      }
      break;
    default:
      return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
  }

  esp_err_t ret = gpio_set_direction(static_cast<gpio_num_t>(pin_), mode);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set direction for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
  }

  // Update drive capability for output pins
  if (direction == Direction::Output) {
    SetDriveCapability(drive_capability_);
  }
  #endif

  current_direction_ = direction;
  ESP_LOGD(TAG, "Set GPIO%d direction to %s", static_cast<int>(pin_), 
           (direction == Direction::Input) ? "input" : "output");
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::SetPullModeImpl(PullMode pull_mode) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  #ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret = ESP_OK;
  
  switch (pull_mode) {
    case PullMode::Floating:
      ret = gpio_set_pull_mode(static_cast<gpio_num_t>(pin_), GPIO_FLOATING);
      break;
    case PullMode::PullUp:
      ret = gpio_set_pull_mode(static_cast<gpio_num_t>(pin_), GPIO_PULLUP_ONLY);
      break;
    case PullMode::PullDown:
      ret = gpio_set_pull_mode(static_cast<gpio_num_t>(pin_), GPIO_PULLDOWN_ONLY);
      break;
    default:
      return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
  }

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set pull mode for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_PULL_RESISTOR_FAILURE;
  }
  #endif

  pull_mode_ = pull_mode;
  ESP_LOGD(TAG, "Set GPIO%d pull mode to %d", static_cast<int>(pin_), static_cast<int>(pull_mode));
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::SetOutputModeImpl(OutputMode output_mode) noexcept {
  output_mode_ = output_mode;
  
  // If already initialized as output, update the mode
  if (initialized_ && current_direction_ == Direction::Output) {
    return SetDirectionImpl(Direction::Output);
  }
  
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::WriteImpl(State state) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  if (current_direction_ != Direction::Output) {
    return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
  }

  // Convert logical state to electrical level based on polarity
  int level = (state == State::Active) ? 
    ((active_state_ == ActiveState::High) ? 1 : 0) :
    ((active_state_ == ActiveState::High) ? 0 : 1);

  #ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret = gpio_set_level(static_cast<gpio_num_t>(pin_), level);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write GPIO%d: %s", static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_WRITE_FAILURE;
  }
  #endif

  current_state_ = state;
  ESP_LOGV(TAG, "GPIO%d set to %s (level %d)", static_cast<int>(pin_), 
           (state == State::Active) ? "active" : "inactive", level);
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::ReadImpl(State &state) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  #ifdef HF_MCU_FAMILY_ESP32
  int level = gpio_get_level(static_cast<gpio_num_t>(pin_));
  
  // Convert electrical level to logical state based on polarity
  state = ((level == 1) == (active_state_ == ActiveState::High)) ? 
    State::Active : State::Inactive;
  
  current_state_ = state;
  return HfGpioErr::GPIO_SUCCESS;
  #else
  state = State::Inactive;
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
  #endif
}

HfGpioErr McuGpio::SetActiveImpl() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  return WriteImpl(State::Active);
}

HfGpioErr McuGpio::SetInactiveImpl() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  return WriteImpl(State::Inactive);
}

HfGpioErr McuGpio::ToggleImpl() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  if (current_direction_ != Direction::Output) {
    return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
  }

  State new_state = (current_state_ == State::Active) ? State::Inactive : State::Active;
  return WriteImpl(new_state);
}

HfGpioErr McuGpio::IsActiveImpl(bool &is_active) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  if (current_direction_ != Direction::Input) {
    return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
  }

  State current_state;
  HfGpioErr result = ReadImpl(current_state);
  if (result == HfGpioErr::GPIO_SUCCESS) {
    is_active = (current_state == State::Active);
  }
  return result;
}

//==============================================================================
// ADVANCED ESP32C6 FEATURES IMPLEMENTATION
//==============================================================================

HfGpioErr McuGpio::SetDriveCapability(GpioDriveCapability capability) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  drive_capability_ = capability;

  #ifdef HF_MCU_FAMILY_ESP32
  gpio_drive_cap_t esp_cap;
  switch (capability) {
    case GpioDriveCapability::Weak:
      esp_cap = GPIO_DRIVE_CAP_0;
      break;
    case GpioDriveCapability::Stronger:
      esp_cap = GPIO_DRIVE_CAP_1;
      break;
    case GpioDriveCapability::Medium:
      esp_cap = GPIO_DRIVE_CAP_2;
      break;
    case GpioDriveCapability::Strongest:
      esp_cap = GPIO_DRIVE_CAP_3;
      break;
    default:
      return HfGpioErr::GPIO_ERR_INVALID_PARAMETER;
  }

  esp_err_t ret = gpio_set_drive_capability(static_cast<gpio_num_t>(pin_), esp_cap);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set drive capability for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  ESP_LOGD(TAG, "Set GPIO%d drive capability to %d", static_cast<int>(pin_), static_cast<int>(capability));
  #endif

  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::ConfigureGlitchFilter(GpioGlitchFilterType filter_type,
                                         const FlexGlitchFilterConfig *flex_config) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }
  
  #ifdef HF_MCU_ESP32C6
  glitch_filter_type_ = filter_type;

  // Clean up existing filters
  CleanupGlitchFilters();

  if (filter_type == GpioGlitchFilterType::None) {
    return HfGpioErr::GPIO_SUCCESS;
  }

  esp_err_t ret;

  // Configure pin glitch filter
  if (filter_type == GpioGlitchFilterType::Pin || filter_type == GpioGlitchFilterType::Both) {
    gpio_pin_glitch_filter_config_t pin_filter_config = {
      .gpio_num = static_cast<gpio_num_t>(pin_)
    };

    ret = gpio_new_pin_glitch_filter(&pin_filter_config, 
                                     reinterpret_cast<gpio_glitch_filter_handle_t*>(&glitch_filter_handle_));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create pin glitch filter for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
    }

    ret = gpio_glitch_filter_enable(static_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable pin glitch filter for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
    }

    pin_glitch_filter_enabled_ = true;
    ESP_LOGI(TAG, "Pin glitch filter enabled for GPIO%d", static_cast<int>(pin_));
  }

  // Configure flexible glitch filter
  if ((filter_type == GpioGlitchFilterType::Flex || filter_type == GpioGlitchFilterType::Both) && flex_config) {
    flex_filter_config_ = *flex_config;

    gpio_flex_glitch_filter_config_t flex_filter_config = {
      .gpio_num = static_cast<gpio_num_t>(pin_),
      .window_width_ns = flex_config->window_width_ns,
      .window_thres_ns = flex_config->window_threshold_ns
    };

    ret = gpio_new_flex_glitch_filter(&flex_filter_config, 
                                      reinterpret_cast<gpio_glitch_filter_handle_t*>(&rtc_gpio_handle_));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create flex glitch filter for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
    }

    if (flex_config->enable_on_init) {
      ret = gpio_glitch_filter_enable(static_cast<gpio_glitch_filter_handle_t>(rtc_gpio_handle_));
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable flex glitch filter for GPIO%d: %s", 
                 static_cast<int>(pin_), esp_err_to_name(ret));
        return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
      }
      flex_glitch_filter_enabled_ = true;
    }

    ESP_LOGI(TAG, "Flexible glitch filter configured for GPIO%d (width: %dns, threshold: %dns)", 
             static_cast<int>(pin_), flex_config->window_width_ns, flex_config->window_threshold_ns);
  }

  return HfGpioErr::GPIO_SUCCESS;
  #else
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
  #endif
}

HfGpioErr McuGpio::ConfigureSleepMode(const GpioSleepConfig &sleep_config) noexcept {
  sleep_config_ = sleep_config;

  #ifdef HF_MCU_ESP32C6
  // Configure RTC GPIO if supported and requested
  if (sleep_config.hold_during_sleep && HF_GPIO_IS_VALID_RTC_GPIO(pin_)) {
    esp_err_t ret = rtc_gpio_init(static_cast<gpio_num_t>(pin_));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize RTC GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
    }

    // Set RTC GPIO direction
    rtc_gpio_mode_t rtc_mode;
    switch (sleep_config.sleep_direction) {
      case Direction::Input:
        rtc_mode = RTC_GPIO_MODE_INPUT_ONLY;
        break;
      case Direction::Output:
        if (output_mode_ == OutputMode::OpenDrain) {
          rtc_mode = RTC_GPIO_MODE_OUTPUT_OD;
        } else {
          rtc_mode = RTC_GPIO_MODE_OUTPUT_ONLY;
        }
        break;
      default:
        rtc_mode = RTC_GPIO_MODE_DISABLED;
        break;
    }

    ret = rtc_gpio_set_direction(static_cast<gpio_num_t>(pin_), rtc_mode);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set RTC GPIO%d direction: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      return HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH;
    }

    // Configure RTC pull resistors
    switch (sleep_config.sleep_pull_mode) {
      case PullMode::PullUp:
        rtc_gpio_pullup_en(static_cast<gpio_num_t>(pin_));
        rtc_gpio_pulldown_dis(static_cast<gpio_num_t>(pin_));
        break;
      case PullMode::PullDown:
        rtc_gpio_pullup_dis(static_cast<gpio_num_t>(pin_));
        rtc_gpio_pulldown_en(static_cast<gpio_num_t>(pin_));
        break;
      case PullMode::Floating:
      default:
        rtc_gpio_pullup_dis(static_cast<gpio_num_t>(pin_));
        rtc_gpio_pulldown_dis(static_cast<gpio_num_t>(pin_));
        break;
    }

    rtc_gpio_enabled_ = true;
    ESP_LOGI(TAG, "RTC GPIO%d configured for sleep mode", static_cast<int>(pin_));
  }

  // Configure hold function
  if (sleep_config.hold_during_sleep) {
    esp_err_t ret = gpio_hold_en(static_cast<gpio_num_t>(pin_));
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "Failed to enable hold for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
    } else {
      hold_enabled_ = true;
      ESP_LOGD(TAG, "Hold enabled for GPIO%d", static_cast<int>(pin_));
    }
  }

  return HfGpioErr::GPIO_SUCCESS;
  #else
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
  #endif
}

bool McuGpio::SupportsGlitchFilter() const noexcept {
#ifdef HF_MCU_ESP32C6
  return HF_GPIO_SUPPORTS_GLITCH_FILTER(pin_);
#else
  return false;
#endif
}

HfGpioErr McuGpio::ConfigurePinGlitchFilter(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::HF_GPIO_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_ESP32C6
  if (!HF_GPIO_SUPPORTS_GLITCH_FILTER(pin_)) {
    ESP_LOGW(TAG, "GPIO%d does not support glitch filtering", static_cast<int>(pin_));
    return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
  }

  if (enable) {
    // Configure pin glitch filter (fixed 2 clock cycles)
    gpio_pin_glitch_filter_config_t filter_config = {
      .gpio_num = static_cast<gpio_num_t>(pin_)
    };
    
    esp_err_t err = gpio_new_pin_glitch_filter(&filter_config, 
                                              reinterpret_cast<gpio_glitch_filter_handle_t*>(&glitch_filter_handle_));
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create pin glitch filter for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(err));
      return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
    }
    
    err = gpio_glitch_filter_enable(reinterpret_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable pin glitch filter for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(err));
      gpio_del_glitch_filter(reinterpret_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
      glitch_filter_handle_ = nullptr;
      return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
    }
    
    pin_glitch_filter_enabled_ = true;
    glitch_filter_type_ = hf_gpio_glitch_filter_type_t::HF_GPIO_GLITCH_FILTER_PIN;
    
    ESP_LOGI(TAG, "Pin glitch filter enabled for GPIO%d", static_cast<int>(pin_));
  } else {
    // Disable and cleanup pin glitch filter
    if (glitch_filter_handle_ && pin_glitch_filter_enabled_) {
      gpio_glitch_filter_disable(reinterpret_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
      gpio_del_glitch_filter(reinterpret_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
      glitch_filter_handle_ = nullptr;
    }
    
    pin_glitch_filter_enabled_ = false;
    if (!flex_glitch_filter_enabled_) {
      glitch_filter_type_ = hf_gpio_glitch_filter_type_t::HF_GPIO_GLITCH_FILTER_NONE;
    }
    
    ESP_LOGI(TAG, "Pin glitch filter disabled for GPIO%d", static_cast<int>(pin_));
  }
  
  return HfGpioErr::HF_GPIO_OK;
#else
  ESP_LOGW(TAG, "Glitch filter not supported on this platform");
  return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
#endif
}

HfGpioErr McuGpio::ConfigureFlexGlitchFilter(const FlexGlitchFilterConfig &config) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::HF_GPIO_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_ESP32C6
  if (!HF_GPIO_SUPPORTS_GLITCH_FILTER(pin_)) {
    ESP_LOGW(TAG, "GPIO%d does not support glitch filtering", static_cast<int>(pin_));
    return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
  }

  // Configure flexible glitch filter with custom timing
  gpio_flex_glitch_filter_config_t filter_config = {
    .gpio_num = static_cast<gpio_num_t>(pin_),
    .window_width_ns = config.window_width_ns,
    .window_thres_ns = config.window_threshold_ns
  };
  
  esp_err_t err = gpio_new_flex_glitch_filter(&filter_config, 
                                            reinterpret_cast<gpio_glitch_filter_handle_t*>(&glitch_filter_handle_));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create flex glitch filter for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(err));
    return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
  }
  
  err = gpio_glitch_filter_enable(reinterpret_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable flex glitch filter for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(err));
    gpio_del_glitch_filter(reinterpret_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
    glitch_filter_handle_ = nullptr;
    return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
  }
  
  flex_glitch_filter_enabled_ = true;
  flex_filter_config_ = config;
  glitch_filter_type_ = hf_gpio_glitch_filter_type_t::HF_GPIO_GLITCH_FILTER_FLEX;
  
  ESP_LOGI(TAG, "Flex glitch filter enabled for GPIO%d (window: %uns, threshold: %uns)", 
           static_cast<int>(pin_), config.window_width_ns, config.window_threshold_ns);
  
  return HfGpioErr::HF_GPIO_OK;
#else
  ESP_LOGW(TAG, "Flexible glitch filter not supported on this platform");
  return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
#endif
}

HfGpioErr McuGpio::EnableGlitchFilters() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::HF_GPIO_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_ESP32C6
  if (!glitch_filter_handle_) {
    ESP_LOGW(TAG, "No glitch filter configured for GPIO%d", static_cast<int>(pin_));
    return HfGpioErr::HF_GPIO_ERR_INVALID_STATE;
  }

  esp_err_t err = gpio_glitch_filter_enable(reinterpret_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable glitch filters for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(err));
    return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
  }
  
  ESP_LOGI(TAG, "Glitch filters enabled for GPIO%d", static_cast<int>(pin_));
  return HfGpioErr::HF_GPIO_OK;
#else
  return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
#endif
}

HfGpioErr McuGpio::DisableGlitchFilters() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::HF_GPIO_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_ESP32C6
  if (!glitch_filter_handle_) {
    ESP_LOGD(TAG, "No glitch filter to disable for GPIO%d", static_cast<int>(pin_));
    return HfGpioErr::HF_GPIO_OK;
  }

  esp_err_t err = gpio_glitch_filter_disable(reinterpret_cast<gpio_glitch_filter_handle_t>(glitch_filter_handle_));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disable glitch filters for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(err));
    return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
  }
  
  ESP_LOGI(TAG, "Glitch filters disabled for GPIO%d", static_cast<int>(pin_));
  return HfGpioErr::HF_GPIO_OK;
#else
  return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
#endif
}

bool McuGpio::SupportsRtcGpio() const noexcept {
#ifdef HF_MCU_ESP32C6
  return HF_GPIO_IS_RTC_CAPABLE(pin_);
#else
  return false;
#endif
}

HfGpioErr McuGpio::ConfigureSleep(const GpioSleepConfig &config) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::HF_GPIO_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_ESP32C6
  if (!HF_GPIO_IS_RTC_CAPABLE(pin_)) {
    ESP_LOGW(TAG, "GPIO%d does not support RTC functionality", static_cast<int>(pin_));
    return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
  }

  esp_err_t err;
  
  // Configure sleep-specific settings
  if (config.enable_sleep_retain) {
    err = rtc_gpio_init(static_cast<gpio_num_t>(pin_));
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize RTC GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(err));
      return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
    }

    // Set sleep mode direction
    rtc_gpio_mode_t rtc_mode;
    switch (config.sleep_direction) {
      case Direction::Input:
        rtc_mode = RTC_GPIO_MODE_INPUT_ONLY;
        break;
      case Direction::Output:
        rtc_mode = RTC_GPIO_MODE_OUTPUT_ONLY;
        break;
      default:
        rtc_mode = RTC_GPIO_MODE_DISABLED;
        break;
    }
    
    err = rtc_gpio_set_direction(static_cast<gpio_num_t>(pin_), rtc_mode);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set RTC direction for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(err));
      return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
    }

    // Configure sleep pull mode
    if (config.sleep_pull_mode != PullMode::Floating) {
      rtc_gpio_pull_mode_t rtc_pull;
      switch (config.sleep_pull_mode) {
        case PullMode::PullUp:
          rtc_pull = GPIO_PULLUP_ONLY;
          break;
        case PullMode::PullDown:
          rtc_pull = GPIO_PULLDOWN_ONLY;
          break;
        default:
          rtc_pull = GPIO_FLOATING;
          break;
      }
      
      err = rtc_gpio_pullup_dis(static_cast<gpio_num_t>(pin_));
      if (err == ESP_OK) err = rtc_gpio_pulldown_dis(static_cast<gpio_num_t>(pin_));
      
      if (rtc_pull == GPIO_PULLUP_ONLY) {
        err = rtc_gpio_pullup_en(static_cast<gpio_num_t>(pin_));
      } else if (rtc_pull == GPIO_PULLDOWN_ONLY) {
        err = rtc_gpio_pulldown_en(static_cast<gpio_num_t>(pin_));
      }
      
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure RTC pull mode for GPIO%d: %s", 
                 static_cast<int>(pin_), esp_err_to_name(err));
        return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
      }
    }

    rtc_gpio_enabled_ = true;
  } else {
    // Disable RTC GPIO functionality
    if (rtc_gpio_enabled_) {
      rtc_gpio_deinit(static_cast<gpio_num_t>(pin_));
      rtc_gpio_enabled_ = false;
    }
  }
  
  sleep_config_ = config;
  ESP_LOGI(TAG, "Sleep configuration updated for GPIO%d (RTC: %s)", 
           static_cast<int>(pin_), config.enable_sleep_retain ? "enabled" : "disabled");
  
  return HfGpioErr::HF_GPIO_OK;
#else
  ESP_LOGW(TAG, "Sleep configuration not supported on this platform");
  return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
#endif
}

HfGpioErr McuGpio::ConfigureHold(bool enable) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::HF_GPIO_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_ESP32C6
  esp_err_t err;
  
  if (enable) {
    // Enable hold feature
    if (HF_GPIO_IS_RTC_CAPABLE(pin_)) {
      // Use RTC hold for RTC-capable pins
      err = rtc_gpio_hold_en(static_cast<gpio_num_t>(pin_));
    } else {
      // Use digital hold for non-RTC pins
      err = gpio_hold_en(static_cast<gpio_num_t>(pin_));
    }
  } else {
    // Disable hold feature
    if (HF_GPIO_IS_RTC_CAPABLE(pin_)) {
      err = rtc_gpio_hold_dis(static_cast<gpio_num_t>(pin_));
    } else {
      err = gpio_hold_dis(static_cast<gpio_num_t>(pin_));
    }
  }
  
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to %s hold for GPIO%d: %s", 
             enable ? "enable" : "disable", static_cast<int>(pin_), esp_err_to_name(err));
    return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
  }
  
  hold_enabled_ = enable;
  ESP_LOGI(TAG, "Hold %s for GPIO%d", enable ? "enabled" : "disabled", static_cast<int>(pin_));
  
  return HfGpioErr::HF_GPIO_OK;
#else
  ESP_LOGW(TAG, "Hold configuration not supported on this platform");
  return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
#endif
}

HfGpioErr McuGpio::ConfigureWakeUp(const GpioWakeUpConfig &config) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::HF_GPIO_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_ESP32C6
  if (!HF_GPIO_IS_RTC_CAPABLE(pin_)) {
    ESP_LOGW(TAG, "GPIO%d does not support wake-up functionality", static_cast<int>(pin_));
    return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
  }

  esp_err_t err;
  
  if (config.enable_wakeup) {
    gpio_int_type_t wakeup_type;
    switch (config.wakeup_trigger) {
      case InterruptTrigger::RisingEdge:
        wakeup_type = GPIO_INTR_HIGH_LEVEL;  // Wake on high level for edge
        break;
      case InterruptTrigger::FallingEdge:
        wakeup_type = GPIO_INTR_LOW_LEVEL;   // Wake on low level for edge
        break;
      default:
        ESP_LOGE(TAG, "Invalid wake-up trigger for GPIO%d", static_cast<int>(pin_));
        return HfGpioErr::HF_GPIO_ERR_INVALID_ARG;
    }
    
    err = rtc_gpio_wakeup_enable(static_cast<gpio_num_t>(pin_), wakeup_type);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable wake-up for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(err));
      return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
    }
    
    ESP_LOGI(TAG, "Wake-up enabled for GPIO%d (trigger: %s)", 
             static_cast<int>(pin_), 
             (wakeup_type == GPIO_INTR_HIGH_LEVEL) ? "high" : "low");
  } else {
    err = rtc_gpio_wakeup_disable(static_cast<gpio_num_t>(pin_));
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to disable wake-up for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(err));
      return HfGpioErr::HF_GPIO_ERR_DRIVER_ERROR;
    }
    
    ESP_LOGI(TAG, "Wake-up disabled for GPIO%d", static_cast<int>(pin_));
  }
  
  wakeup_config_ = config;
  return HfGpioErr::HF_GPIO_OK;
#else
  ESP_LOGW(TAG, "Wake-up configuration not supported on this platform");
  return HfGpioErr::HF_GPIO_ERR_NOT_SUPPORTED;
#endif
}

GpioConfigDump McuGpio::GetConfigurationDump() const noexcept {
  hf_gpio_status_info_t dump = {};
  
#ifdef HF_MCU_ESP32C6
  dump.pin_number = pin_;
  dump.direction = GetDirection();
  dump.output_mode = GetOutputMode();
  dump.pull_mode = GetPullMode();
  dump.drive_capability = drive_capability_;
  dump.glitch_filter_enabled = pin_glitch_filter_enabled_ || flex_glitch_filter_enabled_;
  dump.glitch_filter_type = glitch_filter_type_;
  dump.rtc_gpio_enabled = rtc_gpio_enabled_;
  dump.hold_enabled = hold_enabled_;
  dump.interrupt_enabled = interrupt_enabled_;
  dump.interrupt_count = interrupt_count_.load();
  dump.last_level = IsActive();
#endif  
  return dump;
}

bool McuGpio::IsHeld() const noexcept {
  return hold_enabled_;
}

HfGpioErr McuGpio::GetPinCapabilities(hf_gpio_pin_capabilities_t &capabilities) const noexcept {
#ifdef HF_MCU_ESP32C6
  capabilities.pin_number = pin_;
  capabilities.supports_input = true;
  capabilities.supports_output = true;
  capabilities.supports_pullup = true;
  capabilities.supports_pulldown = true;
  capabilities.supports_glitch_filter = HF_GPIO_SUPPORTS_GLITCH_FILTER(pin_);
  capabilities.supports_rtc = HF_GPIO_IS_RTC_CAPABLE(pin_);
  capabilities.supports_adc = HF_GPIO_IS_ADC_CAPABLE(pin_);
  capabilities.is_strapping_pin = HF_GPIO_IS_STRAPPING_PIN(pin_);
  capabilities.is_spi_pin = HF_GPIO_IS_SPI_PIN(pin_);
  capabilities.is_usb_jtag_pin = HF_GPIO_IS_USB_JTAG_PIN(pin_);
  
  return HfGpioErr::HF_GPIO_OK;
#else
  // Generic capabilities for non-ESP32C6 platforms
  capabilities.pin_number = pin_;
  capabilities.supports_input = true;
  capabilities.supports_output = true;
  capabilities.supports_pullup = true;
  capabilities.supports_pulldown = true;
  capabilities.supports_glitch_filter = false;
  capabilities.supports_rtc = false;
  capabilities.supports_adc = false;
  capabilities.is_strapping_pin = false;
  capabilities.is_spi_pin = false;
  capabilities.is_usb_jtag_pin = false;
  
  return HfGpioErr::HF_GPIO_OK;
#endif
}

HfGpioErr McuGpio::GetStatusInfo(hf_gpio_status_info_t &status) const noexcept {
  return static_cast<HfGpioErr>(GetConfigurationDump());
}

uint32_t McuGpio::GetTotalInterruptCount() noexcept {
#ifdef HF_MCU_ESP32C6
  return total_interrupt_count_.load();
#else
  return 0;
#endif
}

uint32_t McuGpio::GetActiveGpioCount() noexcept {
#ifdef HF_MCU_ESP32C6
  return active_gpio_count_.load();
#else
  return 0;
#endif
}

bool McuGpio::IsValidPin(HfPinNumber pin_num) noexcept {
#ifdef HF_MCU_ESP32C6
  return HF_GPIO_IS_VALID_PIN(pin_num);
#else
  return (pin_num >= 0 && pin_num < 32);  // Generic validation
#endif
}

bool McuGpio::IsRtcGpio(HfPinNumber pin_num) noexcept {
#ifdef HF_MCU_ESP32C6
  return HF_GPIO_IS_RTC_CAPABLE(pin_num);
#else
  return false;
#endif
}

bool McuGpio::IsStrappingPin(HfPinNumber pin_num) noexcept {
#ifdef HF_MCU_ESP32C6
  return HF_GPIO_IS_STRAPPING_PIN(pin_num);
#else
  return false;
#endif
}

//==============================================================================
// STATIC UTILITY METHODS
//==============================================================================

uint32_t McuGpio::GetTotalInterruptCount() noexcept {
  return g_total_gpio_interrupts.load(std::memory_order_relaxed);
}

uint32_t McuGpio::GetActiveGpioCount() noexcept {
  return g_active_gpio_count.load(std::memory_order_relaxed);
}

bool McuGpio::IsValidPin(HfPinNumber pin_num) noexcept {
  return HF_GPIO_IS_VALID_GPIO(pin_num);
}

bool McuGpio::IsRtcGpio(HfPinNumber pin_num) noexcept {
  return HF_GPIO_IS_VALID_RTC_GPIO(pin_num);
}

bool McuGpio::IsStrappingPin(HfPinNumber pin_num) noexcept {
  return HF_GPIO_IS_STRAPPING_PIN(pin_num);
}

#ifdef HF_MCU_FAMILY_ESP32
// Static flag to track ISR service installation
bool McuGpio::gpio_isr_handler_installed_ = false;
#endif

//==============================================================================
// LAZY INITIALIZATION IMPLEMENTATION
//==============================================================================

bool McuGpio::EnsureInitialized() noexcept {
  if (initialized_) {
    return true;  // Already initialized
  }
  
  ESP_LOGD(TAG, "Lazy initialization triggered for GPIO%d", static_cast<int>(pin_));
  
  // Call the full Initialize() method which will set initialized_ flag
  return Initialize();
}

//==============================================================================
// ETM (EVENT TASK MATRIX) IMPLEMENTATION FOR ESP32C6
//==============================================================================

#ifdef HF_MCU_ESP32C6
// ESP32C6 ETM support includes
#include "esp_etm.h"
#include "driver/gpio_etm.h"

// ETM handles storage
static esp_etm_channel_handle_t etm_channels[HF_MCU_GPIO_ETM_CHANNEL_COUNT] = {nullptr};
static gpio_etm_event_handle_t gpio_etm_events[HF_MCU_GPIO_PIN_COUNT] = {nullptr};
static gpio_etm_task_handle_t gpio_etm_tasks[HF_MCU_GPIO_PIN_COUNT] = {nullptr};
static uint8_t etm_channel_usage_count = 0;

HfGpioErr McuGpio::ConfigureETM(const hf_gpio_etm_config_t &etm_config) noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }

  if (!HF_GPIO_SUPPORTS_ETM(pin_)) {
    ESP_LOGW(TAG, "GPIO%d does not support ETM functionality", static_cast<int>(pin_));
    return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
  }

  esp_err_t ret;

  // Clean up existing ETM configuration
  CleanupETM();

  if (!etm_config.enable_etm) {
    ESP_LOGD(TAG, "ETM disabled for GPIO%d", static_cast<int>(pin_));
    return HfGpioErr::GPIO_SUCCESS;
  }

  // Create ETM channel
  esp_etm_channel_config_t channel_config = {};
  ret = esp_etm_new_channel(&channel_config, &etm_channels[pin_]);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create ETM channel for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  // Create GPIO ETM event
  gpio_etm_event_config_t event_config = {};
  switch (etm_config.event_config.edge) {
    case hf_gpio_etm_event_edge_t::HF_GPIO_ETM_EVENT_EDGE_POS:
      event_config.edge = GPIO_ETM_EVENT_EDGE_POS;
      break;
    case hf_gpio_etm_event_edge_t::HF_GPIO_ETM_EVENT_EDGE_NEG:
      event_config.edge = GPIO_ETM_EVENT_EDGE_NEG;
      break;
    case hf_gpio_etm_event_edge_t::HF_GPIO_ETM_EVENT_EDGE_ANY:
      event_config.edge = GPIO_ETM_EVENT_EDGE_ANY;
      break;
    default:
      event_config.edge = GPIO_ETM_EVENT_EDGE_POS;
      break;
  }

  ret = gpio_new_etm_event(&event_config, &gpio_etm_events[pin_]);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create GPIO ETM event for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    esp_etm_del_channel(etm_channels[pin_]);
    etm_channels[pin_] = nullptr;
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  // Bind GPIO to ETM event
  if (etm_config.auto_bind_gpio) {
    ret = gpio_etm_event_bind_gpio(gpio_etm_events[pin_], static_cast<int>(pin_));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to bind GPIO%d to ETM event: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      esp_etm_del_event(gpio_etm_events[pin_]);
      esp_etm_del_channel(etm_channels[pin_]);
      gpio_etm_events[pin_] = nullptr;
      etm_channels[pin_] = nullptr;
      return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
    }
  }

  // Create GPIO ETM task
  gpio_etm_task_config_t task_config = {};
  switch (etm_config.task_config.action) {
    case hf_gpio_etm_task_action_t::HF_GPIO_ETM_TASK_ACTION_SET:
      task_config.action = GPIO_ETM_TASK_ACTION_SET;
      break;
    case hf_gpio_etm_task_action_t::HF_GPIO_ETM_TASK_ACTION_CLR:
      task_config.action = GPIO_ETM_TASK_ACTION_CLR;
      break;
    case hf_gpio_etm_task_action_t::HF_GPIO_ETM_TASK_ACTION_TOG:
      task_config.action = GPIO_ETM_TASK_ACTION_TOG;
      break;
    default:
      task_config.action = GPIO_ETM_TASK_ACTION_TOG;
      break;
  }

  ret = gpio_new_etm_task(&task_config, &gpio_etm_tasks[pin_]);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create GPIO ETM task for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    esp_etm_del_event(gpio_etm_events[pin_]);
    esp_etm_del_channel(etm_channels[pin_]);
    gpio_etm_events[pin_] = nullptr;
    etm_channels[pin_] = nullptr;
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  // Add GPIO to ETM task
  if (etm_config.auto_bind_gpio) {
    ret = gpio_etm_task_add_gpio(gpio_etm_tasks[pin_], static_cast<int>(pin_));
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to add GPIO%d to ETM task: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      esp_etm_del_task(gpio_etm_tasks[pin_]);
      esp_etm_del_event(gpio_etm_events[pin_]);
      esp_etm_del_channel(etm_channels[pin_]);
      gpio_etm_tasks[pin_] = nullptr;
      gpio_etm_events[pin_] = nullptr;
      etm_channels[pin_] = nullptr;
      return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
    }
  }

  // Connect event and task to ETM channel
  ret = esp_etm_channel_connect(etm_channels[pin_], gpio_etm_events[pin_], gpio_etm_tasks[pin_]);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to connect ETM event and task for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    CleanupETM();
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  // Enable ETM channel if requested
  if (etm_config.event_config.enable_on_init) {
    ret = esp_etm_channel_enable(etm_channels[pin_]);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to enable ETM channel for GPIO%d: %s", 
               static_cast<int>(pin_), esp_err_to_name(ret));
      CleanupETM();
      return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
    }
  }

  etm_channel_usage_count++;
  
  ESP_LOGI(TAG, "ETM configured for GPIO%d (event edge: %d, task action: %d, channel: %p)", 
           static_cast<int>(pin_), 
           static_cast<int>(etm_config.event_config.edge),
           static_cast<int>(etm_config.task_config.action),
           etm_channels[pin_]);

  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::EnableETM() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }

  if (!etm_channels[pin_]) {
    ESP_LOGW(TAG, "No ETM channel configured for GPIO%d", static_cast<int>(pin_));
    return HfGpioErr::GPIO_ERR_INVALID_STATE;
  }

  esp_err_t ret = esp_etm_channel_enable(etm_channels[pin_]);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable ETM channel for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  ESP_LOGD(TAG, "ETM channel enabled for GPIO%d", static_cast<int>(pin_));
  return HfGpioErr::GPIO_SUCCESS;
}

HfGpioErr McuGpio::DisableETM() noexcept {
  if (!EnsureInitialized()) {
    return HfGpioErr::GPIO_ERR_NOT_INITIALIZED;
  }

  if (!etm_channels[pin_]) {
    ESP_LOGD(TAG, "No ETM channel to disable for GPIO%d", static_cast<int>(pin_));
    return HfGpioErr::GPIO_SUCCESS;
  }

  esp_err_t ret = esp_etm_channel_disable(etm_channels[pin_]);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disable ETM channel for GPIO%d: %s", 
             static_cast<int>(pin_), esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  ESP_LOGD(TAG, "ETM channel disabled for GPIO%d", static_cast<int>(pin_));
  return HfGpioErr::GPIO_SUCCESS;
}

void McuGpio::CleanupETM() noexcept {
  if (etm_channels[pin_]) {
    esp_etm_channel_disable(etm_channels[pin_]);
    esp_etm_channel_connect(etm_channels[pin_], nullptr, nullptr);
    esp_etm_del_channel(etm_channels[pin_]);
    etm_channels[pin_] = nullptr;
    etm_channel_usage_count--;
  }

  if (gpio_etm_tasks[pin_]) {
    gpio_etm_task_rm_gpio(gpio_etm_tasks[pin_], static_cast<int>(pin_));
    esp_etm_del_task(gpio_etm_tasks[pin_]);
    gpio_etm_tasks[pin_] = nullptr;
  }

  if (gpio_etm_events[pin_]) {
    esp_etm_del_event(gpio_etm_events[pin_]);
    gpio_etm_events[pin_] = nullptr;
  }
}

bool McuGpio::SupportsETM() const noexcept {
  return HF_GPIO_SUPPORTS_ETM(pin_);
}

HfGpioErr McuGpio::GetETMStatus(hf_gpio_etm_status_t &status) const noexcept {
  status.etm_enabled = (etm_channels[pin_] != nullptr);
  status.event_handle = gpio_etm_events[pin_];
  status.task_handle = gpio_etm_tasks[pin_];
  status.channel_handle = etm_channels[pin_];
  status.total_etm_channels_used = etm_channel_usage_count;
  status.max_etm_channels = HF_MCU_GPIO_ETM_CHANNEL_COUNT;
  
  return HfGpioErr::GPIO_SUCCESS;
}

// Static ETM utility functions
uint8_t McuGpio::GetETMChannelUsage() noexcept {
  return etm_channel_usage_count;
}

uint8_t McuGpio::GetMaxETMChannels() noexcept {
  return HF_MCU_GPIO_ETM_CHANNEL_COUNT;
}

HfGpioErr McuGpio::DumpETMConfiguration(FILE* output_stream) noexcept {
  if (!output_stream) {
    output_stream = stdout;
  }

  esp_err_t ret = esp_etm_dump(output_stream);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to dump ETM configuration: %s", esp_err_to_name(ret));
    return HfGpioErr::GPIO_ERR_INVALID_CONFIGURATION;
  }

  return HfGpioErr::GPIO_SUCCESS;
}

#else
// Stub implementations for non-ESP32C6 platforms

HfGpioErr McuGpio::ConfigureETM(const hf_gpio_etm_config_t &etm_config) noexcept {
  ESP_LOGW(TAG, "ETM not supported on this platform");
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
}

HfGpioErr McuGpio::EnableETM() noexcept {
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
}

HfGpioErr McuGpio::DisableETM() noexcept {
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
}

void McuGpio::CleanupETM() noexcept {
  // No-op for non-ESP32C6
}

bool McuGpio::SupportsETM() const noexcept {
  return false;
}

HfGpioErr McuGpio::GetETMStatus(hf_gpio_etm_status_t &status) const noexcept {
  memset(& status, 0, sizeof(status));
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
}

uint8_t McuGpio::GetETMChannelUsage() noexcept {
  return 0;
}

uint8_t McuGpio::GetMaxETMChannels() noexcept {
  return 0;
}

HfGpioErr McuGpio::DumpETMConfiguration(FILE* output_stream) noexcept {
  return HfGpioErr::GPIO_ERR_NOT_SUPPORTED;
}

#endif // HF_MCU_ESP32C6
