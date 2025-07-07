/**
 * @file McuPwm.cpp
 * @brief Implementation of MCU-integrated PWM controller for ESP32C6.
 *
 * This file provides the implementation for PWM generation using the
 * microcontroller's built-in LEDC peripheral. All platform-specific types and
 * implementations are isolated through McuTypes.h. The implementation supports
 * multiple channels, configurable frequency and resolution, complementary outputs
 * with deadtime, hardware fade support, and interrupt-driven period callbacks.
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */
#include "McuPwm.h"
#include <algorithm>

// Platform-specific includes and definitions
#ifdef HF_MCU_FAMILY_ESP32
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/ledc_hal.h"
#include "soc/ledc_reg.h"

#else
#error "Unsupported MCU platform. Please add support for your target MCU."
#endif

static const char *TAG = "McuPwm";

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

McuPwm::McuPwm(uint32_t base_clock_hz) noexcept
    : initialized_(false), base_clock_hz_(base_clock_hz), period_callback_(nullptr),
      period_callback_user_data_(nullptr), fault_callback_(nullptr),
      fault_callback_user_data_(nullptr), last_global_error_(HfPwmErr::PWM_SUCCESS) {
  ESP_LOGI(TAG, "McuPwm constructor - base clock: %lu Hz", base_clock_hz_);
}

McuPwm::~McuPwm() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  if (initialized_) {
    ESP_LOGI(TAG, "McuPwm destructor - cleaning up");
    Deinitialize();
  }
}

//==============================================================================
// LIFECYCLE (BasePwm Interface)
//==============================================================================

HfPwmErr McuPwm::Initialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (initialized_) {
    ESP_LOGW(TAG, "PWM already initialized");
    return HfPwmErr::PWM_ERR_ALREADY_INITIALIZED;
  }

  ESP_LOGI(TAG, "Initializing MCU PWM system");

#ifdef HF_MCU_FAMILY_ESP32
  // ESP32C6 LEDC initialization - configure speed mode
  for (uint8_t timer_id = 0; timer_id < MAX_TIMERS; timer_id++) {
    timers_[timer_id] = TimerState{};
  }

  // Reset all channel states
  for (uint8_t channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    channels_[channel_id] = ChannelState{};
  }

  // Reset complementary pairs
  for (auto &pair : complementary_pairs_) {
    pair = ComplementaryPair{};
  }

  initialized_ = true;
  last_global_error_ = HfPwmErr::PWM_SUCCESS;

  ESP_LOGI(TAG, "MCU PWM system initialized successfully");
  return HfPwmErr::PWM_SUCCESS;

#else
  ESP_LOGE(TAG, "Platform not supported");
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

HfPwmErr McuPwm::Deinitialize() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  ESP_LOGI(TAG, "Deinitializing MCU PWM system");

#ifdef HF_MCU_FAMILY_ESP32
  // Stop all channels first
  for (HfChannelId channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
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

  initialized_ = false;
  ESP_LOGI(TAG, "MCU PWM system deinitialized");
  return HfPwmErr::PWM_SUCCESS;

#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

bool McuPwm::IsInitialized() const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  return initialized_;
}

//==============================================================================
// CHANNEL MANAGEMENT (BasePwm Interface)
//==============================================================================

