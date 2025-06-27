/**
 * @file SfAdc.cpp
 * @brief Implementation of enhanced thread-safe ADC interface wrapper.
 *
 * This file implements the enhanced thread-safe wrapper around the BaseAdc interface
 * for safe use in multi-threaded applications with advanced features like:
 * - Lock-free read operations
 * - Batch conversion operations
 * - Advanced threading statistics
 * - Configurable timeout handling
 * - Non-blocking and blocking convenience methods
 */
#include "../thread_safe/SfAdc.h"
#include <algorithm>

namespace {
// Default timeout for mutex operations (milliseconds)
constexpr uint32_t DEFAULT_TIMEOUT_MS = 5000;

// Maximum batch size for conversions
constexpr size_t MAX_BATCH_SIZE = 32;
} // namespace

//==============================================================================
// CONSTRUCTOR AND DESTRUCTOR
//==============================================================================

SfAdc::SfAdc(std::unique_ptr<BaseAdc> adc_impl) noexcept
    : adc_impl_(std::move(adc_impl)), initialized_(false), mutex_timeout_ms_(DEFAULT_TIMEOUT_MS),
      stats_{} {
  if (!adc_impl_) {
    return;
  }
}

SfAdc::~SfAdc() noexcept {
  if (initialized_.load()) {
    Deinitialize();
  }
}

//==============================================================================
// CONFIGURATION AND CONTROL
//==============================================================================

HfAdcErr SfAdc::Initialize() noexcept {
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_, mutex_timeout_ms_);
  if (!lock.IsLocked()) {
    stats_.timeout_count_.fetch_add(1);
    return HfAdcErr::ADC_ERR_TIMEOUT;
  }

  if (!adc_impl_) {
    return HfAdcErr::ADC_ERR_NULL_POINTER;
  }

  if (initialized_.load()) {
    return HfAdcErr::ADC_ERR_ALREADY_INITIALIZED;
  }

  auto result = adc_impl_->Initialize();
  if (result == HfAdcErr::ADC_SUCCESS) {
    initialized_.store(true);
    stats_.operation_count_.fetch_add(1);
  }

  return result;
}

HfAdcErr SfAdc::Deinitialize() noexcept {
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_, mutex_timeout_ms_);
  if (!lock.IsLocked()) {
    stats_.timeout_count_.fetch_add(1);
    return HfAdcErr::ADC_ERR_TIMEOUT;
  }

  if (!adc_impl_) {
    return HfAdcErr::ADC_ERR_NULL_POINTER;
  }

  if (!initialized_.load()) {
    return HfAdcErr::ADC_ERR_NOT_INITIALIZED;
  }

  auto result = adc_impl_->Deinitialize();
  if (result == HfAdcErr::ADC_SUCCESS) {
    initialized_.store(false);
    stats_.operation_count_.fetch_add(1);
  }

  return result;
}

//==============================================================================
// CHANNEL OPERATIONS
//==============================================================================

HfAdcErr SfAdc::ConfigureChannel(hf_adc_channel_t channel,
                                 const AdcChannelConfig &config) noexcept {
  RtosUniqueLock<RtosSharedMutex> lock(rw_mutex_, mutex_timeout_ms_);
  if (!lock.IsLocked()) {
    stats_.timeout_count_.fetch_add(1);
    return HfAdcErr::ADC_ERR_TIMEOUT;
  }

  if (!adc_impl_) {
    return HfAdcErr::ADC_ERR_NULL_POINTER;
  }

  auto result = adc_impl_->ConfigureChannel(channel, config);
  if (result == HfAdcErr::ADC_SUCCESS) {
    stats_.operation_count_.fetch_add(1);
  }

  return result;
}

