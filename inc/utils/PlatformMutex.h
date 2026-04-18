/**
 * @file PlatformMutex.h
 * @ingroup utils
 * @brief Cross-platform mutex and synchronization primitives with pluggable backend.
 *
 * This header is FULLY SELF-CONTAINED within hf-internal-interface-wrap.
 * It has NO external dependencies beyond C++ standard headers (<atomic>, <cstdint>).
 *
 * === BACKEND ARCHITECTURE ===
 *
 * All mutex operations delegate to a compile-time Backend policy class.
 * By default, the backend is NullMutexBackend -- all operations are constexpr
 * inline no-ops that the optimizer eliminates completely (zero code generated).
 *
 * At the core/ layer (where hf-internal-interface-wrap and hf-utils-rtos-wrap
 * are brought together), a "PlatformMutexBackend.h" configuration header may
 * be placed on the include path. This header defines a concrete backend struct
 * that delegates to the RTOS abstraction layer (OsAbstraction.h) for real
 * mutex protection under FreeRTOS or another RTOS.
 *
 * === ZERO-OVERHEAD PRINCIPLE ===
 *
 * When no backend config is found (standalone interface-wrap builds, host
 * builds, MCU=NONE, RTOS=NONE), NullMutexBackend is used:
 *   - Every mutex operation is constexpr and compiles to nothing
 *   - Handle types are uint8_t (1 byte member instead of pointer)
 *   - No RTOS headers are included, no external dependencies exist
 *
 * === CUSTOMIZATION (for core/ layer or other consumers) ===
 *
 * Create "PlatformMutexBackend.h" on the include path and:
 *   1. Define a backend struct satisfying the same contract as NullMutexBackend
 *   2. typedef: using PlatformMutexActiveBackend = YourBackendType;
 *   3. #define PLATFORM_MUTEX_BACKEND_CONFIGURED
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

#pragma once

#include <atomic>
#include <cstdint>

//==============================================================================
// ##  1. NULL (NO-OP) MUTEX BACKEND -- zero-overhead default
//==============================================================================

/**
 * @brief No-op mutex backend -- all operations compile to nothing.
 *
 * This is the default backend when no RTOS or threading support is configured.
 * Handle types are minimal (uint8_t) so mutex members occupy a single byte.
 * Every operation is constexpr / inline and produces no machine code.
 *
 * This struct also serves as the DOCUMENTATION of the backend contract:
 * any concrete backend must provide the exact same set of static methods
 * with compatible signatures.
 */
struct NullMutexBackend {
    // -- Handle Types ------------------------------------------------------
    /// Opaque handle for a recursive mutex (1 byte, unused by no-op backend)
    using RecursiveMutexHandle = uint8_t;
    /// Opaque handle for a regular (non-recursive) mutex (1 byte, unused)
    using MutexHandle = uint8_t;

    // -- Constants ---------------------------------------------------------
    static constexpr uint32_t MAX_DELAY    = 0xFFFFFFFFU;  ///< Infinite wait
    static constexpr uint32_t TICK_RATE_HZ = 1000U;        ///< Assumed tick rate

    // -- Recursive Mutex Operations ----------------------------------------
    /// Create a recursive mutex. Backend writes into *h.
    static constexpr void createRecursive(RecursiveMutexHandle*) noexcept {}
    /// Destroy a recursive mutex pointed to by *h.
    static constexpr void destroyRecursive(RecursiveMutexHandle*) noexcept {}
    /// Lock recursively with timeout (ticks). Returns true on success.
    static constexpr bool lockRecursive(RecursiveMutexHandle*, uint32_t) noexcept { return true; }
    /// Try lock (non-blocking). Returns true on success.
    static constexpr bool tryLockRecursive(RecursiveMutexHandle*) noexcept { return true; }
    /// Unlock recursive mutex.
    static constexpr void unlockRecursive(RecursiveMutexHandle*) noexcept {}

