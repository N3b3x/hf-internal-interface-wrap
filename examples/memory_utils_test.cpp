/**
 * @file memory_utils_test.cpp
 * @brief Test program demonstrating make_unique_nothrow functionality
 *
 * This example shows how to use the make_unique_nothrow template for
 * exception-free memory allocation with unique_ptr.
 *
 * @author HardFOC Team
 * @date 2025
 * @copyright HardFOC
 */

#include "utils/memory_utils.h"
#include <array>
#include <vector>

// ESP-IDF headers
#ifdef __cplusplus
extern "C" {
#endif
#include "esp_log.h"
#ifdef __cplusplus
}
#endif

static const char* TAG = "MemoryUtilsTest";

// Test class with constructor parameters
class TestDevice {
private:
  int id_;
  std::string name_;

public:
  TestDevice(int id, const std::string& name) : id_(id), name_(name) {
    ESP_LOGI("TestDevice", "Created: ID=%d, Name=%s", id_, name_.c_str());
  }

  ~TestDevice() {
    ESP_LOGI("TestDevice", "Destroyed: ID=%d, Name=%s", id_, name_.c_str());
  }

  int getId() const {
    return id_;
  }
  const std::string& getName() const {
    return name_;
  }
};

// Function demonstrating make_unique_nothrow usage
bool createAndTestDevice(int id, const std::string& name) {
  ESP_LOGI(TAG, "=== Testing make_unique_nothrow for single object ===");

  // Create device using nothrow allocation
  auto device = hf::utils::make_unique_nothrow<TestDevice>(id, name);
  if (!device) {
    ESP_LOGE(TAG, "Failed to allocate memory for TestDevice");
    return false;
  }

  ESP_LOGI(TAG, "Device created successfully");
  ESP_LOGI(TAG, "Device ID: %d", device->getId());
  ESP_LOGI(TAG, "Device Name: %s", device->getName().c_str());

  return true;
  // device automatically destroyed when going out of scope
}

// Function demonstrating array allocation
bool createAndTestArray(size_t size) {
  ESP_LOGI(TAG, "=== Testing make_unique_array_nothrow ===");

  // Create array using nothrow allocation
  auto buffer = hf::utils::make_unique_array_nothrow<int>(size);
  if (!buffer) {
    ESP_LOGE(TAG, "Failed to allocate memory for array of size %zu", size);
    return false;
  }

  ESP_LOGI(TAG, "Array allocated successfully (size: %zu)", size);

  // Initialize array with some values
  for (size_t i = 0; i < size; ++i) {
    buffer[i] = static_cast<int>(i * 2);
  }

  // Print first few values using fixed-size array for display
  constexpr size_t DISPLAY_COUNT = 5;
  std::array<int, DISPLAY_COUNT> display_values{};
  size_t values_to_show = std::min(size, DISPLAY_COUNT);

  for (size_t i = 0; i < values_to_show; ++i) {
    display_values[i] = buffer[i];
  }

  ESP_LOGI(TAG, "First %zu values: %d %d %d %d %d", values_to_show,
           values_to_show > 0 ? display_values[0] : 0, values_to_show > 1 ? display_values[1] : 0,
           values_to_show > 2 ? display_values[2] : 0, values_to_show > 3 ? display_values[3] : 0,
           values_to_show > 4 ? display_values[4] : 0);

  return true;
  // buffer automatically destroyed when going out of scope
}

// Function demonstrating error handling for large allocations
void testLargeAllocation() {
  ESP_LOGI(TAG, "=== Testing large allocation (should fail gracefully) ===");

  // Try to allocate an impossibly large amount of memory
  constexpr size_t HUGE_SIZE = SIZE_MAX - 1;
  auto huge_buffer = hf::utils::make_unique_array_nothrow<char>(HUGE_SIZE);

  if (!huge_buffer) {
    ESP_LOGI(TAG, "Large allocation failed gracefully (as expected)");
    ESP_LOGI(TAG, "No exception thrown, returned nullptr instead");
  } else {
    ESP_LOGW(TAG, "Unexpected: Large allocation succeeded");
  }
}

// Function demonstrating vector of unique_ptrs
void testVectorOfUniquePtr() {
  ESP_LOGI(TAG, "=== Testing vector of unique_ptr ===");

  std::vector<std::unique_ptr<TestDevice>> devices;

  // Create multiple devices using array for device names
  constexpr size_t DEVICE_COUNT = 3;
  std::array<const char*, DEVICE_COUNT> device_names = {"SensorDevice", "ActuatorDevice",
                                                        "ControllerDevice"};

  for (size_t i = 0; i < DEVICE_COUNT; ++i) {
    int device_id = static_cast<int>(i + 1);
    auto device = hf::utils::make_unique_nothrow<TestDevice>(device_id, device_names[i]);
    if (device) {
      devices.push_back(std::move(device));
      ESP_LOGI(TAG, "Added device %d (%s) to vector", device_id, device_names[i]);
    } else {
      ESP_LOGE(TAG, "Failed to create device %d", device_id);
    }
  }

  ESP_LOGI(TAG, "Vector contains %zu devices", devices.size());

  // Devices will be automatically destroyed when vector goes out of scope
}

int main() {
  ESP_LOGI(TAG, "HardFOC Memory Utils Test Program");
  ESP_LOGI(TAG, "=====================================");

  // Test configuration using array
  struct TestConfig {
    int device_id;
    const char* device_name;
    size_t array_size;
  };

  constexpr TestConfig test_config = {
      .device_id = 42, .device_name = "TestSensor", .array_size = 10};

  // Test 1: Single object allocation
  if (!createAndTestDevice(test_config.device_id, test_config.device_name)) {
    ESP_LOGE(TAG, "Single object allocation test failed");
    return 1;
  }

  // Test 2: Array allocation
  if (!createAndTestArray(test_config.array_size)) {
    ESP_LOGE(TAG, "Array allocation test failed");
    return 1;
  }

  // Test 3: Large allocation (failure test)
  testLargeAllocation();

  // Test 4: Vector of unique_ptrs
  testVectorOfUniquePtr();

  ESP_LOGI(TAG, "All tests completed successfully!");
  ESP_LOGI(TAG, "No exceptions were thrown, all memory was managed safely");

  return 0;
}