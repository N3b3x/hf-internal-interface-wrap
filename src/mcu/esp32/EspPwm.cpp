/**
 * @file EspPwm.cpp
 * @brief Implementation of ESP32C6 LEDC (PWM) controller for the HardFOC system.
 *
 * This file provides the implementation for PWM generation using the ESP32C6's
 * built-in LEDC peripheral. All platform-specific types and implementations are
 * isolated through EspTypes_PWM.h. The implementation supports multiple channels,
 * configurable frequency and resolution, complementary outputs with deadtime,
 * hardware fade support, and interrupt-driven period callbacks.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */
#include "EspPwm.h"
#include <algorithm>

// Platform-specific includes and definitions
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "hal/ledc_hal.h"
#include "soc/ledc_reg.h"

static const char *TAG = "EspPwm";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

EspPwm::EspPwm(const hf_pwm_unit_config_t &config) noexcept
    : BasePwm()
    , mutex_(), initialized_(false), base_clock_hz_(config.base_clock_hz),
      clock_source_(config.clock_source), channels_(), timers_(), complementary_pairs_(),
      period_callback_(nullptr), period_callback_user_data_(nullptr), fault_callback_(nullptr),
      fault_callback_user_data_(nullptr), last_global_error_(hf_pwm_err_t::PWM_SUCCESS),
      fade_functionality_installed_(false), unit_config_(config), current_mode_(config.mode),
      statistics_(), diagnostics_() {
  ESP_LOGD(TAG, "EspPwm constructed with unit_id=%d, mode=%d, clock_hz=%lu", config.unit_id,
           static_cast<int>(config.mode), config.base_clock_hz);
}

EspPwm::EspPwm(uint32_t base_clock_hz) noexcept : EspPwm(hf_pwm_unit_config_t{}) {
  // Override base clock if provided
  if (base_clock_hz != 0) {
    base_clock_hz_ = base_clock_hz;
    unit_config_.base_clock_hz = base_clock_hz;
  }
  ESP_LOGD(TAG, "EspPwm legacy constructor with clock_hz=%lu", base_clock_hz);
}

EspPwm::~EspPwm() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (initialized_.load()) {
    ESP_LOGI(TAG, "EspPwm destructor - cleaning up");
    Deinitialize();
  }
}

//==============================================================================
// LIFECYCLE (BasePwm Interface)
//==============================================================================

hf_pwm_err_t EspPwm::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_.load()) {
    ESP_LOGW(TAG, "PWM already initialized");
    return hf_pwm_err_t::PWM_ERR_ALREADY_INITIALIZED;
  }

  ESP_LOGI(TAG, "Initializing ESP32C6 PWM system with unit_id=%d, mode=%d, base_clock=%lu Hz",
           unit_config_.unit_id, static_cast<int>(unit_config_.mode), base_clock_hz_);

  // Initialize timers and channels using lifecycle helpers
  hf_pwm_err_t result = InitializeTimers();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize timers: %d", static_cast<int>(result));
    return result;
  }

  result = InitializeChannels();
  if (result != hf_pwm_err_t::PWM_SUCCESS) {
    ESP_LOGE(TAG, "Failed to initialize channels: %d", static_cast<int>(result));
    return result;
  }

  // Enable fade functionality if requested
  if (unit_config_.enable_fade) {
    result = EnableFade();
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      ESP_LOGW(TAG, "Failed to enable fade functionality: %d", static_cast<int>(result));
      // Don't fail initialization if fade fails
    }
  }

  initialized_.store(true);
  last_global_error_ = hf_pwm_err_t::PWM_SUCCESS;
  statistics_.initialization_timestamp = esp_timer_get_time();
  statistics_.last_activity_timestamp = statistics_.initialization_timestamp;

  ESP_LOGI(TAG, "ESP32C6 PWM system initialized successfully");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_.load()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  ESP_LOGI(TAG, "Deinitializing ESP32C6 PWM system");

  // Stop all channels first
  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured) {
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), 0);
    }
  }

  // Reset all timers
  for (uint8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use) {
      ledc_timer_rst(LEDC_LOW_SPEED_MODE, static_cast<ledc_timer_t>(timer_id));
    }
  }

  initialized_.store(false);
  ESP_LOGI(TAG, "ESP32C6 PWM system deinitialized");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::SetMode(hf_pwm_mode_t mode) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (mode == hf_pwm_mode_t::Fade && !fade_functionality_installed_) {
    hf_pwm_err_t result = EnableFade();
    if (result != hf_pwm_err_t::PWM_SUCCESS) {
      return result;
    }
  }

  current_mode_ = mode;
  unit_config_.mode = mode;

  ESP_LOGD(TAG, "PWM mode set to %d", static_cast<int>(mode));
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_mode_t EspPwm::GetMode() const noexcept {
  return current_mode_;
}

