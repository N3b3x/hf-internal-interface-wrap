/**
 * @file test_esp32_bluetooth_compilation.cpp
 * @brief Test file to verify ESP32 Bluetooth conditional compilation
 *
 * This file tests that the ESP32 Bluetooth implementation compiles correctly
 * for different ESP32 variants with proper conditional compilation.
 */

// Simulate different ESP32 target configurations for testing
#ifdef TEST_ESP32C6
#define CONFIG_IDF_TARGET_ESP32C6 1
#undef CONFIG_IDF_TARGET_ESP32
#undef CONFIG_IDF_TARGET_ESP32S3
#elif defined(TEST_ESP32)
#define CONFIG_IDF_TARGET_ESP32 1
#undef CONFIG_IDF_TARGET_ESP32C6
#undef CONFIG_IDF_TARGET_ESP32S3
#elif defined(TEST_ESP32S3)
#define CONFIG_IDF_TARGET_ESP32S3 1
#undef CONFIG_IDF_TARGET_ESP32
#undef CONFIG_IDF_TARGET_ESP32C6
#endif

#include "inc/mcu/esp32/EspBluetooth.h"

int main() {
    // Test that macros are properly defined
#if defined(CONFIG_IDF_TARGET_ESP32C6)
    static_assert(HAS_CLASSIC_BLUETOOTH == 0, "ESP32C6 should not have Classic Bluetooth");
    static_assert(HAS_A2DP_SUPPORT == 0, "ESP32C6 should not have A2DP support");
    static_assert(HAS_BLUETOOTH_SUPPORT == 1, "ESP32C6 should have BLE support");
#elif defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3)
    static_assert(HAS_CLASSIC_BLUETOOTH == 1, "ESP32/ESP32S3 should have Classic Bluetooth");
    static_assert(HAS_A2DP_SUPPORT == 1, "ESP32/ESP32S3 should have A2DP support");
    static_assert(HAS_BLUETOOTH_SUPPORT == 1, "ESP32/ESP32S3 should have Bluetooth support");
#endif

    // Test that we can create an EspBluetooth instance
    EspBluetooth bluetooth;
    
    // Test basic functionality that should be available on all variants
    bluetooth.Initialize(hf_bluetooth_mode_t::HF_BLUETOOTH_MODE_BLE);
    
#if HAS_CLASSIC_BLUETOOTH
    // Test Classic Bluetooth methods only if supported
    bluetooth.EnableSpp(true);
    bluetooth.EnableA2dp(false);
    bluetooth.EnableAvrcp(false);
#endif

    bluetooth.Deinitialize();
    
    return 0;
}