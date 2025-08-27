# 🔄 GitHub Actions CI/CD - HardFOC Internal Interface Wrapper

<div align="center">

![CI/CD](https://img.shields.io/badge/CI%2FCD-GitHub%20Actions-blue?style=for-the-badge&logo=github)
![ESP32](https://img.shields.io/badge/ESP32-Automated%20Builds-green?style=for-the-badge&logo=espressif)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-Auto%20Management-orange?style=for-the-badge&logo=espressif)

**🎯 Professional CI/CD Pipeline with Automated ESP-IDF Management for HardFOC ESP32 Development**

*Enterprise-grade continuous integration and deployment with intelligent build matrix generation, automated ESP-IDF setup, and comprehensive artifact management*

</div>

---

## 📚 **Table of Contents**

- [🎯 **Overview**](#-overview)
- [🏗️ **CI/CD Architecture**](#️-cicd-architecture)
- [📊 **Workflow Structure**](#-workflow-structure)
- [🚀 **ESP-IDF Management**](#-esp-idf-management)
- [🔧 **Build Process**](#-build-process)
- [📦 **Artifact Management**](#-artifact-management)
- [⚙️ **Configuration**](#️-configuration)
- [🔄 **Matrix Generation**](#️-matrix-generation)
- [🔍 **Monitoring and Debugging**](#️-monitoring-and-debugging)
- [🤝 **Contributing**](#-contributing)

---

## 🎯 **Overview**

The GitHub Actions CI/CD pipeline provides comprehensive automation for HardFOC ESP32 development, featuring intelligent build matrix generation, automated ESP-IDF management, and professional-grade artifact handling.

### 🏆 **Key Features**

- **🔧 Automated ESP-IDF Management** - Auto-detection, installation, and environment setup
- **📊 Dynamic Build Matrix Generation** - CI matrix generation from centralized configuration
- **🔄 Intelligent Caching** - Multi-layer caching for faster builds
- **📦 Complete Artifact Management** - All build outputs properly organized and uploaded
- **🌐 Cross-Platform Support** - Linux and Windows compatibility
- **🔍 Comprehensive Testing** - Build, size analysis, and static analysis
- **🛡️ Enhanced Validation** - Smart combination validation and error prevention in CI
- **🧠 Smart Defaults** - Automatic ESP-IDF version selection for CI builds

---

## 🏗️ **CI/CD Architecture**

### **Pipeline Overview**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           🚀 TRIGGER LAYER                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│  Push to main      ──┐                                                      │
│  Pull Request        │                                                      │
│  Manual Dispatch   ──┘                                                      │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           🔧 SETUP LAYER                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│  Setup Environment  ──┐                                                     │
│  Install Tools        │                                                     │
│  Cache Dependencies ──┘                                                     │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           📊 MATRIX LAYER                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│  Generate Matrix    ──┐                                                     │
│  Parse Configuration  │                                                     │
│  Create Build Jobs  ──┘                                                     │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           🏗️ BUILD LAYER                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│  Build Applications ──┐                                                     │
│  Generate Artifacts   │                                                     │
│  Export Paths       ──┘                                                     │
└─────────────────────┬───────────────────────────────────────────────────────┘
                      │
                      ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           📦 OUTPUT LAYER                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│  Upload Artifacts    ──┐                                                    │
│  Static Analysis       │                                                    │
│  Workflow Validation ──┘                                                    │
└─────────────────────────────────────────────────────────────────────────────┘

Data Flow:
Trigger → Setup → Matrix → Build → Output
```

### **Workflow Components**

1. **Setup Environment** - Install development tools and ESP-IDF
2. **Generate Matrix** - Create build matrix from `app_config.yml`
3. **Build Applications** - Build all applications with different configurations
4. **Static Analysis** - Code quality and security analysis
5. **Workflow Validation** - YAML and action validation

---

## 📊 **Workflow Structure**

### **Main Workflow: `esp32-component-ci.yml`**

#### **Workflow Triggers**
```yaml
on:
  push:
    branches: [main]
  pull_request:
    branches: [main]
  workflow_dispatch:
    inputs:
      clean_build:
        description: 'Force clean build (ignore all caches)'
        required: false
        default: false
        type: boolean
```

#### **Job Structure**
```yaml
jobs:
  setup-environment:     # Setup development tools
  generate-matrix:       # Generate build matrix from config
  build:                 # Build all applications
  static-analysis:       # Code quality analysis
  workflow-lint:         # Workflow validation
```

#### **Concurrency Control**
```yaml
concurrency:
  group: ci-${{ github.ref }}
  cancel-in-progress: true
```

### **Environment Variables**

```yaml
env:
  BUILD_PATH: ci_build_path
  IDF_CCACHE_ENABLE: "1"  # Enables ccache inside ESP-IDF
  ESP32_PROJECT_PATH: examples/esp32  # Centralized ESP32 project location
```

---

## 🚀 **ESP-IDF Management**

### **Automated ESP-IDF Setup**

The CI pipeline automatically manages ESP-IDF versions without manual intervention:

#### **Setup Process**
```yaml
- name: Setup development environment
  run: |
    echo "Setting up development environment..."
    chmod +x ${{ env.ESP32_PROJECT_PATH }}/scripts/setup_ci.sh
    ./${{ env.ESP32_PROJECT_PATH }}/scripts/setup_ci.sh
```

#### **ESP-IDF Installation**
```bash
# The setup_ci.sh script automatically:
1. Detects system requirements
2. Installs development tools
3. Downloads required ESP-IDF version
4. Installs ESP-IDF tools and dependencies
5. Configures build environment
6. Exports necessary environment variables
```

#### **Supported ESP-IDF Versions**
- **v4.4** - Legacy support for older projects
- **v5.0** - Stable release with modern features
- **v5.1** - Enhanced performance and security
- **v5.2** - Improved toolchain and debugging
- **v5.3** - Latest stable with full ESP32-C6 support
- **v5.4** - Performance optimizations
- **v5.5** - Current latest release (recommended)

### **Environment Configuration**

```yaml
- name: ESP-IDF Build with caching
  uses: espressif/esp-idf-ci-action@v1
  with:
    esp_idf_version: ${{ matrix.idf_version_docker }}
    target: ${{ matrix.target }}
    path: .
    extra_docker_args: >-
      -v $HOME/.ccache:/root/.ccache
      -e CCACHE_DIR=/root/.ccache
      -e IDF_CCACHE_ENABLE=1
    command: |
      # Set environment variables for the build
      export BUILD_PATH="${{ env.BUILD_PATH }}"
      export ESP32_PROJECT_PATH="${{ env.ESP32_PROJECT_PATH }}"
      export IDF_TARGET="${{ matrix.target }}"
      export BUILD_TYPE="${{ matrix.build_type }}"
      export APP_TYPE="${{ matrix.app_name }}"
      export IDF_VERSION="${{ matrix.idf_version }}"
      
      # Source the CI setup script
      source ${{ env.ESP32_PROJECT_PATH }}/scripts/setup_ci.sh
      
      # Build using standard build_app.sh for consistency
      ./${{ env.ESP32_PROJECT_PATH }}/scripts/build_app.sh "${{ matrix.app_name }}" "${{ matrix.build_type }}" "${{ matrix.idf_version }}"
```

---

## 🔧 **Build Process**

### **Build Job Configuration**

#### **Matrix Strategy**
```yaml
strategy:
  fail-fast: false
  matrix: ${{fromJson(needs.generate-matrix.outputs.matrix)}}
```

#### **Build Steps**
```yaml
- name: ESP-IDF Build with caching
  id: build
  uses: espressif/esp-idf-ci-action@v1
  with:
    esp_idf_version: ${{ matrix.idf_version_docker }}
    target: ${{ matrix.target }}
    path: .
    command: |
      # Build the application using the same tool as local development
      ./${{ env.ESP32_PROJECT_PATH }}/scripts/build_app.sh "${{ matrix.app_name }}" "${{ matrix.build_type }}" "${{ matrix.idf_version }}"
      
      # Get the build directory that build_app.sh exported
      if [ -n "$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY" ]; then
        echo "Build completed. Build directory from build_app.sh: $ESP32_BUILD_APP_MOST_RECENT_DIRECTORY"
        
        # Verify build artifacts exist
        if [ -d "$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY" ]; then
          echo "Build artifacts found in: $ESP32_BUILD_APP_MOST_RECENT_DIRECTORY"
          ls -la "$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY/"
          
          # Set output for artifact upload step to use
          echo "build_dir=$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY" >> $GITHUB_OUTPUT
        else
          echo "ERROR: Build directory not found: $ESP32_BUILD_APP_MOST_RECENT_DIRECTORY"
          exit 1
        fi
      else
        echo "ERROR: ESP32_BUILD_APP_MOST_RECENT_DIRECTORY not set by build_app.sh"
        exit 1
      fi
```

### **Build Output Capture**

The build process captures the build directory path for artifact upload:

```bash
# build_app.sh exports the build directory
export ESP32_BUILD_APP_MOST_RECENT_DIRECTORY="$BUILD_DIR"

# CI captures this for artifact upload
echo "build_dir=$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY" >> $GITHUB_OUTPUT
```

### **Build Artifacts**

Each build produces comprehensive artifacts:

- **Main Binary**: `{app_name}.bin` - Flashable firmware
- **ELF File**: `{app_name}.elf` - Debugging and analysis
- **Map File**: `{app_name}.map` - Memory layout and symbol information
- **Bootloader**: `bootloader/bootloader.bin` - ESP32 bootloader
- **Partition Table**: `partition_table/partition-table.bin` - Flash layout
- **Build Configuration**: `sdkconfig` - ESP-IDF configuration
- **Compile Commands**: `compile_commands.json` - IDE integration

---

## 📦 **Artifact Management**

### **Artifact Upload**

```yaml
- name: Upload artifacts
  uses: actions/upload-artifact@v4
  if: always()
  with:
    name: fw-${{ matrix.app_name }}-${{ matrix.idf_version_docker }}-${{ matrix.build_type }}
    retention-days: 7
    path: ${{ steps.build.outputs.build_dir }}
```

### **Artifact Naming Convention**

Artifacts are named using a structured format:

```
fw-{app_name}-{idf_version_docker}-{build_type}
```

**Examples:**
- `fw-gpio_test-release-v5.5-Release`
- `fw-adc_test-release-v5.4-Debug`
- `fw-wifi_test-release-v5.3-Release`

### **Artifact Retention**

- **Retention Period**: 7 days
- **Storage**: GitHub Actions artifact storage
- **Access**: Available for download and analysis
- **Cleanup**: Automatic cleanup after retention period

---

## ⚙️ **Configuration**

### **Centralized Configuration**

The CI pipeline reads configuration from `examples/esp32/app_config.yml`:

```yaml
metadata:
  idf_versions: ["release/v5.5", "release/v5.4", "release/v5.3"]
  build_types: [["Debug", "Release"], ["Debug", "Release"], ["Debug"]]
  target: "esp32c6"
  build_directory_pattern: "build-app-{app_type}-type-{build_type}-target-{target}-idf-{idf_version}"

apps:
  gpio_test:
    ci_enabled: true
    description: "GPIO peripheral comprehensive testing"
    idf_versions: ["release/v5.5"]
    build_types: [["Debug", "Release"]]
    
  adc_test:
    ci_enabled: true
    description: "ADC peripheral testing"
    # Uses global configuration
    
  wifi_test:
    ci_enabled: false  # Exclude from CI
    description: "WiFi functionality testing"
    idf_versions: ["release/v5.4"]
    build_types: [["Release"]]
```

### **CI Configuration**

```yaml
ci_config:
  exclude_combinations:
    - app_name: "wifi_test"
      idf_version: "release/v5.3"
      build_type: "Release"
    - app_name: "bluetooth_test"
      idf_version: "release/v5.4"
      build_type: "Debug"
```

---

## 🔄 **Matrix Generation**

### **Matrix Generation Process**

The CI pipeline automatically generates build matrices from configuration:

#### **Generation Script**
```yaml
- name: Generate matrix
  id: generate-matrix
  run: |
    MATRIX=$(python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py)
    echo "matrix=${MATRIX}" >> "$GITHUB_OUTPUT"
    echo "Generated matrix:"
    python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --format json | jq .
```

#### **Generated Matrix Example**
```json
{
  "include": [
    {
      "idf_version": "release/v5.5",
      "idf_version_docker": "release-v5.5",
      "build_type": "Debug",
      "app_name": "gpio_test",
      "target": "esp32c6",
      "config_source": "app"
    },
    {
      "idf_version": "release/v5.5",
      "idf_version_docker": "release-v5.5",
      "build_type": "Release",
      "app_name": "gpio_test",
      "target": "esp32c6",
      "config_source": "app"
    }
  ]
}
```

#### **Matrix Features**
- **Dynamic Generation** - Automatically generated from configuration
- **Hierarchical Overrides** - Per-app configuration overrides global settings
- **CI Control** - Enable/disable applications in CI builds
- **Exclusion Rules** - Exclude specific combinations from CI
- **Flexible Mapping** - Different build types per IDF version

### **Matrix Outputs**

The matrix generation provides multiple output formats and features:

```bash
# Full matrix (default JSON output)
python3 scripts/generate_matrix.py

# YAML format output
python3 scripts/generate_matrix.py --format yaml

# Filter for specific app
python3 scripts/generate_matrix.py --filter gpio_test

# Validate configuration
python3 scripts/generate_matrix.py --validate

# Verbose output with validation
python3 scripts/generate_matrix.py --verbose --validate

# Output to file
python3 scripts/generate_matrix.py --output matrix.json

# Complex combination
python3 scripts/generate_matrix.py --filter wifi_test --validate --verbose --format yaml --output wifi_matrix.yaml
```

**Current Features:**
- **Configuration Validation**: Validates `app_config.yml` structure and content
- **Flexible Output**: JSON (GitHub Actions) and YAML formats
- **App Filtering**: Filter matrix for specific applications
- **Verbose Processing**: Detailed processing information and statistics
- **Smart Path Detection**: Works from any directory
- **CI Integration**: Ready for GitHub Actions, GitLab CI, and Jenkins

---

## 🔍 **Monitoring and Debugging**

### **Build Monitoring**

#### **Real-time Logs**
```yaml
- name: ESP-IDF Build with caching
  id: build
  uses: espressif/esp-idf-ci-action@v1
  # Build logs are automatically displayed in real-time
```

#### **Build Verification**
```bash
# Verify build artifacts exist
if [ -d "$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY" ]; then
  echo "Build artifacts found in: $ESP32_BUILD_APP_MOST_RECENT_DIRECTORY"
  ls -la "$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY/"
else
  echo "ERROR: Build directory not found: $ESP32_BUILD_APP_MOST_RECENT_DIRECTORY"
  exit 1
fi
```

### **Error Handling**

#### **Comprehensive Error Checking**
```bash
# Check build directory export
if [ -n "$ESP32_BUILD_APP_MOST_RECENT_DIRECTORY" ]; then
  echo "Build completed successfully"
else
  echo "ERROR: ESP32_BUILD_APP_MOST_RECENT_DIRECTORY not set by build_app.sh"
  echo "Available directories:"
  ls -la build_*/ || echo "No build directories found"
  exit 1
fi
```

#### **Debug Information**
```bash
# Display available build directories
ls -la build_*/ || echo "No build directories found"

# Show environment variables
echo "BUILD_PATH: $BUILD_PATH"
echo "ESP32_PROJECT_PATH: $ESP32_PROJECT_PATH"
echo "IDF_TARGET: $IDF_TARGET"
echo "BUILD_TYPE: $BUILD_TYPE"
echo "APP_TYPE: $APP_TYPE"
echo "IDF_VERSION: $IDF_VERSION"
```

### **Cache Monitoring**

#### **Cache Status**
```yaml
- name: Log cache results
  run: |
    if [ "${{ inputs.clean_build }}" = "true" ]; then
      echo "🧹 Clean build requested - all caches skipped"
    else
      echo "ESP-IDF cache hit - ${{ steps.esp-idf-cache.outputs.cache-hit }}"
      echo "Python deps cache hit - ${{ steps.python-cache.outputs.cache-hit }}"
      echo "ccache cache hit - ${{ steps.ccache-cache.outputs.cache-hit }}"
    fi
```

#### **Cache Configuration**
```yaml
- name: Cache ESP-IDF and tools
  id: esp-idf-cache
  uses: actions/cache@v4
  if: ${{ !inputs.clean_build }}
  with:
    path: |
      ~/.espressif
      ~/esp
    key: >-
      esp-idf-${{ matrix.idf_version_docker }}-${{ runner.os }}-
      ${{ hashFiles('${{ env.ESP32_PROJECT_PATH }}/scripts/setup_common.sh') }}
    restore-keys: |
      esp-idf-${{ matrix.idf_version_docker }}-${{ runner.os }}-
      esp-idf-${{ matrix.idf_version_docker }}-
      esp-idf-
```

---

## 🤝 **Contributing**

### **Adding New Applications**

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

2. **Test Locally**
   ```bash
   # Test build locally
   ./examples/esp32/scripts/build_app.sh new_app_test Release
   ```

3. **Verify CI Integration**
   - Check matrix generation
   - Verify build process
   - Confirm artifact upload

### **Modifying CI Pipeline**

1. **Update Workflow**
   - Modify `.github/workflows/esp32-component-ci.yml`
   - Test changes locally
   - Verify GitHub Actions syntax

2. **Update Configuration**
   - Modify `examples/esp32/app_config.yml`
   - Test matrix generation
   - Verify build compatibility

3. **Test Changes**
   - Test locally first
   - Create test branch
   - Verify CI behavior

### **CI Pipeline Standards**

- **Error Handling** - Comprehensive error checking and reporting
- **Logging** - Detailed logs for debugging
- **Caching** - Intelligent caching for performance
- **Artifacts** - Complete artifact management
- **Documentation** - Clear usage instructions and examples

---

## 📄 **License**

This project is licensed under the GPL-3.0 License - see the [LICENSE](../../LICENSE) file for details.

---

## 🔗 **Related Documentation**

- [Main Project README](../../README.md) - Project overview and architecture
- [ESP32 Examples README](../../examples/esp32/README.md) - Examples overview and usage
- [Scripts Documentation](../../examples/esp32/scripts/README.md) - Build system scripts
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/) - ESP-IDF reference

---

<div align="center">

**🚀 Built with ❤️ for the HardFOC Community**

*Enterprise-grade CI/CD with professional automation and comprehensive testing*

</div>
