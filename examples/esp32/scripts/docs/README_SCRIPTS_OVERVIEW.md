# 🚀 **ESP32 HardFOC Interface Wrapper - Scripts Overview**

<div align="center">

![Scripts Overview](https://img.shields.io/badge/Scripts-Overview-blue?style=for-the-badge&logo=espressif)
![Comprehensive](https://img.shields.io/badge/Comprehensive-100%25-green?style=for-the-badge&logo=check)
![Well-Documented](https://img.shields.io/badge/Well--Documented-Professional-blue?style=for-the-badge&logo=book)

**🎯 Complete Overview of All Available Scripts and Their Integration**

*Professional automation suite providing consistent, reliable, and efficient ESP32 development workflows*

</div>

---

## 📚 **Table of Contents**

- [🌟 **Overview**](#️-overview)
- [🏗️ **Directory Structure**](#️-directory-structure)
- [📱 **Script Categories**](#️-script-categories)
- [🔗 **Quick Reference**](#️-quick-reference)
- [🚀 **Essential Commands**](#️-essential-commands)
- [🔄 **Common Workflows**](#️-common-workflows)
- [🔧 **Integration Points**](#️-integration-points)
- [📊 **Script Dependencies**](#️-script-dependencies)
- [🎯 **Best Practices**](#️-best-practices)
- [🔍 **Getting Help**](#️-getting-help)

---

## 🌟 **Overview**

The **Scripts Directory** provides a **comprehensive automation suite** for ESP32 development projects. All scripts are designed to work together seamlessly, providing a **consistent and reliable development workflow** that can be easily adopted across different projects and teams.

### ✨ **Key Features**

- **🔗 Seamless Integration** - All scripts work together as a unified system
- **⚙️ Configuration-Driven** - Behavior controlled entirely via `app_config.yml`
- **🔒 Zero Dependencies** - No external dependencies beyond ESP-IDF
- **🌐 Cross-Platform** - Linux, macOS, and Windows (WSL) support
- **📚 Self-Documenting** - Comprehensive help and examples
- **🔄 CI/CD Ready** - Designed for automated workflows

---

## 🏗️ **Directory Structure**

```
examples/esp32/scripts/
├── 📁 docs/                           # 📚 Comprehensive documentation
│   ├── 📋 README_SCRIPTS_OVERVIEW.md  # 🎯 This file - Complete system overview
│   ├── 🔨 README_BUILD_SYSTEM.md      # 🏗️ Build system documentation
│   ├── 🔍 README_PORT_DETECTION.md    # 🔌 Port detection and troubleshooting
│   ├── 📱 README_FLASH_SYSTEM.md      # ⚡ Flash and monitor system
│   ├── 📊 README_LOGGING_SYSTEM.md    # 📈 Logging and log management
│   ├── ⚙️ README_CONFIG_SYSTEM.md     # ⚙️ Configuration management
│   └── 🔧 README_UTILITY_SCRIPTS.md  # 🛠️ Utility and helper scripts
├── ⚡ flash_app.sh                    # 🚀 Main flash and monitor script
├── 🔨 build_app.sh                    # 🏗️ Build system script
├── 📊 manage_logs.sh                  # 📈 Log management and analysis
├── ⚙️ config_loader.sh                # ⚙️ Configuration loading and validation
├── 🔍 detect_ports.sh                 # 🔌 Port detection and troubleshooting
├── 🛠️ setup_common.sh                 # 🔧 Common setup and environment
├── 🚀 setup_ci.sh                     # 🔄 CI/CD environment setup
├── 📂 setup_repo.sh                   # 🏗️ Repository setup and initialization
└── 🐍 get_app_info.py                 # 🐍 Python script for app information
```

---

## 📱 **Script Categories**

### 🔨 **1. Core Development Scripts**
These scripts form the backbone of the development workflow:

- **`flash_app.sh`** - 🚀 **Main development workflow script**
  - Flash firmware to ESP32 devices
  - Monitor serial output with logging
  - Port detection and validation
  - Error handling and recovery

- **`build_app.sh`** - 🏗️ **Build system management**
  - Configuration-driven building
  - Multiple build types (Debug/Release)
  - Clean build options
  - Build optimization and caching

- **`manage_logs.sh`** - 📊 **Log file management and analysis**
  - Log rotation and cleanup
  - Search and filtering
  - Performance analysis
  - Error pattern detection

### ⚙️ **2. Configuration and Setup Scripts**
These scripts handle environment and configuration:

- **`config_loader.sh`** - ⚙️ **Configuration management**
  - YAML configuration parsing
  - Environment variable setup
  - Configuration validation
  - Default value handling

- **`setup_common.sh`** - 🔧 **Environment setup**
  - Common setup functions
  - ESP-IDF environment detection
  - Platform-specific configurations
  - Shared utilities

- **`setup_ci.sh`** - 🔄 **CI/CD setup**
  - Automated environment setup
  - Build environment preparation
  - Test environment configuration
  - CI/CD workflow integration

- **`setup_repo.sh`** - 🏗️ **Repository initialization**
  - Local development environment
  - ESP-IDF setup and configuration
  - Development tools installation
  - Environment validation

### 🛠️ **3. Utility and Helper Scripts**
These scripts provide specialized functionality:

- **`detect_ports.sh`** - 🔌 **Port detection and troubleshooting**
  - ESP32 device detection
  - Port validation and testing
  - Connection troubleshooting
  - Device information display

- **`get_app_info.py`** - 🐍 **App information extraction**
  - CMake integration helper
  - App metadata extraction
  - Build information parsing
  - Configuration validation

---

## 🔗 **Quick Reference**

### 📱 **Current Command Syntax (Recommended)**

The scripts use an **operation-first** approach for better usability:

```bash
# Operation-first format (RECOMMENDED)
./scripts/flash_app.sh <operation> [app_type] [build_type] [--log [log_name]]

# Examples:
./scripts/flash_app.sh flash gpio_test Release --log
./scripts/flash_app.sh flash_monitor gpio_test Release --log
./scripts/flash_app.sh monitor --log

# Legacy format (still supported)
./scripts/flash_app.sh [app_type] [build_type] [operation] [--log [log_name]]
```

### 🔄 **Operation Types**

- **`flash`** - ⚡ Flash firmware only
- **`monitor`** - 📺 Monitor serial output only
- **`flash_monitor`** - 🚀 Flash and then monitor (most common)
- **`list`** - 📋 List available apps and configurations
- **`help`** - ❓ Show help information

---

## 🚀 **Essential Commands for Development**

### 🏗️ **Building Applications**

```bash
# Build an app with default settings
./scripts/build_app.sh <app_type> <build_type>

# Examples:
./scripts/build_app.sh gpio_test Release
./scripts/build_app.sh ascii_art Debug
./scripts/build_app.sh adc_test Release --clean

# List available apps
./scripts/build_app.sh list

# Get build help
./scripts/build_app.sh --help
```

### ⚡ **Flashing and Monitoring**

```bash
# Flash and monitor with logging (recommended workflow)
./scripts/flash_app.sh flash_monitor <app_type> <build_type> --log

# Examples:
./scripts/flash_app.sh flash_monitor gpio_test Release --log
./scripts/flash_app.sh flash_monitor ascii_art Debug --log

# Monitor existing firmware (standalone command)
./scripts/flash_app.sh monitor --log

# Flash only (for batch operations)
./scripts/flash_app.sh flash gpio_test Release
```

### 📊 **Log Management**

```bash
# View latest log
./scripts/manage_logs.sh latest

# Search logs for specific content
./scripts/manage_logs.sh search "ERROR"
./scripts/manage_logs.sh search "WARNING"
./scripts/manage_logs.sh search "SUCCESS"

# List all available logs
./scripts/manage_logs.sh list

# Clean old logs
./scripts/manage_logs.sh cleanup
```

### 🔍 **Port Detection and Troubleshooting**

```bash
# Detect available ports
./scripts/detect_ports.sh --verbose

# Test specific port
./scripts/detect_ports.sh --port /dev/ttyUSB0

# Get detailed port information
./scripts/detect_ports.sh --info
```

### ❓ **Getting Help**

```bash
# Help for any script
./scripts/flash_app.sh --help
./scripts/build_app.sh --help
./scripts/manage_logs.sh --help
./scripts/detect_ports.sh --help
```

---

## 🔄 **Common Workflows**

### 🚀 **1. Development Workflow**

```bash
# 1. Build the app
./scripts/build_app.sh gpio_test Release

# 2. Flash and monitor with logging
./scripts/flash_app.sh flash_monitor gpio_test Release --log

# 3. Monitor logs for issues
./scripts/manage_logs.sh latest
./scripts/manage_logs.sh search "ERROR"
```

### 🔄 **2. Debugging Workflow**

```bash
# 1. Build debug version
./scripts/build_app.sh gpio_test Debug

# 2. Flash and monitor with detailed logging
./scripts/flash_app.sh flash_monitor gpio_test Debug --log debug_session

# 3. Analyze debug logs
./scripts/manage_logs.sh search "DEBUG"
./scripts/manage_logs.sh search "TRACE"
```

### 🧹 **3. Clean Build Workflow**

```bash
# 1. Clean previous build
./scripts/build_app.sh gpio_test Release --clean

# 2. Rebuild from scratch
./scripts/build_app.sh gpio_test Release

# 3. Flash and test
./scripts/flash_app.sh flash_monitor gpio_test Release --log
```

### 🔄 **4. CI/CD Workflow**

```bash
# 1. Setup CI environment
./scripts/setup_ci.sh

# 2. Build all apps
./scripts/build_app.sh ascii_art Release
./scripts/build_app.sh gpio_test Release
./scripts/build_app.sh adc_test Release

# 3. Run automated tests
./scripts/flash_app.sh flash ascii_art Release
# ... automated testing ...
```

---

## 🔧 **Integration Points**

### ⚙️ **Configuration Integration**

All scripts integrate through the centralized `app_config.yml`:

```yaml
# app_config.yml drives all script behavior
metadata:
  default_app: "ascii_art"
  default_build_type: "Release"
  target: "esp32c6"

apps:
  gpio_test:
    description: "GPIO testing application"
    source_file: "main/GpioComprehensiveTest.cpp"
    category: "peripheral"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true
```

### 🔄 **Environment Integration**

Scripts automatically detect and integrate with:

- **ESP-IDF Environment** - Auto-sources if available
- **Project Structure** - Detects project root automatically
- **Build Directories** - Creates and manages build artifacts
- **Log Directories** - Organizes and rotates log files

### 🏗️ **CMake Integration**

Scripts integrate with CMake through:

- **`APP_TYPE` Variable** - Passes app selection to CMake
- **Build Type Support** - Configures Debug/Release builds
- **Source File Selection** - Dynamically selects source files
- **Component Integration** - Works with ESP-IDF component system

---

## 📊 **Script Dependencies**

### 🔗 **Internal Dependencies**

```
config_loader.sh ← All scripts depend on this
    ↓
setup_common.sh ← Common functions and utilities
    ↓
Individual script files
```

### 🌐 **External Dependencies**

- **ESP-IDF** - Required for building and flashing
- **`yq`** - YAML parsing (optional, with fallback)
- **Standard Unix Tools** - `grep`, `sed`, `awk`, `find`
- **Python 3** - For `get_app_info.py` script

### 🔧 **Platform Dependencies**

- **Linux** - Full support with all features
- **macOS** - Full support with minor path adjustments
- **Windows (WSL)** - Full support through WSL environment

---

## 🎯 **Best Practices**

### 🚀 **Script Usage**

1. **Always Use Configuration** - Let `app_config.yml` drive behavior
2. **Use Operation-First Syntax** - More intuitive and consistent
3. **Enable Logging** - Use `--log` flag for debugging
4. **Check Help** - Use `--help` for script-specific options
5. **Validate Configuration** - Ensure `app_config.yml` is correct

### 🔧 **Configuration Management**

1. **Centralize App Definitions** - Keep all apps in one place
2. **Use Descriptive Names** - Clear, meaningful app and category names
3. **Enable CI Integration** - Set `ci_enabled: true` for automated builds
4. **Document Special Features** - Add notes for complex configurations
5. **Version Control** - Track configuration changes with source code

### 📊 **Logging and Monitoring**

1. **Use Descriptive Log Names** - Help identify different sessions
2. **Regular Log Cleanup** - Prevent disk space issues
3. **Search for Patterns** - Use search to find common issues
4. **Monitor Build Logs** - Track build performance and issues
5. **Archive Important Logs** - Keep logs for debugging

---

## 🔍 **Getting Help**

### 📚 **Documentation Resources**

- **📋 [Scripts Overview](README_SCRIPTS_OVERVIEW.md)** - This comprehensive overview
- **🔨 [Build System](README_BUILD_SYSTEM.md)** - Detailed build documentation
- **📱 [Flash System](README_FLASH_SYSTEM.md)** - Flash and monitor guide
- **⚙️ [Configuration System](README_CONFIG_SYSTEM.md)** - Configuration management
- **📊 [Logging System](README_LOGGING_SYSTEM.md)** - Log management guide
- **🔧 [Utility Scripts](README_UTILITY_SCRIPTS.md)** - Utility script details
- **🔍 [Port Detection](README_PORT_DETECTION.md)** - Port troubleshooting

### ❓ **Help Commands**

```bash
# Built-in help for any script
./scripts/flash_app.sh --help
./scripts/build_app.sh --help
./scripts/manage_logs.sh --help

# List available options
./scripts/flash_app.sh list
./scripts/build_app.sh list
```

### 🆘 **Troubleshooting**

- **Check Configuration** - Verify `app_config.yml` syntax
- **Validate Environment** - Ensure ESP-IDF is properly sourced
- **Review Logs** - Check script and build logs for errors
- **Test Ports** - Use `detect_ports.sh` for connection issues
- **Clean Builds** - Use `--clean` flag for build issues

---

<div align="center">

**🚀 Ready to explore the complete automation suite?**

*Navigate through the documentation to master every aspect of ESP32 development automation!*

</div>
