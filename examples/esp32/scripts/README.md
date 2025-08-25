# 🚀 ESP32 Project Scripts Directory

This directory contains a **portable, self-contained collection of scripts** for building, flashing, monitoring, and managing ESP32 projects. The scripts are designed to be **copied between projects** and work immediately without modification.

## 📋 Table of Contents

- [🎯 Purpose](#-purpose)
- [🔧 Project Integration](#-project-integration)
- [⚙️ How app_config.yml Works](#️-how-app_configyml-works)
- [💡 Usage Examples](#️-usage-examples)
- [📚 Script Categories](#️-script-categories)
- [🌐 Portability Features](#️-portability-features)
- [🚨 Troubleshooting](#️-troubleshooting)
- [📚 Documentation](#️-documentation)
- [🔄 Migration Guide](#️-migration-guide)
- [🤝 Contributing](#️-contributing)
- [📄 License and Support](#️-license-and-support)

## 🎯 Purpose

The scripts directory provides:

- **Consistent ESP32 development workflow** across projects
- **Configuration-driven build system** using `app_config.yml`
- **Structured logging and monitoring** capabilities
- **Port detection and troubleshooting** tools
- **CI/CD integration** support
- **Cross-platform compatibility** (Linux, macOS)

## 🔧 Project Integration

### **📁 Where to Place This Directory**
```
your_esp32_project/
├── scripts/                   # Copy this entire directory here
├── app_config.yml             # Create this configuration file
├── main/                      # Your source code directory
├── components/                # ESP32 components
├── CMakeLists.txt             # Project CMake file
└── sdkconfig                  # ESP32 SDK configuration
```

### **⚙️ Required Setup Steps**

#### 1. **📋 Copy Scripts Directory**
```bash
# Copy the entire scripts directory to your project root
cp -r /path/to/scripts /your/esp32/project/

# Verify structure
ls -la /your/esp32/project/scripts/
```

#### 2. **📝 Create app_config.yml**
Create this file in your **project root** (same level as `scripts/`):

```yaml
# app_config.yml - REQUIRED configuration file
metadata:
  default_app: "ascii_art"     # Default app to build
  default_build_type: "Release"       # Default build configuration
  target: "esp32c6"                   # Target MCU (esp32, esp32c3, esp32s3, etc.)
  idf_versions: ["release/v5.5"]      # Supported ESP-IDF versions

apps:
  ascii_art:
    description: "Main application app"
    source_file: "main/MainExample.cpp"  # Path relative to project root
    category: "utility"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true

  gpio_test:
    description: "GPIO interface app"
    source_file: "main/GpioExample.cpp"
    category: "peripheral"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: true

  adc_test:
    description: "Sensor interface app"
    source_file: "main/SensorExample.cpp"
    category: "sensor"
    build_types: ["Debug", "Release"]
    ci_enabled: true
    featured: false

build_config:
  build_types:
    Debug:
      description: "Debug build with symbols"
      cmake_build_type: "Debug"
      optimization: "-O0"
      debug_level: "-g3"
      defines: ["DEBUG"]
    Release:
      description: "Release build optimized"
      cmake_build_type: "Release"
      optimization: "-O2"
      debug_level: "-g"
      defines: ["NDEBUG"]
  
  build_directory_pattern: "build_{app_type}_{build_type}"
  project_name_pattern: "esp32_project_{app_type}_app"
```

#### 3. **🔨 Update CMakeLists.txt**
Your main `CMakeLists.txt` must support the `APP_TYPE` variable:

```cmake
# CMakeLists.txt - Add this to your project root
cmake_minimum_required(VERSION 3.16)

# Set app type from command line or default
if(DEFINED APP_TYPE)
    set(APP_SOURCE_FILE "${APP_TYPE}")
else()
    set(APP_SOURCE_FILE "ascii_art")  # Default from config
endif()

# Include the app source file
idf_component_register(
    SRCS "main/${APP_SOURCE_FILE}.cpp"
    INCLUDE_DIRS "main"
    REQUIRES "your_components"
)
```

#### 4. **💻 Create Source Files**
Create the source files referenced in your `app_config.yml`:

```cpp
// main/MainExample.cpp
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
    printf("Main App Running\n");
    // Your main application code here
}
```

```cpp
// main/GpioExample.cpp
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
    printf("GPIO App Running\n");
    // Your GPIO testing code here
}
```

## ⚙️ How app_config.yml Works

### **🔄 Configuration Loading Process**
1. **Scripts automatically detect** `app_config.yml` in project root
2. **Configuration is parsed** using `config_loader.sh`
3. **Environment variables are set** for all scripts to use
4. **Validation occurs** before any build operations

### **🔑 Key Configuration Sections**

#### **📊 Metadata Section**
```yaml
metadata:
  default_app: "ascii_art"     # What gets built by default
  default_build_type: "Release"       # Default build configuration
  target: "esp32c6"                   # ESP32 target variant
  idf_versions: ["release/v5.5"]      # ESP-IDF version compatibility
```

#### **📱 Apps Section**
```yaml
apps:
  app_name:
    description: "Human-readable description"
    source_file: "main/AppFile.cpp"  # Path to source file
    category: "peripheral|utility|sensor|connectivity|storage"
    build_types: ["Debug", "Release"]    # Supported build types
    ci_enabled: true                     # Include in CI builds
    featured: true                       # Show in featured list
```

#### **🔨 Build Configuration Section**
```yaml
build_config:
  build_types:
    Debug:
      cmake_build_type: "Debug"
      optimization: "-O0"
      debug_level: "-g3"
      defines: ["DEBUG"]
    Release:
      cmake_build_type: "Release"
      optimization: "-O2"
      debug_level: "-g"
      defines: ["NDEBUG"]
  
  build_directory_pattern: "build_{app_type}_{build_type}"
  project_name_pattern: "esp32_project_{app_type}_app"
```

### **📁 Automatic Build Directory Creation**
The build system automatically creates directories based on patterns:

```bash
# For gpio_test Release build:
build_gpio_test_Release/

# For adc_test Debug build:
build_adc_test_Debug/

# For ascii_art Release build:
build_ascii_art_Release/
```

### **🏷️ Project Name Generation**
Binary files are named using the project name pattern:

```bash
# For gpio_test:
esp32_project_gpio_test_app.bin
esp32_project_gpio_test_app.elf

# For adc_test:
esp32_project_adc_test_app.bin
esp32_project_adc_test_app.elf
```

## 💡 Usage Examples

### **🚀 Basic Commands**
```bash
# Navigate to your project root
cd /your/esp32/project

# Build with defaults (from app_config.yml)
./scripts/build_app.sh

# Build specific app
./scripts/build_app.sh gpio_test Release

# List all available apps
./scripts/build_app.sh list

# Get help
./scripts/build_app.sh --help
```

### **⚙️ Build Control**
```bash
# Clean build (remove existing build directory)
./scripts/build_app.sh gpio_test Release --clean

# Disable ccache
./scripts/build_app.sh gpio_test Release --no-cache

# Environment variable overrides
export CLEAN=1
export USE_CCACHE=0
./scripts/build_app.sh gpio_test Debug
```

### **📱 Flash and Monitor**
```bash
# Flash and monitor
./scripts/flash_app.sh flash_monitor gpio_test Release --log

# Flash only
./scripts/flash_app.sh flash gpio_test Release

# Monitor only (existing firmware)
./scripts/flash_app.sh monitor --log
```

### **Logging and Management**
```bash
# View latest log
./scripts/manage_logs.sh latest

# Search logs for errors
./scripts/manage_logs.sh search "ERROR"

# List all logs
./scripts/manage_logs.sh list
```

## 📚 Script Categories

### **🔨 Core Build System**
- **`build_app.sh`** - Configuration-driven ESP32 building
- **`config_loader.sh`** - YAML configuration parser and loader

### **📱 Flash and Monitor System**
- **`flash_app.sh`** - Flash firmware with monitoring and logging
- **`detect_ports.sh`** - ESP32 port detection and troubleshooting

### **📋 Logging and Management**
- **`manage_logs.sh`** - Log rotation, search, and management
- **`get_app_info.py`** - CMake integration helper

### **🔧 Setup and Environment**
- **`setup_repo.sh`** - Local development environment setup
- **`setup_ci.sh`** - CI/CD environment setup
- **`setup_common.sh`** - Shared setup functions
- **`setup_build_directory.sh`** - Complete build directory setup and building
- **`prepare_build_directory.sh`** - Prepare build directory structure only (no building)

## 🌐 Portability Features

### **🔒 No Hardcoded Dependencies**
- **Relative paths only** - everything works from project root
- **Configuration-driven** - apps defined in YAML, not scripts
- **ESP-IDF agnostic** - works with any ESP-IDF version 5.5+
- **Cross-platform** - Linux, macOS, Windows (WSL)

### **🔍 Automatic Detection**
- **Project root detection** - scripts find project root automatically
- **Configuration validation** - checks for required files
- **ESP-IDF environment** - auto-sources ESP-IDF if needed
- **Port detection** - finds ESP32 devices automatically

## 🚨 Troubleshooting

### **⚠️ Common Issues**

#### **📝 Configuration Not Found**
```bash
# Error: Configuration file not found
ERROR: app_config.yml not found

# Solution: Create app_config.yml in project root
touch app_config.yml
# Then add configuration as shown above
```

#### **💻 Source Files Not Found**
```bash
# Error: Source file not found
ERROR: main/GpioExample.cpp not found

# Solution: Create the source file or fix path in app_config.yml
mkdir -p main
touch main/GpioExample.cpp
```

#### **🔧 ESP-IDF Not Found**
```bash
# Error: ESP-IDF environment not found
ERROR: ESP-IDF export.sh not found

# Solution: Source ESP-IDF or use setup script
source $HOME/esp/esp-idf/export.sh
# Or
./scripts/setup_repo.sh
```

### **💡 Getting Help**
```bash
# Script help
./scripts/build_app.sh --help
./scripts/flash_app.sh --help

# List apps
./scripts/build_app.sh list

# Check configuration
./scripts/config_loader.sh --help
```

## 📚 Documentation

For detailed information about each script and system:

- **📋 Scripts Overview**: [docs/README_SCRIPTS_OVERVIEW.md](docs/README_SCRIPTS_OVERVIEW.md)
- **🔨 Build System**: [docs/README_BUILD_SYSTEM.md](docs/README_BUILD_SYSTEM.md)
- **📱 Flash System**: [docs/README_FLASH_SYSTEM.md](docs/README_FLASH_SYSTEM.md)
- **⚙️ Configuration System**: [docs/README_CONFIG_SYSTEM.md](docs/README_CONFIG_SYSTEM.md)
- **📋 Logging System**: [docs/README_LOGGING_SYSTEM.md](docs/README_LOGGING_SYSTEM.md)
- **🔧 Utility Scripts**: [docs/README_UTILITY_SCRIPTS.md](docs/README_UTILITY_SCRIPTS.md)
- **🔍 Port Detection**: [docs/README_PORT_DETECTION.md](docs/README_PORT_DETECTION.md)

## 🔄 Migration Guide

### **🔄 From Manual ESP-IDF Commands**
```bash
# Instead of:
idf.py build -DAPP_TYPE=gpio -DBUILD_TYPE=Release

# Use:
./scripts/build_app.sh gpio_test Release

# Instead of:
idf.py flash monitor

# Use:
./scripts/flash_app.sh flash_monitor gpio_test Release
```

### **🔧 From Other Build Systems**
1. **Copy scripts directory** to your project root
2. **Create app_config.yml** based on your apps
3. **Update CMakeLists.txt** to use `APP_TYPE` variable
4. **Test with a simple app** first
5. **Gradually migrate** other apps

## 🤝 Contributing

### **➕ Adding New Apps**
1. **Add to app_config.yml**:
```yaml
new_app:
  description: "New app description"
  source_file: "main/NewApp.cpp"
  category: "peripheral"
  build_types: ["Debug", "Release"]
  ci_enabled: true
  featured: false
```

2. **Create source file** in `main/NewApp.cpp`
3. **Test the build**: `./scripts/build_app.sh new_app Release`

### **🔧 Script Improvements**
- **Keep scripts portable** - no hardcoded project paths
- **Use configuration functions** - leverage `config_loader.sh`
- **Add help support** - every script should have `--help`
- **Test across platforms** - ensure Linux/macOS/Windows compatibility

## 📄 License and Support

This scripts directory is designed to be **freely copied and adapted** for any ESP32 project. The scripts provide a **consistent, practical approach** to ESP32 development that can be shared across teams and projects.

For questions or improvements, refer to the documentation in the `docs/` subdirectory or create issues in the original repository.