//==============================================================================
// CHANNEL MANAGEMENT (BasePwm Interface)
//==============================================================================

hf_pwm_err_t EspPwm::ConfigureChannel(hf_channel_id_t channel_id,
                                      const hf_pwm_channel_config_t &config) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Validate configuration
  if (config.gpio_pin == static_cast<hf_gpio_num_t>(HF_INVALID_PIN)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  if (!BasePwm::IsValidDutyCycle(config.initial_duty_cycle)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  // Frequency validation will be done when configuring the timer

  // Find or allocate a timer for this frequency/resolution combination
  // Note: We need to get frequency and resolution from the channel's timer assignment
  // For now, use default values since they're not in the config struct
  uint32_t frequency_hz = HF_PWM_DEFAULT_FREQUENCY;
  uint8_t resolution_bits = HF_PWM_DEFAULT_RESOLUTION;
  
  int8_t timer_id = FindOrAllocateTimer(frequency_hz, resolution_bits);
  if (timer_id < 0) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
    return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
  }

  // Configure the platform timer if needed
  hf_pwm_err_t timer_result =
      ConfigurePlatformTimer(timer_id, frequency_hz, resolution_bits);
  if (timer_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, timer_result);
    return timer_result;
  }

  // Configure the platform channel
  hf_pwm_err_t channel_result = ConfigurePlatformChannel(channel_id, config, timer_id);
  if (channel_result != hf_pwm_err_t::PWM_SUCCESS) {
    SetChannelError(channel_id, channel_result);
    return channel_result;
  }

  // Update internal state
  channels_[channel_id].configured = true;
  channels_[channel_id].config = config;
  channels_[channel_id].assigned_timer = timer_id;
  channels_[channel_id].raw_duty_value =
      BasePwm::DutyCycleToRaw(config.initial_duty_cycle, config.resolution_bits);
  channels_[channel_id].last_error = hf_pwm_err_t::PWM_SUCCESS;

    ESP_LOGI(TAG, "Channel %lu configured: pin=%d, freq=%lu Hz, res=%d bits, timer=%d", channel_id,
            config.gpio_pin, frequency_hz, resolution_bits, timer_id);

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::EnableChannel(hf_channel_id_t channel_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_CHANNEL_NOT_AVAILABLE);
    return hf_pwm_err_t::PWM_ERR_CHANNEL_NOT_AVAILABLE;
  }

  if (channels_[channel_id].enabled) {
    return hf_pwm_err_t::PWM_SUCCESS; // Already enabled
  }

  // Start the channel with current duty cycle
  esp_err_t ret =
      ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                               channels_[channel_id].raw_duty_value,
                               0 // No hpoint (phase shift)
      );

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].enabled = true;
  ESP_LOGI(TAG, "Channel %lu enabled", channel_id);
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::DisableChannel(hf_channel_id_t channel_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].enabled) {
    return hf_pwm_err_t::PWM_SUCCESS; // Already disabled
  }

  // Stop the channel based on idle state
  uint32_t idle_level =
      (channels_[channel_id].config.idle_state == hf_pwm_idle_state_t::High) ? 1 : 0;
  if (channels_[channel_id].config.invert_output) {
    idle_level = 1 - idle_level;
  }

  esp_err_t ret =
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), idle_level);

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disable channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].enabled = false;
  ESP_LOGI(TAG, "Channel %lu disabled", channel_id);
  return hf_pwm_err_t::PWM_SUCCESS;
}