HfPwmErr McuPwm::ConfigureChannel(HfChannelId channel_id, const PwmChannelConfig &config) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  // Validate configuration
  if (config.output_pin < 0) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_PARAMETER);
    return HfPwmErr::PWM_ERR_INVALID_PARAMETER;
  }

  if (!BasePwm::IsValidDutyCycle(config.initial_duty_cycle)) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_DUTY_CYCLE);
    return HfPwmErr::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  if (!BasePwm::IsValidFrequency(config.frequency_hz, MIN_FREQUENCY, MAX_FREQUENCY)) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_FREQUENCY);
    return HfPwmErr::PWM_ERR_INVALID_FREQUENCY;
  }

  // Find or allocate a timer for this frequency/resolution combination
  int8_t timer_id = FindOrAllocateTimer(config.frequency_hz, config.resolution_bits);
  if (timer_id < 0) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_TIMER_CONFLICT);
    return HfPwmErr::PWM_ERR_TIMER_CONFLICT;
  }

  // Configure the platform timer if needed
  HfPwmErr timer_result =
      ConfigurePlatformTimer(timer_id, config.frequency_hz, config.resolution_bits);
  if (timer_result != HfPwmErr::PWM_SUCCESS) {
    SetChannelError(channel_id, timer_result);
    return timer_result;
  }

  // Configure the platform channel
  HfPwmErr channel_result = ConfigurePlatformChannel(channel_id, config, timer_id);
  if (channel_result != HfPwmErr::PWM_SUCCESS) {
    SetChannelError(channel_id, channel_result);
    return channel_result;
  }

  // Update internal state
  channels_[channel_id].configured = true;
  channels_[channel_id].config = config;
  channels_[channel_id].assigned_timer = timer_id;
  channels_[channel_id].raw_duty_value =
      BasePwm::DutyCycleToRaw(config.initial_duty_cycle, config.resolution_bits);
  channels_[channel_id].last_error = HfPwmErr::PWM_SUCCESS;

  ESP_LOGI(TAG, "Channel %lu configured: pin=%d, freq=%lu Hz, res=%d bits, timer=%d", channel_id,
           config.output_pin, config.frequency_hz, config.resolution_bits, timer_id);

  return HfPwmErr::PWM_SUCCESS;
}

HfPwmErr McuPwm::EnableChannel(HfChannelId channel_id) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_CHANNEL_NOT_AVAILABLE);
    return HfPwmErr::PWM_ERR_CHANNEL_NOT_AVAILABLE;
  }

  if (channels_[channel_id].enabled) {
    return HfPwmErr::PWM_SUCCESS; // Already enabled
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Start the channel with current duty cycle
  esp_err_t ret =
      ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                               channels_[channel_id].raw_duty_value,
                               0 // No hpoint (phase shift)
      );

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_HARDWARE_FAULT);
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].enabled = true;
  ESP_LOGI(TAG, "Channel %lu enabled", channel_id);
  return HfPwmErr::PWM_SUCCESS;

#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

HfPwmErr McuPwm::DisableChannel(HfChannelId channel_id) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].enabled) {
    return HfPwmErr::PWM_SUCCESS; // Already disabled
  }

#ifdef HF_MCU_FAMILY_ESP32
  // Stop the channel based on idle state
  uint32_t idle_level = (channels_[channel_id].config.idle_state == PwmIdleState::High) ? 1 : 0;
  if (channels_[channel_id].config.invert_output) {
    idle_level = 1 - idle_level;
  }

  esp_err_t ret =
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), idle_level);

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disable channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_HARDWARE_FAULT);
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].enabled = false;
  ESP_LOGI(TAG, "Channel %lu disabled", channel_id);
  return HfPwmErr::PWM_SUCCESS;

#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

bool McuPwm::IsChannelEnabled(HfChannelId channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return false;
  }
  return channels_[channel_id].enabled;
}

//==============================================================================
// PWM CONTROL (BasePwm Interface)
//==============================================================================

HfPwmErr McuPwm::SetDutyCycle(HfChannelId channel_id, float duty_cycle) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_CHANNEL);
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidDutyCycle(duty_cycle)) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_DUTY_CYCLE);
    return HfPwmErr::PWM_ERR_INVALID_DUTY_CYCLE;
  }

  uint32_t raw_duty =
      BasePwm::DutyCycleToRaw(duty_cycle, channels_[channel_id].config.resolution_bits);
  return SetDutyCycleRaw(channel_id, raw_duty);
}

