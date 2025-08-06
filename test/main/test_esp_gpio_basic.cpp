/**
 * @file test_esp_gpio_basic.cpp
 * @brief Basic functionality tests for EspGpio class
 * 
 * This file contains comprehensive unit tests for the basic functionality
 * of the EspGpio class, including initialization, pin configuration,
 * basic I/O operations, and error handling.
 * 
 * @author HardFOC Team
 * @date 2025
 * @copyright HardFOC
 */

#include "unity.h"
#include "unity_config.h"
#include "mock/mock_state_manager.h"

// Include the class under test
// Note: In actual implementation, we would need to create a test-friendly version
// that uses our mocks instead of actual ESP-IDF calls
extern "C" {
    // Mock includes
    #include "driver/gpio.h"
    #include "esp_log.h"
}

// We'll create a simplified test interface for EspGpio since we can't easily
// include the full implementation in this context
namespace test {
    
    // Test data structures matching the real EspGpio interface
    enum class hf_gpio_direction_t {
        HF_GPIO_DIRECTION_INPUT = 0,
        HF_GPIO_DIRECTION_OUTPUT = 1
    };
    
    enum class hf_gpio_active_state_t {
        HF_GPIO_ACTIVE_LOW = 0,
        HF_GPIO_ACTIVE_HIGH = 1
    };
    
    enum class hf_gpio_output_mode_t {
        HF_GPIO_OUTPUT_MODE_PUSH_PULL = 0,
        HF_GPIO_OUTPUT_MODE_OPEN_DRAIN = 1
    };
    
    enum class hf_gpio_pull_mode_t {
        HF_GPIO_PULL_MODE_FLOATING = 0,
        HF_GPIO_PULL_MODE_PULL_UP = 1,
        HF_GPIO_PULL_MODE_PULL_DOWN = 2
    };
    
    enum class hf_gpio_drive_cap_t {
        HF_GPIO_DRIVE_CAP_WEAK = 0,
        HF_GPIO_DRIVE_CAP_MEDIUM = 1,
        HF_GPIO_DRIVE_CAP_STRONG = 2,
        HF_GPIO_DRIVE_CAP_STRONGEST = 3
    };
    
    enum class hf_gpio_state_t {
        HF_GPIO_STATE_INACTIVE = 0,
        HF_GPIO_STATE_ACTIVE = 1
    };
    
    enum class hf_gpio_err_t {
        GPIO_SUCCESS = 0,
        GPIO_ERR_INVALID_PIN = -1,
        GPIO_ERR_INVALID_MODE = -2,
        GPIO_ERR_NOT_INITIALIZED = -3,
        GPIO_ERR_ALREADY_INITIALIZED = -4,
        GPIO_ERR_HARDWARE_FAILURE = -5
    };
    
    // Simplified test version of EspGpio for testing
    class TestEspGpio {
    private:
        uint8_t pin_;
        hf_gpio_direction_t direction_;
        hf_gpio_active_state_t active_state_;
        hf_gpio_output_mode_t output_mode_;
        hf_gpio_pull_mode_t pull_mode_;
        hf_gpio_drive_cap_t drive_capability_;
        bool initialized_;
        
    public:
        TestEspGpio(uint8_t pin, 
                   hf_gpio_direction_t direction = hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                   hf_gpio_active_state_t active_state = hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                   hf_gpio_output_mode_t output_mode = hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                   hf_gpio_pull_mode_t pull_mode = hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
                   hf_gpio_drive_cap_t drive_capability = hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM)
            : pin_(pin), direction_(direction), active_state_(active_state),
              output_mode_(output_mode), pull_mode_(pull_mode), 
              drive_capability_(drive_capability), initialized_(false) {
            mock_record_call("TestEspGpio::constructor", nullptr, 0);
        }
        
        ~TestEspGpio() {
            if (initialized_) {
                Deinitialize();
            }
            mock_record_call("TestEspGpio::destructor", nullptr, 0);
        }
        