    // -- Regular Mutex Operations ------------------------------------------
    /// Create a regular (non-recursive) mutex. Backend writes into *h.
    static constexpr void createMutex(MutexHandle*) noexcept {}
    /// Destroy a regular mutex pointed to by *h.
    static constexpr void destroyMutex(MutexHandle*) noexcept {}
    /// Lock with timeout (ticks). Returns true on success.
    static constexpr bool lockMutex(MutexHandle*, uint32_t) noexcept { return true; }
    /// Try lock (non-blocking). Returns true on success.
    static constexpr bool tryLockMutex(MutexHandle*) noexcept { return true; }
    /// Unlock regular mutex.
    static constexpr void unlockMutex(MutexHandle*) noexcept {}

    // -- Time / Scheduling -------------------------------------------------
    /// Get current RTOS tick count.
    static constexpr uint32_t getTickCount() noexcept { return 0; }
    /// Convert milliseconds to RTOS ticks.
    static constexpr uint32_t msToTicks(uint32_t ms) noexcept { return ms; }
    /// Yield to other tasks (no-op in single-threaded).
    static constexpr void yield() noexcept {}
};

//==============================================================================
// ##  2. BACKEND SELECTION -- compile-time injection point
//==============================================================================
//
// If the core/ layer (or any consumer) places a "PlatformMutexBackend.h" file
// on the include path, it is picked up here via C++17 __has_include.
// That file must:
//   1. Define a struct with the same static methods as NullMutexBackend
//   2. Set:  using PlatformMutexActiveBackend = YourBackendType;
//   3. Set:  #define PLATFORM_MUTEX_BACKEND_CONFIGURED
//
// When building interface-wrap standalone (no core/ layer), the file is absent
// and NullMutexBackend is used -- zero overhead, no external dependencies.
//==============================================================================

#if __has_include("PlatformMutexBackend.h")
    #include "PlatformMutexBackend.h"
#endif

#if !defined(PLATFORM_MUTEX_BACKEND_CONFIGURED)
    /// Default: all mutex operations compile to nothing
    using PlatformMutexActiveBackend = NullMutexBackend;
#endif

//==============================================================================
// ##  3. PLATFORM TIME -- tick counting and conversion utilities
//==============================================================================

class PlatformTime {
public:
    /**
     * @brief Get current time in microseconds (from RTOS tick count).
     * @return Microseconds since RTOS start, or 0 if no RTOS.
     */
    static uint64_t GetCurrentTimeUs() noexcept {
        const uint32_t hz = PlatformMutexActiveBackend::TICK_RATE_HZ;
        if (hz == 0) return 0;
        return static_cast<uint64_t>(PlatformMutexActiveBackend::getTickCount())
               * 1000000ULL / hz;
    }

    /**
     * @brief Convert milliseconds to RTOS ticks (minimum 1 tick for non-zero input).
     */
    static uint32_t MsToTicks(uint32_t ms) noexcept {
        if (ms == 0) return 0;
        const uint32_t ticks = PlatformMutexActiveBackend::msToTicks(ms);
        return (ticks > 0) ? ticks : 1;
    }
};

//==============================================================================
// ##  4. PLATFORM MUTEX -- recursive mutex with STL-like + FreeRTOS-style API
//==============================================================================

/**
 * @brief Recursive mutex templated on a backend policy.
 *
 * When Backend = NullMutexBackend, the entire class compiles to nothing
 * (1-byte handle member, all methods are no-ops). When a real RTOS backend
 * is injected, this provides thread-safe recursive locking.
 *
 * @tparam Backend  Static policy class satisfying the NullMutexBackend contract.
 */
template <typename Backend = PlatformMutexActiveBackend>
class PlatformMutexImpl {
public:
    PlatformMutexImpl() noexcept : handle_{} {
        Backend::createRecursive(&handle_);
    }

    ~PlatformMutexImpl() noexcept {
        Backend::destroyRecursive(&handle_);
    }

    // Non-copyable
    PlatformMutexImpl(const PlatformMutexImpl&) = delete;
    PlatformMutexImpl& operator=(const PlatformMutexImpl&) = delete;

