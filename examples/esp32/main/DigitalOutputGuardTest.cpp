/**
 * @file DigitalOutputGuardTest.cpp * @brief Simple test to verify DigitalOutputGuard functionality
 * with unified BaseGpio
 *
 * This test ensures that the DigitalOutputGuard properly integrates with
 * the new BaseGpio interface and provides the expected RAII behavior.
 */

#include "DigitalOutputGuard.h"
#include "DigitalGpio.h"
#include <cassert>
#include <iostream>

class MockDigitalGpio : public BaseGpio {
public:
  MockDigitalGpio(hf_gpio_num_t pin_num, Direction direction = Direction::Input)
      : BaseGpio(pin_num, direction), init_called_(false), set_direction_called_(false),
        set_active_called_(false), set_inactive_called_(false) {}

  bool Initialize() noexcept override {
    init_called_ = true;
    return true;
  }

  bool IsPinAvailable() const noexcept override {
    return true;
  }

  uint8_t GetMaxPins() const noexcept override {
    return 48;
  }

protected:
  HfGpioErr SetDirectionImpl(Direction direction) noexcept override {
    set_direction_called_ = true;
    return HfGpioErr::GPIO_SUCCESS;
  }

  HfGpioErr SetOutputModeImpl(OutputMode mode) noexcept override {
    return HfGpioErr::GPIO_SUCCESS;
  }

  HfGpioErr SetActiveImpl() noexcept override {
    set_active_called_ = true;
    return HfGpioErr::GPIO_SUCCESS;
  }

  HfGpioErr SetInactiveImpl() noexcept override {
    set_inactive_called_ = true;
    return HfGpioErr::GPIO_SUCCESS;
  }

  HfGpioErr ToggleImpl() noexcept override {
    return HfGpioErr::GPIO_SUCCESS;
  }

  HfGpioErr IsActiveImpl(bool &is_active) noexcept override {
    is_active = (current_state_ == State::Active);
    return HfGpioErr::GPIO_SUCCESS;
  }

  HfGpioErr SetPullModeImpl(PullMode mode) noexcept override {
    return HfGpioErr::GPIO_SUCCESS;
  }

  PullMode GetPullModeImpl() const noexcept override {
    return pull_mode_;
  }

public:
  // Test verification methods
  bool WasInitCalled() const {
    return init_called_;
  }
  bool WasSetDirectionCalled() const {
    return set_direction_called_;
  }
  bool WasSetActiveCalled() const {
    return set_active_called_;
  }
  bool WasSetInactiveCalled() const {
    return set_inactive_called_;
  }

  void ResetFlags() {
    init_called_ = false;
    set_direction_called_ = false;
    set_active_called_ = false;
    set_inactive_called_ = false;
  }

private:
  bool init_called_;
  bool set_direction_called_;
  bool set_active_called_;
  bool set_inactive_called_;
};

void TestBasicGuardFunctionality() {
  std::cout << "Testing basic guard functionality..." << std::endl;

  MockDigitalGpio gpio(GPIO_NUM_2, DigitalGpio::Direction::Input);

  // Test guard creation and automatic configuration
  {
    DigitalOutputGuard guard(gpio);

    // Verify guard is valid
    assert(guard.IsValid());
    assert(guard.GetLastError() == HfGpioErr::GPIO_SUCCESS);

    // Verify GPIO was configured correctly
    assert(gpio.WasInitCalled());
    assert(gpio.WasSetDirectionCalled()); // Should switch to output
    assert(gpio.WasSetActiveCalled());    // Should set active
    assert(gpio.IsOutput());
    assert(gpio.GetCurrentState() == DigitalGpio::State::Active);

    gpio.ResetFlags();

    // Test manual control
    HfGpioErr result = guard.SetInactive();
    assert(result == HfGpioErr::GPIO_SUCCESS);
    assert(gpio.WasSetInactiveCalled());

    gpio.ResetFlags();

    result = guard.SetActive();
    assert(result == HfGpioErr::GPIO_SUCCESS);
    assert(gpio.WasSetActiveCalled());

    gpio.ResetFlags();
  }

  // Verify destructor behavior
  assert(gpio.WasSetInactiveCalled()); // Destructor should set inactive
  assert(gpio.IsOutput());             // Should preserve output mode

  std::cout << "✅ Basic guard functionality test passed" << std::endl;
}

