/**
 * @file EspTime.h
 * @brief ESP32-family implementation of the high-resolution monotonic
 *        time source consumed via `HfTime.h`.
 *
 * Backed by `esp_timer_get_time()`, a 64-bit µs counter driven by a
 * dedicated hardware timer that ESP-IDF starts before `app_main`.
 * The counter is monotonic for >292 000 years so wrap is not a concern.
 *
 * Header-only inline implementation: every call lowers to a single
 * `esp_timer_get_time` call with no extra frames so it is safe inside
 * hot-path measurement loops (valve self-test sweeps, parallel-stress
 * sampler, ramp settling detectors).
 *
 * Do not include this file directly from middleware/app code — go
 * through `HfTime.h` so MCU dispatch stays in one place.
 *
 * @ingroup mcu_esp32
 */

#pragma once

#include <cstdint>

#include "esp_timer.h"

namespace hf_time::esp32 {

/**
 * @brief No-op on ESP-IDF: the high-resolution timer is started by
 *        the system bootstrap before `app_main` runs.
 *
 * Provided so callers writing portable init code can keep a single
 * call site regardless of target.
 */
inline void InitMonotonic() noexcept {}

/**
 * @brief Microseconds since boot from the hardware-backed timer.
 *
 * `esp_timer_get_time()` returns `int64_t` but is documented to be
 * non-negative once started; widening to `uint64_t` matches the
 * unsigned arithmetic used at call sites (deltas, comparisons).
 *
 * Safe from any task or ISR.
 */
inline std::uint64_t MonotonicUs() noexcept {
    return static_cast<std::uint64_t>(::esp_timer_get_time());
}

}  // namespace hf_time::esp32