    // Movable (transfers handle ownership)
    PlatformMutexImpl(PlatformMutexImpl&& other) noexcept : handle_(other.handle_) {
        other.handle_ = {};
    }

    PlatformMutexImpl& operator=(PlatformMutexImpl&& other) noexcept {
        if (this != &other) {
            Backend::destroyRecursive(&handle_);
            handle_ = other.handle_;
            other.handle_ = {};
        }
        return *this;
    }

    // -- STL-like API ------------------------------------------------------

    bool lock() noexcept {
        return Backend::lockRecursive(&handle_, Backend::MAX_DELAY);
    }

    bool try_lock() noexcept {
        return Backend::tryLockRecursive(&handle_);
    }

    bool try_lock_for(uint32_t timeout_ms) noexcept {
        const uint32_t ticks = PlatformTime::MsToTicks(timeout_ms);
        return Backend::lockRecursive(&handle_, ticks);
    }

    void unlock() noexcept {
        Backend::unlockRecursive(&handle_);
    }

    /// Access the underlying platform handle (for interop / diagnostics)
    auto native_handle() const noexcept { return handle_; }

    // -- FreeRTOS-style convenience API ------------------------------------

    bool Take(uint32_t timeout_ms = 0) noexcept {
        return (timeout_ms > 0) ? try_lock_for(timeout_ms) : lock();
    }

    void Give() noexcept { unlock(); }

    // -- Shared-lock pass-through (for generic lock-guard compatibility) ---

    bool lock_shared() noexcept { return lock(); }
    bool try_lock_shared() noexcept { return try_lock(); }
    bool try_lock_shared_for(uint32_t timeout_ms) noexcept { return try_lock_for(timeout_ms); }
    void unlock_shared() noexcept { unlock(); }

private:
    typename Backend::RecursiveMutexHandle handle_;
};

//==============================================================================
// ##  5. PLATFORM SHARED MUTEX -- reader-writer mutex
//==============================================================================

/**
 * @brief Reader-writer mutex using two regular mutexes + atomic counters.
 *
 * Multiple readers can hold the lock concurrently; writers have exclusive
 * access. When Backend = NullMutexBackend, all operations compile away.
 *
 * @tparam Backend  Static policy class satisfying the NullMutexBackend contract.
 */
template <typename Backend = PlatformMutexActiveBackend>
class PlatformSharedMutexImpl {
public:
    PlatformSharedMutexImpl() noexcept
        : writer_mutex_{}, reader_mutex_{}, readers_(0), writer_active_(false) {
        Backend::createMutex(&writer_mutex_);
        Backend::createMutex(&reader_mutex_);
    }

    ~PlatformSharedMutexImpl() noexcept {
        Backend::destroyMutex(&writer_mutex_);
        Backend::destroyMutex(&reader_mutex_);
    }

    // Non-copyable
    PlatformSharedMutexImpl(const PlatformSharedMutexImpl&) = delete;
    PlatformSharedMutexImpl& operator=(const PlatformSharedMutexImpl&) = delete;

    // Movable
    PlatformSharedMutexImpl(PlatformSharedMutexImpl&& other) noexcept
        : writer_mutex_(other.writer_mutex_), reader_mutex_(other.reader_mutex_),
          readers_(other.readers_.load()), writer_active_(other.writer_active_.load()) {
        other.writer_mutex_ = {};
        other.reader_mutex_ = {};
        other.readers_.store(0);
        other.writer_active_.store(false);
    }

    PlatformSharedMutexImpl& operator=(PlatformSharedMutexImpl&& other) noexcept {
        if (this != &other) {
            Backend::destroyMutex(&writer_mutex_);
            Backend::destroyMutex(&reader_mutex_);
            writer_mutex_ = other.writer_mutex_;
            reader_mutex_ = other.reader_mutex_;
            readers_.store(other.readers_.load());
            writer_active_.store(other.writer_active_.load());
            other.writer_mutex_ = {};
            other.reader_mutex_ = {};
            other.readers_.store(0);
            other.writer_active_.store(false);
        }
        return *this;
    }