bool EspPwm::IsChannelEnabled(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return false;
  }
  return channels_[channel_id].enabled;
}

//==============================================================================
// PWM CONTROL (BasePwm Interface)
//==============================================================================

hf_pwm_err_t EspPwm::SetDutyCycle(hf_channel_id_t channel_id, float duty_cycle) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidDutyCycle(duty_cycle)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  uint8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id >= MAX_TIMERS) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }
  uint32_t raw_duty =
      BasePwm::DutyCycleToRaw(duty_cycle, timers_[timer_id].resolution_bits);
  return SetDutyCycleRaw(channel_id, raw_duty);
}

hf_pwm_err_t EspPwm::SetDutyCycleRaw(hf_channel_id_t channel_id, uint32_t raw_value) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  uint8_t timer_id = channels_[channel_id].assigned_timer;
  uint32_t max_duty = (1U << timers_[timer_id].resolution_bits) - 1;
  if (raw_value > max_duty) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_DUTY_OUT_OF_RANGE);
    return hf_pwm_err_t::PWM_ERR_DUTY_OUT_OF_RANGE;
  }

  // Apply inversion if configured
  uint32_t actual_duty = raw_value;
  if (channels_[channel_id].config.output_invert) {
    actual_duty = max_duty - raw_value;
  }

  hf_pwm_err_t result = UpdatePlatformDuty(channel_id, actual_duty);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    channels_[channel_id].raw_duty_value = raw_value;
    channels_[channel_id].last_error = hf_pwm_err_t::PWM_SUCCESS;

    statistics_.duty_updates_count++;
    statistics_.last_activity_timestamp = esp_timer_get_time();
  } else {
    statistics_.error_count++;
    SetChannelError(channel_id, result);
  }

  return result;
}

hf_pwm_err_t EspPwm::SetFrequency(hf_channel_id_t channel_id,
                                  hf_frequency_hz_t frequency_hz) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidFrequency(frequency_hz, MIN_FREQUENCY, MAX_FREQUENCY)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY);
    return hf_pwm_err_t::PWM_ERR_INVALID_FREQUENCY;
  }

  // Check if we can update the existing timer or need a new one
  uint8_t current_timer = channels_[channel_id].assigned_timer;
  bool can_update_existing = (timers_[current_timer].channel_count == 1);

  if (can_update_existing) {
    // We can update the existing timer since this is the only channel using it
    hf_pwm_err_t result = ConfigurePlatformTimer(current_timer, frequency_hz,
                                                 channels_[channel_id].config.resolution_bits);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      timers_[current_timer].frequency_hz = frequency_hz;

      statistics_.frequency_changes_count++;
      statistics_.last_activity_timestamp = esp_timer_get_time();
    } else {
      statistics_.error_count++;
      SetChannelError(channel_id, result);
    }
    return result;
  } else {
    // Need to find a new timer
    int8_t new_timer =
        FindOrAllocateTimer(frequency_hz, channels_[channel_id].config.resolution_bits);
    if (new_timer < 0) {
      SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT);
      return hf_pwm_err_t::PWM_ERR_TIMER_CONFLICT;
    }

    // Reconfigure the channel with the new timer
    hf_pwm_err_t result =
        ConfigurePlatformChannel(channel_id, channels_[channel_id].config, new_timer);
    if (result == hf_pwm_err_t::PWM_SUCCESS) {
      // Release the old timer and assign the new one
      ReleaseTimerIfUnused(current_timer);
      channels_[channel_id].assigned_timer = new_timer;

      statistics_.frequency_changes_count++;
      statistics_.last_activity_timestamp = esp_timer_get_time();
    } else {
      statistics_.error_count++;
      SetChannelError(channel_id, result);
    }
    return result;
  }
}

