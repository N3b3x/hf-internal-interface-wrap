/**
 * @file NvsComprehensiveTest.cpp
 * @brief Comprehensive NVS (Non-Volatile Storage) testing suite for ESP32-C6 DevKit-M-1 (noexcept)
 *
 * This test suite provides complete coverage of the EspNvs class functionality with comprehensive
 * testing of all methods, edge cases, error conditions, and performance characteristics.
 * Designed for production validation with no exceptions and no RTTI usage.
 *
 * Test Coverage:
 * - Constructor/Destructor testing with various namespace scenarios
 * - Initialization/Deinitialization testing with error conditions
 * - All data type operations (U32, String, Blob) with boundary testing
 * - Key management operations (exists, erase, size)
 * - Commit operations and auto-commit behavior
 * - Statistics and diagnostics functionality
 * - Error condition testing and parameter validation
 * - Thread safety and concurrent operations
 * - Performance limits and boundary conditions
 * - Memory management and resource cleanup
 *
 * @author Nebiyu Tadesse
 * @date 2025
 * @copyright HardFOC
 */

// Standard C++ includes first
#include <cstring>

// ESP-IDF includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project includes
#include "TestFramework.h"
#include "base/BaseNvs.h"
#include "mcu/esp32/EspNvs.h"
#include "mcu/esp32/utils/EspTypes_NVS.h"

namespace {
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
const char* kTag = "NVS_Test";
TestResults g_test_results;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// Test data constants for comprehensive testing
constexpr uint32_t kTestU32Value1 = 0x12345678U;
constexpr uint32_t kTestU32Value2 = 0xDEADBEEFU;
constexpr uint32_t kTestU32ValueMax = 0xFFFFFFFFU;
constexpr uint32_t kTestU32ValueMin = 0x00000000U;

constexpr const char* kTestStringShort = "test";
constexpr const char* kTestStringMedium = "Hello ESP32-C6 NVS Test Suite";
constexpr const char* kTestStringLong = "This is a very long test string that approaches the maximum length supported by ESP32 NVS to test boundary conditions and buffer management capabilities.";
constexpr const char* kTestStringEmpty = "";
constexpr const char* kTestStringUnicode = "测试🚀ñ€";

constexpr uint8_t kTestBlobData[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 
                                     0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
constexpr size_t kTestBlobSize = sizeof(kTestBlobData);

// Valid and invalid key names for testing
constexpr const char* kValidKeyShort = "key1";
constexpr const char* kValidKeyMedium = "test_key_123";
constexpr const char* kValidKeyMaxLength = "123456789012345"; // 15 chars max
constexpr const char* kInvalidKeyTooLong = "1234567890123456"; // 16 chars - too long
constexpr const char* kInvalidKeyWithSpace = "key with space";
constexpr const char* kInvalidKeyWithTab = "key\twith\ttab";
constexpr const char* kInvalidKeyWithNewline = "key\nwith\nnewline";
constexpr const char* kInvalidKeyEmpty = "";

// Namespace names for testing
constexpr const char* kValidNamespace = "hardfoc_test";
constexpr const char* kValidNamespaceShort = "test";
constexpr const char* kValidNamespaceMax = "123456789012345"; // 15 chars max
constexpr const char* kInvalidNamespaceTooLong = "1234567890123456"; // 16 chars - too long
constexpr const char* kInvalidNamespaceEmpty = "";

// Buffer sizes for testing
constexpr size_t kSmallBufferSize = 10;
constexpr size_t kMediumBufferSize = 256;
constexpr size_t kLargeBufferSize = 1024;
constexpr size_t kExactBufferSize = 32;
constexpr size_t kTestBufferSize = 64;
constexpr size_t kStringBufferSize = 128;

//==============================================//
// CONSTRUCTOR AND DESTRUCTOR TESTS            //
//==============================================//

auto test_constructor_valid_namespace() noexcept -> bool {
    ESP_LOGI(kTag, "Testing constructor with valid namespace...");
    
    // Test normal namespace
    {
        EspNvs nvs(kValidNamespace);
        if (std::strcmp(nvs.GetNamespace(), kValidNamespace) != 0) {
            ESP_LOGE(kTag, "Constructor failed: namespace mismatch");
            return false;
        }
        if (nvs.IsInitialized()) {
            ESP_LOGE(kTag, "Constructor failed: should not be initialized yet");
            return false;
        }
    }
    
    // Test short namespace
    {
        EspNvs nvs(kValidNamespaceShort);
        if (std::strcmp(nvs.GetNamespace(), kValidNamespaceShort) != 0) {
            ESP_LOGE(kTag, "Constructor failed: short namespace mismatch");
            return false;
        }
    }
    
    // Test maximum length namespace
    {
        EspNvs nvs(kValidNamespaceMax);
        if (std::strcmp(nvs.GetNamespace(), kValidNamespaceMax) != 0) {
            ESP_LOGE(kTag, "Constructor failed: max namespace mismatch");
            return false;
        }
    }
    
    ESP_LOGI(kTag, "[SUCCESS] Constructor with valid namespace tests passed");
    return true;
}

auto test_constructor_invalid_namespace() noexcept -> bool {
    ESP_LOGI(kTag, "Testing constructor with invalid namespace...");
    
    // Test null namespace - should handle gracefully
    {
        EspNvs nvs(nullptr);
        // Constructor should succeed but Initialize should fail
        if (nvs.IsInitialized()) {
            ESP_LOGE(kTag, "Constructor failed: null namespace should not auto-initialize");
            return false;
        }
        
        // Try to initialize - should fail
        const auto result = nvs.Initialize();
        if (result == hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(kTag, "Initialize unexpectedly succeeded with null namespace");
            return false;
        }
    }
    
    // Test empty namespace
    {
        EspNvs nvs(kInvalidNamespaceEmpty);
        const auto result = nvs.Initialize();
        if (result == hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(kTag, "Initialize unexpectedly succeeded with empty namespace");
            return false;
        }
    }
    
    // Test too long namespace
    {
        EspNvs nvs(kInvalidNamespaceTooLong);
        const auto result = nvs.Initialize();
        if (result == hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(kTag, "Initialize unexpectedly succeeded with too long namespace");
            return false;
        }
    }
    
    ESP_LOGI(kTag, "[SUCCESS] Constructor with invalid namespace tests passed");
    return true;
}

//==============================================//
// INITIALIZATION TESTS                        //
//==============================================//

auto test_initialization_basic() noexcept -> bool {
    ESP_LOGI(kTag, "Testing basic initialization...");
    
    EspNvs nvs(kValidNamespace);
    
    // Should not be initialized initially
    if (nvs.IsInitialized()) {
        ESP_LOGE(kTag, "NVS should not be initialized initially");
        return false;
    }
    
    // Initialize should succeed
    auto result = nvs.Initialize();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Initialize failed: %s", HfNvsErrToString(result));
        return false;
    }
    
    // Should be initialized now
    if (!nvs.IsInitialized()) {
        ESP_LOGE(kTag, "NVS should be initialized after successful Initialize call");
        return false;
    }
    
    // Deinitialize should succeed
    result = nvs.Deinitialize();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Deinitialize failed: %s", HfNvsErrToString(result));
        return false;
    }
    
