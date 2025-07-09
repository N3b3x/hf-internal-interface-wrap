#pragma once

#include "EspTypes_Base.h"
#include "HardwareTypes.h"
#include "McuSelect.h"

#ifdef HF_MCU_FAMILY_ESP32
#include "esp_timer.h"
using hf_timer_handle_t = esp_timer_handle_t;
using hf_timer_args_native_t = esp_timer_create_args_t;
#else
using hf_timer_handle_t = void *;
struct hf_timer_args_native_t {
  int dummy;
};
#endif