        bool Initialize() {
            mock_record_call("TestEspGpio::Initialize", nullptr, 0);
            
            // Check for error injection
            uint32_t error = mock_should_fail("TestEspGpio::Initialize");
            if (error != 0) {
                return false;
            }
            
            // Validate pin number
            if (pin_ >= SOC_GPIO_PIN_COUNT) {
                return false;
            }
            
            // Configure mock GPIO
            gpio_mode_t mode;
            switch (direction_) {
                case hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT:
                    mode = GPIO_MODE_INPUT;
                    break;
                case hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT:
                    mode = output_mode_ == hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_OPEN_DRAIN 
                           ? GPIO_MODE_OUTPUT_OD : GPIO_MODE_OUTPUT;
                    break;
                default:
                    return false;
            }
            
            // Mock GPIO configuration
            gpio_config_t io_conf = {};
            io_conf.pin_bit_mask = (1ULL << pin_);
            io_conf.mode = mode;
            io_conf.pull_up_en = (pull_mode_ == hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_PULL_UP) 
                                 ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = (pull_mode_ == hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_PULL_DOWN) 
                                   ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
            io_conf.intr_type = GPIO_INTR_DISABLE;
            
            esp_err_t ret = gpio_config(&io_conf);
            if (ret != ESP_OK) {
                return false;
            }
            
            // Set drive capability
            gpio_drive_cap_t drive_cap;
            switch (drive_capability_) {
                case hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_WEAK:
                    drive_cap = GPIO_DRIVE_CAP_0;
                    break;
                case hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_MEDIUM:
                    drive_cap = GPIO_DRIVE_CAP_2;
                    break;
                case hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONG:
                    drive_cap = GPIO_DRIVE_CAP_3;
                    break;
                default:
                    drive_cap = GPIO_DRIVE_CAP_DEFAULT;
                    break;
            }
            
            ret = gpio_set_drive_capability((gpio_num_t)pin_, drive_cap);
            if (ret != ESP_OK) {
                return false;
            }
            
            initialized_ = true;
            mock_gpio_configure_pin(pin_, (uint32_t)mode, (uint32_t)pull_mode_);
            
            return true;
        }
        
        bool Deinitialize() {
            mock_record_call("TestEspGpio::Deinitialize", nullptr, 0);
            
            if (!initialized_) {
                return false;
            }
            
            esp_err_t ret = gpio_reset_pin((gpio_num_t)pin_);
            if (ret != ESP_OK) {
                return false;
            }
            
            initialized_ = false;
            return true;
        }
        
        bool IsInitialized() const {
            return initialized_;
        }
        
        hf_gpio_err_t SetState(hf_gpio_state_t state) {
            mock_record_call("TestEspGpio::SetState", nullptr, 0);
            
            if (!initialized_) {
                return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
            }
            
            if (direction_ != hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT) {
                return hf_gpio_err_t::GPIO_ERR_INVALID_MODE;
            }
            
            uint32_t level;
            if (active_state_ == hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH) {
                level = (state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) ? 1 : 0;
            } else {
                level = (state == hf_gpio_state_t::HF_GPIO_STATE_ACTIVE) ? 0 : 1;
            }
            
            esp_err_t ret = gpio_set_level((gpio_num_t)pin_, level);
            if (ret != ESP_OK) {
                return hf_gpio_err_t::GPIO_ERR_HARDWARE_FAILURE;
            }
            
            mock_gpio_set_pin_state(pin_, level);
            return hf_gpio_err_t::GPIO_SUCCESS;
        }
        
        hf_gpio_err_t GetState(hf_gpio_state_t& state) {
            mock_record_call("TestEspGpio::GetState", nullptr, 0);
            
            if (!initialized_) {
                return hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED;
            }
            
            int level = gpio_get_level((gpio_num_t)pin_);
            if (level < 0) {
                return hf_gpio_err_t::GPIO_ERR_HARDWARE_FAILURE;
            }
            
            if (active_state_ == hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH) {
                state = (level == 1) ? hf_gpio_state_t::HF_GPIO_STATE_ACTIVE 
                                     : hf_gpio_state_t::HF_GPIO_STATE_INACTIVE;
            } else {
                state = (level == 0) ? hf_gpio_state_t::HF_GPIO_STATE_ACTIVE 
                                     : hf_gpio_state_t::HF_GPIO_STATE_INACTIVE;
            }
            
            return hf_gpio_err_t::GPIO_SUCCESS;
        }
        
        uint8_t GetPin() const { return pin_; }
        hf_gpio_direction_t GetDirection() const { return direction_; }
        hf_gpio_active_state_t GetActiveState() const { return active_state_; }
    };
}

