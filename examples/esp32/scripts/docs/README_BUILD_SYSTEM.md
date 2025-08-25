# 🔨 **ESP32 HardFOC Interface Wrapper - Build System Guide**

<div align="center">

![Build System](https://img.shields.io/badge/Build-System-blue?style=for-the-badge&logo=espressif)
![Configuration-Driven](https://img.shields.io/badge/Configuration--Driven-YAML-blue?style=for-the-badge&logo=yaml)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.5+-green?style=for-the-badge&logo=espressif)

**🏗️ Configuration-Driven ESP32 Build System with Advanced Optimization**

*Professional build automation that automatically extracts configuration from YAML and provides flexible, optimized building for all ESP32 applications*

</div>

---

## 📚 **Table of Contents**

- [🌟 **Overview**](#️-overview)
- [⚙️ **Configuration Structure**](#️-configuration-structure)
- [🔧 **How app_config.yml Drives the Build System**](#️-how-app_configyml-drives-the-build-system)
- [🚀 **Usage Examples**](#️-usage-examples)
- [🔍 **Troubleshooting**](#️-troubleshooting)
- [📚 **Getting Help**](#️-getting-help)
- [🔗 **Related Documentation**](#️-related-documentation)

---

## 🌟 **Overview**

The **Build System** is a **configuration-driven automation platform** that automatically extracts build information from `app_config.yml` and provides flexible build options for ESP32 applications. It's designed to be **independent of specific implementations** and focuses entirely on the build process itself.

### ✨ **Key Features**

- **⚙️ Configuration-Driven** - All build information extracted from `app_config.yml`
- **🔧 Flexible Build Types** - Debug and Release configurations with customizable parameters
- **🎯 Smart Defaults** - Automatic fallbacks when configuration is incomplete
- **⚡ Build Optimization** - ccache support and incremental build options
- **🌐 Cross-Platform** - Works on Linux, macOS, and Windows (WSL)
- **🔗 ESP-IDF Integration** - Direct integration with ESP-IDF build system
- **📊 Build Analytics** - Performance tracking and optimization insights

### 🎯 **Primary Benefits**

- **🚀 Consistent Builds** - Same process across all projects and environments
- **⚡ Fast Development** - Optimized builds with intelligent caching
- **🔧 Easy Configuration** - Simple YAML-based app definitions
- **🔄 CI/CD Ready** - Seamless integration with automated workflows
- **📈 Scalable** - From single apps to complex multi-app projects

---

## ⚙️ **Configuration Structure**

### 📝 **Configuration File: `app_config.yml`**

The build system reads from a **centralized YAML configuration file** located at:
```
examples/esp32/app_config.yml
```

#### 🔧 **Configuration Structure Overview**

```yaml
# Metadata section - global defaults
metadata:
  default_app: "ascii_art"            # 🎯 Default app to build
  default_build_type: "Release"       # ⚙️ Default build configuration
  target: "esp32c6"                   # 🎛️ Target MCU
  idf_versions: ["release/v5.5"]      # 🔧 Supported ESP-IDF versions

# Apps section - defines all available apps
apps:
  ascii_art:
    description: "ASCII art generator app"
    source_file: "AsciiArtComprehensiveTest.cpp"     # 📂 Source file to compile
    category: "utility"
    build_types: ["Debug", "Release"]  # ⚙️ Supported build types
    ci_enabled: true                   # 🔄 Include in CI builds
    featured: true                     # ⭐ Show in featured list

  gpio_test:
    description: "GPIO peripheral testing app"
    source_file: "GpioComprehensiveTest.cpp"     # 📂 Source file to compile
    category: "peripheral"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true

  adc_test:
    description: "ADC peripheral testing app"
    source_file: "AdcComprehensiveTest.cpp"   # 📂 Source file to compile
    category: "peripheral"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: false

# Build configuration section
build_config:
  build_types:
    Debug:
      description: "Debug symbols, verbose logging, assertions enabled"
      cmake_build_type: "Debug"
      optimization: "-O0"           # No optimization for debugging
      debug_level: "-g3"            # Maximum debug information
      defines: ["DEBUG"]            # Debug macros enabled
      
    Release:
      description: "Optimized for performance and size"
      cmake_build_type: "Release"
      optimization: "-O2"           # High optimization level
      debug_level: "-g"             # Minimal debug info
      defines: ["NDEBUG"]           # Debug macros disabled
```

### 🔄 **Configuration Loading Process**

The build system uses `config_loader.sh` to extract information through a **multi-stage process**:

#### 1️⃣ **Primary Method: `yq` for Reliable YAML Parsing**
```bash
# Preferred method using yq (if available)
if command -v yq &> /dev/null; then
    DEFAULT_APP=$(yq '.metadata.default_app' "$CONFIG_FILE")
    DEFAULT_BUILD_TYPE=$(yq '.metadata.default_build_type' "$CONFIG_FILE")
    TARGET=$(yq '.metadata.target' "$CONFIG_FILE")
fi
```

#### 2️⃣ **Fallback Method: Native Bash Parsing**
```bash
# Fallback to native bash parsing if yq unavailable
if [ -z "$DEFAULT_APP" ]; then
    DEFAULT_APP=$(grep "default_app:" "$CONFIG_FILE" | sed 's/.*default_app:[[:space:]]*//')
    DEFAULT_BUILD_TYPE=$(grep "default_build_type:" "$CONFIG_FILE" | sed 's/.*default_build_type:[[:space:]]*//')
    TARGET=$(grep "target:" "$CONFIG_FILE" | sed 's/.*target:[[:space:]]*//')
fi
```

#### 3️⃣ **Validation and Environment Setup**
```bash
# Validate configuration and set environment variables
if [ -n "$DEFAULT_APP" ] && [ -n "$DEFAULT_BUILD_TYPE" ]; then
    export DEFAULT_APP
    export DEFAULT_BUILD_TYPE
    export TARGET
    echo "✅ Configuration loaded successfully"
else
    echo "❌ Configuration validation failed"
    exit 1
fi
```

---

## 🔧 **How app_config.yml Drives the Build System**

### 🎯 **Automatic App Discovery**

The build system **automatically discovers** all available apps from the configuration:

```bash
# List all available apps
./scripts/build_app.sh list

# Output example:
# Available Apps:
#   ascii_art (utility) - ASCII art generator app
#   gpio_test (peripheral) - GPIO peripheral testing app
#   adc_test (peripheral) - ADC peripheral testing app
#   uart_test (peripheral) - UART peripheral testing app
#   spi_test (peripheral) - SPI peripheral testing app
#   i2c_test (peripheral) - I2C peripheral testing app
#   pwm_test (peripheral) - PWM peripheral testing app
#   can_test (peripheral) - CAN peripheral testing app
#   pio_test (peripheral) - PIO/RMT comprehensive testing
#   temperature_test (sensor) - Temperature sensor comprehensive testing
#   nvs_test (storage) - NVS (Non-Volatile Storage) comprehensive testing
#   timer_test (peripheral) - Timer peripheral comprehensive testing
#   logger_test (utility) - Logger system comprehensive testing
#   wifi_test (connectivity) - WiFi connectivity comprehensive testing
#   bluetooth_test (connectivity) - Comprehensive Bluetooth testing suite
#   utils_test (utility) - Utilities testing suite
```

### 🏗️ **Dynamic Build Directory Creation**

Build directories are **automatically created** based on configuration patterns:

```bash
# Build directory pattern: build_{app_type}_{build_type}
# Examples:
build_ascii_art_Release/     # ASCII art Release build
build_gpio_test_Debug/       # GPIO test Debug build
build_adc_test_Release/      # ADC test Release build
build_pio_test_Debug/        # PIO test Debug build
```

### 🏷️ **Automatic Project Naming**

Binary files are **automatically named** using the project name pattern:

```bash
# Project name pattern: esp32_project_{app_type}_app
# Examples:
esp32_project_ascii_art_app.bin      # ASCII art binary
esp32_project_gpio_test_app.elf      # GPIO test ELF file
esp32_project_adc_test_app.map       # ADC test map file
```

### ⚙️ **Build Type Configuration**

Each build type has **specific optimization settings**:

```yaml
build_config:
  build_types:
    Debug:
      description: "Debug symbols, verbose logging, assertions enabled"
      cmake_build_type: "Debug"
      optimization: "-O0"           # No optimization for debugging
      debug_level: "-g3"            # Maximum debug information
      defines: ["DEBUG"]            # Debug macros enabled
      
    Release:
      description: "Optimized for performance and size"
      cmake_build_type: "Release"
      optimization: "-O2"           # High optimization level
      debug_level: "-g"             # Minimal debug info
      defines: ["NDEBUG"]           # Debug macros disabled
```

---

## 🚀 **Usage Examples**

### 🎯 **Basic Build Commands**

```bash
# Build with defaults (from app_config.yml)
./scripts/build_app.sh

# Build specific app and build type
./scripts/build_app.sh gpio_test Release
./scripts/build_app.sh ascii_art Debug
./scripts/build_app.sh adc_test Release

# List available apps
./scripts/build_app.sh list

# Get help
./scripts/build_app.sh --help
```

### ⚡ **Advanced Build Options**

```bash
# Clean build (remove existing build directory)
./scripts/build_app.sh gpio_test Release --clean

# Disable ccache for this build
./scripts/build_app.sh gpio_test Release --no-cache

# Verbose output
./scripts/build_app.sh gpio_test Release --verbose

# Environment variable overrides
export CLEAN=1
export USE_CCACHE=0
./scripts/build_app.sh gpio_test Debug
```

### 🔄 **Build Workflows**

#### 🚀 **Development Workflow**
```bash
# 1. Build debug version for development
./scripts/build_app.sh gpio_test Debug

# 2. Build release version for testing
./scripts/build_app.sh gpio_test Release

# 3. Clean build for fresh start
./scripts/build_app.sh gpio_test Release --clean
```

#### 🔄 **CI/CD Workflow**
```bash
# 1. Setup CI environment
./scripts/setup_ci.sh

# 2. Build all apps in Release mode
for app in ascii_art gpio_test adc_test pio_test; do
    ./scripts/build_app.sh "$app" Release
done

# 3. Build featured apps in Debug mode
for app in ascii_art gpio_test pio_test bluetooth_test utils_test; do
    ./scripts/build_app.sh "$app" Debug
done
```

#### 🧹 **Maintenance Workflow**
```bash
# 1. Clean all build directories
find . -name "build_*" -type d -exec rm -rf {} +

# 2. Rebuild from scratch
./scripts/build_app.sh ascii_art Release --clean
./scripts/build_app.sh gpio_test Release --clean
./scripts/build_app.sh adc_test Release --clean
```

### 📊 **Build Monitoring and Analysis**

```bash
# Monitor build progress
./scripts/build_app.sh gpio_test Release --verbose

# Check build statistics
./scripts/build_app.sh gpio_test Release --stats

# Analyze build performance
./scripts/build_app.sh gpio_test Release --analyze
```

---

## 🔍 **Troubleshooting**

### ⚠️ **Common Build Issues**

#### 📝 **Configuration Not Found**
```bash
# Error: Configuration file not found
ERROR: app_config.yml not found in project root

# Solution: Create app_config.yml in project root
touch app_config.yml
# Then add configuration as shown above
```

#### 💻 **Source Files Not Found**
```bash
# Error: Source file not found
ERROR: main/GpioComprehensiveTest.cpp not found

# Solution: Create the source file or fix path in app_config.yml
mkdir -p main
touch main/GpioComprehensiveTest.cpp
# Or update source_file path in app_config.yml
```

#### 🔧 **ESP-IDF Not Found**
```bash
# Error: ESP-IDF environment not found
ERROR: ESP-IDF export.sh not found

# Solution: Source ESP-IDF or use setup script
source $HOME/esp/esp-idf/export.sh
# Or
./scripts/setup_repo.sh
```

#### 🏗️ **Build Directory Issues**
```bash
# Error: Cannot create build directory
ERROR: Permission denied creating build directory

# Solution: Check permissions and clean existing directories
chmod 755 .
rm -rf build_*
./scripts/build_app.sh gpio_test Release --clean
```

### 💡 **Debugging Build Issues**

```bash
# Enable debug mode
export DEBUG=1
./scripts/build_app.sh gpio_test Release

# Check configuration loading
./scripts/config_loader.sh --debug

# Verify ESP-IDF environment
echo "IDF_PATH: $IDF_PATH"
echo "IDF_TARGET: $IDF_TARGET"
echo "PATH: $PATH"
```

---

## 📚 **Getting Help**

### ❓ **Built-in Help**

```bash
# Script help
./scripts/build_app.sh --help
./scripts/build_app.sh -h

# Configuration help
./scripts/config_loader.sh --help

# Setup help
./scripts/setup_repo.sh --help
```

### 📋 **Command Reference**

```bash
# List available apps
./scripts/build_app.sh list

# Show current configuration
./scripts/config_loader.sh --show

# Validate configuration
./scripts/config_loader.sh --validate
```

---

## 🔗 **Related Documentation**

For comprehensive information about the build system and related components:

- **📋 [Scripts Overview](README_SCRIPTS_OVERVIEW.md)** - Complete system overview
- **📱 [Flash System](README_FLASH_SYSTEM.md)** - Flash and monitor operations
- **⚙️ [Configuration System](README_CONFIG_SYSTEM.md)** - YAML configuration management
- **📊 [Logging System](README_LOGGING_SYSTEM.md)** - Log management and analysis
- **🔧 [Utility Scripts](README_UTILITY_SCRIPTS.md)** - Helper and utility functions
- **🔍 [Port Detection](README_PORT_DETECTION.md)** - Device detection and troubleshooting

---

<div align="center">

**🚀 Ready to build your ESP32 applications with confidence?**

*The configuration-driven build system makes ESP32 development simple, reliable, and efficient!*

</div>


