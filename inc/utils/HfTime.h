/**
 * @file HfTime.h
 * @brief MCU-agnostic high-resolution monotonic time source.
 *
 * Higher layers (HAL handlers, middleware, app self-tests) that need a
 * sub-millisecond timebase — e.g. valve diagnostics measuring
 * propagation delays, leak-check ramp timings, scope-style traces —
 * MUST use this surface instead of reaching into the MCU SDK
 * directly. Two reasons:
 *
 *   1. Layering. iiwrap is the one component allowed to know the MCU.
 *      Every layer above it ports for free when we change MCUs because
 *      they only see `hf_time::MonotonicUs()`.
 *   2. Precision. Tick-based helpers (e.g. `OsAbstraction` /
 *      `os_get_elapsed_time_msec`) are quantised at the FreeRTOS tick
 *      (typically 1 ms) and are unfit for diagnostics that need µs
 *      resolution.
 *
 * On ESP32 family targets this maps to `esp_timer_get_time()`, which
 * returns a 64-bit microsecond counter backed by a hardware timer
 * (ROLLOVER >292 000 years — overflow not a practical concern). On
 * other MCUs add a parallel `#elif` block here; the surface stays
 * identical.
 *
 * Header-only inline implementation: no extra source registration is
 * required, and the call lowers to a single ABI call with no extra
 * frames so it is safe to use inside hot-path measurement loops.
 */

#ifndef HF_TIME_H_
#define HF_TIME_H_

#include <cstdint>

#if defined(ESP_PLATFORM) || defined(IDF_VER) || defined(__has_include)
#  if defined(ESP_PLATFORM) || defined(IDF_VER) || __has_include("esp_timer.h")
#    include "esp_timer.h"
#    define HF_TIME_HAS_ESP_TIMER 1
#  endif
#endif

namespace hf_time {

/**
 * @brief Initialise the monotonic time source if the platform requires it.
 *
 * On ESP-IDF the timer is started by the system bootstrap before
 * app_main, so this is a no-op. Provided for portability so callers
 * on bare-metal targets that need explicit init can keep a single
 * call site.
 */
inline void InitMonotonic() noexcept {}

/**
 * @brief Microseconds since boot from a hardware-backed monotonic counter.
 *
 * Guarantees: monotonically non-decreasing within the lifetime of the
 * process (no wraparound for >292 000 years on a 64-bit µs counter).
 * Safe from any task / ISR.
 */
inline std::uint64_t MonotonicUs() noexcept {
#if HF_TIME_HAS_ESP_TIMER
    // esp_timer_get_time returns int64_t but is documented to be
    // non-negative once started. Cast is a safe widening to uint64_t
    // for the unsigned arithmetic typical at call sites.
    return static_cast<std::uint64_t>(::esp_timer_get_time());
#else
    return 0;  // Portable fallback — replace per target as needed.
#endif
}

/**
 * @brief Convenience milliseconds view of the same monotonic counter.
 *
 * Implemented in terms of `MonotonicUs()` so the underlying timebase
 * stays consistent across `Us` and `Ms` queries. Resolution is still
 * 1 µs internally; only the unit conversion differs.
 */
inline std::uint64_t MonotonicMs() noexcept {
    return MonotonicUs() / 1000ULL;
}

/// Convenience: elapsed-since-`t0` in microseconds.
inline std::uint64_t ElapsedUs(std::uint64_t t0) noexcept {
    return MonotonicUs() - t0;
}

}  // namespace hf_time

#endif  // HF_TIME_H_