HfPwmErr McuPwm::SetDutyCycleRaw(HfChannelId channel_id, uint32_t raw_value) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_CHANNEL);
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  uint32_t max_duty = (1U << channels_[channel_id].config.resolution_bits) - 1;
  if (raw_value > max_duty) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_DUTY_OUT_OF_RANGE);
    return HfPwmErr::PWM_ERR_DUTY_OUT_OF_RANGE;
  }

  // Apply inversion if configured
  uint32_t actual_duty = raw_value;
  if (channels_[channel_id].config.invert_output) {
    actual_duty = max_duty - raw_value;
  }

  HfPwmErr result = UpdatePlatformDuty(channel_id, actual_duty);
  if (result == HfPwmErr::PWM_SUCCESS) {
    channels_[channel_id].raw_duty_value = raw_value;
    channels_[channel_id].last_error = HfPwmErr::PWM_SUCCESS;
  }

  return result;
}

HfPwmErr McuPwm::SetFrequency(HfChannelId channel_id, HfFrequencyHz frequency_hz) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_CHANNEL);
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidFrequency(frequency_hz, MIN_FREQUENCY, MAX_FREQUENCY)) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_FREQUENCY);
    return HfPwmErr::PWM_ERR_INVALID_FREQUENCY;
  }

  // Check if we can update the existing timer or need a new one
  uint8_t current_timer = channels_[channel_id].assigned_timer;
  bool can_update_existing = (timers_[current_timer].channel_count == 1);

  if (can_update_existing) {
    // We can update the existing timer since this is the only channel using it
    HfPwmErr result = ConfigurePlatformTimer(current_timer, frequency_hz,
                                             channels_[channel_id].config.resolution_bits);
    if (result == HfPwmErr::PWM_SUCCESS) {
      channels_[channel_id].config.frequency_hz = frequency_hz;
      timers_[current_timer].frequency_hz = frequency_hz;
    }
    return result;
  } else {
    // Need to find a new timer
    int8_t new_timer =
        FindOrAllocateTimer(frequency_hz, channels_[channel_id].config.resolution_bits);
    if (new_timer < 0) {
      SetChannelError(channel_id, HfPwmErr::PWM_ERR_TIMER_CONFLICT);
      return HfPwmErr::PWM_ERR_TIMER_CONFLICT;
    }

    // Reconfigure the channel with the new timer
    HfPwmErr result = ConfigurePlatformChannel(channel_id, channels_[channel_id].config, new_timer);
    if (result == HfPwmErr::PWM_SUCCESS) {
      // Release the old timer and assign the new one
      ReleaseTimerIfUnused(current_timer);
      channels_[channel_id].assigned_timer = new_timer;
      channels_[channel_id].config.frequency_hz = frequency_hz;
    }
    return result;
  }
}

HfPwmErr McuPwm::SetPhaseShift(HfChannelId channel_id, float phase_shift_degrees) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  // ESP32C6 LEDC doesn't support phase shifting directly
  // This would require advanced timer configuration
  ESP_LOGW(TAG, "Phase shift not supported on ESP32C6 LEDC peripheral");
  SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_PARAMETER);
  return HfPwmErr::PWM_ERR_INVALID_PARAMETER;
}

//==============================================================================
// ADVANCED FEATURES (BasePwm Interface)
//==============================================================================

HfPwmErr McuPwm::StartAll() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  HfPwmErr result = HfPwmErr::PWM_SUCCESS;

  for (HfChannelId channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured && !channels_[channel_id].enabled) {
      HfPwmErr channel_result = EnableChannel(channel_id);
      if (channel_result != HfPwmErr::PWM_SUCCESS) {
        result = channel_result; // Keep the last error
      }
    }
  }

  return result;
}

HfPwmErr McuPwm::StopAll() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  HfPwmErr result = HfPwmErr::PWM_SUCCESS;

  for (HfChannelId channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].enabled) {
      HfPwmErr channel_result = DisableChannel(channel_id);
      if (channel_result != HfPwmErr::PWM_SUCCESS) {
        result = channel_result; // Keep the last error
      }
    }
  }

  return result;
}

HfPwmErr McuPwm::UpdateAll() noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

