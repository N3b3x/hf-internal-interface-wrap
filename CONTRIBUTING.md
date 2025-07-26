# 🤝 Contributing to HardFOC Internal Interface Wrapper

<div align="center">

![Contributing](https://img.shields.io/badge/Contributing-Welcome-brightgreen?style=for-the-badge&logo=opensourceinitiative)

**🎯 We welcome contributions from developers, engineers, and motor control enthusiasts!**

</div>

---

## 📚 **Table of Contents**

- [🎯 **Getting Started**](#-getting-started)
- [🔧 **Development Setup**](#-development-setup)
- [📋 **Code Standards**](#-code-standards)
- [🧪 **Testing**](#-testing)
- [📖 **Documentation**](#-documentation)
- [🐛 **Bug Reports**](#-bug-reports)
- [✨ **Feature Requests**](#-feature-requests)
- [📝 **Pull Request Process**](#-pull-request-process)
- [📄 **License**](#-license)

---

## 🎯 **Getting Started**

### 🔍 **What Can You Contribute?**

- **🐛 Bug Fixes**: Help us improve reliability and performance
- **✨ New Features**: Add support for new hardware or interfaces
- **📖 Documentation**: Improve guides, examples, and API documentation
- **🧪 Testing**: Add unit tests, integration tests, or performance benchmarks
- **🎨 Code Quality**: Refactoring, optimization, and code cleanup
- **🔌 Hardware Support**: Add new MCU platforms or external chips

### 📋 **Prerequisites**

- **C++17** compatible compiler
- **CMake 3.16+** for building
- **ESP-IDF v5.0+** for ESP32 development
- **Git** for version control
- **Doxygen** for documentation generation (optional)

---

## 🔧 **Development Setup**

### 1. **Fork and Clone**

```bash
# Fork the repository on GitHub, then clone your fork
git clone https://github.com/YOUR_USERNAME/hf-internal-interface-wrap.git
cd hf-internal-interface-wrap

# Add upstream remote
git remote add upstream https://github.com/hardfoc/hf-internal-interface-wrap.git
```

### 2. **Create a Feature Branch**

```bash
# Create and switch to a new branch
git checkout -b feature/your-feature-name

# Or for bug fixes
git checkout -b fix/bug-description
```

### 3. **Set Up ESP-IDF Environment**

```bash
# Install ESP-IDF v5.0+
curl -LO https://github.com/espressif/esp-idf/releases/download/v5.0/esp-idf-v5.0.zip
unzip esp-idf-v5.0.zip
cd esp-idf-v5.0
./install.sh

# Source the environment
source export.sh
```

### 4. **Build and Test**

```bash
# Build the ESP32 example
cd examples/esp32
idf.py build

# Generate documentation
cd ../..
doxygen Doxyfile
```

---

## 📋 **Code Standards**

### 🎯 **Naming Conventions**

```cpp
// Classes: CamelCase
class BaseAdc {};
class EspGpio {};

// Functions and variables: snake_case
bool initialize_hardware();
hf_u32_t channel_count;

// Constants: UPPER_SNAKE_CASE
constexpr hf_pin_num_t HF_INVALID_PIN = -1;

// Types: snake_case with _t suffix
using hf_u32_t = uint32_t;
enum class hf_gpio_direction_t { /* ... */ };
```

### 🔧 **Code Style**

- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 100 characters
- **Braces**: Opening brace on same line
- **Headers**: Include guards using `#pragma once`
- **Documentation**: Doxygen comments for all public APIs

### 📝 **Example Code Style**

```cpp
#pragma once

#include "base/BaseGpio.h"
#include "base/HardwareTypes.h"

/**
 * @brief ESP32 GPIO implementation
 * 
 * Provides GPIO functionality for ESP32-C6 using native GPIO driver.
 * Supports input, output, interrupt, and pull resistor configuration.
 */
class EspGpio : public BaseGpio {
public:
    /**
     * @brief Constructor
     * @param pin_number GPIO pin number to control
     * @param direction Initial direction (input/output)
     */
    EspGpio(hf_pin_num_t pin_number, hf_gpio_direction_t direction);
    
    /**
     * @brief Destructor - cleanup GPIO resources
     */
    ~EspGpio() override;
    
    // Implementation methods...
    
private:
    hf_pin_num_t pin_number_;
    hf_gpio_direction_t current_direction_;
    bool is_initialized_;
};
```

### 🛡️ **Error Handling**

- **Always check return values** from hardware operations
- **Use comprehensive error enums** for each interface
- **Provide meaningful error messages** in documentation
- **Validate parameters** before use

```cpp
hf_gpio_err_t EspGpio::SetDirection(hf_gpio_direction_t direction) {
    if (!IsValidPin(pin_number_)) {
        return hf_gpio_err_t::GPIO_ERR_INVALID_PIN;
    }
    
    esp_err_t err = gpio_set_direction(static_cast<gpio_num_t>(pin_number_), 
                                       ConvertDirection(direction));
    if (err != ESP_OK) {
        return hf_gpio_err_t::GPIO_ERR_HARDWARE_FAILURE;
    }
    
    current_direction_ = direction;
    return hf_gpio_err_t::GPIO_SUCCESS;
}
```

---

## 🧪 **Testing**

### 📝 **Testing Guidelines**

- **Unit Tests**: Test individual functions and methods
- **Integration Tests**: Test hardware interfaces together
- **Hardware Tests**: Test on actual ESP32-C6 hardware
- **Performance Tests**: Benchmark critical operations

### 🎯 **Test Categories**

| Test Type | Purpose | Requirements |
|-----------|---------|--------------|
| **Unit Tests** | Function-level validation | Software only |
| **Integration Tests** | Multi-component testing | Hardware simulation |
| **Hardware Tests** | Real hardware validation | ESP32-C6 board |
| **Performance Tests** | Timing and throughput | Precision timing |

### ⚡ **Performance Requirements**

- **GPIO Operations**: < 10μs per operation
- **ADC Readings**: < 100μs per channel
- **I2C Transactions**: Based on bus speed
- **Memory Usage**: Minimal heap allocation

---

## 📖 **Documentation**

### 📚 **Documentation Standards**

- **API Documentation**: Complete Doxygen comments for all public methods
- **Examples**: Working code examples for common use cases
- **Guides**: Step-by-step tutorials for complex features
- **Architecture**: High-level design documentation

### 🎯 **Required Documentation**

```cpp
/**
 * @brief Brief description of the method
 * 
 * Detailed description explaining what the method does,
 * when to use it, and any important considerations.
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Return value description and possible error codes
 * 
 * @code
 * // Example usage
 * EspGpio led_pin(GPIO_NUM_2, hf_gpio_direction_t::HF_GPIO_DIRECTION_OUTPUT);
 * led_pin.SetHigh();
 * @endcode
 * 
 * @note Any important notes or warnings
 * @warning Critical warnings about usage
 * @see Related functions or classes
 */
```

### 🔄 **Documentation Updates**

- **Update API docs** when changing method signatures
- **Add examples** for new features
- **Update guides** when changing workflows
- **Test documentation** links and references

---

## 🐛 **Bug Reports**

### 📝 **Bug Report Template**

When reporting bugs, please include:

**🎯 Bug Description**
- Clear description of the issue
- Expected vs actual behavior

**🔧 Environment**
- ESP-IDF version
- Hardware platform (ESP32-C6, etc.)
- Operating system
- Compiler version

**📋 Steps to Reproduce**
1. Step-by-step instructions
2. Minimal code example
3. Configuration details

**📊 Additional Information**
- Error messages or logs
- Screenshots if applicable
- Workarounds attempted

### 🏷️ **Bug Labels**

- `bug`: Confirmed bug
- `hardware`: Hardware-specific issue
- `documentation`: Documentation error
- `performance`: Performance issue
- `critical`: Critical/blocking issue

---

## ✨ **Feature Requests**

### 💡 **Feature Request Guidelines**

- **Describe the use case**: Why is this feature needed?
- **Propose a solution**: How should it work?
- **Consider alternatives**: Are there other approaches?
- **Breaking changes**: Will this affect existing code?

### 🎯 **Feature Categories**

| Category | Description | Priority |
|----------|-------------|----------|
| **Hardware Support** | New MCU platforms or chips | High |
| **Interface Extensions** | New communication protocols | Medium |
| **Performance** | Optimization improvements | Medium |
| **Developer Experience** | Tools and utilities | Low |

---

## 📝 **Pull Request Process**

### 1. **Pre-submission Checklist**

- [ ] Code follows style guidelines
- [ ] All tests pass
- [ ] Documentation is updated
- [ ] No breaking changes (or documented)
- [ ] Commit messages are clear

### 2. **Commit Message Format**

```
type(scope): brief description

Detailed explanation of the change, including:
- What was changed and why
- Any breaking changes
- References to issues

Fixes #123
```

**Types**: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `chore`

### 3. **Review Process**

1. **Automated Checks**: CI/CD must pass
2. **Code Review**: At least one maintainer approval
3. **Testing**: Verify on hardware if applicable
4. **Documentation**: Ensure docs are complete

### 4. **Merge Requirements**

- ✅ All CI checks pass
- ✅ Code review approved
- ✅ Documentation updated
- ✅ No merge conflicts
- ✅ Squash commits if needed

---

## 📄 **License**

By contributing to this project, you agree that your contributions will be licensed under the same [MIT License](LICENSE) that covers the project.

---

## 🙏 **Recognition**

All contributors will be recognized in our documentation and release notes. We appreciate every contribution, no matter how small!

### 🌟 **Types of Recognition**

- **Contributors list** in README
- **Changelog mentions** for significant features
- **Special thanks** for major contributions
- **Maintainer status** for ongoing contributors

---

<div align="center">

**🚀 Thank you for contributing to HardFOC Internal Interface Wrapper!**

*Together, we're building better motor control systems*

</div>