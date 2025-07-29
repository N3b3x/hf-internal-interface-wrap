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
#include <iostream>
#include <vector>

// Test class with constructor parameters
class TestDevice {
private:
    int id_;
    std::string name_;
    
public:
    TestDevice(int id, const std::string& name) : id_(id), name_(name) {
        std::cout << "TestDevice created: ID=" << id_ << ", Name=" << name_ << std::endl;
    }
    
    ~TestDevice() {
        std::cout << "TestDevice destroyed: ID=" << id_ << ", Name=" << name_ << std::endl;
    }
    
    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
};

// Function demonstrating make_unique_nothrow usage
bool createAndTestDevice(int id, const std::string& name) {
    std::cout << "\n=== Testing make_unique_nothrow for single object ===" << std::endl;
    
    // Create device using nothrow allocation
    auto device = hf::utils::make_unique_nothrow<TestDevice>(id, name);
    if (!device) {
        std::cout << "❌ Failed to allocate memory for TestDevice" << std::endl;
        return false;
    }
    
    std::cout << "✅ Device created successfully" << std::endl;
    std::cout << "   Device ID: " << device->getId() << std::endl;
    std::cout << "   Device Name: " << device->getName() << std::endl;
    
    return true;
    // device automatically destroyed when going out of scope
}

// Function demonstrating array allocation
bool createAndTestArray(size_t size) {
    std::cout << "\n=== Testing make_unique_array_nothrow ===" << std::endl;
    
    // Create array using nothrow allocation
    auto buffer = hf::utils::make_unique_array_nothrow<int>(size);
    if (!buffer) {
        std::cout << "❌ Failed to allocate memory for array of size " << size << std::endl;
        return false;
    }
    
    std::cout << "✅ Array allocated successfully (size: " << size << ")" << std::endl;
    
    // Initialize array with some values
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = static_cast<int>(i * 2);
    }
    
    // Print first few values
    std::cout << "   First few values: ";
    for (size_t i = 0; i < std::min(size, size_t(5)); ++i) {
        std::cout << buffer[i] << " ";
    }
    std::cout << std::endl;
    
    return true;
    // buffer automatically destroyed when going out of scope
}

// Function demonstrating error handling for large allocations
void testLargeAllocation() {
    std::cout << "\n=== Testing large allocation (should fail gracefully) ===" << std::endl;
    
    // Try to allocate an impossibly large amount of memory
    constexpr size_t HUGE_SIZE = SIZE_MAX / 2;
    auto huge_buffer = hf::utils::make_unique_array_nothrow<char>(HUGE_SIZE);
    
    if (!huge_buffer) {
        std::cout << "✅ Large allocation failed gracefully (as expected)" << std::endl;
        std::cout << "   No exception thrown, returned nullptr instead" << std::endl;
    } else {
        std::cout << "⚠️  Unexpected: Large allocation succeeded" << std::endl;
    }
}

// Function demonstrating vector of unique_ptrs
void testVectorOfUniquePtr() {
    std::cout << "\n=== Testing vector of unique_ptr ===" << std::endl;
    
    std::vector<std::unique_ptr<TestDevice>> devices;
    
    // Create multiple devices
    for (int i = 1; i <= 3; ++i) {
        auto device = hf::utils::make_unique_nothrow<TestDevice>(i, "Device_" + std::to_string(i));
        if (device) {
            devices.push_back(std::move(device));
            std::cout << "✅ Added device " << i << " to vector" << std::endl;
        } else {
            std::cout << "❌ Failed to create device " << i << std::endl;
        }
    }
    
    std::cout << "📊 Vector contains " << devices.size() << " devices" << std::endl;
    
    // Devices will be automatically destroyed when vector goes out of scope
}

int main() {
    std::cout << "🧪 HardFOC Memory Utils Test Program" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Test 1: Single object allocation
    if (!createAndTestDevice(42, "TestSensor")) {
        return 1;
    }
    
    // Test 2: Array allocation
    if (!createAndTestArray(10)) {
        return 1;
    }
    
    // Test 3: Large allocation (failure test)
    testLargeAllocation();
    
    // Test 4: Vector of unique_ptrs
    testVectorOfUniquePtr();
    
    std::cout << "\n🎉 All tests completed successfully!" << std::endl;
    std::cout << "💡 No exceptions were thrown, all memory was managed safely" << std::endl;
    
    return 0;
}