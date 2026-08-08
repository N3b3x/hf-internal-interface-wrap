/**
 * @file SoftChipSelectGuard.h
 * @ingroup utils
 * @brief RAII soft chip-select (assert on construct, deassert on destroy).
 *
 * Use around a single SPI frame (or an intentional multi-byte burst that must
 * stay under one CS assertion). Pair with a bus mutex held for the whole
 * mode + FIFO + transfer window — this guard only owns the CS pin edges.
 *
 * @note Does not replace bus locking. Callers must already own the SPI bus
 *       mutex so peers cannot change CPOL/CPHA or steal the peripheral.
 */

#pragma once

#include <utility>

/**
 * @brief RAII soft-CS: runs @p assert_fn on construction and @p deassert_fn
 *        on destruction (including early return / error paths).
 *
 * @tparam AssertFn   Callable `void()`
 * @tparam DeassertFn Callable `void()`
 *
 * Typical STM32 soft-CS device usage (inside a bus lock):
 * @code
 * SoftChipSelectGuard cs{[&]{ AssertCS(); }, [&]{ DeassertCS(); }};
 * // HAL_SPI_TransmitReceive...
 * @endcode
 */
template <typename AssertFn, typename DeassertFn>
class SoftChipSelectGuard {
public:
  SoftChipSelectGuard(AssertFn assert_fn, DeassertFn deassert_fn) noexcept
      : deassert_(std::move(deassert_fn)), armed_(true) {
    assert_fn();
  }

  ~SoftChipSelectGuard() noexcept {
    Release();
  }

  SoftChipSelectGuard(const SoftChipSelectGuard&) = delete;
  SoftChipSelectGuard& operator=(const SoftChipSelectGuard&) = delete;

  SoftChipSelectGuard(SoftChipSelectGuard&& other) noexcept
      : deassert_(std::move(other.deassert_)), armed_(other.armed_) {
    other.armed_ = false;
  }

  SoftChipSelectGuard& operator=(SoftChipSelectGuard&& other) noexcept {
    if (this != &other) {
      Release();
      deassert_ = std::move(other.deassert_);
      armed_ = other.armed_;
      other.armed_ = false;
    }
    return *this;
  }

  /** @brief Early deassert (idempotent). Destructor becomes a no-op. */
  void Release() noexcept {
    if (armed_) {
      deassert_();
      armed_ = false;
    }
  }

  [[nodiscard]] bool OwnsCs() const noexcept { return armed_; }

private:
  DeassertFn deassert_;
  bool armed_;
};

/** @brief Deduce guard type from assert/deassert callables. */
template <typename AssertFn, typename DeassertFn>
[[nodiscard]] SoftChipSelectGuard<AssertFn, DeassertFn> MakeSoftChipSelectGuard(
    AssertFn assert_fn, DeassertFn deassert_fn) noexcept {
  return SoftChipSelectGuard<AssertFn, DeassertFn>(std::move(assert_fn),
                                                     std::move(deassert_fn));
}
