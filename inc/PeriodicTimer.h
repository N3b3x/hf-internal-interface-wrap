#ifndef PERIODIC_TIMER_H
#define PERIODIC_TIMER_H

#include "esp_timer.h"
#include <cstdint>

/**
 * @file PeriodicTimer.h
 * @brief Lightweight wrapper around esp_timer for periodic callbacks.
 *
 * This class creates an esp_timer that repeatedly invokes a user provided
 * callback function. It follows the lazy initialization approach used by
 * the other abstractions in this component.
 */
class PeriodicTimer {
public:
    using Callback = void (*)(void*); //!< Callback type used by the timer

    /**
     * @brief Construct a PeriodicTimer.
     * @param cb   Callback function to invoke on timer expiry
     * @param arg  User argument passed to the callback
     */
    explicit PeriodicTimer(Callback cb, void* arg = nullptr) noexcept;
    PeriodicTimer(const PeriodicTimer&) = delete;
    PeriodicTimer& operator=(const PeriodicTimer&) = delete;
    ~PeriodicTimer() noexcept;

    /**
     * @brief Start the timer with the given period.
     * @param periodUs Period in microseconds
     * @return true if the timer was started
     */
    bool Start(uint64_t periodUs) noexcept;

    /**
     * @brief Stop the timer if running.
     * @return true if stopped successfully
     */
    bool Stop() noexcept;

    /**
     * @brief Check if the timer is currently running.
     */
    bool IsRunning() const noexcept { return running; }

private:
    static void Dispatch(void* arg);
    bool CreateHandle() noexcept;

    esp_timer_handle_t handle;
    Callback userCb;
    void* userArg;
    bool running;
};

#endif // PERIODIC_TIMER_H