hf_pwm_err_t EspPwm::SetPhaseShift(hf_channel_id_t channel_id, float phase_shift_degrees) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // ESP32C6 LEDC doesn't support phase shifting directly
  // This would require advanced timer configuration
  ESP_LOGW(TAG, "Phase shift not supported on ESP32C6 LEDC peripheral");
  SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
  return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
}

//==============================================================================
// ADVANCED FEATURES (BasePwm Interface)
//==============================================================================

hf_pwm_err_t EspPwm::StartAll() noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  hf_pwm_err_t result = hf_pwm_err_t::PWM_SUCCESS;

  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured && !channels_[channel_id].enabled) {
      hf_pwm_err_t channel_result = EnableChannel(channel_id);
      if (channel_result != hf_pwm_err_t::PWM_SUCCESS) {
        result = channel_result; // Keep the last error
      }
    }
  }

  return result;
}

hf_pwm_err_t EspPwm::StopAll() noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  hf_pwm_err_t result = hf_pwm_err_t::PWM_SUCCESS;

  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].enabled) {
      hf_pwm_err_t channel_result = DisableChannel(channel_id);
      if (channel_result != hf_pwm_err_t::PWM_SUCCESS) {
        result = channel_result; // Keep the last error
      }
    }
  }

  return result;
}

hf_pwm_err_t EspPwm::UpdateAll() noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // For ESP32C6, we can update all channels simultaneously
  for (hf_channel_id_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured && channels_[channel_id].enabled) {
      esp_err_t ret =
          ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ledc_update_duty failed for channel %lu: %s", channel_id,
                 esp_err_to_name(ret));
        SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
        return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
      }
    }
  }
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::SetComplementaryOutput(hf_channel_id_t primary_channel,
                                            hf_channel_id_t complementary_channel,
                                            uint32_t deadtime_ns) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(primary_channel) || !IsValidChannelId(complementary_channel)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (primary_channel == complementary_channel) {
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  // ESP32C6 LEDC doesn't have direct complementary output support
  // We would need to implement this in software using interrupts
  ESP_LOGW(TAG, "Complementary outputs require software implementation on ESP32C6");

  // Find an available complementary pair slot
  for (auto &pair : complementary_pairs_) {
    if (!pair.active) {
      pair.primary_channel = static_cast<uint8_t>(primary_channel);
      pair.complementary_channel = static_cast<uint8_t>(complementary_channel);
      pair.deadtime_ns = deadtime_ns;
      pair.active = true;

      ESP_LOGI(TAG, "Complementary pair configured: primary=%lu, comp=%lu, deadtime=%lu ns",
               primary_channel, complementary_channel, deadtime_ns);
      return hf_pwm_err_t::PWM_SUCCESS;
    }
  }

  return hf_pwm_err_t::PWM_ERR_INSUFFICIENT_CHANNELS;
}

//==============================================================================
// STATUS AND INFORMATION (BasePwm Interface)
//==============================================================================

float EspPwm::GetDutyCycle(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return -1.0f;
  }

  return BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value,
                                 channels_[channel_id].config.resolution_bits);
}

hf_frequency_hz_t EspPwm::GetFrequency(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return 0;
  }

  uint8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id < MAX_TIMERS) {
    return timers_[timer_id].frequency_hz;
  }
  return 0;
}

