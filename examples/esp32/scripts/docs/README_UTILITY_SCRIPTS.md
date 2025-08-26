# ESP32 HardFOC Interface Wrapper - Utility Scripts Guide

This document provides comprehensive documentation for the ESP32 utility scripts, including port detection, setup automation, and helper tools for development workflow management.

---

**Navigation**: [← Previous: Logging System](README_LOGGING_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Port Detection →](README_PORT_DETECTION.md)

---

## 📋 **Table of Contents**

- [📋 Overview](#-overview)
- [🏗️ Architecture and Design](#️-architecture-and-design)
- [🔌 Port Detection and Troubleshooting](#️-port-detection-and-troubleshooting)
- [⚙️ Environment Setup and Automation](#️-environment-setup-and-automation)
- [🔧 Configuration and Information Tools](#️-configuration-and-information-tools)
- [🚀 Usage Examples and Patterns](#️-usage-examples-and-patterns)
- [🔍 Troubleshooting and Debugging](#️-troubleshooting-and-debugging)
- [📚 Reference and Examples](#️-reference-and-examples)

## 📋 **Overview**

The ESP32 utility scripts provide essential tools for development environment setup, port detection, troubleshooting, and workflow automation. These scripts ensure consistent development environments across different platforms and provide intelligent automation for common development tasks.

### **Core Features**
- **Cross-Platform Port Detection**: Automatic ESP32 device identification
- **Environment Automation**: Complete development environment setup
- **Intelligent Troubleshooting**: Automated problem detection and resolution
- **Configuration Management**: Centralized configuration and information access
- **CI/CD Integration**: Optimized for automated environments

### **Key Capabilities**
- Automatic ESP32 device detection across platforms
- Complete development environment setup and configuration
- Intelligent dependency management and installation
- Cross-platform compatibility and optimization
- Automated troubleshooting and problem resolution

## 🏗️ **Architecture and Design**

### **System Architecture**
```
Utility Scripts → Platform Detection → Environment Setup → Tool Installation → Validation
      ↓              ↓                    ↓                ↓                ↓
Port Detection   OS Detection      Dependency Mgmt    Tool Setup      Environment
& Troubleshooting  & Adaptation     & Installation     & Config        Verification
```

### **Component Interaction**
- **Port Detection**: Cross-platform ESP32 device identification
- **Environment Setup**: Automated dependency and tool installation
- **Configuration Tools**: Centralized configuration management
- **Troubleshooting**: Automated problem detection and resolution
- **Platform Adaptation**: Cross-platform compatibility and optimization

### **Design Principles**
- **Cross-Platform**: Consistent behavior across Linux, macOS, and Windows (WSL2)
- **Automated Operation**: Minimal user intervention required
- **Intelligent Fallbacks**: Graceful degradation when tools unavailable
- **Performance Optimized**: Efficient execution and resource usage
- **User Experience**: Clear feedback and error handling

## 🔌 **Port Detection and Troubleshooting**

### **Cross-Platform Port Detection**

#### **Linux Port Detection**
The system automatically detects ESP32 devices on Linux:

```bash
# USB serial device patterns
/dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyACM0

# ESP32-specific USB identifiers
CP210x: Silicon Labs CP210x USB to UART Bridge
CH340: WCH CH340 USB to Serial
FTDI: FTDI FT232R USB UART
CDC ACM: USB CDC ACM devices

# Automatic detection
./detect_ports.sh --verbose
```

#### **macOS Port Detection**
The system adapts to macOS-specific device patterns:

```bash
# macOS device patterns
/dev/cu.usbserial-*, /dev/cu.SLAB_USBtoUART*
/dev/cu.usbmodem*, /dev/cu.usbserial*

# System information
system_profiler SPUSBDataType | grep -i esp

# Automatic detection
./detect_ports.sh --verbose
```

#### **Windows (WSL2) Port Detection**
The system provides WSL2 compatibility:

```bash
# WSL2 port mapping
/dev/ttyS* (COM port equivalents)

# USB device detection
lsusb for device identification

# Port accessibility testing
./detect_ports.sh --test-connection
```

### **Port Validation and Testing**

#### **Connectivity Testing**
```bash
# Test port connectivity
./detect_ports.sh --test-connection

# Verify port accessibility
./detect_ports.sh --verbose

# Check port permissions and status
./detect_ports.sh --verbose --test-connection
```

#### **Permission Management**
The system handles common permission issues:

```bash
# Linux udev rules for ESP32 devices
SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", MODE="0666"
SUBSYSTEM=="tty", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="7523", MODE="0666"

# User group membership
sudo usermod -a -G dialout $USER
sudo usermod -a -G tty $USER

# Permission verification
ls -la /dev/ttyUSB*
groups $USER
```

### **Troubleshooting Capabilities**

#### **Automatic Problem Detection**
```bash
# Common issue detection
- USB driver availability
- Port permission problems
- Device enumeration issues
- Connection stability problems
```

#### **Problem Resolution**
```bash
# Automatic resolution attempts
- Permission fixing
- Driver installation guidance
- Port configuration
- Connection testing
```

## ⚙️ **Environment Setup and Automation**

### **Local Development Setup**

#### **Complete Environment Initialization**
```bash
# Full development environment setup
./setup_repo.sh

# What it installs
- System dependencies (build tools, libraries)
- Clang-20 toolchain (compiler, formatter, analyzer)
- ESP-IDF v5.5 (ESP32 development framework)
- Python dependencies (PyYAML)
- yq (YAML processor)
- Development aliases and environment variables
```

#### **Interactive Setup Process**
```bash
# User-friendly setup
- OS detection and adaptation
- Dependency verification
- Installation confirmation
- Progress feedback
- Completion verification
```

### **CI/CD Environment Setup**

#### **Optimized CI Environment**
```bash
# CI-specific setup
./setup_ci.sh

# CI optimizations
- Cache-aware installation
- Minimal dependency installation
- Non-interactive operation
- Cache statistics and reporting
```

#### **Cache Management**
```bash
# Cache optimization
- ESP-IDF toolchain caching
- Python dependency caching
- Build tool caching
- Cache hit rate monitoring
```

### **Cross-Platform Compatibility**

#### **Operating System Support**
```bash
# Supported platforms
- Linux (Ubuntu 20.04+, Fedora, CentOS)
- macOS (10.15+)
- Windows (WSL2)

# Platform-specific adaptations
- Package manager detection
- Tool installation methods
- Path handling
- Permission management
```

#### **Dependency Management**
```bash
# Automatic dependency detection
- Required tools identification
- Version compatibility checking
- Installation method selection
- Fallback mechanisms
```

## 🔧 **Configuration and Information Tools**

### **Configuration Management**

#### **Centralized Configuration Access**
```bash
# Configuration information
./get_app_info.py list                    # List all applications
./get_app_info.py source_file gpio_test   # Get source file path
./get_app_info.py validate adc_test       # Validate application

# Configuration validation
- Application existence verification
- Source file path validation
- Configuration integrity checking
```

#### **Configuration Integration**
```bash
# CMake integration
execute_process(
    COMMAND python3 get_app_info.py source_file ${APP_TYPE}
    OUTPUT_VARIABLE APP_SOURCE_FILE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Build system integration
- Source file resolution
- Configuration validation
- Build parameter extraction
```

### **Information Extraction**

#### **Application Information**
```bash
# Available information
- Application descriptions
- Source file paths
- Build type support
- ESP-IDF version compatibility
- CI/CD configuration
- Featured status
```

#### **System Information**
```bash
# System details
- Operating system detection
- Tool availability
- Version information
- Path configurations
- Environment variables
```

## 🚀 **Usage Examples and Patterns**

### **Port Detection Workflows**

#### **1. Basic Port Detection**
```bash
# Quick port detection
./detect_ports.sh

# Expected output
- Available ESP32 devices
- Port accessibility status
- Basic device information
```

#### **2. Detailed Port Analysis**
```bash
# Comprehensive port analysis
./detect_ports.sh --verbose

# Expected output
- Detailed device information
- USB device details
- Permission status
- Driver information
```

#### **3. Port Connectivity Testing**
```bash
# Test port connectivity
./detect_ports.sh --test-connection

# Expected output
- Port accessibility verification
- Connection stability testing
- Error detection and reporting
```

### **Environment Setup Workflows**

#### **1. Local Development Setup**
```bash
# Complete local setup
./setup_repo.sh

# Setup process
1. OS detection and adaptation
2. Dependency verification
3. Tool installation
4. Environment configuration
5. Verification and testing
```

#### **2. CI/CD Environment Setup**
```bash
# CI environment setup
./setup_ci.sh

# CI process
1. Cache-aware installation
2. Minimal dependency setup
3. Environment optimization
4. Cache statistics
5. Verification
```

#### **3. Environment Verification**
```bash
# Verify setup
./setup_repo.sh --verify

# Verification checks
- Tool availability
- Version compatibility
- Path configuration
- Environment variables
- Functionality testing
```

### **Configuration Management Workflows**

#### **1. Application Information Access**
```bash
# Get application information
./get_app_info.py list
./get_app_info.py source_file gpio_test
./get_app_info.py validate adc_test

# Information usage
- Build system integration
- Configuration validation
- Documentation generation
- CI/CD pipeline configuration
```

#### **2. Configuration Validation**
```bash
# Validate configuration
./get_app_info.py validate gpio_test
./get_app_info.py validate adc_test

# Validation process
- Application existence check
- Source file verification
- Configuration integrity
- Dependency validation
```

### **Troubleshooting Workflows**

#### **1. Port Problem Resolution**
```bash
# Port troubleshooting
./detect_ports.sh --verbose --test-connection

# Problem resolution
1. Issue identification
2. Automatic resolution attempts
3. Manual resolution guidance
4. Verification testing
```

#### **2. Environment Problem Resolution**
```bash
# Environment troubleshooting
./setup_repo.sh --troubleshoot

# Resolution process
1. Problem diagnosis
2. Dependency verification
3. Tool reinstallation
4. Configuration repair
5. Verification testing
```

## 🔍 **Troubleshooting and Debugging**

### **Common Port Issues**

#### **1. No ESP32 Devices Detected**
**Problem**: No ESP32 devices found
**Symptoms**: "No ports detected" or "No devices found" messages
**Solutions**:
```bash
# Check device connections
./detect_ports.sh --verbose

# Verify USB drivers
lsusb | grep -i esp
system_profiler SPUSBDataType | grep -i esp

# Check device enumeration
dmesg | grep -i usb
```

#### **2. Port Permission Issues**
**Problem**: Port access denied
**Symptoms**: "Permission denied" or "Access denied" errors
**Solutions**:
```bash
# Check user permissions
ls -la /dev/ttyUSB*
groups $USER

# Add user to required groups
sudo usermod -a -G dialout,tty $USER

# Create udev rules
sudo nano /etc/udev/rules.d/99-esp32.rules
```

#### **3. Port Connectivity Issues**
**Problem**: Port not accessible or unstable
**Symptoms**: "Port not accessible" or connection failures
**Solutions**:
```bash
# Test port connectivity
./detect_ports.sh --test-connection

# Check port stability
./detect_ports.sh --verbose --test-connection

# Verify device mode
# Check for bootloader mode
# Reset device if necessary
```

### **Environment Setup Issues**

#### **1. Dependency Installation Failures**
**Problem**: Required tools not installed
**Symptoms**: "Command not found" or installation errors
**Solutions**:
```bash
# Check tool availability
which git cmake ninja ccache

# Manual installation
sudo apt-get install git cmake ninja-build ccache

# Verify installation
./setup_repo.sh --verify
```

#### **2. ESP-IDF Installation Issues**
**Problem**: ESP-IDF not properly installed
**Symptoms**: "ESP-IDF not found" or environment errors
**Solutions**:
```bash
# Check ESP-IDF installation
ls -la ~/esp/esp-idf/
echo $IDF_PATH

# Reinstall ESP-IDF
./setup_repo.sh --reinstall-esp-idf

# Verify environment
source ~/esp/esp-idf/export.sh
idf.py --version
```

#### **3. Permission and Path Issues**
**Problem**: Insufficient permissions or incorrect paths
**Symptoms**: "Permission denied" or "Path not found" errors
**Solutions**:
```bash
# Check permissions
ls -la ~/esp/
ls -la ~/.espressif/

# Fix permissions
chmod -R 755 ~/esp/
chmod -R 755 ~/.espressif/

# Verify paths
echo $IDF_PATH
echo $PATH
```

### **Debug and Verbose Mode**

#### **Enabling Debug Output**
```bash
# Enable debug mode
export DEBUG=1
export VERBOSE=1

# Run with debug output
./detect_ports.sh --verbose
./setup_repo.sh --debug
./get_app_info.py --verbose
```

#### **Debug Information Available**
```bash
# Debug information
- Port detection process details
- Device enumeration information
- Permission checking details
- Installation process information
- Configuration loading details
- Error context and resolution
```

## 📚 **Reference and Examples**

### **Command Reference**

#### **Port Detection Commands**
```bash
./detect_ports.sh [options]

# Options:
#   --verbose              - Show detailed device information
#   --test-connection     - Test port connectivity
#   --help, -h           - Show usage information
```

#### **Setup Commands**
```bash
./setup_repo.sh [options]     # Local development setup
./setup_ci.sh [options]       # CI/CD environment setup

# Common options:
#   --help, -h           - Show usage information
#   --verify             - Verify installation
#   --debug             - Enable debug output
#   --reinstall-esp-idf - Reinstall ESP-IDF
```

#### **Configuration Commands**
```bash
./get_app_info.py <command> [args...]

# Commands:
#   list                    - List all available applications
#   source_file <app_type>  - Get source file path for application
#   validate <app_type>     - Validate application configuration
#   --help, -h             - Show usage information
```

### **Environment Variables**

#### **Port Detection Variables**
```bash
# Port detection configuration
export PORT_DETECTION_VERBOSE=1    # Enable verbose output
export PORT_TEST_TIMEOUT=5         # Set connection test timeout
export PORT_SCAN_TIMEOUT=3         # Set port scan timeout
```

#### **Setup Configuration Variables**
```bash
# Setup configuration
export SETUP_MODE="local"          # Set setup mode (local/ci)
export ESP_IDF_VERSION="v5.5"      # Set ESP-IDF version
export CLANG_VERSION="20"          # Set Clang version
export PYTHON_VERSION="3.9"        # Set Python version
```

#### **Debug Configuration Variables**
```bash
# Debug configuration
export DEBUG=1                     # Enable debug mode
export VERBOSE=1                   # Enable verbose output
export SETUP_DEBUG=1               # Enable setup debug mode
export PORT_DEBUG=1                # Enable port debug mode
```

### **Configuration Examples**

#### **Minimal Port Detection Configuration**
```bash
# Basic port detection
./detect_ports.sh

# Expected behavior
- Automatic ESP32 device detection
- Basic port information display
- Error reporting for issues
```

#### **Advanced Port Detection Configuration**
```bash
# Comprehensive port analysis
./detect_ports.sh --verbose --test-connection

# Expected behavior
- Detailed device information
- Port connectivity testing
- Permission verification
- Troubleshooting guidance
```

#### **Environment Setup Configuration**
```bash
# Complete environment setup
./setup_repo.sh

# Expected behavior
- OS detection and adaptation
- Dependency installation
- Tool configuration
- Environment verification
```

### **Integration Examples**

#### **CMake Integration**
```cmake
# CMakeLists.txt utility integration
cmake_minimum_required(VERSION 3.16)

# Port detection integration
add_custom_target(detect_ports
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/detect_ports.sh --verbose
    COMMENT "Detecting ESP32 ports"
)

# Configuration validation
add_custom_target(validate_config
    COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/get_app_info.py validate ${APP_TYPE}
    COMMENT "Validating application configuration"
)
```

#### **CI/CD Integration**
```yaml
# GitHub Actions utility integration
- name: Setup ESP32 Environment
  run: |
    cd examples/esp32
    ./scripts/setup_ci.sh

- name: Detect ESP32 Ports
  run: |
    cd examples/esp32
    ./scripts/detect_ports.sh --verbose

- name: Validate Configuration
  run: |
    cd examples/esp32
    python3 ./scripts/get_app_info.py validate gpio_test
```

#### **Automation Scripts**
```bash
#!/bin/bash
# Automated development environment setup

cd examples/esp32

# Setup development environment
echo "Setting up development environment..."
./setup_repo.sh

# Verify setup
echo "Verifying setup..."
./setup_repo.sh --verify

# Detect available ports
echo "Detecting ESP32 ports..."
./detect_ports.sh --verbose

# Validate configuration
echo "Validating configuration..."
python3 ./get_app_info.py validate gpio_test

echo "Setup complete!"
```

### **Best Practices**

#### **1. Port Detection**
- Always use verbose mode for troubleshooting
- Test port connectivity before operations
- Verify permissions and user group membership
- Use automatic detection when possible

#### **2. Environment Setup**
- Use appropriate setup script for your environment
- Verify installation after setup
- Monitor cache usage and optimization
- Regular environment verification

#### **3. Configuration Management**
- Validate configuration before use
- Use centralized configuration access
- Monitor configuration changes
- Regular configuration verification

#### **4. Troubleshooting**
- Enable debug mode for detailed information
- Use systematic problem resolution approach
- Document solutions for future reference
- Regular system health checks

---

**Navigation**: [← Previous: Logging System](README_LOGGING_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Port Detection →](README_PORT_DETECTION.md)
