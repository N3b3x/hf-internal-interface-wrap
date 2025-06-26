# 🏗️ Porting Guide

Follow these steps to add a new MCU platform while keeping the high level API untouched.

## 1. Platform Types

Create a header under `include/mcu/` defining all `hf_*` types for your MCU. Example:

```cpp
using hf_gpio_num_t = int32_t;
using hf_spi_host_t = uint8_t;
```

These aliases allow the rest of the code to remain platform agnostic.

## 2. MCU Implementations

Implement each base interface in `src/mcu/<your_mcu>/`. Start with GPIO and ADC then expand to I2C, SPI, CAN and PWM as needed.

## 3. Platform Selection

Modify `include/mcu/McuSelect.h` to detect your target (e.g. using compiler defines) and include the appropriate headers.

## 4. Build Integration

Add the new source files to the build system (CMakeLists.txt or idf_component_register). Ensure both examples and tests link to your implementation.

## 5. Testing

Run the unit tests on your hardware. Update the [Testing Guide](testing-guide.md) if additional setup is required.

## 6. Documentation

Document any platform-specific quirks or limitations. Submit a pull request with your changes following the [Contributing Guide](../CONTRIBUTING.md).

## 7. Feature Matrix
Maintain a table of supported features per MCU. This helps identify missing capabilities.

## 8. Continuous Integration
- Create test jobs for the new platform
- Use hardware-in-the-loop tests if possible

## 🔗 Related Examples
- [🏎️ Motor Control](../examples/motor-control.md)
