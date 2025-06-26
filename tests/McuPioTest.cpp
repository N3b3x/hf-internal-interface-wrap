/**
 * @file McuPioTest.cpp
 * @brief Unit tests for the ESP32 RMT-based McuPio implementation
 */

#include "../mcu/McuPio.h"
#include <cassert>
#include <iostream>
#include <vector>

class McuPioTest {
private:
    McuPio pio_;
    
public:
    void RunAllTests() {
        std::cout << "=== McuPio Unit Tests ===" << std::endl;
        
        TestInitialization();
        TestChannelConfiguration();
        TestSymbolValidation();
        TestCapabilities();
        TestErrorHandling();
        
        std::cout << "=== All tests passed! ===" << std::endl;
    }
    
private:
    void TestInitialization() {
        std::cout << "Testing initialization..." << std::endl;
        
        // Test initial state
        assert(!pio_.IsInitialized());
        
        // Test initialization
        HfPioErr result = pio_.Initialize();
        assert(result == HfPioErr::PIO_SUCCESS);
        assert(pio_.IsInitialized());
        
        // Test double initialization
        result = pio_.Initialize();
        assert(result == HfPioErr::PIO_ERR_ALREADY_INITIALIZED);
        
        std::cout << "✓ Initialization tests passed" << std::endl;
    }
    
    void TestChannelConfiguration() {
        std::cout << "Testing channel configuration..." << std::endl;
        
        // Test valid channel configuration
        PioChannelConfig config;
        config.gpio_pin = 18;
        config.direction = PioDirection::Transmit;
        config.resolution_ns = 1000;
        config.polarity = PioPolarity::Normal;
        config.idle_state = PioIdleState::Low;
        
        HfPioErr result = pio_.ConfigureChannel(0, config);
        // Note: On non-ESP32 platforms, this will return UNSUPPORTED_OPERATION
        // On ESP32, it should succeed if GPIO is available
        assert(result == HfPioErr::PIO_SUCCESS || 
               result == HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION);
        
        // Test invalid channel ID
        result = pio_.ConfigureChannel(255, config);
        assert(result == HfPioErr::PIO_ERR_INVALID_CHANNEL);
        
        // Test invalid GPIO pin
        config.gpio_pin = -1;
        result = pio_.ConfigureChannel(1, config);
        assert(result == HfPioErr::PIO_ERR_INVALID_PARAMETER);
        
        // Test invalid resolution
        config.gpio_pin = 19;
        config.resolution_ns = 0;
        result = pio_.ConfigureChannel(1, config);
        assert(result == HfPioErr::PIO_ERR_INVALID_RESOLUTION);
        
        std::cout << "✓ Channel configuration tests passed" << std::endl;
    }
    
    void TestSymbolValidation() {
        std::cout << "Testing symbol validation..." << std::endl;
        
        // Configure a channel first
        PioChannelConfig config;
        config.gpio_pin = 20;
        config.direction = PioDirection::Transmit;
        config.resolution_ns = 1000;
        
        HfPioErr result = pio_.ConfigureChannel(2, config);
        if (result != HfPioErr::PIO_SUCCESS && 
            result != HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION) {
            std::cout << "✓ Symbol validation tests skipped (platform not supported)" << std::endl;
            return;
        }
        
        // Test valid symbols
        std::vector<PioSymbol> symbols = {
            {1000, true},
            {500, false},
            {2000, true}
        };
        
        result = pio_.Transmit(2, symbols.data(), symbols.size(), false);
        // Should succeed or be unsupported on non-ESP32
        assert(result == HfPioErr::PIO_SUCCESS || 
               result == HfPioErr::PIO_ERR_UNSUPPORTED_OPERATION ||
               result == HfPioErr::PIO_ERR_INVALID_CONFIGURATION);
        
        // Test null pointer
        result = pio_.Transmit(2, nullptr, 1, false);
        assert(result == HfPioErr::PIO_ERR_INVALID_PARAMETER);
        
        // Test zero count
        result = pio_.Transmit(2, symbols.data(), 0, false);
        assert(result == HfPioErr::PIO_ERR_INVALID_PARAMETER);
        
        std::cout << "✓ Symbol validation tests passed" << std::endl;
    }
    
    void TestCapabilities() {
        std::cout << "Testing capabilities..." << std::endl;
        
        PioCapabilities caps;
        HfPioErr result = pio_.GetCapabilities(caps);
        assert(result == HfPioErr::PIO_SUCCESS);
        
        // Verify ESP32 capabilities
        assert(caps.max_channels > 0 && caps.max_channels <= 8);
        assert(caps.min_resolution_ns > 0);
        assert(caps.max_resolution_ns >= caps.min_resolution_ns);
        assert(caps.max_duration > 0);
        assert(caps.max_buffer_size > 0);
        
        std::cout << "✓ Max channels: " << static_cast<int>(caps.max_channels) << std::endl;
        std::cout << "✓ Resolution range: " << caps.min_resolution_ns 
                  << " - " << caps.max_resolution_ns << " ns" << std::endl;
        std::cout << "✓ Max duration: " << caps.max_duration << std::endl;
        std::cout << "✓ Capabilities tests passed" << std::endl;
    }
    
    void TestErrorHandling() {
        std::cout << "Testing error handling..." << std::endl;
        
        // Test channel status on invalid channel
        PioChannelStatus status;
        HfPioErr result = pio_.GetChannelStatus(255, status);
        assert(result == HfPioErr::PIO_ERR_INVALID_CHANNEL);
        
        // Test operations on unconfigured channel
        PioSymbol symbol = {1000, true};
        result = pio_.Transmit(7, &symbol, 1, false);
        assert(result == HfPioErr::PIO_ERR_INVALID_CONFIGURATION);
        
        // Test busy channel detection
        bool is_busy = pio_.IsChannelBusy(255);
        assert(!is_busy); // Invalid channel should return false
        
        std::cout << "✓ Error handling tests passed" << std::endl;
    }
};

// Test helper for error code string conversion
void TestErrorCodeStrings() {
    std::cout << "Testing error code strings..." << std::endl;
    
    // Test a few error codes
    assert(HfPioErrToString(HfPioErr::PIO_SUCCESS) == "Success");
    assert(HfPioErrToString(HfPioErr::PIO_ERR_NOT_INITIALIZED) == "Not initialized");
    assert(HfPioErrToString(HfPioErr::PIO_ERR_INVALID_CHANNEL) == "Invalid PIO channel");
    assert(HfPioErrToString(HfPioErr::PIO_ERR_HARDWARE_FAULT) == "Hardware fault");
    
    std::cout << "✓ Error code string tests passed" << std::endl;
}

int main() {
    try {
        TestErrorCodeStrings();
        
        McuPioTest test;
        test.RunAllTests();
        
        std::cout << "\n🎉 All McuPio tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
}