#ifdef HF_MCU_FAMILY_ESP32
  // For ESP32C6, we can update all channels simultaneously
  for (HfChannelId channel_id = 0; channel_id < MAX_CHANNELS; channel_id++) {
    if (channels_[channel_id].configured && channels_[channel_id].enabled) {
      esp_err_t ret =
          ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
      if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ledc_update_duty failed for channel %lu: %s", channel_id,
                 esp_err_to_name(ret));
        SetChannelError(channel_id, HfPwmErr::PWM_ERR_HARDWARE_FAULT);
        return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
      }
    }
  }
  return HfPwmErr::PWM_SUCCESS;
#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

HfPwmErr McuPwm::SetComplementaryOutput(HfChannelId primary_channel,
                                        HfChannelId complementary_channel,
                                        uint32_t deadtime_ns) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(primary_channel) || !IsValidChannelId(complementary_channel)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (primary_channel == complementary_channel) {
    return HfPwmErr::PWM_ERR_INVALID_PARAMETER;
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
      return HfPwmErr::PWM_SUCCESS;
    }
  }

  return HfPwmErr::PWM_ERR_INSUFFICIENT_CHANNELS;
}

//==============================================================================
// STATUS AND INFORMATION (BasePwm Interface)
//==============================================================================

float McuPwm::GetDutyCycle(HfChannelId channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return -1.0f;
  }

  return BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value,
                                 channels_[channel_id].config.resolution_bits);
}

HfFrequencyHz McuPwm::GetFrequency(HfChannelId channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return 0;
  }

  return channels_[channel_id].config.frequency_hz;
}

HfPwmErr McuPwm::GetChannelStatus(HfChannelId channel_id, PwmChannelStatus &status) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    status = PwmChannelStatus{}; // Reset to default
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  status.is_enabled = channels_[channel_id].enabled;
  status.is_running = channels_[channel_id].enabled;
  status.current_frequency_hz = channels_[channel_id].config.frequency_hz;
  status.current_duty_cycle = BasePwm::RawToDutyCycle(channels_[channel_id].raw_duty_value,
                                                      channels_[channel_id].config.resolution_bits);
  status.raw_duty_value = channels_[channel_id].raw_duty_value;
  status.last_error = channels_[channel_id].last_error;

  return HfPwmErr::PWM_SUCCESS;
}

HfPwmErr McuPwm::GetCapabilities(PwmCapabilities &capabilities) const noexcept {
  capabilities.max_channels = MAX_CHANNELS;
  capabilities.max_timers = MAX_TIMERS;
  capabilities.min_frequency_hz = MIN_FREQUENCY;
  capabilities.max_frequency_hz = MAX_FREQUENCY;
  capabilities.min_resolution_bits = 1;
  capabilities.max_resolution_bits = MAX_RESOLUTION;
  capabilities.supports_complementary = true;   // Software implementation
  capabilities.supports_center_aligned = false; // Not supported by LEDC
  capabilities.supports_deadtime = true;        // Software implementation
  capabilities.supports_phase_shift = false;    // Not supported by LEDC

  return HfPwmErr::PWM_SUCCESS;
}

HfPwmErr McuPwm::GetLastError(HfChannelId channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  return channels_[channel_id].last_error;
}

//==============================================================================
// CALLBACKS (BasePwm Interface)
//==============================================================================

void McuPwm::SetPeriodCallback(PwmPeriodCallback callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  period_callback_ = callback;
  period_callback_user_data_ = user_data;
}

void McuPwm::SetFaultCallback(PwmFaultCallback callback, void *user_data) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);
  fault_callback_ = callback;
  fault_callback_user_data_ = user_data;
}

//==============================================================================
// ESP32C6-SPECIFIC FEATURES
//==============================================================================

HfPwmErr McuPwm::SetHardwareFade(HfChannelId channel_id, float target_duty_cycle,
                                 uint32_t fade_time_ms) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_CHANNEL);
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!BasePwm::IsValidDutyCycle(target_duty_cycle)) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_DUTY_CYCLE);
    return HfPwmErr::PWM_ERR_INVALID_DUTY_CYCLE;
  }

