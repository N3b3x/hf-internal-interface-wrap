/**
 * @file EspPwm_FadeCallbacks_Clean.h
 * @brief Clean implementation of EspPwm with ONLY ESP-IDF supported fade callbacks
 * 
 * This is the corrected implementation that includes ONLY the callbacks that
 * ESP-IDF v5.5 LEDC peripheral actually supports: FADE COMPLETION CALLBACKS
 */

#pragma once

// Add to EspPwm class in the CALLBACKS section:

//==============================================================================
// CALLBACKS - ESP-IDF LEDC Native Support (Fade Only)
//==============================================================================

/**
 * @brief Set per-channel callback for PWM fade completion events
 * @param channel_id Channel identifier to set callback for
 * @param callback Function to call on fade completion (or nullptr to disable)
 * @return PWM_SUCCESS on success, error code on failure
 * 
 * @details Registers a per-channel callback function that is triggered when a hardware
 * fade operation completes on the specified channel. This uses the native ESP32-C6 LEDC
 * fade completion interrupt mechanism provided by ESP-IDF v5.5.
 * 
 * **IMPORTANT: This is the ONLY callback type supported by ESP-IDF LEDC peripheral!**
 * - Period callbacks are NOT supported by LEDC hardware
 * - Fault callbacks are NOT supported by LEDC hardware
 * - Only fade completion callbacks are natively supported via LEDC_INTR_FADE_END
 * 
 * **Fade Completion Detection:**
 * - **LEDC Hardware Interrupt:** Native ESP32-C6 LEDC_INTR_FADE_END interrupt
 * - **Per-Channel Granularity:** Each channel can have its own fade callback
 * - **ESP-IDF Integration:** Uses `ledc_cb_register()` for proper registration
 * 
 * @note This callback is ONLY triggered for hardware fade operations (SetHardwareFade())
 * @warning Callback functions must be ISR-safe and execute quickly (< 10μs recommended)
 * @warning Do not call blocking functions or start new fade operations in the callback
 * 
 * **Example Usage:**
 * ```cpp
 * void my_fade_callback(hf_channel_id_t channel) {
 *     // ISR-safe operations only
 *     fade_complete_flags |= (1 << channel); // Set completion flag
 *     xSemaphoreGiveFromISR(fade_semaphore, NULL); // Signal task
 * }
 * 
 * pwm.SetChannelFadeCallback(0, my_fade_callback);
 * pwm.SetHardwareFade(0, 0.8f, 1000); // Fade will trigger callback when complete
 * ```
 * 
 * @see SetHardwareFade() for hardware fade operations
 */
hf_pwm_err_t SetChannelFadeCallback(hf_channel_id_t channel_id, 
                                    std::function<void(hf_channel_id_t)> callback) noexcept;

// Add to ChannelState structure:
struct ChannelState {
    // ... existing members ...
    
    // Per-channel fade callback support (ESP-IDF LEDC native support ONLY)
    std::function<void(hf_channel_id_t)> fade_callback; ///< Per-channel fade completion callback

    ChannelState() noexcept
        : /* existing initializers */, fade_callback(nullptr) {}
};

// Private helper methods:
private:
    /**
     * @brief Handle fade complete interrupt (ESP-IDF LEDC native)
     * @param channel_id Channel that completed fade
     */
    void HandleFadeComplete(hf_channel_id_t channel_id) noexcept;

    /**
     * @brief Register LEDC fade callback using ESP-IDF API
     * @param channel_id Channel to register callback for
     * @return PWM_SUCCESS on success, error code on failure
     */
    hf_pwm_err_t RegisterLedcFadeCallback(hf_channel_id_t channel_id) noexcept;

    /**
     * @brief Unregister LEDC fade callback
     * @param channel_id Channel to unregister callback for
     * @return PWM_SUCCESS on success, error code on failure
     */
    hf_pwm_err_t UnregisterLedcFadeCallback(hf_channel_id_t channel_id) noexcept;

    /**
     * @brief Static callback wrapper for ESP-IDF LEDC callback system
     * @param param ESP-IDF callback parameter structure
     */
    static void IRAM_ATTR LedcFadeEndCallback(const ledc_cb_param_t* param, void* user_arg);