hf_pwm_err_t EspPwm::GetChannelStatus(hf_channel_id_t channel_id,
                                      hf_pwm_channel_status_t &status) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    status = hf_pwm_channel_status_t{}; // Reset to default
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  status.is_enabled = channels_[channel_id].enabled;
  status.is_running = channels_[channel_id].enabled;
  uint8_t timer_id = channels_[channel_id].assigned_timer;
  if (timer_id < MAX_TIMERS) {
    status.current_frequency = timers_[timer_id].frequency_hz;
  } else {
    status.current_frequency = 0;
  }
  status.current_duty_cycle = BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value,
                                                      channels_[channel_id].config.resolution_bits);
  status.raw_duty_value = channels_[channel_id].raw_duty_value;
  status.last_error = channels_[channel_id].last_error;

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::GetCapabilities(hf_pwm_capabilities_t &capabilities) const noexcept {
  capabilities.num_channels = MAX_CHANNELS;
  capabilities.num_timers = MAX_TIMERS;
  capabilities.min_frequency_hz = MIN_FREQUENCY;
  capabilities.max_frequency_hz = MAX_FREQUENCY;
  capabilities.max_resolution_bits = MAX_RESOLUTION;
  capabilities.supports_complementary = true;   // Software implementation
  capabilities.supports_deadtime = true;        // Software implementation
  capabilities.supports_phase_shift = false;    // Not supported by LEDC

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::GetLastError(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  return channels_[channel_id].last_error;
}

hf_pwm_err_t EspPwm::GetStatistics(hf_pwm_statistics_t &statistics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  statistics = statistics_;
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::GetDiagnostics(hf_pwm_diagnostics_t &diagnostics) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  diagnostics.hardware_initialized = initialized_.load();
  diagnostics.fade_functionality_ready = fade_functionality_installed_;
  diagnostics.last_global_error = last_global_error_;

  // Count active channels and timers
  diagnostics.active_channels = 0;
  diagnostics.active_timers = 0;

  for (const auto &channel : channels_) {
    if (channel.enabled) {
      diagnostics.active_channels++;
    }
  }

  for (const auto &timer : timers_) {
    if (timer.in_use) {
      diagnostics.active_timers++;
    }
  }

  // Get system uptime (simplified - in real implementation would use esp_timer_get_time())
  diagnostics.system_uptime_ms = static_cast<uint32_t>(esp_timer_get_time() / 1000);

  return hf_pwm_err_t::PWM_SUCCESS;
}

//==============================================================================
// CALLBACKS (BasePwm Interface)
//==============================================================================

void EspPwm::SetPeriodCallback(hf_pwm_period_callback_t callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  period_callback_ = callback;
  period_callback_user_data_ = user_data;
}

void EspPwm::SetFaultCallback(hf_pwm_fault_callback_t callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  fault_callback_ = callback;
  fault_callback_user_data_ = user_data;
}

//==============================================================================
// ESP32C6-SPECIFIC FEATURES
//==============================================================================

hf_pwm_err_t EspPwm::SetHardwareFade(hf_channel_id_t channel_id, float target_duty_cycle,
                                     uint32_t fade_time_ms) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidDutyCycle(target_duty_cycle)) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE);
    return hf_pwm_err_t::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  // Ensure fade functionality is installed
  hf_pwm_err_t fade_result = InitializeFadeFunctionality();
  if (fade_result != hf_pwm_err_t::PWM_SUCCESS) {
    return fade_result;
  }

  uint32_t target_duty_raw =
      BasePwm::DutyCycleToRaw(target_duty_cycle, channels_[channel_id].config.resolution_bits);

  // Apply inversion if configured
  if (channels_[channel_id].config.output_invert) {
    uint8_t timer_id = channels_[channel_id].assigned_timer;
  uint32_t max_duty = (1U << timers_[timer_id].resolution_bits) - 1;
    target_duty_raw = max_duty - target_duty_raw;
  }

  esp_err_t ret = ledc_set_fade_with_time(
      LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), target_duty_raw, fade_time_ms);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_set_fade_with_time failed for channel %lu: %s", channel_id,
             esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  ret = ledc_fade_start(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                        LEDC_FADE_NO_WAIT);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_fade_start failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].fade_active = true;
  channels_[channel_id].raw_duty_value = target_duty_raw;

  statistics_.fade_operations_count++;
  statistics_.last_activity_timestamp = esp_timer_get_time();

  ESP_LOGD(TAG, "Hardware fade started for channel %lu: target=%.2f%%, time=%lu ms", channel_id,
           target_duty_cycle * 100.0f, fade_time_ms);

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::StopHardwareFade(hf_channel_id_t channel_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  esp_err_t ret = ledc_fade_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_fade_stop failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].fade_active = false;
  statistics_.fade_operations_count++;
  statistics_.last_activity_timestamp = esp_timer_get_time();
  ESP_LOGD(TAG, "Hardware fade stopped for channel %lu", channel_id);

  return hf_pwm_err_t::PWM_SUCCESS;
}