    // -- Exclusive (writer) lock -------------------------------------------

    bool lock() noexcept {
        if (!Backend::lockMutex(&writer_mutex_, Backend::MAX_DELAY)) return false;
        writer_active_.store(true);
        while (readers_.load() > 0) { Backend::yield(); }
        return true;
    }

    bool try_lock() noexcept {
        if (!Backend::tryLockMutex(&writer_mutex_)) return false;
        writer_active_.store(true);
        if (readers_.load() > 0) {
            writer_active_.store(false);
            Backend::unlockMutex(&writer_mutex_);
            return false;
        }
        return true;
    }

    bool try_lock_for(uint32_t timeout_ms) noexcept {
        const uint32_t ticks = PlatformTime::MsToTicks(timeout_ms);
        const uint32_t start = Backend::getTickCount();
        if (!Backend::lockMutex(&writer_mutex_, ticks)) return false;
        writer_active_.store(true);
        const uint32_t elapsed = Backend::getTickCount() - start;
        const uint32_t remaining = (elapsed < ticks) ? ticks - elapsed : 0;
        const uint32_t deadline = Backend::getTickCount() + remaining;
        while (readers_.load() > 0) {
            if (Backend::getTickCount() >= deadline) {
                writer_active_.store(false);
                Backend::unlockMutex(&writer_mutex_);
                return false;
            }
            Backend::yield();
        }
        return true;
    }

    void unlock() noexcept {
        writer_active_.store(false);
        Backend::unlockMutex(&writer_mutex_);
    }

    // -- Shared (reader) lock ----------------------------------------------

    bool lock_shared() noexcept {
        while (true) {
            if (!Backend::lockMutex(&reader_mutex_, Backend::MAX_DELAY)) return false;
            if (!writer_active_.load()) {
                readers_++;
                Backend::unlockMutex(&reader_mutex_);
                return true;
            }
            Backend::unlockMutex(&reader_mutex_);
            Backend::yield();
        }
    }

    bool try_lock_shared() noexcept {
        if (!Backend::tryLockMutex(&reader_mutex_)) return false;
        if (!writer_active_.load()) {
            readers_++;
            Backend::unlockMutex(&reader_mutex_);
            return true;
        }
        Backend::unlockMutex(&reader_mutex_);
        return false;
    }

    bool try_lock_shared_for(uint32_t timeout_ms) noexcept {
        const uint32_t ticks = PlatformTime::MsToTicks(timeout_ms);
        const uint32_t start = Backend::getTickCount();
        while (true) {
            const uint32_t elapsed = Backend::getTickCount() - start;
            if (elapsed >= ticks) return false;
            const uint32_t remaining = ticks - elapsed;
            if (!Backend::lockMutex(&reader_mutex_, remaining)) return false;
            if (!writer_active_.load()) {
                readers_++;
                Backend::unlockMutex(&reader_mutex_);
                return true;
            }
            Backend::unlockMutex(&reader_mutex_);
            const uint32_t new_elapsed = Backend::getTickCount() - start;
            if (new_elapsed >= ticks) return false;
            Backend::yield();
        }
    }

    void unlock_shared() noexcept {
        if (Backend::lockMutex(&reader_mutex_, Backend::MAX_DELAY)) {
            if (readers_.load() > 0) readers_--;
            Backend::unlockMutex(&reader_mutex_);
        }
    }

private:
    typename Backend::MutexHandle writer_mutex_;
    typename Backend::MutexHandle reader_mutex_;
    std::atomic<int> readers_;
    std::atomic<bool> writer_active_;
};

//==============================================================================
// ##  6. RAII LOCK GUARDS (backend-agnostic -- work with any mutex type)
//==============================================================================

/**
 * @brief RAII exclusive lock guard for any mutex with lock()/try_lock_for()/unlock().
 * @tparam Mutex  Type providing lock(), try_lock_for(uint32_t), unlock().
 */
