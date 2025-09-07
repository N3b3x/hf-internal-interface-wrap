# 🚀 ESP32 Examples - HardFOC Internal Interface Wrapper

<div align="center">

![ESP32 Examples](https://img.shields.io/badge/ESP32-Examples-blue?style=for-the-badge&logo=espressif)
![Build System](https://img.shields.io/badge/Build-System-green?style=for-the-badge&logo=github)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.5-orange?style=for-the-badge&logo=espressif)

**🎯 Comprehensive Examples with Advanced Build System for HardFOC ESP32 Development**

*Professional-grade examples demonstrating all HardFOC ESP32 interface capabilities with automated
ESP-IDF management, structured build system, and CI/CD integration*

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **Build System Architecture**](#-build-system-architecture)
- [🚀 **ESP-IDF Management**](#-esp-idf-management)
- [📁 **Project Structure**](#-project-structure)
- [🔧 **Quick Start**](#-quick-start)
- [📖 **Detailed Usage**](#-detailed-usage)
- [⚙️ **Configuration**](#-configuration)
- [📦 **Build Artifacts**](#-build-artifacts)
- [🔄 **CI/CD Integration**](#-cicd-integration)
- [🔍 **Troubleshooting**](#-troubleshooting)
- [📋 **Examples List**](#-examples-list)

---

## 🎯 **Overview**

The ESP32 examples directory provides comprehensive demonstrations of all HardFOC interface
capabilities, featuring a sophisticated build system that automatically manages ESP-IDF versions,
generates build matrices, and produces structured, parseable build outputs.

### 🏆 **Key Features**

- **🔧 Automated ESP-IDF Management** - Auto-detection, installation, and environment setup
- **📊 Dynamic Build Matrix Generation** - CI/CD matrix generation from configuration
- **📁 Structured Build Directories** - Parseable naming convention for automation
- **🔄 Incremental Builds** - Fast rebuilds with intelligent caching
- **📦 Complete Artifact Management** - All build outputs properly organized
- **🌐 CI/CD Ready** - Seamless integration with GitHub Actions
- **🛡️ Enhanced Validation System** - Smart combination validation and error prevention
- **🧠 Smart Defaults** - Automatic ESP-IDF version selection based on app and build type

---

## 🏗️ **Build System Architecture**

### **System Components**

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│                           📁 CONFIGURATION LAYER                            │
├─────────────────────────────────────────────────────────────────────────────┤
│  app_config.yml     ──┐                                                     │
│                       │                                                     │
│  generate_matrix.py ──┘                                                     │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                             🔧 BUILD LAYER                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│  build_app.sh    ──┐                                                        │
│  setup_common.sh   │                                                        │
│  setup_ci.sh       │                                                        │
│  setup_repo.sh   ──┘                                                        │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                             📦 OUTPUT LAYER                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│  Dynamic Build Directories  ──┐                                             │
│  Structured Naming            │                                             │
│  Complete Artifacts         ──┘                                             │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                            🔄 CI/CD LAYER                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│  GitHub Actions     ──┐                                                     │
│  Matrix Generation    │                                                     │
│  Artifact Upload    ──┘                                                     │
└─────────────────────────────────────────────────────────────────────────────┘

Data Flow:
app_config.yml → generate_matrix.py → Matrix Generation
setup_common.sh → build_app.sh → Dynamic Build Directories → Structured Naming → Complete Artifacts
→ Artifact Upload
```

### **Build Process Flow**

1. **Configuration Loading** - Read `app_config.yml` for build parameters
2. **🛡️ Smart Validation** - Validate app + build type + IDF version combination
3. **🧠 Smart Default Selection** - Auto-select ESP-IDF version if not specified
4. **ESP-IDF Detection** - Auto-detect or install required ESP-IDF version
5. **Environment Setup** - Source ESP-IDF and configure build environment
6. **Build Execution** - Run ESP-IDF build with project-specific settings
7. **Output Generation** - Create structured build directory with all artifacts
8. **Path Export** - Export build directory path for CI/CD integration

---

## 🛡️ **Enhanced Validation System**

The build system now includes comprehensive validation to prevent invalid build combinations and
provide clear guidance to users.

### **Validation Features**

- **🔍 Combination Validation** - Validates app + build type + IDF version combinations
- **🚫 Invalid Build Prevention** - Blocks builds with unsupported combinations
- **💡 Smart Error Messages** - Clear guidance on what combinations are allowed
- **🧠 Smart Defaults** - Automatic ESP-IDF version selection when not specified

### **Validation Flow**

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│                           BUILD REQUEST                                     │
│  app: gpio_test, build_type: Release, idf_version: (unspecified)            │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        BASIC VALIDATION FIRST                               │
│  • Validate app type exists                                                 │
│  • Validate build type is supported                                         │
│  • Fail fast if basic validation fails                                      │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        SMART DEFAULT SELECTION                              │
│  • Only if basic validation passes                                          │
│  • Check app-specific IDF versions                                          │
│  • Find first version supporting requested build type                       │
│  • Fallback to global defaults if needed                                    │
│  • Result: release/v5.5                                                     │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        FINAL COMBINATION VALIDATION                         │
│  • Single comprehensive check (no redundant individual validations)         │
│  • Functions remain standalone-safe for independent sourcing                │
│  • Check combination constraints                                            │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           VALIDATION RESULT                                 │
│  ✅ VALID: gpio_test + Release + release/v5.5                               │
│  → Proceed with build                                                       │
│                                                                             │
│  ❌ INVALID: gpio_test + Release + release/v5.4                             │
│  → Show error with valid combinations                                       │
│  → Provide helpful next steps                                               │
└─────────────────────────────────────────────────────────────────────────────┘
```text

**Key Optimization Points:**
- **Early Exit**: Basic validation happens first, failing fast on invalid inputs
- **Smart Defaults**: IDF version selection only occurs after basic validation passes
- **Function Safety**: Individual validation functions remain standalone-safe for independent sourcing
- **No Redundancy**: Combination validation doesn't repeat basic checks already performed

---

## 🚀 **ESP-IDF Management**

### **Automatic ESP-IDF Setup**

The build system automatically manages ESP-IDF versions without manual intervention:

#### **Detection Process**
```bash
## 1. Check existing installations
~/.espressif/esp-idf-{version}/
~/esp/esp-idf-{version}/

## 2. Auto-download if missing
git clone --recursive https://github.com/espressif/esp-idf.git esp-idf-{version}

## 3. Install tools and dependencies
./install.sh

## 4. Source environment
source export.sh
```text

#### **Supported ESP-IDF Versions**
- **v4.4** - Legacy support for older projects
- **v5.0** - Stable release with modern features
- **v5.1** - Enhanced performance and security
- **v5.2** - Improved toolchain and debugging
- **v5.3** - Latest stable with full ESP32-C6 support
- **v5.4** - Performance optimizations
- **v5.5** - Current latest release (recommended)

#### **Target Support**
- **ESP32-C6** - Primary target with full feature support
- **ESP32-S3** - Secondary target for compatibility
- **ESP32** - Legacy target support

### **Environment Variables**

The system automatically sets and manages:

```bash
export IDF_PATH="/path/to/esp-idf-{version}"
export PATH="$IDF_PATH/tools:$PATH"
export ESP_IDF_VERSION="{version}"
export IDF_TARGET="esp32c6"
```text

---

## 📁 **Project Structure**

```text
examples/esp32/
├── 📁 components/              # Custom ESP-IDF components
├── 📁 main/                    # Main application source code
│   ├── 📄 CMakeLists.txt       # App being built cmake list
├── 📁 scripts/                # Build and utility scripts
│   ├── 📄 app_config.yml      # Centralized configuration
│   ├── 📄 generate_matrix.py  # CI matrix generator
│   ├── 📄 build_app.sh        # Main build script
│   ├── 📄 setup_common.sh     # Shared setup functions
│   ├── 📄 setup_ci.sh         # CI environment setup
│   ├── 📄 setup_repo.sh       # Local development setup
│   ├── 📄 flash_app.sh        # Flashing and monitoring
│   └── 📄 config_loader.sh    # Configuration utilities
├── 📁 build-*/                # Generated build directories
└── 📄 app_config.yml          # All possible main applications that can be built
└── 📄 CMakeLists.txt          # Project Cmake list
└── 📄 sdkconfig               # Project config (idf.py menuconfig)
└── 📄 README.md               # This documentation
```yaml

### **Key Files Explained**

#### **`app_config.yml`**
Centralized configuration for all applications, build types, and ESP-IDF versions:

```yaml
metadata:
  idf_versions: ["release/v5.5", "release/v5.4"]
  build_types: [["Debug", "Release"], ["Debug", "Release"]]
  target: "esp32c6"

apps:
  gpio_test:
    ci_enabled: true
    description: "GPIO peripheral comprehensive testing"
    idf_versions: ["release/v5.5"]  # Override global
    build_types: [["Debug", "Release"]]
```python

#### **`generate_matrix.py`**
Python script that generates CI/CD build matrices from centralized configuration:

```bash
## Generate full matrix (default JSON output)
python3 scripts/generate_matrix.py

## YAML format output
python3 scripts/generate_matrix.py --format yaml

## Filter for specific app
python3 scripts/generate_matrix.py --filter gpio_test

## Validate configuration
python3 scripts/generate_matrix.py --validate

## Verbose output with validation
python3 scripts/generate_matrix.py --verbose --validate

## Output to file
python3 scripts/generate_matrix.py --output matrix.json

## Complex combination
python3 scripts/generate_matrix.py --filter wifi_test --validate --verbose --format yaml --output
wifi_matrix.yaml
```yaml

**Features:**
- **Configuration Validation**: Validates `app_config.yml` structure and content
- **Flexible Output**: JSON (GitHub Actions) and YAML formats
- **App Filtering**: Filter matrix for specific applications
- **Verbose Processing**: Detailed processing information and statistics
- **Smart Path Detection**: Works from any directory
- **CI Integration**: Ready for GitHub Actions, GitLab CI, and Jenkins

#### **`build_app.sh`**
Main build script with automatic ESP-IDF management:

```bash
## Basic usage
./scripts/build_app.sh <app_name> <build_type> [idf_version]

## Examples
./scripts/build_app.sh gpio_test Release
./scripts/build_app.sh adc_test Debug release/v5.4
./scripts/build_app.sh wifi_test Release release/v5.5
```text

---

## 🔧 **Quick Start**

### **1. Initial Setup**

```bash
## Clone the repository
git clone <repository-url>
cd hf-internal-interface-wrap

## Navigate to ESP32 examples
cd examples/esp32

## Setup development environment
source scripts/setup_repo.sh
```text

### **2. Build Your First Application**

```bash
## Build GPIO test application
./scripts/build_app.sh gpio_test Release

## Build ADC test with specific ESP-IDF version
./scripts/build_app.sh adc_test Debug release/v5.4

## Build with clean rebuild
CLEAN=1 ./scripts/build_app.sh wifi_test Release
## Or
./scripts/build_app.sh wifi_test Release --clean
```text

### **3. Flash and Monitor**

```bash
## Flash and monitor
./scripts/flash_app.sh flash_monitor gpio_test Release

## Flash only
./scripts/flash_app.sh flash gpio_test Release

## Monitor only
./scripts/flash_app.sh monitor
```text

---

## 📖 **Detailed Usage**

### **Build Script Options**

#### **`build_app.sh`**
```bash
./scripts/build_app.sh [OPTIONS] <app_name> <build_type> [idf_version]

Options:
  -c, --clean          Clean build (remove existing build directory)
  -v, --verbose        Verbose output
  -h, --help           Show this help message

Arguments:
  app_name             Application name from app_config.yml
  build_type           Build type (Debug, Release)
  idf_version          ESP-IDF version (optional, uses default if not specified)

Examples:
  ./scripts/build_app.sh gpio_test Release
  ./scripts/build_app.sh adc_test Debug release/v5.4
  ./scripts/build_app.sh --clean wifi_test Release
```text

#### **`flash_app.sh`**
```bash
./scripts/flash_app.sh <action> [app_name] [build_type]

Actions:
  flash                Flash firmware only
  monitor              Monitor serial output only
  flash_monitor        Flash and then monitor
  flash_erase          Erase flash and flash firmware

Examples:
  ./scripts/flash_app.sh flash_monitor gpio_test Release
  ./scripts/flash_app.sh monitor
  ./scripts/flash_app.sh flash_erase adc_test Debug
```text

### **Environment Setup Scripts**

#### **`setup_repo.sh` (Local Development)**
```bash
## Setup local development environment
source scripts/setup_repo.sh

## This script:
## 1. Installs development tools (clang, clang-format, clang-tidy)
## 2. Sets up ESP-IDF environment
## 3. Configures build tools
## 4. Exports necessary environment variables
```text

#### **`setup_ci.sh` (CI/CD Environment)**
```bash
## Setup CI environment
source scripts/setup_ci.sh

## This script:
## 1. Installs CI-specific tools
## 2. Sets up ESP-IDF environment
## 3. Configures for automated builds
## 4. Exports CI-specific variables
```yaml

---

## ⚙️ **Configuration**

### **Application Configuration**

Each application can be configured in `app_config.yml`:

```yaml
apps:
  gpio_test:
    ci_enabled: true                    # Include in CI builds
    description: "GPIO testing suite"   # Human-readable description
    idf_versions: ["release/v5.5"]     # Override global IDF versions
    build_types: [["Debug", "Release"]] # Override global build types
    
  adc_test:
    ci_enabled: true
    description: "ADC peripheral testing"
    # Uses global IDF versions and build types
    
  wifi_test:
    ci_enabled: false                   # Exclude from CI builds
    description: "WiFi functionality testing"
    idf_versions: ["release/v5.4"]     # Specific IDF version only
    build_types: [["Release"]]         # Release builds only
```text

### **Build Configuration**

#### **Global Settings**
```yaml
metadata:
  # ESP-IDF versions to support
  idf_versions: ["release/v5.5", "release/v5.4", "release/v5.3"]
  
  # Build types per IDF version (nested array)
  build_types: [
    ["Debug", "Release"],  # For release/v5.5
    ["Debug", "Release"],  # For release/v5.4
    ["Debug"]              # For release/v5.3
  ]
  
  # Target MCU
  target: "esp32c6"
  
  # Build directory pattern
build_directory_pattern: "build-app-{app_type}-type-{build_type}-target-{target}-idf-{idf_version}"
```text

#### **CI Configuration**
```yaml
ci_config:
  # Exclude specific combinations
  exclude_combinations:
    - app_name: "wifi_test"
      idf_version: "release/v5.3"
      build_type: "Release"
    - app_name: "bluetooth_test"
      idf_version: "release/v5.4"
      build_type: "Debug"
```text

### **Build Directory Naming**

The system generates structured build directory names:

```text
build-app-{app_type}-type-{build_type}-target-{target}-idf-{idf_version}
```json

**Examples:**
- `build-app-gpio_test-type-Release-target-esp32c6-idf-release_v5_5`
- `build-app-adc_test-type-Debug-target-esp32c6-idf-release_v5_4`
- `build-app-wifi_test-type-Release-target-esp32c6-idf-release_v5_3`

**Benefits:**
- ✅ **ESP-IDF Compatible** - No special characters
- ✅ **Cross-Platform Safe** - Works on all file systems
- ✅ **Handles Hyphenated Names** - No parsing ambiguity
- ✅ **Structured & Parsable** - Clear section boundaries
- ✅ **CI/CD Ready** - Easy automation integration

---

## 📦 **Build Artifacts**

### **Generated Files**

Each build produces comprehensive artifacts:

#### **Main Application Files**
- **`{app_name}.bin`** - Flashable firmware binary
- **`{app_name}.elf`** - ELF file for debugging and analysis
- **`{app_name}.map`** - Memory layout and symbol information
- **`{app_name}.hex`** - Intel HEX format (if enabled)

#### **Bootloader and System Files**
- **`bootloader/bootloader.bin`** - ESP32 bootloader
- **`partition_table/partition-table.bin`** - Flash partition layout
- **`sdkconfig`** - ESP-IDF configuration file
- **`config.env`** - Environment configuration

#### **Build System Files**
- **`build.ninja`** - Ninja build system file
- **`CMakeCache.txt`** - CMake cache
- **`compile_commands.json`** - IDE integration
- **`.ninja_deps`** - Build dependencies

#### **Documentation and Analysis**
- **`project_description.json`** - Project metadata
- **`size.txt`** - Memory usage analysis
- **`ccache_stats.txt`** - Cache statistics (if enabled)

### **Artifact Organization**

```text
build-app-{app_name}-type-{build_type}-target-{target}-idf-{idf_version}/
├── 📄 {app_name}.bin              # Main firmware
├── 📄 {app_name}.elf              # Debug information
├── 📄 {app_name}.map              # Memory layout
├── 📁 bootloader/                 # Bootloader files
│   ├── 📄 bootloader.bin
│   └── 📄 bootloader.elf
├── 📁 partition_table/            # Partition information
│   └── 📄 partition-table.bin
├── 📁 esp-idf/                    # ESP-IDF components
├── 📄 sdkconfig                   # Configuration
├── 📄 build.ninja                 # Build system
└── 📄 compile_commands.json       # IDE integration
```text

---

## 🔄 **CI/CD Integration**

### **GitHub Actions Workflow**

The project includes a comprehensive CI/CD pipeline:

#### **Workflow Structure**
```yaml
name: ESP32 Component CI • Build • Size • Static Analysis

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
  workflow_dispatch:

jobs:
  setup-environment:     # Setup development tools
  generate-matrix:       # Generate build matrix from config
  build:                 # Build all applications
  static-analysis:       # Code quality analysis
  workflow-lint:         # Workflow validation
```yaml

#### **Matrix Generation**
The CI automatically generates build matrices from `app_config.yml`:

```yaml
## Generated matrix example
matrix:
  include:
    - idf_version: "release/v5.5"
      build_type: "Debug"
      app_name: "gpio_test"
      target: "esp32c6"
    - idf_version: "release/v5.5"
      build_type: "Release"
      app_name: "gpio_test"
      target: "esp32c6"
    # ... more combinations
```text

#### **Build Process**
```yaml
- name: ESP-IDF Build
  run: |
    # Source CI setup
    source ${{ env.ESP32_PROJECT_PATH }}/scripts/setup_ci.sh
    
    # Build using standard script
    ./scripts/build_app.sh "${{ matrix.app_name }}" "${{ matrix.build_type }}" "${{ matrix.idf_version }}"
    
    # Capture build directory for artifacts
    echo "build_dir=$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY" >> $GITHUB_OUTPUT
```text

#### **Artifact Upload**
```yaml
- name: Upload artifacts
  uses: actions/upload-artifact@v4
  with:
    name: fw-${{ matrix.app_name }}-${{ matrix.idf_version_docker }}-${{ matrix.build_type }}
    path: ${{ steps.build.outputs.build_dir }}
```text

### **CI Environment Variables**

```yaml
env:
  BUILD_PATH: ci_build_path
  IDF_CCACHE_ENABLE: "1"
  ESP32_PROJECT_PATH: examples/esp32
```text

### **Caching Strategy**

The CI implements intelligent caching:

```yaml
- name: Cache ESP-IDF and tools
  uses: actions/cache@v4
  with:
    path: |
      ~/.espressif
      ~/esp
    key: esp-idf-${{ matrix.idf_version_docker }}-${{ runner.os }}
    
- name: Cache ccache
  uses: actions/cache@v4
  with:
    path: ~/.ccache
    key: ccache-${{ matrix.idf_version_docker }}-${{ matrix.build_type }}
```text

---

## 🔍 **Troubleshooting**

### **Common Issues and Solutions**

#### **ESP-IDF Not Found**
```bash
## Error: ESP-IDF environment not found
## Solution: The build system will auto-install ESP-IDF

## Manual installation if needed:
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git esp-idf-release_v5_5
cd esp-idf-release_v5_5
./install.sh
source export.sh
```text

#### **Build Directory Issues**
```bash
## Error: Invalid build directory name
## Solution: Check app_config.yml build_directory_pattern

## Ensure pattern follows format:
build_directory_pattern: "build-app-{app_type}-type-{build_type}-target-{target}-idf-{idf_version}"
```text

#### **Permission Issues**
```bash
## Error: Permission denied
## Solution: Make scripts executable
chmod +x scripts/*.sh
chmod +x scripts/*.py
```python

#### **Python Dependencies**
```bash
## Error: Module not found
## Solution: Install required packages
pip install pyyaml
pip install esptool
```text

### **Debug Mode**

Enable verbose output for debugging:

```bash
## Verbose build
./scripts/build_app.sh --verbose gpio_test Release

## Clean rebuild
CLEAN=1 ./scripts/build_app.sh gpio_test Release

## Check environment
source scripts/setup_repo.sh
echo $IDF_PATH
echo $IDF_TARGET
```text

### **Log Files**

Build logs are available in:

```bash
## Build log
cat build-*/log/build.log

## CMake log
cat build-*/CMakeFiles/CMakeOutput.log

## Ninja log
cat build-*/.ninja_log
```cpp

---

## 📋 **Examples List**

### **Available Applications**

| Application | Description | CI Status | IDF Versions | Build Types |

|-------------|-------------|-----------|--------------|-------------|

| `gpio_test` | GPIO peripheral comprehensive testing | ✅ Enabled | v5.5 | Debug, Release |

| `adc_test` | ADC peripheral testing | ✅ Enabled | v5.5 | Debug, Release |

| `uart_test` | UART communication testing | ✅ Enabled | v5.5 | Debug, Release |

| `spi_test` | SPI interface testing | ✅ Enabled | v5.5 | Debug, Release |

| `i2c_test` | I2C interface testing | ✅ Enabled | v5.5 | Debug, Release |

| `pwm_test` | PWM generation testing | ✅ Enabled | v5.5 | Debug, Release |

| `can_test` | CAN bus testing | ✅ Enabled | v5.5 | Debug, Release |

| `pio_test` | Programmable I/O testing | ✅ Enabled | v5.5 | Debug, Release |

| `temperature_test` | Temperature sensor testing | ✅ Enabled | v5.5 | Debug, Release |

| `nvs_test` | Non-volatile storage testing | ✅ Enabled | v5.5 | Debug, Release |

| `timer_test` | Timer functionality testing | ✅ Enabled | v5.5 | Debug, Release |

| `logger_test` | Logging system testing | ✅ Enabled | v5.5 | Debug, Release |

| `wifi_test` | WiFi functionality testing | ✅ Enabled | v5.5 | Debug, Release |

| `bluetooth_test` | Bluetooth testing | ✅ Enabled | v5.5 | Debug, Release |

| `utils_test` | Utility functions testing | ✅ Enabled | v5.5 | Debug, Release |

| `ascii_art` | ASCII art generation demo | ✅ Enabled | v5.5 | Debug, Release |

### **Application Categories**

#### **Core Peripherals**
- **GPIO** - Digital input/output testing
- **ADC** - Analog-to-digital conversion
- **PWM** - Pulse-width modulation
- **Timer** - Hardware timer functionality

#### **Communication Interfaces**
- **UART** - Serial communication
- **SPI** - Serial peripheral interface
- **I2C** - Inter-integrated circuit
- **CAN** - Controller area network

#### **Wireless Technologies**
- **WiFi** - Wireless networking
- **Bluetooth** - Short-range communication

#### **System Features**
- **NVS** - Non-volatile storage
- **Logger** - Logging and debugging
- **PIO** - Programmable I/O
- **Temperature** - Thermal monitoring

#### **Utilities**
- **ASCII Art** - Text-based graphics
- **Utils** - Common utility functions

### **Test Documentation**

Each application includes comprehensive test documentation:

- **[Test Documentation](docs/README.md)** - Complete test documentation index
- **[CAN Test Documentation](docs/README_CAN_TEST.md)** - CAN bus testing with SN65 transceiver
- **[GPIO Test Documentation](docs/README_GPIO_TEST.md)** - GPIO testing and validation
- **[ADC Test Documentation](docs/README_ADC_TEST.md)** - ADC testing and calibration
- **[PWM Test Documentation](docs/README_PWM_TEST.md)** - PWM testing and frequency control
- **[UART Test Documentation](docs/README_UART_TESTING.md)** - UART communication testing
- **[SPI Test Documentation](docs/README_SPI_TEST.md)** - SPI interface testing
- **[I2C Test Documentation](docs/README_I2C_TEST.md)** - I2C device testing
- **[PIO Test Documentation](docs/README_PIO_TEST.md)** - Programmable I/O testing
- **[Temperature Test Documentation](docs/README_TEMPERATURE_TEST.md)** - Temperature sensor testing
- **[NVS Test Documentation](docs/README_NVS_TEST.md)** - Non-volatile storage testing
- **[Logger Test Documentation](docs/README_LOGGER_TEST.md)** - Logging system testing

---

## 🤝 **Contributing**

### **Adding New Applications**

1. **Create Application Source**
   ```cpp
   // main/NewAppTest.cpp
   #include "TestFramework.h"
   
   class NewAppTest : public TestFramework {
   public:
       void RunTests() override {
           // Test implementation
       }
   };
   
   TEST_MAIN(NewAppTest)
   ```

1. **Update Configuration**
   ```yaml
   # app_config.yml
   apps:
     new_app_test:
       ci_enabled: true
       description: "New application testing"
       idf_versions: ["release/v5.5"]
       build_types: [["Debug", "Release"]]
   ```

1. **Test Build**
```bash
   ./scripts/build_app.sh new_app_test Release
   ```

### **Modifying Build System**

1. **Update Scripts** - Modify scripts in `scripts/` directory
2. **Test Locally** - Verify changes work in local environment
3. **Update CI** - Ensure CI pipeline compatibility
4. **Update Documentation** - Keep this README current

---

## 📄 **License**

This project is licensed under the GPL-3.0 License - see the [LICENSE](../../LICENSE) file for
details.

---

## 🔗 **Related Documentation**

- [Main Project README](../../README.md) - Project overview and architecture
- [API Documentation](../../docs/) - Interface API documentation
- [Test Documentation](docs/README.md) - Comprehensive test documentation and examples
- [CI/CD Workflows](../../.github/workflows/) - GitHub Actions workflows
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/) - ESP-IDF reference

---

<div align="center">

**🚀 Built with ❤️ for the HardFOC Community**

*Professional-grade examples with enterprise-ready build system*

</div>
