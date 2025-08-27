# ESP32 HardFOC Interface Wrapper - Flash System Guide

This document provides comprehensive documentation for the ESP32 flash system, including port detection, firmware flashing, monitoring, and troubleshooting.

---

**Navigation**: [← Previous: Build System](README_BUILD_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Configuration System →](README_CONFIG_SYSTEM.md)

---

## 📋 **Table of Contents**

- [📋 Overview](#-overview)
- [🏗️ Architecture and Design](#️-architecture-and-design)
- [🔌 Port Detection and Management](#️-port-detection-and-management)
- [⚡ Flash Operations and Workflows](#️-flash-operations-and-workflows)
- [📺 Monitoring and Logging](#️-monitoring-and-logging)
- [🚀 Usage Examples and Patterns](#️-usage-examples-and-patterns)
- [🔍 Troubleshooting and Debugging](#️-troubleshooting-and-debugging)
- [📚 Reference and Examples](#️-reference-and-examples)

## 📋 **Overview**

The ESP32 flash system is a comprehensive solution for firmware deployment, device monitoring, and development workflow management. It provides intelligent port detection, robust flashing operations, and integrated logging for professional ESP32 development.

### **Core Features**
- **Intelligent Port Detection**: Automatic ESP32 device identification across platforms
- **Operation-First Syntax**: Intuitive command structure for better usability
- **Comprehensive Logging**: Built-in logging system with automatic rotation
- **Cross-Platform Compatibility**: Consistent behavior on Linux and macOS
- **Error Prevention**: Validation and error handling with clear troubleshooting guidance

### **Key Capabilities**
- Automatic ESP32 device detection and port identification
- Firmware flashing with validation and error checking
- Real-time device monitoring and debugging
- Integrated logging and log management
- Port connectivity testing and troubleshooting
- Cross-platform serial port management

## 🏗️ **Architecture and Design**

### **System Architecture**
```
User Commands → flash_app.sh → Port Detection → ESP-IDF Tools → Device Communication
     ↓              ↓              ↓              ↓              ↓
Operation      Parameter      Device ID      Flash/Monitor   Serial I/O
Specification  Validation     & Port         Commands        & Control
```

### **Component Interaction**
- **`flash_app.sh`**: Main flash orchestration script
- **`detect_ports.sh`**: Port detection and device identification
- **`manage_logs.sh`**: Logging system integration
- **ESP-IDF Tools**: esptool, idf.py for device communication
- **Serial Port Management**: Cross-platform port access and control

### **Design Principles**
- **Operation-First**: Commands start with the operation for clarity
- **Intelligent Defaults**: Sensible fallbacks when parameters are omitted
- **Fail-Safe Operations**: Validation before execution to prevent errors
- **Cross-Platform Consistency**: Uniform behavior across operating systems
- **User Experience**: Clear error messages and troubleshooting guidance

## 🔌 **Port Detection and Management**

### **Automatic Device Detection**

#### **Cross-Platform Detection**
The flash system automatically detects ESP32 devices across different platforms:

**Linux Detection**:
```bash
# USB serial devices
/dev/ttyUSB0, /dev/ttyUSB1, /dev/ttyACM0

# ESP32-specific patterns
/dev/ttyUSB* (CP210x, CH340, FTDI)
/dev/ttyACM* (CDC ACM devices)
```

**macOS Detection**:
```bash
# USB serial devices
/dev/cu.usbserial-*, /dev/cu.SLAB_USBtoUART*

# ESP32-specific patterns
/dev/cu.usbmodem*, /dev/cu.usbserial*
```



#### **Device Identification Patterns**
The system recognizes common ESP32 development board USB identifiers:

```bash
# Common ESP32 USB identifiers
CP210x: Silicon Labs CP210x USB to UART Bridge
CH340: WCH CH340 USB to Serial
FTDI: FTDI FT232R USB UART
CDC ACM: USB CDC ACM devices
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
```

### **Port Selection Logic**

#### **Automatic Port Selection**
When multiple ports are available, the system uses intelligent selection:

```bash
# Priority order for port selection
1. ESP32-specific USB identifiers (CP210x, CH340, FTDI)
2. Previously used ports (from configuration)
3. First available USB serial port
4. Fallback to manual selection
```

#### **Manual Port Override**
```bash
# Override automatic port detection
export ESPPORT="/dev/ttyUSB0"
./flash_app.sh flash gpio_test Release

# Specify port in command
./flash_app.sh flash gpio_test Release --port /dev/ttyUSB1
```

## ⚡ **Flash Operations and Workflows**

### **Operation Types**

#### **1. Flash Operations**
- **`flash`**: Flash firmware only (no monitoring)
- **`flash_monitor`**: Flash firmware and start monitoring (default)
- **`monitor`**: Monitor existing firmware (no flashing)
- **`size`**: Show firmware size information and memory usage analysis
- **`list`**: List available applications and configurations

#### **2. Operation Syntax**
The system supports both operation-first and legacy syntax:

```bash
# Operation-first syntax (RECOMMENDED)
./flash_app.sh flash gpio_test Release
./flash_app.sh monitor --log
./flash_app.sh flash_monitor adc_test Debug
./flash_app.sh size gpio_test Release
./flash_app.sh size gpio_test Release release/v5.5

# Legacy syntax (still supported)
./flash_app.sh gpio_test Release flash
./flash_app.sh gpio_test Release flash_monitor
./flash_app.sh gpio_test Release size
```

### **Flash Process Workflow**

#### **1. Pre-Flash Validation**
```bash
# Validate configuration
- Check app exists in configuration
- Verify build type support
- Validate ESP-IDF version compatibility
- Confirm target device compatibility
```

#### **2. Port Detection and Selection**
```bash
# Automatic port detection
- Scan for available ESP32 devices
- Identify compatible ports
- Test port connectivity
- Select optimal port for operation
```

#### **3. Flash Execution**
```bash
# ESP-IDF flash process
- Set target MCU (esp32c6)
- Configure flash parameters
- Execute flash operation
- Validate flash completion
```

#### **4. Post-Flash Operations**
```bash
# Post-flash actions
- Verify firmware integrity
- Start monitoring (if requested)
- Generate operation logs
- Update port configuration
```

### **Flash Configuration Options**

#### **Build Type Integration**
```bash
# Flash with specific build type
./flash_app.sh flash gpio_test Release
./flash_app.sh flash gpio_test Debug

# Automatic build type validation
- Ensures build type is supported by app
- Validates against app_config.yml configuration
- Provides clear error messages for incompatibilities
```

#### **ESP-IDF Version Support**
```bash
# Flash with specific ESP-IDF version
./flash_app.sh flash gpio_test Release release/v5.5
./flash_app.sh flash gpio_test Release release/v5.4

# Version compatibility validation
- Checks app support for specified version
- Validates against app configuration
- Ensures consistent toolchain usage
```

#### **Size Analysis Operations**
The size operation provides comprehensive firmware analysis without requiring device connection:

```bash
# Basic size analysis
./flash_app.sh size gpio_test Release

# Size analysis with specific ESP-IDF version
./flash_app.sh size gpio_test Release release/v5.5

# Size analysis with app-first syntax
./flash_app.sh gpio_test Release size
```

**Size Operation Features**:
- **Firmware Size Analysis**: Total image size and memory usage breakdown
- **Component Size Breakdown**: Per-archive contributions to ELF file
- **Memory Usage Summary**: Flash, DIRAM, and LP SRAM usage analysis
- **Build Validation**: Ensures build exists before analysis
- **No Port Required**: Works without device connection
- **Smart Build Detection**: Automatically finds correct build directory

## 📺 **Monitoring and Logging**

### **Integrated Logging System**

#### **Log Generation**
The flash system automatically generates comprehensive logs:

```bash
# Automatic log creation
./flash_app.sh flash_monitor gpio_test Release --log

# Log file naming convention
gpio_test_Release_20250115_143022.log
# Format: {app}_{build_type}_{date}_{time}.log
```

#### **Log Content and Structure**
```bash
# Log file contents
- Command execution details
- Port detection results
- Flash operation output
- Monitor session data
- Error messages and warnings
- Performance metrics
```

### **Monitoring Capabilities**

#### **Real-Time Monitoring**
```bash
# Start monitoring after flash
./flash_app.sh flash_monitor gpio_test Release --log

# Monitor existing firmware
./flash_app.sh monitor --log

# Monitor with custom log name
./flash_app.sh monitor --log debug_session
```

#### **Monitor Configuration**
```bash
# Monitor options
- Baud rate: 115200 (configurable)
- Data bits: 8
- Parity: None
- Stop bits: 1
- Flow control: None
```

### **Log Management Integration**

#### **Automatic Log Rotation**
```bash
# Log rotation features
- Timestamped log files
- Automatic log directory management
- Configurable retention policies
- Storage optimization
```

#### **Log Analysis Tools**
```bash
# Integrated log management
./manage_logs.sh list          # List all logs
./manage_logs.sh latest        # Show latest log
./manage_logs.sh search "ERROR" # Search for errors
./manage_logs.sh stats         # Log statistics
```

## 🚀 **Usage Examples and Patterns**

### **Basic Flash Workflows**

#### **1. Development Flash Workflow**
```bash
# Build and flash for development
./build_app.sh gpio_test Debug
./flash_app.sh flash_monitor gpio_test Debug --log

# Expected result:
# - Firmware flashed to device
# - Monitoring started automatically
# - Debug output captured in logs
# - Real-time debugging available
```

#### **2. Production Flash Workflow**
```bash
# Build and flash for production
./build_app.sh gpio_test Release
./flash_app.sh flash gpio_test Release --log production_deploy

# Expected result:
# - Optimized firmware flashed
# - No monitoring (production mode)
# - Deployment log generated
# - Ready for production use
```

#### **3. Debugging Workflow**
```bash
# Monitor existing firmware
./flash_app.sh monitor --log debug_session

# Search for issues
./manage_logs.sh search "ERROR"
./manage_logs.sh search "WARNING"

# Analyze log patterns
./manage_logs.sh stats
```

#### **4. Size Analysis Workflow**
```bash
# Analyze firmware size before deployment
./flash_app.sh size gpio_test Release

# Check memory usage for optimization
./flash_app.sh size gpio_test Debug

# Compare sizes between versions
./flash_app.sh size gpio_test Release release/v5.5
./flash_app.sh size gpio_test Release release/v5.4

# Expected result:
# - Comprehensive size analysis
# - Component breakdown
# - Memory usage summary
# - No device connection required
```

### **Advanced Flash Patterns**

#### **1. Multi-Device Deployment**
```bash
# Flash to multiple devices
for port in /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2; do
    export ESPPORT="$port"
    ./flash_app.sh flash gpio_test Release --log "deploy_${port}"
done
```

#### **2. Conditional Flash Operations**
```bash
# Flash only if build is newer
if [ "build_gpio_test_Release/gpio_test.bin" -nt "last_flash" ]; then
    ./flash_app.sh flash gpio_test Release --log
    touch last_flash
fi
```

#### **3. Automated Testing Flash**
```bash
# Flash for automated testing
./flash_app.sh flash gpio_test Release --log "test_$(date +%Y%m%d_%H%M%S)"

# Run automated tests
# Monitor test results
# Collect test logs
```

### **Integration with Build System**

#### **Build-Flash Integration**
```bash
# Combined build and flash
./build_app.sh gpio_test Release && \
./flash_app.sh flash_monitor gpio_test Release --log

# Automatic build verification
- Ensures build exists before flashing
- Validates build type compatibility
- Checks firmware integrity
```

#### **CI/CD Integration**
```yaml
# GitHub Actions flash workflow
- name: Flash ESP32 Application
  run: |
    cd examples/esp32
    ./scripts/flash_app.sh flash gpio_test Release --log ci_deploy

- name: Verify Flash
  run: |
    ./scripts/flash_app.sh monitor --log ci_verify
    timeout 30s ./scripts/flash_app.sh monitor --log ci_verify
```

## 🔍 **Troubleshooting and Debugging**

### **Common Flash Issues**

#### **1. Port Detection Issues**
**Problem**: No ESP32 devices detected
**Symptoms**: "No ports detected" or "Port not accessible" errors
**Solutions**:
```bash
# Check device connections
./detect_ports.sh --verbose

# Test port connectivity
./detect_ports.sh --test-connection

# Verify USB drivers
lsusb | grep -i esp
system_profiler SPUSBDataType | grep -i esp
```

#### **2. Permission Issues**
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

#### **3. Flash Failures**
**Problem**: Firmware flash fails
**Symptoms**: "Flash failed" or "Upload failed" errors
**Solutions**:
```bash
# Check device connection
./detect_ports.sh --test-connection

# Verify firmware file
ls -la build_gpio_test_Release/*.bin

# Check device mode
./flash_app.sh monitor --log flash_debug

# Reset device and retry
# Hold BOOT button during flash
```

#### **4. Monitor Issues**
**Problem**: Cannot monitor device
**Symptoms**: "Monitor failed" or "No output" errors
**Solutions**:
```bash
# Check baud rate compatibility
./flash_app.sh monitor --log

# Verify device is running
./flash_app.sh monitor --log debug_monitor

# Check for bootloader mode
# Press RESET button to restart
```

### **Debug and Verbose Mode**

#### **Enabling Debug Output**
```bash
# Enable debug mode
export DEBUG=1
./flash_app.sh flash_monitor gpio_test Release --log

# Enable verbose ESP-IDF output
export IDF_VERBOSE=1
./flash_app.sh flash gpio_test Release --log
```

#### **Debug Information Available**
- Port detection and selection details
- Flash operation progress and status
- Monitor session configuration
- Error context and troubleshooting suggestions
- Performance metrics and timing

### **Log Analysis for Troubleshooting**

#### **Flash Log Analysis**
```bash
# Check flash operation logs
./manage_logs.sh latest
./manage_logs.sh search "ERROR"
./manage_logs.sh search "FAILED"

# Analyze flash patterns
./manage_logs.sh search "Flash completed"
./manage_logs.sh search "Upload failed"
```

#### **Monitor Log Analysis**
```bash
# Check monitor session logs
./manage_logs.sh search "monitor"
./manage_logs.sh search "serial"

# Analyze device output
./manage_logs.sh search "ESP32"
./manage_logs.sh search "boot"
```

## 📚 **Reference and Examples**

### **Command Reference**

#### **Flash Script Parameters**
```bash
./flash_app.sh [operation] [app_type] [build_type] [idf_version] [options]

# Parameters:
#   operation    - What to do (flash, monitor, flash_monitor, list)
#   app_type     - Application to flash (from app_config.yml)
#   build_type   - Build type to flash (Debug, Release)
#   idf_version  - ESP-IDF version used for build
#   options      - Flash options (--log, --port, etc.)
```

#### **Flash Options**
- **`--log [name]`**: Enable logging with optional custom name
- **`--port <port>`**: Override automatic port detection
- **`--baud <rate>`**: Set custom baud rate for monitoring
- **`--help`**: Show usage information
- **`list`**: List available applications and configurations

#### **Environment Variables**
```bash
# Override default application
export CONFIG_DEFAULT_APP="gpio_test"

# Override default build type
export CONFIG_DEFAULT_BUILD_TYPE="Debug"

# Override default ESP-IDF version
export CONFIG_DEFAULT_IDF_VERSION="release/v5.4"

# Override port detection
export ESPPORT="/dev/ttyUSB0"

# Enable debug mode
export DEBUG=1
```

### **Configuration Examples**

#### **Minimal Flash Configuration**
```yaml
# app_config.yml minimal configuration
metadata:
  default_app: "gpio_test"
  default_build_type: "Release"
  target: "esp32c6"

apps:
  gpio_test:
    source_file: "GpioComprehensiveTest.cpp"
    build_types: ["Debug", "Release"]
```

#### **Advanced Flash Configuration**
```yaml
# app_config.yml advanced configuration
metadata:
  default_app: "gpio_test"
  default_build_type: "Release"
  target: "esp32c6"
  idf_versions: ["release/v5.5", "release/v5.4"]

apps:
  gpio_test:
    description: "GPIO testing application"
    source_file: "GpioComprehensiveTest.cpp"
    category: "peripheral"
    build_types: ["Debug", "Release"]
    idf_versions: ["release/v5.5"]
    ci_enabled: true
    featured: true
```

### **Integration Examples**

#### **CMake Integration**
```cmake
# CMakeLists.txt flash integration
cmake_minimum_required(VERSION 3.16)

# Flash target integration
add_custom_target(flash
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/flash_app.sh flash ${APP_TYPE} ${BUILD_TYPE}
    DEPENDS ${PROJECT_NAME}
    COMMENT "Flashing ${APP_TYPE} ${BUILD_TYPE} to device"
)
```

#### **CI/CD Integration**
```yaml
# GitHub Actions flash workflow
- name: Flash ESP32 Application
  run: |
    cd examples/esp32
    ./scripts/flash_app.sh flash gpio_test Release --log ci_deploy

- name: Verify Flash
  run: |
    ./scripts/flash_app.sh monitor --log ci_verify
    timeout 30s ./scripts/flash_app.sh monitor --log ci_verify
```

### **Best Practices**

#### **1. Flash Operations**
- Always use logging for debugging and troubleshooting
- Verify device connections before flashing
- Use appropriate build types for different purposes
- Test flash operations in development before production

#### **2. Port Management**
- Use automatic port detection when possible
- Verify port permissions and access rights
- Test port connectivity before operations
- Handle multiple device scenarios gracefully

#### **3. Error Handling**
- Always check flash operation results
- Use debug mode for troubleshooting
- Analyze logs for error patterns
- Implement proper error handling in automation

#### **4. Performance Optimization**
- Use appropriate flash parameters
- Optimize monitor settings for your use case
- Regular log maintenance and cleanup
- Monitor flash performance and reliability

---

**Navigation**: [← Previous: Build System](README_BUILD_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Configuration System →](README_CONFIG_SYSTEM.md)