using namespace test;

// Test cases
extern "C" {

/**
 * @brief Test basic GPIO initialization
 */
void test_esp_gpio_basic_initialization(void) {
    // Test successful initialization
    TestEspGpio gpio(5, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
    
    TEST_ASSERT_FALSE(gpio.IsInitialized());
    TEST_ASSERT_TRUE(gpio.Initialize());
    TEST_ASSERT_TRUE(gpio.IsInitialized());
    
    // Verify mock calls
    TEST_ASSERT_TRUE(mock_was_called("TestEspGpio::constructor"));
    TEST_ASSERT_TRUE(mock_was_called("TestEspGpio::Initialize"));
    
    // Test pin configuration in mock state
    TEST_ASSERT_EQUAL_UINT32(GPIO_MODE_OUTPUT, 
        g_mock_state.gpio_pins[5].direction);
}

/**
 * @brief Test GPIO initialization with invalid pin
 */
void test_esp_gpio_initialization_invalid_pin(void) {
    // Test with invalid pin number
    TestEspGpio gpio(99); // Pin 99 doesn't exist
    
    TEST_ASSERT_FALSE(gpio.Initialize());
    TEST_ASSERT_FALSE(gpio.IsInitialized());
}

/**
 * @brief Test GPIO initialization error injection
 */
void test_esp_gpio_initialization_error_injection(void) {
    // Setup error injection
    mock_inject_error("TestEspGpio::Initialize", ESP_ERR_INVALID_ARG, 0);
    
    TestEspGpio gpio(10, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);
    
    TEST_ASSERT_FALSE(gpio.Initialize());
    TEST_ASSERT_FALSE(gpio.IsInitialized());
    
    // Clear error injection
    mock_clear_error_injection();
}

/**
 * @brief Test basic GPIO operations
 */
void test_esp_gpio_basic_operations(void) {
    TestEspGpio gpio(8, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH);
    
    TEST_ASSERT_TRUE(gpio.Initialize());
    
    // Test setting active state
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_SUCCESS, 
        gpio.SetState(hf_gpio_state_t::HF_GPIO_STATE_ACTIVE));
    
    // Verify pin level in mock
    TEST_ASSERT_EQUAL_UINT32(1, mock_gpio_get_pin_state(8));
    
    // Test setting inactive state
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_SUCCESS,
        gpio.SetState(hf_gpio_state_t::HF_GPIO_STATE_INACTIVE));
    
    // Verify pin level in mock
    TEST_ASSERT_EQUAL_UINT32(0, mock_gpio_get_pin_state(8));
}

/**
 * @brief Test GPIO operations with active low configuration
 */
void test_esp_gpio_active_low_operations(void) {
    TestEspGpio gpio(12, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                     hf_gpio_active_state_t::HF_GPIO_ACTIVE_LOW);
    
    TEST_ASSERT_TRUE(gpio.Initialize());
    
    // Test setting active state (should result in low level)
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_SUCCESS,
        gpio.SetState(hf_gpio_state_t::HF_GPIO_STATE_ACTIVE));
    
    // Verify pin level in mock (active low = 0)
    TEST_ASSERT_EQUAL_UINT32(0, mock_gpio_get_pin_state(12));
    
    // Test setting inactive state (should result in high level)
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_SUCCESS,
        gpio.SetState(hf_gpio_state_t::HF_GPIO_STATE_INACTIVE));
    
    // Verify pin level in mock (inactive = 1)
    TEST_ASSERT_EQUAL_UINT32(1, mock_gpio_get_pin_state(12));
}

/**
 * @brief Test GPIO direction setting
 */
void test_esp_gpio_direction_setting(void) {
    // Test output direction
    TestEspGpio output_gpio(15, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
    TEST_ASSERT_TRUE(output_gpio.Initialize());
    TEST_ASSERT_EQUAL_INT32(hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                           output_gpio.GetDirection());
    
    // Test input direction
    TestEspGpio input_gpio(16, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);
    TEST_ASSERT_TRUE(input_gpio.Initialize());
    TEST_ASSERT_EQUAL_INT32(hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                           input_gpio.GetDirection());
    
    // Test error when trying to set state on input pin
    hf_gpio_state_t state;
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_ERR_INVALID_MODE,
        input_gpio.SetState(hf_gpio_state_t::HF_GPIO_STATE_ACTIVE));
}