bool EspPwm::IsFadeActive(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return false;
  }

  return channels_[channel_id].fade_active;
}

hf_pwm_err_t EspPwm::SetIdleLevel(hf_channel_id_t channel_id, uint8_t idle_level) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (idle_level > 1) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  esp_err_t ret =
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), idle_level);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_stop failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT);
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  ESP_LOGD(TAG, "Idle level set to %d for channel %lu", idle_level, channel_id);
  return hf_pwm_err_t::PWM_SUCCESS;
}

int8_t EspPwm::GetTimerAssignment(hf_channel_id_t channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return -1;
  }

  return channels_[channel_id].assigned_timer;
}

hf_pwm_err_t EspPwm::ForceTimerAssignment(hf_channel_id_t channel_id, uint8_t timer_id) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  if (timer_id >= MAX_TIMERS) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER);
    return hf_pwm_err_t::PWM_ERR_INVALID_PARAMETER;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL);
    return hf_pwm_err_t::PWM_ERR_INVALID_CHANNEL;
  }

  // Release current timer and assign new one
  uint8_t old_timer = channels_[channel_id].assigned_timer;
  ReleaseTimerIfUnused(old_timer);

  // Configure new timer
  uint8_t current_timer = channels_[channel_id].assigned_timer;
  if (current_timer < MAX_TIMERS) {
    hf_pwm_err_t result = ConfigurePlatformTimer(timer_id, timers_[current_timer].frequency_hz,
                                                 timers_[current_timer].resolution_bits);
  if (result == hf_pwm_err_t::PWM_SUCCESS) {
    channels_[channel_id].assigned_timer = timer_id;
    timers_[timer_id].channel_count++;

    // Reconfigure the channel with new timer
    result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, timer_id);
  }

  return result;
}

//==============================================================================
// INTERNAL METHODS
//==============================================================================

bool EspPwm::IsValidChannelId(hf_channel_id_t channel_id) const noexcept {
  return (channel_id < MAX_CHANNELS);
}

int8_t EspPwm::FindOrAllocateTimer(uint32_t frequency_hz, uint8_t resolution_bits) noexcept {
  // First, try to find an existing timer with the same configuration
  for (uint8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use && timers_[timer_id].frequency_hz == frequency_hz &&
        timers_[timer_id].resolution_bits == resolution_bits) {
      return timer_id;
    }
  }

  // If not found, allocate a new timer
  for (uint8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (!timers_[timer_id].in_use) {
      timers_[timer_id].in_use = true;
      timers_[timer_id].frequency_hz = frequency_hz;
      timers_[timer_id].resolution_bits = resolution_bits;
      timers_[timer_id].channel_count = 0;
      return timer_id;
    }
  }

  return -1; // No timer available
}

void EspPwm::ReleaseTimerIfUnused(uint8_t timer_id) noexcept {
  if (timer_id >= MAX_TIMERS)
    return;

  // Decrement channel count
  if (timers_[timer_id].channel_count > 0) {
    timers_[timer_id].channel_count--;
  }

  // If no channels are using this timer, release it
  if (timers_[timer_id].channel_count == 0) {
    timers_[timer_id].in_use = false;
    ESP_LOGD(TAG, "Timer %d released", timer_id);
  }
}