template <typename Mutex>
class PlatformUniqueLock {
public:
    explicit PlatformUniqueLock(Mutex& mutex, uint32_t timeout_ms = 0) noexcept
        : mutex_(&mutex), locked_(false) {
        if (timeout_ms > 0) {
            locked_ = mutex_->try_lock_for(timeout_ms);
        } else {
            locked_ = mutex_->lock();
        }
    }

    ~PlatformUniqueLock() noexcept {
        if (locked_ && mutex_) {
            mutex_->unlock();
        }
    }

    // Non-copyable
    PlatformUniqueLock(const PlatformUniqueLock&) = delete;
    PlatformUniqueLock& operator=(const PlatformUniqueLock&) = delete;

    // Movable
    PlatformUniqueLock(PlatformUniqueLock&& other) noexcept
        : mutex_(other.mutex_), locked_(other.locked_) {
        other.mutex_ = nullptr;
        other.locked_ = false;
    }

    PlatformUniqueLock& operator=(PlatformUniqueLock&& other) noexcept {
        if (this != &other) {
            if (locked_ && mutex_) { mutex_->unlock(); }
            mutex_ = other.mutex_;
            locked_ = other.locked_;
            other.mutex_ = nullptr;
            other.locked_ = false;
        }
        return *this;
    }

    [[nodiscard]] bool IsLocked() const noexcept { return locked_; }

    void Unlock() noexcept {
        if (locked_ && mutex_) {
            mutex_->unlock();
            locked_ = false;
        }
    }

private:
    Mutex* mutex_;
    bool locked_;
};

/**
 * @brief RAII shared (reader) lock guard for any shared mutex.
 * @tparam SharedMutex  Type providing lock_shared()/try_lock_shared_for()/unlock_shared().
 */
template <typename SharedMutex>
class PlatformSharedLock {
public:
    explicit PlatformSharedLock(SharedMutex& mutex, uint32_t timeout_ms = 0) noexcept
        : mutex_(&mutex), locked_(false) {
        if (timeout_ms > 0) {
            locked_ = mutex_->try_lock_shared_for(timeout_ms);
        } else {
            locked_ = mutex_->lock_shared();
        }
    }

    ~PlatformSharedLock() noexcept {
        if (locked_ && mutex_) {
            mutex_->unlock_shared();
        }
    }

    // Non-copyable
    PlatformSharedLock(const PlatformSharedLock&) = delete;
    PlatformSharedLock& operator=(const PlatformSharedLock&) = delete;

    // Movable
    PlatformSharedLock(PlatformSharedLock&& other) noexcept
        : mutex_(other.mutex_), locked_(other.locked_) {
        other.mutex_ = nullptr;
        other.locked_ = false;
    }

    PlatformSharedLock& operator=(PlatformSharedLock&& other) noexcept {
        if (this != &other) {
            if (locked_ && mutex_) { mutex_->unlock_shared(); }
            mutex_ = other.mutex_;
            locked_ = other.locked_;
            other.mutex_ = nullptr;
            other.locked_ = false;
        }
        return *this;
    }

    [[nodiscard]] bool IsLocked() const noexcept { return locked_; }

    void Unlock() noexcept {
        if (locked_ && mutex_) {
            mutex_->unlock_shared();
            locked_ = false;
        }
    }

private:
    SharedMutex* mutex_;
    bool locked_;
};

//==============================================================================
// ##  7. CONVENIENCE TYPE ALIASES
//==============================================================================

/// @brief Concrete recursive mutex using the active backend
using PlatformMutex = PlatformMutexImpl<PlatformMutexActiveBackend>;

/// @brief Concrete reader-writer mutex using the active backend
using PlatformSharedMutex = PlatformSharedMutexImpl<PlatformMutexActiveBackend>;

/// @brief RAII exclusive lock guard for PlatformMutex
using PlatformMutexLockGuard = PlatformUniqueLock<PlatformMutex>;

/// @brief Generic RAII lock guard alias
template <typename Mutex>
using PlatformLockGuard = PlatformUniqueLock<Mutex>;