/**
 * @brief Test GPIO pull mode configuration
 */
void test_esp_gpio_pull_mode_configuration(void) {
    // Test pull-up configuration
    TestEspGpio pullup_gpio(20, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                           hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                           hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                           hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_PULL_UP);
    
    TEST_ASSERT_TRUE(pullup_gpio.Initialize());
    TEST_ASSERT_EQUAL_UINT32(hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_PULL_UP,
        g_mock_state.gpio_pins[20].pull_mode);
    
    // Test pull-down configuration
    TestEspGpio pulldown_gpio(21, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                             hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                             hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                             hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_PULL_DOWN);
    
    TEST_ASSERT_TRUE(pulldown_gpio.Initialize());
    TEST_ASSERT_EQUAL_UINT32(hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_PULL_DOWN,
        g_mock_state.gpio_pins[21].pull_mode);
    
    // Test floating configuration
    TestEspGpio floating_gpio(22, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT,
                             hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                             hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                             hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING);
    
    TEST_ASSERT_TRUE(floating_gpio.Initialize());
    TEST_ASSERT_EQUAL_UINT32(hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
        g_mock_state.gpio_pins[22].pull_mode);
}

/**
 * @brief Test GPIO drive capability configuration
 */
void test_esp_gpio_drive_capability(void) {
    // Test different drive capabilities
    TestEspGpio weak_drive(25, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                          hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                          hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                          hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
                          hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_WEAK);
    
    TEST_ASSERT_TRUE(weak_drive.Initialize());
    
    TestEspGpio strong_drive(26, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT,
                            hf_gpio_active_state_t::HF_GPIO_ACTIVE_HIGH,
                            hf_gpio_output_mode_t::HF_GPIO_OUTPUT_MODE_PUSH_PULL,
                            hf_gpio_pull_mode_t::HF_GPIO_PULL_MODE_FLOATING,
                            hf_gpio_drive_cap_t::HF_GPIO_DRIVE_CAP_STRONGEST);
    
    TEST_ASSERT_TRUE(strong_drive.Initialize());
    
    // Verify mock calls for drive capability setting
    TEST_ASSERT_TRUE(mock_was_called("gpio_set_drive_capability"));
}

/**
 * @brief Test GPIO state reading
 */
void test_esp_gpio_state_reading(void) {
    TestEspGpio input_gpio(18, hf_gpio_direction_t::HF_GPIO_DIRECTION_INPUT);
    TEST_ASSERT_TRUE(input_gpio.Initialize());
    
    // Set mock pin state
    mock_gpio_set_pin_state(18, 1);
    
    hf_gpio_state_t state;
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_SUCCESS, input_gpio.GetState(state));
    TEST_ASSERT_EQUAL_INT32(hf_gpio_state_t::HF_GPIO_STATE_ACTIVE, state);
    
    // Change mock pin state
    mock_gpio_set_pin_state(18, 0);
    
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_SUCCESS, input_gpio.GetState(state));
    TEST_ASSERT_EQUAL_INT32(hf_gpio_state_t::HF_GPIO_STATE_INACTIVE, state);
}

/**
 * @brief Test GPIO operations on uninitialized pin
 */
void test_esp_gpio_uninitialized_operations(void) {
    TestEspGpio gpio(10);
    
    // Test operations on uninitialized GPIO
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED,
        gpio.SetState(hf_gpio_state_t::HF_GPIO_STATE_ACTIVE));
    
    hf_gpio_state_t state;
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED,
        gpio.GetState(state));
}

/**
 * @brief Test GPIO deinitialization
 */
void test_esp_gpio_deinitialization(void) {
    TestEspGpio gpio(7, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
    
    TEST_ASSERT_TRUE(gpio.Initialize());
    TEST_ASSERT_TRUE(gpio.IsInitialized());
    
    TEST_ASSERT_TRUE(gpio.Deinitialize());
    TEST_ASSERT_FALSE(gpio.IsInitialized());
    
    // Test that operations fail after deinitialization
    TEST_ASSERT_EQUAL_INT32(hf_gpio_err_t::GPIO_ERR_NOT_INITIALIZED,
        gpio.SetState(hf_gpio_state_t::HF_GPIO_STATE_ACTIVE));
}

} // extern "C"