#ifdef HF_MCU_FAMILY_ESP32
  uint32_t target_duty_raw =
      BasePwm::DutyCycleToRaw(target_duty_cycle, channels_[channel_id].config.resolution_bits);

  // Apply inversion if configured
  if (channels_[channel_id].config.invert_output) {
    uint32_t max_duty = (1U << channels_[channel_id].config.resolution_bits) - 1;
    target_duty_raw = max_duty - target_duty_raw;
  }

  esp_err_t ret = ledc_set_fade_with_time(
      LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), target_duty_raw, fade_time_ms);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_set_fade_with_time failed for channel %lu: %s", channel_id,
             esp_err_to_name(ret));
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_HARDWARE_FAULT);
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  ret = ledc_fade_start(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id),
                        LEDC_FADE_NO_WAIT);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_fade_start failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_HARDWARE_FAULT);
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].fade_active = true;
  ESP_LOGD(TAG, "Hardware fade started for channel %lu: target=%.2f%%, time=%lu ms", channel_id,
           target_duty_cycle * 100.0f, fade_time_ms);

  return HfPwmErr::PWM_SUCCESS;
#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

HfPwmErr McuPwm::StopHardwareFade(HfChannelId channel_id) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_CHANNEL);
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret = ledc_fade_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_fade_stop failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_HARDWARE_FAULT);
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  channels_[channel_id].fade_active = false;
  ESP_LOGD(TAG, "Hardware fade stopped for channel %lu", channel_id);

  return HfPwmErr::PWM_SUCCESS;
#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

bool McuPwm::IsFadeActive(HfChannelId channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return false;
  }

  return channels_[channel_id].fade_active;
}

HfPwmErr McuPwm::SetIdleLevel(HfChannelId channel_id, uint8_t idle_level) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_CHANNEL);
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (idle_level > 1) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_PARAMETER);
    return HfPwmErr::PWM_ERR_INVALID_PARAMETER;
  }

#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret =
      ledc_stop(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), idle_level);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_stop failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_HARDWARE_FAULT);
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  ESP_LOGD(TAG, "Idle level set to %d for channel %lu", idle_level, channel_id);
  return HfPwmErr::PWM_SUCCESS;
#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

int8_t McuPwm::GetTimerAssignment(HfChannelId channel_id) const noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!IsValidChannelId(channel_id) || !channels_[channel_id].configured) {
    return -1;
  }

  return channels_[channel_id].assigned_timer;
}

