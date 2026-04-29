/**
 * @file HfTime.h
 * @brief MCU-agnostic high-resolution monotonic time source.
 *
 * Public surface for higher layers (HAL handlers, middleware,
 * application self-tests) that need a sub-millisecond timebase. The
 * concrete implementation lives under `mcu/<vendor>/<X>Time.h` and is
 * selected at compile time from `McuSelect.h`. Adding a new MCU is a
 * one-folder change:
 *
 *   1. drop in `mcu/<vendor>/<X>Time.h` exposing
 *      `namespace hf_time::<vendor> { void InitMonotonic();
 *      uint64_t MonotonicUs(); }`,
 *   2. extend `McuSelect.h` if it doesn't already detect that target,
 *   3. add one `#elif` block here aliasing the new namespace into
 *      `hf_time`.
 *
 * Why this is a static-policy header and not a virtual `BaseTime`:
 *   - `MonotonicUs()` is a single hardware-counter read used inside
 *     tight sample loops; a vtable load + indirect call would
 *     measurably degrade self-test sampling at 50 Hz × N channels.
 *   - There is no per-instance state to vary — the underlying counter
 *     is a process-wide resource. Virtual dispatch buys nothing.
 *   - Header-only inlining keeps the call site to a single `call`
 *     instruction.
 *
 * Use virtual `Base*` bases (`BaseUart`, `BaseSpi`, …) when the
 * primitive owns state and dispatch happens once per logical
 * operation; use static policy headers like this one for stateless
 * hot-path primitives.
 */

#pragma once

#include <cstdint>

#include "McuSelect.h"

#if defined(HF_TARGET_MCU_ESP32)   || defined(HF_TARGET_MCU_ESP32S2) || \
    defined(HF_TARGET_MCU_ESP32S3) || defined(HF_TARGET_MCU_ESP32C3) || \
    defined(HF_TARGET_MCU_ESP32C2) || defined(HF_TARGET_MCU_ESP32C6) || \
    defined(HF_TARGET_MCU_ESP32H2)
#  include "EspTime.h"
   namespace hf_time { using ::hf_time::esp32::MonotonicUs;
                       using ::hf_time::esp32::InitMonotonic; }

#elif defined(HF_TARGET_MCU_STM32F4) || defined(HF_TARGET_MCU_STM32H7)
#  include "Stm32Time.h"
   namespace hf_time { using ::hf_time::stm32::MonotonicUs;
                       using ::hf_time::stm32::InitMonotonic; }

#elif defined(HF_TARGET_MCU_RP2040)
#  include "Rp2040Time.h"
   namespace hf_time { using ::hf_time::rp2040::MonotonicUs;
                       using ::hf_time::rp2040::InitMonotonic; }

#elif defined(HF_TARGET_MCU_NONE)
   // Software-only / unit-test build: monotonic clock is a stub. Tests
   // that need a controllable clock should inject their own.
   namespace hf_time {
       inline std::uint64_t MonotonicUs() noexcept { return 0; }
       inline void          InitMonotonic() noexcept {}
   }

#else
#  error "HfTime: no implementation for this target — set HF_TARGET_MCU_* (see McuSelect.h)"
#endif

namespace hf_time {

/**
 * @brief Convenience milliseconds view of the monotonic counter.
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