void TestPreConfiguredOutput() {
  std::cout << "Testing pre-configured output behavior..." << std::endl;

  MockDigitalGpio gpio(GPIO_NUM_4, DigitalGpio::Direction::Output);

  {
    // Don't force output mode since it's already configured
    DigitalOutputGuard guard(gpio, false);

    assert(guard.IsValid());
    assert(gpio.WasInitCalled());
    assert(!gpio.WasSetDirectionCalled()); // Should NOT switch direction
    assert(gpio.WasSetActiveCalled());     // Should set active

    gpio.ResetFlags();
  }

  assert(gpio.WasSetInactiveCalled()); // Destructor should set inactive

  std::cout << "✅ Pre-configured output test passed" << std::endl;
}

void TestErrorHandling() {
  std::cout << "Testing error handling..." << std::endl;

  // Test null pointer
  {
    DigitalOutputGuard guard(nullptr);
    assert(!guard.IsValid());
    assert(guard.GetLastError() == HfGpioErr::GPIO_ERR_NULL_POINTER);

    // Test operations on invalid guard
    HfGpioErr result = guard.SetActive();
    assert(result != HfGpioErr::GPIO_SUCCESS);

    result = guard.SetInactive();
    assert(result != HfGpioErr::GPIO_SUCCESS);
  }

  // Test guard with input pin and ensure_output_mode=false
  {
    MockDigitalGpio gpio(GPIO_NUM_5, DigitalGpio::Direction::Input);
    DigitalOutputGuard guard(gpio, false); // Don't auto-switch to output

    assert(!guard.IsValid());
    assert(guard.GetLastError() == HfGpioErr::GPIO_ERR_DIRECTION_MISMATCH);
  }

  std::cout << "✅ Error handling test passed" << std::endl;
}

void TestStateManagement() {
  std::cout << "Testing state management..." << std::endl;

  MockDigitalGpio gpio(GPIO_NUM_6);

  {
    DigitalOutputGuard guard(gpio);
    assert(guard.IsValid());

    // Test state retrieval
    DigitalGpio::State state = guard.GetCurrentState();
    assert(state == DigitalGpio::State::Active);

    // Change state and verify
    guard.SetInactive();
    state = guard.GetCurrentState();
    assert(state == DigitalGpio::State::Inactive);

    guard.SetActive();
    state = guard.GetCurrentState();
    assert(state == DigitalGpio::State::Active);
  }

  // After destruction, GPIO should be inactive
  assert(gpio.GetCurrentState() == DigitalGpio::State::Inactive);

  std::cout << "✅ State management test passed" << std::endl;
}

void TestMoveSemantics() {
  std::cout << "Testing move semantics..." << std::endl;

  MockDigitalGpio gpio(GPIO_NUM_7);

  // Test move constructor
  {
    DigitalOutputGuard guard1(gpio);
    assert(guard1.IsValid());

    DigitalOutputGuard guard2 = std::move(guard1);
    assert(guard2.IsValid());
    // guard1 should be in moved-from state (implementation-defined)

    gpio.ResetFlags();
  }

  // Only one destructor should have set inactive
  assert(gpio.WasSetInactiveCalled());

  std::cout << "✅ Move semantics test passed" << std::endl;
}

extern "C" int RunDigitalOutputGuardTests() {
  std::cout << "DigitalOutputGuard Test Suite" << std::endl;
  std::cout << "=============================" << std::endl;

  try {
    TestBasicGuardFunctionality();
    TestPreConfiguredOutput();
    TestErrorHandling();
    TestStateManagement();
    TestMoveSemantics();

    std::cout << std::endl;
    std::cout << "🎉 All tests passed successfully!" << std::endl;
    std::cout << "DigitalOutputGuard is properly integrated with DigitalGpio interface."
              << std::endl;

    return 0;
  } catch (const std::exception &e) {
    std::cout << "❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cout << "❌ Test failed with unknown exception" << std::endl;
    return 1;
  }
}