HfAdcErr SfAdc::ReadChannel(hf_adc_channel_t channel, uint32_t &raw_value) noexcept {
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_, mutex_timeout_ms_);
  if (!lock.IsLocked()) {
    stats_.timeout_count_.fetch_add(1);
    return HfAdcErr::ADC_ERR_TIMEOUT;
  }

  if (!adc_impl_) {
    return HfAdcErr::ADC_ERR_NULL_POINTER;
  }

  auto result = adc_impl_->ReadChannel(channel, raw_value);
  if (result == HfAdcErr::ADC_SUCCESS) {
    stats_.read_count_.fetch_add(1);
  }

  return result;
}

HfAdcErr SfAdc::ReadChannelVoltage(hf_adc_channel_t channel, float &voltage_v) noexcept {
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_, mutex_timeout_ms_);
  if (!lock.IsLocked()) {
    stats_.timeout_count_.fetch_add(1);
    return HfAdcErr::ADC_ERR_TIMEOUT;
  }

  if (!adc_impl_) {
    return HfAdcErr::ADC_ERR_NULL_POINTER;
  }

  auto result = adc_impl_->ReadChannelVoltage(channel, voltage_v);
  if (result == HfAdcErr::ADC_SUCCESS) {
    stats_.read_count_.fetch_add(1);
  }

  return result;
}

//==============================================================================
// ADVANCED FEATURES
//==============================================================================

void SfAdc::SetMutexTimeout(uint32_t timeout_ms) {
  mutex_timeout_ms_ = timeout_ms;
}

SfAdc::ThreadingStats SfAdc::GetThreadingStats() const {
  return stats_;
}

void SfAdc::ResetThreadingStats() {
  stats_.operation_count_.store(0);
  stats_.read_count_.store(0);
  stats_.timeout_count_.store(0);
  stats_.error_count_.store(0);
}

bool SfAdc::IsInitialized() const noexcept {
  return initialized_.load();
}

//==============================================================================
// BATCH OPERATIONS
//==============================================================================

HfAdcErr SfAdc::ReadMultipleChannels(const hf_adc_channel_t *channels, uint32_t *raw_values,
                                     size_t channel_count) noexcept {
  if (!channels || !raw_values || channel_count == 0 || channel_count > MAX_BATCH_SIZE) {
    return HfAdcErr::ADC_ERR_INVALID_PARAMETER;
  }

  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_, mutex_timeout_ms_);
  if (!lock.IsLocked()) {
    stats_.timeout_count_.fetch_add(1);
    return HfAdcErr::ADC_ERR_TIMEOUT;
  }

  if (!adc_impl_) {
    return HfAdcErr::ADC_ERR_NULL_POINTER;
  }

  HfAdcErr first_error = HfAdcErr::ADC_SUCCESS;
  size_t successful_reads = 0;

  for (size_t i = 0; i < channel_count; ++i) {
    auto result = adc_impl_->ReadChannel(channels[i], raw_values[i]);
    if (result == HfAdcErr::ADC_SUCCESS) {
      ++successful_reads;
    } else if (first_error == HfAdcErr::ADC_SUCCESS) {
      first_error = result;
    }
  }

  stats_.read_count_.fetch_add(successful_reads);
  if (first_error != HfAdcErr::ADC_SUCCESS) {
    stats_.error_count_.fetch_add(1);
  }

  return first_error;
}

//==============================================================================
// UTILITY METHODS
//==============================================================================

const char *SfAdc::GetErrorString(HfAdcErr error) const noexcept {
  if (adc_impl_) {
    return adc_impl_->GetErrorString(error);
  }
  return "Unknown error - ADC implementation not available";
}

uint8_t SfAdc::GetMaxChannels() const noexcept {
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_, mutex_timeout_ms_);
  if (!lock.IsLocked() || !adc_impl_) {
    return 0;
  }

  return adc_impl_->GetMaxChannels();
}

bool SfAdc::IsChannelConfigured(hf_adc_channel_t channel) const noexcept {
  RtosSharedLock<RtosSharedMutex> lock(rw_mutex_, mutex_timeout_ms_);
  if (!lock.IsLocked() || !adc_impl_) {
    return false;
  }

  return adc_impl_->IsChannelConfigured(channel);
}
