# 🚀 **ESP32 HardFOC Interface Wrapper - Scripts Directory**

<div align="center">

![ESP32 Scripts](https://img.shields.io/badge/ESP32-Scripts%20Directory-blue?style=for-the-badge&logo=espressif)
![Portable](https://img.shields.io/badge/Portable-100%25-green?style=for-the-badge&logo=rocket)
![Configuration-Driven](https://img.shields.io/badge/Configuration--Driven-YAML-blue?style=for-the-badge&logo=yaml)
![Cross-Platform](https://img.shields.io/badge/Cross--Platform-Linux%20%7C%20macOS%20%7C%20Windows-green?style=for-the-badge&logo=linux)

**🎯 Professional, Portable, and Configuration-Driven ESP32 Development Scripts**

*Complete automation suite for ESP32 projects with zero hardcoded dependencies and seamless portability across projects*

</div>

---

## 📚 **Table of Contents**

- [🌟 **Overview & Purpose**](#️-overview--purpose)
- [🔧 **Project Integration**](#️-project-integration)
- [⚙️ **Configuration System**](#️-configuration-system)
- [🚀 **Quick Start Guide**](#️-quick-start-guide)
- [📱 **Script Categories**](#️-script-categories)
- [🌐 **Portability Features**](#️-portability-features)
- [🔗 **Documentation Navigation**](#️-documentation-navigation)
- [🚨 **Troubleshooting**](#️-troubleshooting)
- [🔄 **Migration Guide**](#️-migration-guide)
- [🤝 **Contributing**](#️-contributing)
- [📄 **License & Support**](#️-license--support)

---

## 🌟 **Overview & Purpose**

The **Scripts Directory** is a **100% portable, self-contained collection** of professional-grade automation tools designed specifically for ESP32 development projects. These scripts provide a **consistent, reliable, and efficient development workflow** that can be seamlessly copied between projects without any modifications.

### 🎯 **Primary Objectives**

- **🚀 Consistent Development Workflow** - Standardized build, flash, and monitoring processes
- **⚙️ Configuration-Driven Architecture** - All behavior controlled via `app_config.yml`
- **🔒 Zero Hardcoded Dependencies** - Fully portable across projects and environments
- **🌐 Cross-Platform Compatibility** - Linux, macOS, and Windows (WSL) support
- **📊 Professional Logging & Monitoring** - Comprehensive debugging and troubleshooting tools
- **🔧 CI/CD Integration Ready** - Seamless integration with automated workflows

### ✨ **Key Benefits**

- **📦 Copy & Deploy** - Copy entire directory to any ESP32 project
- **🔧 Zero Configuration** - Works immediately with minimal setup
- **📈 Scalable** - From simple prototypes to complex production systems
- **🛡️ Production Ready** - Professional-grade error handling and validation
- **📚 Fully Documented** - Comprehensive guides and examples

---

## 🔧 **Project Integration**

### 📁 **Directory Structure**

```
your_esp32_project/
├── 📁 scripts/                    # 🚀 Copy this entire directory here
│   ├── 📁 docs/                   # 📚 Comprehensive documentation
│   ├── 🔨 build_app.sh            # 🏗️ Configuration-driven building
│   ├── 📱 flash_app.sh            # ⚡ Flash and monitor system
│   ├── 📋 config_loader.sh        # ⚙️ YAML configuration parser
│   ├── 🔍 detect_ports.sh         # 🔌 Port detection and troubleshooting
│   ├── 📊 manage_logs.sh          # 📈 Log management and analysis
│   ├── 🛠️ setup_common.sh         # 🔧 Common setup functions
│   ├── 🚀 setup_ci.sh             # 🔄 CI/CD environment setup
│   ├── 📂 setup_repo.sh           # 🏗️ Repository initialization
│   └── 🐍 get_app_info.py         # 🐍 Python CMake integration helper
├── ⚙️ app_config.yml               # 📝 Create this configuration file
├── 📁 main/                       # 📂 Your source code directory
├── 📁 components/                 # 🔌 ESP32 components
├── 📄 CMakeLists.txt              # 🏗️ Project CMake file
└── 📄 sdkconfig                   # ⚙️ ESP32 SDK configuration
```

### 🚀 **Setup Process**

#### 1️⃣ **📋 Copy Scripts Directory**
```bash
# Copy the entire scripts directory to your project root
cp -r /path/to/scripts /your/esp32/project/

# Verify structure
ls -la /your/esp32/project/scripts/
```

#### 2️⃣ **📝 Create app_config.yml**
Create this file in your **project root** (same level as `scripts/`):

```yaml
# app_config.yml - REQUIRED configuration file
metadata:
  default_app: "ascii_art"          # 🎯 Default app to build
  default_build_type: "Release"     # ⚙️ Default build configuration
  target: "esp32c6"                 # 🎛️ Target MCU (esp32, esp32c3, esp32s3, etc.)
  idf_versions: ["release/v5.5"]    # 🔧 Supported ESP-IDF versions

apps:
  ascii_art:
    description: "Main application app"
    source_file: "main/MainExample.cpp"  # 📂 Path relative to project root
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

#### 3️⃣ **🔨 Update CMakeLists.txt**
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

#### 4️⃣ **💻 Create Source Files**
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

---

## ⚙️ **Configuration System**

### 🔄 **Configuration Loading Process**

1. **🔍 Automatic Detection** - Scripts automatically detect `app_config.yml` in project root
2. **📊 YAML Parsing** - Configuration parsed using `config_loader.sh`
3. **🌍 Environment Setup** - All scripts use consistent environment variables
4. **✅ Validation** - Comprehensive validation before any operations

### 🔑 **Configuration Sections**

#### 📊 **Metadata Section**
```yaml
metadata:
  default_app: "ascii_art"          # 🎯 What gets built by default
  default_build_type: "Release"     # ⚙️ Default build configuration
  target: "esp32c6"                 # 🎛️ ESP32 target variant
  idf_versions: ["release/v5.5"]    # 🔧 ESP-IDF version compatibility
```

#### 📱 **Apps Section**
```yaml
apps:
  app_name:
    description: "Human-readable description"
    source_file: "main/AppFile.cpp"  # 📂 Path to source file
    category: "peripheral|utility|sensor|connectivity|storage"
    build_types: ["Debug", "Release"]    # ⚙️ Supported build types
    ci_enabled: true                     # 🔄 Include in CI builds
    featured: true                       # ⭐ Show in featured list
```

#### 🔨 **Build Configuration Section**
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

---

## 🚀 **Quick Start Guide**

### 🎯 **Essential Commands**

```bash
# Navigate to your project root
cd /your/esp32/project

# 🏗️ Build with defaults (from app_config.yml)
./scripts/build_app.sh

# 🏗️ Build specific app
./scripts/build_app.sh gpio_test Release

# 📋 List all available apps
./scripts/build_app.sh list

# ❓ Get help
./scripts/build_app.sh --help
```

### ⚡ **Build Control**

```bash
# 🧹 Clean build (remove existing build directory)
./scripts/build_app.sh gpio_test Release --clean

# 🚫 Disable ccache
./scripts/build_app.sh gpio_test Release --no-cache

# 🌍 Environment variable overrides
export CLEAN=1
export USE_CCACHE=0
./scripts/build_app.sh gpio_test Debug
```

### 📱 **Flash and Monitor**

```bash
# ⚡ Flash and monitor
./scripts/flash_app.sh flash_monitor gpio_test Release --log

# ⚡ Flash only
./scripts/flash_app.sh flash gpio_test Release

# 📺 Monitor only (existing firmware)
./scripts/flash_app.sh monitor --log
```

### 📊 **Logging and Management**

```bash
# 📋 View latest log
./scripts/manage_logs.sh latest

# 🔍 Search logs for errors
./scripts/manage_logs.sh search "ERROR"

# 📋 List all logs
./scripts/manage_logs.sh list
```

---

## 📱 **Script Categories**

### 🔨 **Core Build System**
- **`build_app.sh`** - 🏗️ Configuration-driven ESP32 building
- **`config_loader.sh`** - ⚙️ YAML configuration parser and loader

### 📱 **Flash and Monitor System**
- **`flash_app.sh`** - ⚡ Flash firmware with monitoring and logging
- **`detect_ports.sh`** - 🔌 ESP32 port detection and troubleshooting

### 📋 **Logging and Management**
- **`manage_logs.sh`** - 📊 Log rotation, search, and management
- **`get_app_info.py`** - 🐍 CMake integration helper

### 🔧 **Setup and Environment**
- **`setup_repo.sh`** - 🏗️ Local development environment setup
- **`setup_ci.sh`** - 🔄 CI/CD environment setup
- **`setup_common.sh`** - 🔧 Shared setup functions

---

## 🌐 **Portability Features**

### 🔒 **Zero Hardcoded Dependencies**
- **📂 Relative Paths Only** - Everything works from project root
- **⚙️ Configuration-Driven** - Apps defined in YAML, not scripts
- **🔧 ESP-IDF Agnostic** - Works with any ESP-IDF version 5.5+
- **🌐 Cross-Platform** - Linux, macOS, Windows (WSL)

### 🔍 **Automatic Detection**
- **🎯 Project Root Detection** - Scripts find project root automatically
- **✅ Configuration Validation** - Checks for required files
- **🔧 ESP-IDF Environment** - Auto-sources ESP-IDF if needed
- **🔌 Port Detection** - Finds ESP32 devices automatically

### 📦 **Deployment Ready**
- **🚀 Copy & Deploy** - Entire directory can be copied to any project
- **🔧 Zero Configuration** - Works immediately with minimal setup
- **📚 Self-Documenting** - All scripts include comprehensive help
- **🔄 Version Independent** - No version-specific dependencies

---

## 🔗 **Documentation Navigation**

### 📚 **Comprehensive Documentation Structure**

The scripts directory includes extensive documentation organized for easy navigation:

- **📋 [Scripts Overview](docs/README_SCRIPTS_OVERVIEW.md)** - Complete system overview
- **🔨 [Build System](docs/README_BUILD_SYSTEM.md)** - Configuration-driven building
- **📱 [Flash System](docs/README_FLASH_SYSTEM.md)** - Flash and monitor operations
- **⚙️ [Configuration System](docs/README_CONFIG_SYSTEM.md)** - YAML configuration management
- **📊 [Logging System](docs/README_LOGGING_SYSTEM.md)** - Log management and analysis
- **🔧 [Utility Scripts](docs/README_UTILITY_SCRIPTS.md)** - Helper and utility functions
- **🔍 [Port Detection](docs/README_PORT_DETECTION.md)** - Device detection and troubleshooting

### 🧭 **Navigation Features**

- **🔗 Cross-References** - Every document links to related sections
- **📚 Breadcrumb Navigation** - Clear path through documentation
- **🔍 Quick Reference** - Fast access to common operations
- **📋 Examples** - Practical usage examples for every feature

---

## 🚨 **Troubleshooting**

### ⚠️ **Common Issues**

#### 📝 **Configuration Not Found**
```bash
# Error: Configuration file not found
ERROR: app_config.yml not found

# Solution: Create app_config.yml in project root
touch app_config.yml
# Then add configuration as shown above
```

#### 💻 **Source Files Not Found**
```bash
# Error: Source file not found
ERROR: main/GpioExample.cpp not found

# Solution: Create the source file or fix path in app_config.yml
mkdir -p main
touch main/GpioExample.cpp
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

### 💡 **Getting Help**
```bash
# Script help
./scripts/build_app.sh --help
./scripts/flash_app.sh --help

# List apps
./scripts/build_app.sh list

# Check configuration
./scripts/config_loader.sh --help
```

---

## 🔄 **Migration Guide**

### 🚀 **From Manual ESP-IDF Commands**
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

### 🔧 **From Other Build Systems**
1. **📋 Copy Scripts Directory** to your project root
2. **📝 Create app_config.yml** based on your apps
3. **🔨 Update CMakeLists.txt** to use `APP_TYPE` variable
4. **🧪 Test with a Simple App** first
5. **🔄 Gradually Migrate** other apps

---

## 🤝 **Contributing**

### ➕ **Adding New Apps**
1. **📝 Add to app_config.yml**:
```yaml
new_app:
  description: "New app description"
  source_file: "main/NewApp.cpp"
  category: "peripheral"
  build_types: ["Debug", "Release"]
  ci_enabled: true
  featured: false
```

2. **💻 Create Source File** in `main/NewApp.cpp`
3. **🧪 Test the Build**: `./scripts/build_app.sh new_app Release`

### 🔧 **Script Improvements**
- **🔒 Keep Scripts Portable** - No hardcoded project paths
- **⚙️ Use Configuration Functions** - Leverage `config_loader.sh`
- **❓ Add Help Support** - Every script should have `--help`
- **🌐 Test Across Platforms** - Ensure Linux/macOS/Windows compatibility

---

## 📄 **License & Support**

This scripts directory is designed to be **freely copied and adapted** for any ESP32 project. The scripts provide a **consistent, practical approach** to ESP32 development that can be shared across teams and projects.

### 🆘 **Support Resources**
- **📚 Documentation**: Comprehensive guides in the `docs/` subdirectory
- **🔍 Troubleshooting**: Built-in help and error messages
- **🔄 Examples**: Practical usage examples throughout documentation
- **🤝 Community**: Refer to original repository for updates and support

---

<div align="center">

**🚀 Ready to revolutionize your ESP32 development workflow?**

*Copy this directory to any ESP32 project and start building with confidence!*

</div>
