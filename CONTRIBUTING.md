# 🤝 Contributing to HardFOC Internal Interface Wrapper

Thank you for your interest in contributing to the HardFOC Internal
Interface Wrapper! This document provides guidelines and information for
contributors.

## 📋 **Code Standards**

### 🎯 **Coding Style and Best Practices for HardFOC Development**

- **C++17 Standard Compliance** - All code must be compatible with C++17
- **Consistent Naming** - Follow the established naming conventions:
  - Classes: `PascalCase` (e.g., `EspGpio`, `BaseTemperature`)
  - Functions: `PascalCase` (e.g., `EnsureInitialized`, `ReadChannelV`)
  - Variables: `snake*case*` with trailing underscore for members (e.g., `motor*enable*`, `current*sensor*`)
  - Constants: `UPPER*SNAKE*CASE` (e.g., `ADC*CHANNEL*0`)
  - Types: `hf*` prefix with `*t` suffix (e.g., `hf*gpio*err*t`, `hf*pin*num*t`)

### 🏗️ **Architecture Guidelines**

- **Inherit from Base Classes** - All hardware implementations must
  inherit from their respective base classes
- **Lazy Initialization** - Use the `EnsureInitialized()` pattern for
  resource allocation
- **Comprehensive Error Handling** - All functions must return
  appropriate error codes
- **Thread Safety** - Consider thread safety implications and document
  any limitations
- **Platform Agnostic Types** - Use HardFOC type system
  (`hf*u32*t`, `hf*pin*num_t`, etc.)

## 🧪 **Testing**

### 🔧 **Unit Tests and Hardware Validation Requirements for HardFOC Boards**

- **Unit Tests** - Write comprehensive unit tests for all new functionality
- **Hardware Testing** - Test on actual HardFOC boards with ESP32-C6
- **Integration Tests** - Verify compatibility with existing HardFOC systems
- **Performance Tests** - Ensure real-time performance requirements are met
- **Safety Tests** - Validate safety features and error handling

## 📖 **Documentation**

### 📚 **Documentation Standards and Updates for HardFOC Systems**

- **API Documentation** - Update documentation for all public interfaces
- **User Guides** - Create or update guides for new HardFOC features
- **Example Code** - Provide working examples for HardFOC motor controller boards
- **Architecture Documentation** - Document design decisions and patterns

## 🐛 **Bug Reports**

### 🔍 **How to Report Bugs Effectively for HardFOC Applications**

When reporting bugs, please include:

1. **HardFOC Board Information**: Board model, ESP32-C6 version, power supply
2. **Environment Details**: ESP-IDF version, compiler version, operating system
3. **Reproduction Steps**: Minimal code example, configuration settings
4. **Hardware Configuration**: Connected peripherals, pin assignments
5. **Debugging Information**: Error messages, log output, stack traces

## ✨ **Feature Requests**

### 🚀 **Proposing New Features and Enhancements for HardFOC Boards**

When proposing new features:

1. **HardFOC Use Case** - Describe the specific HardFOC motor
   controller board use case
2. **Technical Specification** - Provide detailed technical requirements
3. **API Design** - Propose the interface design following established patterns
4. **Implementation Plan** - Outline the implementation approach
5. **Testing Strategy** - Describe how the feature will be tested

## 🔄 **Development Workflow**

### 📋 **Step-by-Step Development Process**

1. **Fork the Repository**
2. **Create a Feature Branch**
3. **Implement Your Changes with HardFOC-Specific Tests**
4. **Document Your Changes with HardFOC Examples**
5. **Submit a Pull Request**

## 📋 **Code Quality Standards for HardFOC**

- **C++17 Compliance** - Code compiles without warnings
- **HardFOC Compatibility** - Tested on HardFOC boards
- **Error Handling** - All error conditions handled appropriately
- **Documentation** - All public APIs documented
- **Tests** - Adequate test coverage provided
- **Performance** - Real-time requirements met

---

## 🚀 Thank You for Contributing to HardFOC
Your contributions help make HardFOC motor controller boards more
accessible and powerful for everyone.