hf_pwm_err_t EspPwm::ConfigurePlatformTimer(uint8_t timer_id, uint32_t frequency_hz,
                                            uint8_t resolution_bits) noexcept {
  ledc_timer_config_t timer_config = {};
  timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
  timer_config.timer_num = static_cast<ledc_timer_t>(timer_id);
  timer_config.duty_resolution = static_cast<ledc_timer_bit_t>(resolution_bits);
  timer_config.freq_hz = frequency_hz;
  ledc_clk_cfg_t clk_cfg = LEDC_AUTO_CLK;
  switch (clock_source_) {
  case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB:
    clk_cfg = LEDC_USE_APB_CLK;
    break;
  case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL:
    clk_cfg = LEDC_USE_XTAL_CLK;
    break;
  case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST:
    clk_cfg = LEDC_USE_RC_FAST_CLK;
    break;
  default:
    clk_cfg = LEDC_AUTO_CLK;
    break;
  }
  timer_config.clk_cfg = clk_cfg;

  esp_err_t ret = ledc_timer_config(&timer_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_timer_config failed for timer %d: %s", timer_id, esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  statistics_.last_activity_timestamp = esp_timer_get_time();

  ESP_LOGD(TAG, "Timer %d configured: freq=%lu Hz, resolution=%d bits", timer_id, frequency_hz,
           resolution_bits);
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::ConfigurePlatformChannel(hf_channel_id_t channel_id,
                                              const hf_pwm_channel_config_t &config,
                                              uint8_t timer_id) noexcept {
  ledc_channel_config_t channel_config = {};
  channel_config.speed_mode = LEDC_LOW_SPEED_MODE;
  channel_config.channel = static_cast<ledc_channel_t>(channel_id);
  channel_config.timer_sel = static_cast<ledc_timer_t>(timer_id);
  channel_config.intr_type = LEDC_INTR_DISABLE;
  channel_config.gpio_num = config.gpio_pin;

  // Calculate initial duty
  uint32_t initial_duty =
      BasePwm::DutyCycleToRaw(0.0f, resolution_bits);
  if (config.output_invert) {
    uint32_t max_duty = (1U << resolution_bits) - 1;
    initial_duty = max_duty - initial_duty;
  }
  channel_config.duty = initial_duty;

  channel_config.hpoint = 0; // Start at the beginning of the period

  esp_err_t ret = ledc_channel_config(&channel_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_channel_config failed for channel %lu: %s", channel_id,
             esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  // Increment timer usage count
  timers_[timer_id].channel_count++;

  ESP_LOGD(TAG, "Channel %lu configured: pin=%d, timer=%d, duty=%lu", channel_id, config.gpio_pin,
           timer_id, initial_duty);

  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::UpdatePlatformDuty(hf_channel_id_t channel_id,
                                        uint32_t raw_duty_value) noexcept {
  esp_err_t ret =
      ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), raw_duty_value);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_set_duty failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_update_duty failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  statistics_.last_activity_timestamp = esp_timer_get_time();

  return hf_pwm_err_t::PWM_SUCCESS;
}

void EspPwm::SetChannelError(hf_channel_id_t channel_id, hf_pwm_err_t error) noexcept {
  if (IsValidChannelId(channel_id)) {
    channels_[channel_id].last_error = error;
  }
  last_global_error_ = error;
  statistics_.error_count++;
}

void IRAM_ATTR EspPwm::InterruptHandler(hf_channel_id_t channel_id, void *user_data) noexcept {
  auto *self = static_cast<EspPwm *>(user_data);
  if (self) {
    self->HandleFadeComplete(channel_id);
  }
}

void EspPwm::HandleFadeComplete(hf_channel_id_t channel_id) noexcept {
  if (IsValidChannelId(channel_id)) {
    channels_[channel_id].fade_active = false;

    statistics_.fade_operations_count++;
    statistics_.last_activity_timestamp = esp_timer_get_time();

    // Call period callback if set
    if (period_callback_) {
      period_callback_(channel_id, period_callback_user_data_);
    }
  }
}

//==============================================================================
// ADDITIONAL ESP32C6-SPECIFIC METHODS
//==============================================================================

hf_pwm_err_t EspPwm::SetClockSource(hf_pwm_clock_source_t clock_source) noexcept {
  if (!EnsureInitialized()) {
    return hf_pwm_err_t::PWM_ERR_NOT_INITIALIZED;
  }

  RtosUniqueLock<RtosMutex> lock(mutex_);

  // Convert to ESP-IDF clock source
  ledc_clk_cfg_t esp_clock_source;
  switch (clock_source) {
  case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_APB:
    esp_clock_source = LEDC_USE_APB_CLK;
    break;
  case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_XTAL:
    esp_clock_source = LEDC_USE_XTAL_CLK;
    break;
  case hf_pwm_clock_source_t::HF_PWM_CLK_SRC_RC_FAST:
    esp_clock_source = LEDC_USE_RC_FAST_CLK;
    break;
  default:
    esp_clock_source = LEDC_AUTO_CLK;
    break;
  }

  // Update all active timers with new clock source
  for (uint8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    if (timers_[timer_id].in_use) {
      ledc_timer_config_t timer_config = {};
      timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
      timer_config.timer_num = static_cast<ledc_timer_t>(timer_id);
      timer_config.duty_resolution =
          static_cast<ledc_timer_bit_t>(timers_[timer_id].resolution_bits);
      timer_config.freq_hz = timers_[timer_id].frequency_hz;
      timer_config.clk_cfg = esp_clock_source;

      esp_err_t ret = ledc_timer_config(&timer_config);
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update timer %d clock source: %s", timer_id, esp_err_to_name(ret));
        return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
      }
    }
  }

  clock_source_ = clock_source;
  statistics_.last_activity_timestamp = esp_timer_get_time();
  ESP_LOGI(TAG, "Clock source updated to %d", static_cast<int>(clock_source));
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_clock_source_t EspPwm::GetClockSource() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return clock_source_;
}

hf_pwm_err_t EspPwm::InitializeFadeFunctionality() noexcept {
  if (fade_functionality_installed_) {
    return hf_pwm_err_t::PWM_SUCCESS; // Already installed
  }

  esp_err_t ret = ledc_fade_func_install(0); // Install without ISR
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install LEDC fade functionality: %s", esp_err_to_name(ret));
    return hf_pwm_err_t::PWM_ERR_HARDWARE_FAULT;
  }

  fade_functionality_installed_ = true;
  ESP_LOGI(TAG, "LEDC fade functionality installed");
  return hf_pwm_err_t::PWM_SUCCESS;
}