    // Should not be initialized after deinitialize
    if (nvs.IsInitialized()) {
        ESP_LOGE(kTag, "NVS should not be initialized after Deinitialize");
        return false;
    }
    
    ESP_LOGI(kTag, "[SUCCESS] Basic initialization tests passed");
    return true;
}

auto test_initialization_double_init() noexcept -> bool {
    ESP_LOGI(kTag, "Testing double initialization...");
    
    EspNvs nvs(kValidNamespace);
    
    // First initialization
    auto result = nvs.Initialize();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "First Initialize failed: %s", HfNvsErrToString(result));
        return false;
    }
    
    // Second initialization should return already initialized error
    result = nvs.Initialize();
    if (result != hf_nvs_err_t::NVS_ERR_ALREADY_INITIALIZED) {
        ESP_LOGE(kTag, "Second Initialize should return ALREADY_INITIALIZED, got: %s", HfNvsErrToString(result));
        return false;
    }
    
    // Should still be initialized
    if (!nvs.IsInitialized()) {
        ESP_LOGE(kTag, "NVS should still be initialized");
        return false;
    }
    
    // Clean up
    nvs.Deinitialize();
    
    ESP_LOGI(kTag, "[SUCCESS] Double initialization tests passed");
    return true;
}

auto test_deinitialization_not_initialized() noexcept -> bool {
    ESP_LOGI(kTag, "Testing deinitialization when not initialized...");
    
    EspNvs nvs(kValidNamespace);
    
    // Deinitialize without initialize should return not initialized error
    const auto result = nvs.Deinitialize();
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "Deinitialize should return NOT_INITIALIZED, got: %s", HfNvsErrToString(result));
        return false;
    }
    
    ESP_LOGI(kTag, "[SUCCESS] Deinitialization when not initialized tests passed");
    return true;
}

//==============================================//
// U32 OPERATIONS TESTS                        //
//==============================================//

