/**
 * @file SfPwm.cpp
 * @brief Thread-safe PWM interface wrapper implementation.
 */

#include "../thread_safe/SfPwm.h"
#include <chrono>

namespace {
constexpr std::chrono::milliseconds DEFAULT_TIMEOUT{100};
}

SfPwm::SfPwm(std::unique_ptr<BasePwm> pwm_impl)
    : pwm_impl_(std::move(pwm_impl)), is_initialized_(false), mutex_timeout_(DEFAULT_TIMEOUT) {}

SfPwm::~SfPwm() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_initialized_ && pwm_impl_) {
    pwm_impl_->Deinitialize();
  }
}

void SfPwm::SetMutexTimeout(std::chrono::milliseconds timeout) {
  std::lock_guard<std::mutex> lock(mutex_);
  mutex_timeout_ = timeout;
}

HfPwmErr SfPwm::Initialize() {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_) {
    return HfPwmErr::PWM_INVALID_ARGUMENT;
  }

  if (is_initialized_) {
    return HfPwmErr::PWM_SUCCESS; // Already initialized
  }

  HfPwmErr result = pwm_impl_->Initialize();
  if (result == HfPwmErr::PWM_SUCCESS) {
    is_initialized_ = true;
  }

  return result;
}

HfPwmErr SfPwm::Deinitialize() {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  HfPwmErr result = pwm_impl_->Deinitialize();
  if (result == HfPwmErr::PWM_SUCCESS) {
    is_initialized_ = false;
  }

  return result;
}

HfPwmErr SfPwm::ConfigureChannel(uint8_t channel_id, const PwmChannelConfig &config) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->ConfigureChannel(channel_id, config);
}

HfPwmErr SfPwm::SetDutyCycle(uint8_t channel_id, float duty_cycle) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->SetDutyCycle(channel_id, duty_cycle);
}

HfPwmErr SfPwm::SetFrequency(uint8_t channel_id, uint32_t frequency_hz) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->SetFrequency(channel_id, frequency_hz);
}

HfPwmErr SfPwm::Start(uint8_t channel_id) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->Start(channel_id);
}

HfPwmErr SfPwm::Stop(uint8_t channel_id) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->Stop(channel_id);
}

HfPwmErr SfPwm::GetDutyCycle(uint8_t channel_id, float &duty_cycle) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->GetDutyCycle(channel_id, duty_cycle);
}

HfPwmErr SfPwm::GetFrequency(uint8_t channel_id, uint32_t &frequency_hz) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->GetFrequency(channel_id, frequency_hz);
}

bool SfPwm::IsChannelActive(uint8_t channel_id) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return false; // Default to inactive on timeout
  }

  if (!pwm_impl_ || !is_initialized_) {
    return false;
  }

  return pwm_impl_->IsChannelActive(channel_id);
}

uint8_t SfPwm::GetMaxChannels() {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return 0; // Default to no channels on timeout
  }

  if (!pwm_impl_) {
    return 0;
  }

  return pwm_impl_->GetMaxChannels();
}

// Advanced features (ESP32-specific)
HfPwmErr SfPwm::SetPhase(uint8_t channel_id, float phase_degrees) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->SetPhase(channel_id, phase_degrees);
}

HfPwmErr SfPwm::ConfigureFade(uint8_t channel_id, const PwmFadeConfig &fade_config) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->ConfigureFade(channel_id, fade_config);
}

HfPwmErr SfPwm::StartFade(uint8_t channel_id) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->StartFade(channel_id);
}

HfPwmErr SfPwm::ConfigureComplementary(uint8_t primary_channel, uint8_t secondary_channel,
                                       const PwmComplementaryConfig &comp_config) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->ConfigureComplementary(primary_channel, secondary_channel, comp_config);
}

HfPwmErr SfPwm::SetDeadTime(uint8_t channel_id, uint16_t dead_time_ns) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->SetDeadTime(channel_id, dead_time_ns);
}

// Callback management
HfPwmErr SfPwm::RegisterCallback(uint8_t channel_id, PwmCallbackType callback_type,
                                 PwmCallback callback, void *user_data) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->RegisterCallback(channel_id, callback_type, callback, user_data);
}

HfPwmErr SfPwm::UnregisterCallback(uint8_t channel_id, PwmCallbackType callback_type) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->UnregisterCallback(channel_id, callback_type);
}

// Multi-channel operations
HfPwmErr SfPwm::StartMultiple(const uint8_t *channel_ids, uint8_t num_channels) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->StartMultiple(channel_ids, num_channels);
}

HfPwmErr SfPwm::StopMultiple(const uint8_t *channel_ids, uint8_t num_channels) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->StopMultiple(channel_ids, num_channels);
}

HfPwmErr SfPwm::SetDutyCycleMultiple(const uint8_t *channel_ids, const float *duty_cycles,
                                     uint8_t num_channels) {
  std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
  if (!lock.try_lock_for(mutex_timeout_)) {
    return HfPwmErr::PWM_TIMEOUT;
  }

  if (!pwm_impl_ || !is_initialized_) {
    return HfPwmErr::PWM_NOT_INITIALIZED;
  }

  return pwm_impl_->SetDutyCycleMultiple(channel_ids, duty_cycles, num_channels);
}

bool SfPwm::IsInitialized() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return is_initialized_;
}

const BasePwm *SfPwm::GetImplementation() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return pwm_impl_.get();
}