uint32_t EspPwm::CalculateClockDivider(uint32_t frequency_hz,
                                       uint8_t resolution_bits) const noexcept {
  if (frequency_hz == 0 || resolution_bits == 0) {
    return 1;
  }

  // Calculate maximum duty value for resolution
  uint32_t max_duty = (1U << resolution_bits) - 1;

  // Calculate required period
  uint32_t period = base_clock_hz_ / frequency_hz;

  // Calculate clock divider
  uint32_t clock_divider = period / max_duty;

  // Ensure minimum divider value
  if (clock_divider < 1) {
    clock_divider = 1;
  }

  // Ensure maximum divider value (ESP32C6 limit)
  if (clock_divider > 0x3FFFF) {
    clock_divider = 0x3FFFF;
  }

  return clock_divider;
}

hf_pwm_err_t EspPwm::InitializeTimers() noexcept {
  ESP_LOGD(TAG, "Initializing PWM timers");

  // Initialize timer states
  for (auto &timer : timers_) {
    timer = TimerState{};
  }

  ESP_LOGD(TAG, "PWM timers initialized");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::InitializeChannels() noexcept {
  ESP_LOGD(TAG, "Initializing PWM channels");

  // Initialize channel states
  for (auto &channel : channels_) {
    channel = ChannelState{};
  }

  // Initialize complementary pairs
  for (auto &pair : complementary_pairs_) {
    pair = ComplementaryPair{};
  }

  ESP_LOGD(TAG, "PWM channels initialized");
  return hf_pwm_err_t::PWM_SUCCESS;
}

hf_pwm_err_t EspPwm::EnableFade() noexcept {
  if (fade_functionality_installed_) {
    return hf_pwm_err_t::PWM_SUCCESS;
  }

  ESP_LOGD(TAG, "Enabling LEDC fade functionality");

  esp_err_t result = ledc_fade_func_install(0);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install LEDC fade function: %s", esp_err_to_name(result));
    return hf_pwm_err_t::PWM_ERR_FAILURE;
  }

  fade_functionality_installed_ = true;
  ESP_LOGD(TAG, "LEDC fade functionality enabled");
  return hf_pwm_err_t::PWM_SUCCESS;
}