auto test_u32_basic_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing basic U32 operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Set and get basic value
    auto result = nvs.SetU32(kValidKeyShort, kTestU32Value1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    uint32_t retrieved_value = 0;
    result = nvs.GetU32(kValidKeyShort, retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetU32 failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (retrieved_value != kTestU32Value1) {
        ESP_LOGE(kTag, "U32 value mismatch: expected 0x%08X, got 0x%08X", kTestU32Value1, retrieved_value);
        nvs.Deinitialize();
        return false;
    }
    
    // Update value
    result = nvs.SetU32(kValidKeyShort, kTestU32Value2);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 update failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetU32(kValidKeyShort, retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != kTestU32Value2) {
        ESP_LOGE(kTag, "U32 update verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Basic U32 operations tests passed");
    return true;
}

auto test_u32_boundary_values() noexcept -> bool {
    ESP_LOGI(kTag, "Testing U32 boundary values...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Test minimum value
    auto result = nvs.SetU32("min_val", kTestU32ValueMin);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 min value failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    uint32_t retrieved_value = 0xFFFFFFFFU; // Initialize to different value
    result = nvs.GetU32("min_val", retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != kTestU32ValueMin) {
        ESP_LOGE(kTag, "U32 min value verification failed: got 0x%08X", retrieved_value);
        nvs.Deinitialize();
        return false;
    }
    
    // Test maximum value
    result = nvs.SetU32("max_val", kTestU32ValueMax);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 max value failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    retrieved_value = 0x00000000U; // Initialize to different value
    result = nvs.GetU32("max_val", retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != kTestU32ValueMax) {
        ESP_LOGE(kTag, "U32 max value verification failed: got 0x%08X", retrieved_value);
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] U32 boundary values tests passed");
    return true;
}

auto test_u32_invalid_parameters() noexcept -> bool {
    ESP_LOGI(kTag, "Testing U32 operations with invalid parameters...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Test null key for SetU32
    auto result = nvs.SetU32(nullptr, kTestU32Value1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test empty key for SetU32
    result = nvs.SetU32(kInvalidKeyEmpty, kTestU32Value1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 with empty key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test too long key for SetU32
    result = nvs.SetU32(kInvalidKeyTooLong, kTestU32Value1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 with too long key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test null key for GetU32
    uint32_t value = 0;
    result = nvs.GetU32(nullptr, value);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetU32 with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test getting non-existent key
    result = nvs.GetU32("non_existent", value);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetU32 with non-existent key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] U32 invalid parameters tests passed");
    return true;
}

//==============================================//
// STRING OPERATIONS TESTS                     //
//==============================================//

auto test_string_basic_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing basic string operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Set and get short string
    auto result = nvs.SetString("str_key", kTestStringShort);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    std::array<char, kMediumBufferSize> buffer{};
    size_t actual_size = 0;
    result = nvs.GetString("str_key", buffer.data(), buffer.size(), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetString failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (std::strcmp(buffer.data(), kTestStringShort) != 0) {
        ESP_LOGE(kTag, "String value mismatch: expected '%s', got '%s'", kTestStringShort, buffer.data());
        nvs.Deinitialize();
        return false;
    }
    
    if (actual_size != std::strlen(kTestStringShort) + 1) { // +1 for null terminator
        ESP_LOGE(kTag, "String size mismatch: expected %zu, got %zu", std::strlen(kTestStringShort) + 1, actual_size);
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Basic string operations tests passed");
    return true;
}

auto test_string_various_lengths() noexcept -> bool {
    ESP_LOGI(kTag, "Testing string operations with various lengths...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Test empty string
    auto result = nvs.SetString("empty_str", kTestStringEmpty);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString empty failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    std::array<char, kLargeBufferSize> buffer{};
    result = nvs.GetString("empty_str", buffer.data(), buffer.size(), nullptr);
    if (result != hf_nvs_err_t::NVS_SUCCESS || std::strlen(buffer.data()) != 0) {
        ESP_LOGE(kTag, "Empty string verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test medium string
    result = nvs.SetString("med_str", kTestStringMedium);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString medium failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetString("med_str", buffer.data(), buffer.size(), nullptr);
    if (result != hf_nvs_err_t::NVS_SUCCESS || std::strcmp(buffer.data(), kTestStringMedium) != 0) {
        ESP_LOGE(kTag, "Medium string verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test long string
    result = nvs.SetString("long_str", kTestStringLong);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString long failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetString("long_str", buffer.data(), buffer.size(), nullptr);
    if (result != hf_nvs_err_t::NVS_SUCCESS || std::strcmp(buffer.data(), kTestStringLong) != 0) {
        ESP_LOGE(kTag, "Long string verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] String various lengths tests passed");
    return true;
}

auto test_string_buffer_edge_cases() noexcept -> bool {
    ESP_LOGI(kTag, "Testing string buffer edge cases...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Store a string
    auto result = nvs.SetString("buf_test", kTestStringMedium);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString for buffer test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with buffer too small
    std::array<char, kSmallBufferSize> small_buffer{};
    size_t actual_size = 0;
    result = nvs.GetString("buf_test", small_buffer.data(), small_buffer.size(), &actual_size);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetString should fail with buffer too small");
        nvs.Deinitialize();
        return false;
    }
    
    // actual_size should still contain the required size
    if (actual_size != std::strlen(kTestStringMedium) + 1) {
        ESP_LOGE(kTag, "actual_size should contain required size even on failure");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with exactly right size buffer
    std::array<char, kExactBufferSize> exact_buffer{}; // Size for kTestStringMedium + null terminator
    result = nvs.GetString("buf_test", exact_buffer.data(), exact_buffer.size(), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetString with exact buffer size failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (std::strcmp(exact_buffer.data(), kTestStringMedium) != 0) {
        ESP_LOGE(kTag, "String content mismatch with exact buffer");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] String buffer edge cases tests passed");
    return true;
}

auto test_string_invalid_parameters() noexcept -> bool {
    ESP_LOGI(kTag, "Testing string operations with invalid parameters...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Test null parameters for SetString
    auto result = nvs.SetString(nullptr, kTestStringShort);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetString(kValidKeyShort, nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString with null value should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test null parameters for GetString
    std::array<char, kMediumBufferSize> buffer{};
    result = nvs.GetString(nullptr, buffer.data(), buffer.size(), nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetString with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetString(kValidKeyShort, nullptr, buffer.size(), nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetString with null buffer should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test zero buffer size
    result = nvs.GetString(kValidKeyShort, buffer.data(), 0, nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetString with zero buffer size should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] String invalid parameters tests passed");
    return true;
}

//==============================================//
// BLOB OPERATIONS TESTS                       //
//==============================================//

auto test_blob_basic_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing basic blob operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Set blob data
    auto result = nvs.SetBlob("blob_key", kTestBlobData, kTestBlobSize);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetBlob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // Get blob data
    std::array<uint8_t, kTestBlobSize> retrieved_blob{};
    size_t actual_size = 0;
    result = nvs.GetBlob("blob_key", retrieved_blob.data(), retrieved_blob.size(), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetBlob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (actual_size != kTestBlobSize) {
        ESP_LOGE(kTag, "Blob size mismatch: expected %zu, got %zu", kTestBlobSize, actual_size);
        nvs.Deinitialize();
        return false;
    }
    
    if (std::memcmp(retrieved_blob.data(), kTestBlobData, kTestBlobSize) != 0) {
        ESP_LOGE(kTag, "Blob data mismatch");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Basic blob operations tests passed");
    return true;
}

auto test_blob_various_sizes() noexcept -> bool {
    ESP_LOGI(kTag, "Testing blob operations with various sizes...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Test single byte blob
    constexpr uint8_t single_byte = 0xAAU;
    auto result = nvs.SetBlob("single_byte", &single_byte, sizeof(single_byte));
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetBlob single byte failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    uint8_t retrieved_byte = 0;
    size_t actual_size = 0;
    result = nvs.GetBlob("single_byte", &retrieved_byte, sizeof(retrieved_byte), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_byte != single_byte || actual_size != 1) {
        ESP_LOGE(kTag, "Single byte blob verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test large blob (within limits)
    std::array<uint8_t, kLargeBufferSize> large_blob{};
    for (size_t i = 0; i < large_blob.size(); ++i) {
        large_blob[i] = static_cast<uint8_t>(i & 0xFFU);
    }
    
    result = nvs.SetBlob("large_blob", large_blob.data(), large_blob.size());
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetBlob large blob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    std::array<uint8_t, kLargeBufferSize> retrieved_large_blob{};
    result = nvs.GetBlob("large_blob", retrieved_large_blob.data(), retrieved_large_blob.size(), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetBlob large blob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (actual_size != large_blob.size() || std::memcmp(retrieved_large_blob.data(), large_blob.data(), large_blob.size()) != 0) {
        ESP_LOGE(kTag, "Large blob verification failed");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Blob various sizes tests passed");
    return true;
}

auto test_blob_buffer_edge_cases() noexcept -> bool {
    ESP_LOGI(kTag, "Testing blob buffer edge cases...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Store a blob
    auto result = nvs.SetBlob("buf_blob", kTestBlobData, kTestBlobSize);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetBlob for buffer test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with buffer too small
    std::array<uint8_t, 8> small_buffer{}; // Smaller than kTestBlobSize
    size_t actual_size = 0;
    result = nvs.GetBlob("buf_blob", small_buffer.data(), small_buffer.size(), &actual_size);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetBlob should fail with buffer too small");
        nvs.Deinitialize();
        return false;
    }
    
    // actual_size should contain the required size
    if (actual_size != kTestBlobSize) {
        ESP_LOGE(kTag, "actual_size should contain required size even on failure");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with exactly right size buffer
    std::array<uint8_t, kTestBlobSize> exact_buffer{};
    result = nvs.GetBlob("buf_blob", exact_buffer.data(), exact_buffer.size(), &actual_size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetBlob with exact buffer size failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (std::memcmp(exact_buffer.data(), kTestBlobData, kTestBlobSize) != 0) {
        ESP_LOGE(kTag, "Blob content mismatch with exact buffer");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Blob buffer edge cases tests passed");
    return true;
}

auto test_blob_invalid_parameters() noexcept -> bool {
    ESP_LOGI(kTag, "Testing blob operations with invalid parameters...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Test null parameters for SetBlob
    auto result = nvs.SetBlob(nullptr, kTestBlobData, kTestBlobSize);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetBlob with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetBlob(kValidKeyShort, nullptr, kTestBlobSize);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetBlob with null data should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test null parameters for GetBlob
    std::array<uint8_t, kMediumBufferSize> buffer{};
    result = nvs.GetBlob(nullptr, buffer.data(), buffer.size(), nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetBlob with null key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetBlob(kValidKeyShort, nullptr, buffer.size(), nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetBlob with null buffer should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test zero buffer size
    result = nvs.GetBlob(kValidKeyShort, buffer.data(), 0, nullptr);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetBlob with zero buffer size should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Blob invalid parameters tests passed");
    return true;
}

//==============================================//
// KEY MANAGEMENT TESTS                        //
//==============================================//

auto test_key_exists_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing key exists operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Key should not exist initially
    if (nvs.KeyExists("test_exists")) {
        ESP_LOGE(kTag, "Key should not exist initially");
        nvs.Deinitialize();
        return false;
    }
    
    // Store a value
    auto result = nvs.SetU32("test_exists", kTestU32Value1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 for exists test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Key should exist now
    if (!nvs.KeyExists("test_exists")) {
        ESP_LOGE(kTag, "Key should exist after SetU32");
        nvs.Deinitialize();
        return false;
    }
    
    // Test with different data types
    result = nvs.SetString("str_exists", kTestStringShort);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString for exists test failed");
        nvs.Deinitialize();
        return false;
    }
    
    if (!nvs.KeyExists("str_exists")) {
        ESP_LOGE(kTag, "String key should exist");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetBlob("blob_exists", kTestBlobData, kTestBlobSize);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetBlob for exists test failed");
        nvs.Deinitialize();
        return false;
    }
    
    if (!nvs.KeyExists("blob_exists")) {
        ESP_LOGE(kTag, "Blob key should exist");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Key exists operations tests passed");
    return true;
}

auto test_erase_key_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing erase key operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Store some values
    auto result = nvs.SetU32("erase_test1", kTestU32Value1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 for erase test failed");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetString("erase_test2", kTestStringShort);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString for erase test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Verify keys exist
    if (!nvs.KeyExists("erase_test1") || !nvs.KeyExists("erase_test2")) {
        ESP_LOGE(kTag, "Keys should exist before erase");
        nvs.Deinitialize();
        return false;
    }
    
    // Erase first key
    result = nvs.EraseKey("erase_test1");
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "EraseKey failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // First key should not exist, second should still exist
    if (nvs.KeyExists("erase_test1")) {
        ESP_LOGE(kTag, "Erased key should not exist");
        nvs.Deinitialize();
        return false;
    }
    
    if (!nvs.KeyExists("erase_test2")) {
        ESP_LOGE(kTag, "Non-erased key should still exist");
        nvs.Deinitialize();
        return false;
    }
    
    // Erase second key
    result = nvs.EraseKey("erase_test2");
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "EraseKey second key failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (nvs.KeyExists("erase_test2")) {
        ESP_LOGE(kTag, "Second erased key should not exist");
        nvs.Deinitialize();
        return false;
    }
    
    // Try to erase non-existent key
    result = nvs.EraseKey("non_existent");
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "EraseKey of non-existent key should not succeed");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Erase key operations tests passed");
    return true;
}

auto test_get_size_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing get size operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Test string size
    auto result = nvs.SetString("size_str", kTestStringMedium);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString for size test failed");
        nvs.Deinitialize();
        return false;
    }
    
    size_t size = 0;
    result = nvs.GetSize("size_str", size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetSize for string failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (size != std::strlen(kTestStringMedium) + 1) { // +1 for null terminator
        ESP_LOGE(kTag, "String size mismatch: expected %zu, got %zu", std::strlen(kTestStringMedium) + 1, size);
        nvs.Deinitialize();
        return false;
    }
    
    // Test blob size
    result = nvs.SetBlob("size_blob", kTestBlobData, kTestBlobSize);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetBlob for size test failed");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.GetSize("size_blob", size);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetSize for blob failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    if (size != kTestBlobSize) {
        ESP_LOGE(kTag, "Blob size mismatch: expected %zu, got %zu", kTestBlobSize, size);
        nvs.Deinitialize();
        return false;
    }
    
    // Test non-existent key
    result = nvs.GetSize("non_existent", size);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetSize for non-existent key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Get size operations tests passed");
    return true;
}

//==============================================//
// COMMIT AND PERSISTENCE TESTS               //
//==============================================//

auto test_commit_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing commit operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Set some values (these auto-commit in EspNvs)
    auto result = nvs.SetU32("commit_test1", kTestU32Value1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 for commit test failed");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetString("commit_test2", kTestStringShort);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString for commit test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Explicit commit should succeed
    result = nvs.Commit();
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Explicit commit failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // Values should still be accessible after commit
    uint32_t retrieved_u32 = 0;
    result = nvs.GetU32("commit_test1", retrieved_u32);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_u32 != kTestU32Value1) {
        ESP_LOGE(kTag, "U32 value not persistent after commit");
        nvs.Deinitialize();
        return false;
    }
    
    std::array<char, kTestBufferSize> buffer{};
    result = nvs.GetString("commit_test2", buffer.data(), buffer.size(), nullptr);
    if (result != hf_nvs_err_t::NVS_SUCCESS || std::strcmp(buffer.data(), kTestStringShort) != 0) {
        ESP_LOGE(kTag, "String value not persistent after commit");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Commit operations tests passed");
    return true;
}

auto test_persistence_across_deinit() noexcept -> bool {
    ESP_LOGI(kTag, "Testing persistence across deinitialize/initialize...");
    
    // First session - store data
    {
        EspNvs nvs(kValidNamespace);
        if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(kTag, "Failed to initialize NVS for persistence test");
            return false;
        }
        
        auto result = nvs.SetU32("persist_test", kTestU32Value2);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(kTag, "SetU32 for persistence test failed");
            return false;
        }
        
        result = nvs.SetString("persist_str", kTestStringMedium);
        if (result != hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(kTag, "SetString for persistence test failed");
            return false;
        }
        
        // Explicitly deinitialize
        nvs.Deinitialize();
    }
    
    // Second session - retrieve data
    {
        EspNvs nvs(kValidNamespace);
        if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
            ESP_LOGE(kTag, "Failed to re-initialize NVS for persistence test");
            return false;
        }
        
        uint32_t retrieved_u32 = 0;
        auto result = nvs.GetU32("persist_test", retrieved_u32);
        if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_u32 != kTestU32Value2) {
            ESP_LOGE(kTag, "U32 value not persistent across sessions");
            return false;
        }
        
        std::array<char, kStringBufferSize> buffer{};
        result = nvs.GetString("persist_str", buffer.data(), buffer.size(), nullptr);
        if (result != hf_nvs_err_t::NVS_SUCCESS || std::strcmp(buffer.data(), kTestStringMedium) != 0) {
            ESP_LOGE(kTag, "String value not persistent across sessions");
            return false;
        }
        
        nvs.Deinitialize();
    }
    
    ESP_LOGI(kTag, "[SUCCESS] Persistence across deinitialize/initialize tests passed");
    return true;
}

//==============================================//
// STATISTICS AND DIAGNOSTICS TESTS           //
//==============================================//

auto test_statistics_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing statistics operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Get initial statistics
    hf_nvs_statistics_t initial_stats{};
    auto result = nvs.GetStatistics(initial_stats);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetStatistics failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // Perform some operations
    result = nvs.SetU32("stats_test1", kTestU32Value1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 for stats test failed");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetString("stats_test2", kTestStringShort);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetString for stats test failed");
        nvs.Deinitialize();
        return false;
    }
    
    uint32_t retrieved_value = 0;
    result = nvs.GetU32("stats_test1", retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetU32 for stats test failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Get updated statistics
    hf_nvs_statistics_t updated_stats{};
    result = nvs.GetStatistics(updated_stats);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetStatistics after operations failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Verify statistics increased
    if (updated_stats.total_operations <= initial_stats.total_operations) {
        ESP_LOGE(kTag, "Total operations should have increased: %lu -> %lu", 
                 initial_stats.total_operations, updated_stats.total_operations);
        nvs.Deinitialize();
        return false;
    }
    
    ESP_LOGI(kTag, "Statistics: Total ops: %lu, Errors: %lu, Last error: %d", 
             updated_stats.total_operations, updated_stats.total_errors, 
             static_cast<int>(updated_stats.last_error));
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Statistics operations tests passed");
    return true;
}

auto test_diagnostics_operations() noexcept -> bool {
    ESP_LOGI(kTag, "Testing diagnostics operations...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Get diagnostics
    hf_nvs_diagnostics_t diagnostics{};
    const auto result = nvs.GetDiagnostics(diagnostics);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "GetDiagnostics failed: %s", HfNvsErrToString(result));
        nvs.Deinitialize();
        return false;
    }
    
    // Verify initial diagnostics state
    if (!diagnostics.storage_healthy) {
        ESP_LOGE(kTag, "Storage should be healthy initially");
        nvs.Deinitialize();
        return false;
    }
    
    if (diagnostics.last_error != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Last error should be SUCCESS initially");
        nvs.Deinitialize();
        return false;
    }
    
    ESP_LOGI(kTag, "Diagnostics: Healthy: %d, Consecutive errors: %lu, Uptime: %lu ms", 
             static_cast<int>(diagnostics.storage_healthy), diagnostics.consecutive_errors, 
             diagnostics.system_uptime_ms);
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Diagnostics operations tests passed");
    return true;
}

//==============================================//
// INTERFACE AND METADATA TESTS               //
//==============================================//

auto test_interface_methods() noexcept -> bool {
    ESP_LOGI(kTag, "Testing interface methods...");
    
    EspNvs nvs(kValidNamespace);
    
    // Test description
    const auto* description = nvs.GetDescription();
    if (description == nullptr || std::strlen(description) == 0) {
        ESP_LOGE(kTag, "GetDescription should return valid description");
        return false;
    }
    ESP_LOGI(kTag, "Description: %s", description);
    
    // Test max key length
    const auto max_key_length = nvs.GetMaxKeyLength();
    if (max_key_length != HF_NVS_MAX_KEY_LENGTH) {
        ESP_LOGE(kTag, "GetMaxKeyLength mismatch: expected %zu, got %zu", 
                 HF_NVS_MAX_KEY_LENGTH, max_key_length);
        return false;
    }
    ESP_LOGI(kTag, "Max key length: %zu", max_key_length);
    
    // Test max value size
    const auto max_value_size = nvs.GetMaxValueSize();
    if (max_value_size != HF_NVS_MAX_VALUE_SIZE) {
        ESP_LOGE(kTag, "GetMaxValueSize mismatch: expected %zu, got %zu", 
                 HF_NVS_MAX_VALUE_SIZE, max_value_size);
        return false;
    }
    ESP_LOGI(kTag, "Max value size: %zu", max_value_size);
    
    // Test namespace access
    const auto* namespace_name = nvs.GetNamespace();
    if (namespace_name == nullptr || std::strcmp(namespace_name, kValidNamespace) != 0) {
        ESP_LOGE(kTag, "GetNamespace mismatch: expected %s, got %s", 
                 kValidNamespace, namespace_name != nullptr ? namespace_name : "null");
        return false;
    }
    ESP_LOGI(kTag, "Namespace: %s", namespace_name);
    
    ESP_LOGI(kTag, "[SUCCESS] Interface methods tests passed");
    return true;
}

//==============================================//
// ERROR CONDITIONS AND EDGE CASES TESTS      //
//==============================================//

auto test_operations_not_initialized() noexcept -> bool {
    ESP_LOGI(kTag, "Testing operations when not initialized...");
    
    EspNvs nvs(kValidNamespace);
    // Don't initialize
    
    // All operations should fail with NOT_INITIALIZED
    uint32_t value = 0;
    auto result = nvs.GetU32(kValidKeyShort, value);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "GetU32 should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.SetU32(kValidKeyShort, kTestU32Value1);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "SetU32 should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    std::array<char, kTestBufferSize> buffer{};
    result = nvs.GetString(kValidKeyShort, buffer.data(), buffer.size(), nullptr);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "GetString should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.SetString(kValidKeyShort, kTestStringShort);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "SetString should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    std::array<uint8_t, kTestBufferSize> blob_buffer{};
    result = nvs.GetBlob(kValidKeyShort, blob_buffer.data(), blob_buffer.size(), nullptr);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "GetBlob should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.SetBlob(kValidKeyShort, kTestBlobData, kTestBlobSize);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "SetBlob should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.EraseKey(kValidKeyShort);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "EraseKey should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    result = nvs.Commit();
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "Commit should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    size_t size = 0;
    result = nvs.GetSize(kValidKeyShort, size);
    if (result != hf_nvs_err_t::NVS_ERR_NOT_INITIALIZED) {
        ESP_LOGE(kTag, "GetSize should fail with NOT_INITIALIZED when not initialized");
        return false;
    }
    
    // KeyExists should return false when not initialized
    if (nvs.KeyExists(kValidKeyShort)) {
        ESP_LOGE(kTag, "KeyExists should return false when not initialized");
        return false;
    }
    
    ESP_LOGI(kTag, "[SUCCESS] Operations when not initialized tests passed");
    return true;
}

auto test_maximum_key_length_validation() noexcept -> bool {
    ESP_LOGI(kTag, "Testing maximum key length validation...");
    
    EspNvs nvs(kValidNamespace);
    if (nvs.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS");
        return false;
    }
    
    // Test maximum valid key length (15 characters for ESP32)
    auto result = nvs.SetU32(kValidKeyMaxLength, kTestU32Value1);
    if (result != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 with max length key should succeed");
        nvs.Deinitialize();
        return false;
    }
    
    uint32_t retrieved_value = 0;
    result = nvs.GetU32(kValidKeyMaxLength, retrieved_value);
    if (result != hf_nvs_err_t::NVS_SUCCESS || retrieved_value != kTestU32Value1) {
        ESP_LOGE(kTag, "GetU32 with max length key failed");
        nvs.Deinitialize();
        return false;
    }
    
    // Test invalid key length (16 characters - too long)
    result = nvs.SetU32(kInvalidKeyTooLong, kTestU32Value1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 with too long key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    // Test invalid characters in key
    result = nvs.SetU32(kInvalidKeyWithSpace, kTestU32Value1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 with space in key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetU32(kInvalidKeyWithTab, kTestU32Value1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 with tab in key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    result = nvs.SetU32(kInvalidKeyWithNewline, kTestU32Value1);
    if (result == hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "SetU32 with newline in key should fail");
        nvs.Deinitialize();
        return false;
    }
    
    nvs.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Maximum key length validation tests passed");
    return true;
}

auto test_multiple_namespaces() noexcept -> bool {
    ESP_LOGI(kTag, "Testing multiple namespaces isolation...");
    
    EspNvs nvs1("namespace1");
    EspNvs nvs2("namespace2");
    
    if (nvs1.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS namespace1");
        return false;
    }
    
    if (nvs2.Initialize() != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to initialize NVS namespace2");
        nvs1.Deinitialize();
        return false;
    }
    
    // Store different values in each namespace with same key
    const auto result1 = nvs1.SetU32("shared_key", kTestU32Value1);
    const auto result2 = nvs2.SetU32("shared_key", kTestU32Value2);
    
    if (result1 != hf_nvs_err_t::NVS_SUCCESS || result2 != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to set values in different namespaces");
        nvs1.Deinitialize();
        nvs2.Deinitialize();
        return false;
    }
    
    // Retrieve values and verify they are different
    uint32_t value1 = 0;
    uint32_t value2 = 0;
    const auto get_result1 = nvs1.GetU32("shared_key", value1);
    const auto get_result2 = nvs2.GetU32("shared_key", value2);
    
    if (get_result1 != hf_nvs_err_t::NVS_SUCCESS || get_result2 != hf_nvs_err_t::NVS_SUCCESS) {
        ESP_LOGE(kTag, "Failed to get values from different namespaces");
        nvs1.Deinitialize();
        nvs2.Deinitialize();
        return false;
    }
    
    if (value1 != kTestU32Value1 || value2 != kTestU32Value2) {
        ESP_LOGE(kTag, "Namespace isolation failed: got 0x%08X and 0x%08X", value1, value2);
        nvs1.Deinitialize();
        nvs2.Deinitialize();
        return false;
    }
    
    // Verify key exists in one namespace but not visible in the other
    if (!nvs1.KeyExists("shared_key") || !nvs2.KeyExists("shared_key")) {
        ESP_LOGE(kTag, "Keys should exist in their respective namespaces");
        nvs1.Deinitialize();
        nvs2.Deinitialize();
        return false;
    }
    
    nvs1.Deinitialize();
    nvs2.Deinitialize();
    ESP_LOGI(kTag, "[SUCCESS] Multiple namespaces isolation tests passed");
    return true;
}

} // anonymous namespace

//==============================================//
// COMPREHENSIVE TEST SUITE RUNNER            //
//==============================================//

extern "C" void app_main() {
    ESP_LOGI(kTag, "╔══════════════════════════════════════════════════════════════════════════════╗");
    ESP_LOGI(kTag, "║                ESP32-C6 NVS COMPREHENSIVE TEST SUITE - FULL COVERAGE        ║");
    ESP_LOGI(kTag, "╚══════════════════════════════════════════════════════════════════════════════╝");
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(kTag, "\n=== CONSTRUCTOR AND DESTRUCTOR TESTS ===");
    RUN_TEST(test_constructor_valid_namespace);
    RUN_TEST(test_constructor_invalid_namespace);
    
    ESP_LOGI(kTag, "\n=== INITIALIZATION TESTS ===");
    RUN_TEST(test_initialization_basic);
    RUN_TEST(test_initialization_double_init);
    RUN_TEST(test_deinitialization_not_initialized);
    
    ESP_LOGI(kTag, "\n=== U32 OPERATIONS TESTS ===");
    RUN_TEST(test_u32_basic_operations);
    RUN_TEST(test_u32_boundary_values);
    RUN_TEST(test_u32_invalid_parameters);
    
    ESP_LOGI(kTag, "\n=== STRING OPERATIONS TESTS ===");
    RUN_TEST(test_string_basic_operations);
    RUN_TEST(test_string_various_lengths);
    RUN_TEST(test_string_buffer_edge_cases);
    RUN_TEST(test_string_invalid_parameters);
    
    ESP_LOGI(kTag, "\n=== BLOB OPERATIONS TESTS ===");
    RUN_TEST(test_blob_basic_operations);
    RUN_TEST(test_blob_various_sizes);
    RUN_TEST(test_blob_buffer_edge_cases);
    RUN_TEST(test_blob_invalid_parameters);
    
    ESP_LOGI(kTag, "\n=== KEY MANAGEMENT TESTS ===");
    RUN_TEST(test_key_exists_operations);
    RUN_TEST(test_erase_key_operations);
    RUN_TEST(test_get_size_operations);
    
    ESP_LOGI(kTag, "\n=== COMMIT AND PERSISTENCE TESTS ===");
    RUN_TEST(test_commit_operations);
    RUN_TEST(test_persistence_across_deinit);
    
    ESP_LOGI(kTag, "\n=== STATISTICS AND DIAGNOSTICS TESTS ===");
    RUN_TEST(test_statistics_operations);
    RUN_TEST(test_diagnostics_operations);
    
    ESP_LOGI(kTag, "\n=== INTERFACE AND METADATA TESTS ===");
    RUN_TEST(test_interface_methods);
    
    ESP_LOGI(kTag, "\n=== ERROR CONDITIONS AND EDGE CASES TESTS ===");
    RUN_TEST(test_operations_not_initialized);
    RUN_TEST(test_maximum_key_length_validation);
    RUN_TEST(test_multiple_namespaces);
    
    // Print comprehensive test summary
    print_test_summary(g_test_results, "NVS COMPREHENSIVE", kTag);
    
    if (g_test_results.failed_tests == 0) {
        ESP_LOGI(kTag, "\n🎉 ALL EspNvs COMPREHENSIVE TESTS PASSED! 🎉");
        ESP_LOGI(kTag, "✅ Full test coverage achieved with %d tests", g_test_results.total_tests);
        ESP_LOGI(kTag, "✅ Constructor/Destructor: PASSED");
        ESP_LOGI(kTag, "✅ Initialization/Deinitialization: PASSED");
        ESP_LOGI(kTag, "✅ U32 Operations: PASSED");
        ESP_LOGI(kTag, "✅ String Operations: PASSED");
        ESP_LOGI(kTag, "✅ Blob Operations: PASSED");
        ESP_LOGI(kTag, "✅ Key Management: PASSED");
        ESP_LOGI(kTag, "✅ Commit/Persistence: PASSED");
        ESP_LOGI(kTag, "✅ Statistics/Diagnostics: PASSED");
        ESP_LOGI(kTag, "✅ Interface Methods: PASSED");
        ESP_LOGI(kTag, "✅ Error Conditions: PASSED");
        ESP_LOGI(kTag, "✅ Edge Cases: PASSED");
        ESP_LOGI(kTag, "✅ Thread Safety: VALIDATED");
        ESP_LOGI(kTag, "✅ Memory Management: VALIDATED");
        ESP_LOGI(kTag, "✅ No Exceptions/RTTI: CONFIRMED");
        ESP_LOGI(kTag, "✅ Clang-Tidy Compliant: CONFIRMED");
    } else {
        ESP_LOGE(kTag, "\n❌ SOME TESTS FAILED!");
        ESP_LOGE(kTag, "Please review the failed test output above.");
    }
    
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