HfPwmErr McuPwm::ForceTimerAssignment(HfChannelId channel_id, uint8_t timer_id) noexcept {
  RtosUniqueLock<RtosMutex> lock(mutex_);

  if (!initialized_) {
    return HfPwmErr::PWM_ERR_NOT_INITIALIZED;
  }

  if (!IsValidChannelId(channel_id)) {
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  if (timer_id >= MAX_TIMERS) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_PARAMETER);
    return HfPwmErr::PWM_ERR_INVALID_PARAMETER;
  }

  if (!channels_[channel_id].configured) {
    SetChannelError(channel_id, HfPwmErr::PWM_ERR_INVALID_CHANNEL);
    return HfPwmErr::PWM_ERR_INVALID_CHANNEL;
  }

  // Release current timer and assign new one
  uint8_t old_timer = channels_[channel_id].assigned_timer;
  ReleaseTimerIfUnused(old_timer);

  // Configure new timer
  HfPwmErr result = ConfigurePlatformTimer(timer_id, channels_[channel_id].config.frequency_hz,
                                           channels_[channel_id].config.resolution_bits);
  if (result == HfPwmErr::PWM_SUCCESS) {
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

bool McuPwm::IsValidChannelId(HfChannelId channel_id) const noexcept {
  return (channel_id < MAX_CHANNELS);
}

int8_t McuPwm::FindOrAllocateTimer(uint32_t frequency_hz, uint8_t resolution_bits) noexcept {
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

void McuPwm::ReleaseTimerIfUnused(uint8_t timer_id) noexcept {
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

HfPwmErr McuPwm::ConfigurePlatformTimer(uint8_t timer_id, uint32_t frequency_hz,
                                        uint8_t resolution_bits) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  ledc_timer_config_t timer_config = {};
  timer_config.speed_mode = LEDC_LOW_SPEED_MODE;
  timer_config.timer_num = static_cast<ledc_timer_t>(timer_id);
  timer_config.duty_resolution = static_cast<ledc_timer_bit_t>(resolution_bits);
  timer_config.freq_hz = frequency_hz;
  timer_config.clk_cfg = LEDC_AUTO_CLK;

  esp_err_t ret = ledc_timer_config(&timer_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_timer_config failed for timer %d: %s", timer_id, esp_err_to_name(ret));
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  ESP_LOGD(TAG, "Timer %d configured: freq=%lu Hz, resolution=%d bits", timer_id, frequency_hz,
           resolution_bits);
  return HfPwmErr::PWM_SUCCESS;
#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

HfPwmErr McuPwm::ConfigurePlatformChannel(HfChannelId channel_id, const PwmChannelConfig &config,
                                          uint8_t timer_id) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  ledc_channel_config_t channel_config = {};
  channel_config.speed_mode = LEDC_LOW_SPEED_MODE;
  channel_config.channel = static_cast<ledc_channel_t>(channel_id);
  channel_config.timer_sel = static_cast<ledc_timer_t>(timer_id);
  channel_config.intr_type = LEDC_INTR_DISABLE;
  channel_config.gpio_num = config.output_pin;

  // Calculate initial duty
  uint32_t initial_duty =
      BasePwm::DutyCycleToRaw(config.initial_duty_cycle, config.resolution_bits);
  if (config.invert_output) {
    uint32_t max_duty = (1U << config.resolution_bits) - 1;
    initial_duty = max_duty - initial_duty;
  }
  channel_config.duty = initial_duty;

  channel_config.hpoint = 0; // Start at the beginning of the period

  esp_err_t ret = ledc_channel_config(&channel_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_channel_config failed for channel %lu: %s", channel_id,
             esp_err_to_name(ret));
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  // Increment timer usage count
  timers_[timer_id].channel_count++;

  ESP_LOGD(TAG, "Channel %lu configured: pin=%d, timer=%d, duty=%lu", channel_id, config.output_pin,
           timer_id, initial_duty);

  return HfPwmErr::PWM_SUCCESS;
#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

HfPwmErr McuPwm::UpdatePlatformDuty(HfChannelId channel_id, uint32_t raw_duty_value) noexcept {
#ifdef HF_MCU_FAMILY_ESP32
  esp_err_t ret =
      ledc_set_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id), raw_duty_value);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_set_duty failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, static_cast<ledc_channel_t>(channel_id));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "ledc_update_duty failed for channel %lu: %s", channel_id, esp_err_to_name(ret));
    return HfPwmErr::PWM_ERR_HARDWARE_FAULT;
  }

  return HfPwmErr::PWM_SUCCESS;
#else
  return HfPwmErr::PWM_ERR_FAILURE;
#endif
}

void McuPwm::SetChannelError(HfChannelId channel_id, HfPwmErr error) noexcept {
  if (IsValidChannelId(channel_id)) {
    channels_[channel_id].last_error = error;
  }
  last_global_error_ = error;
}

void IRAM_ATTR McuPwm::InterruptHandler(HfChannelId channel_id, void *user_data) noexcept {
  auto *self = static_cast<McuPwm *>(user_data);
  if (self) {
    self->HandleFadeComplete(channel_id);
  }
}

void McuPwm::HandleFadeComplete(HfChannelId channel_id) noexcept {
  if (IsValidChannelId(channel_id)) {
    channels_[channel_id].fade_active = false;

    // Call period callback if set
    if (period_callback_) {
      period_callback_(channel_id, period_callback_user_data_);
    }
  }
}
