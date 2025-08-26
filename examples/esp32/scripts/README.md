# ESP32 Scripts - HardFOC Interface Wrapper

This directory contains a comprehensive suite of scripts for building, flashing, and managing ESP32 applications in the HardFOC Interface Wrapper project. All scripts feature intelligent configuration validation and cross-platform compatibility.

## 🎯 **Core Capabilities**

### **Intelligent Configuration Management**
- **Automatic Validation**: Scripts validate ESP-IDF version compatibility and build type support
- **Smart Defaults**: Intelligent fallbacks from `app_config.yml` with environment variable overrides
- **Configuration Intelligence**: Reads app-specific settings and validates against requirements
- **Error Prevention**: Prevents incompatible build/flash combinations with clear error messages

### **Cross-Platform Support**
- **Linux**: Full support with automatic dependency detection
- **macOS**: Native support with Homebrew integration
- **Linux**: Ubuntu 20.04+, Fedora, CentOS with full feature support
- **CI/CD**: Optimized for GitHub Actions and other CI environments

### **Professional Development Workflow**
- **Unified Interface**: Consistent command structure across all scripts
- **Comprehensive Logging**: Built-in logging system with automatic rotation and search
- **Port Detection**: Automatic ESP32 device detection and troubleshooting
- **Build Optimization**: ccache support and incremental build management

## 📁 **Script Categories**

### **1. Core Development Scripts**
| Script | Purpose | Key Features |
|--------|---------|--------------|
| **`build_app.sh`** | Build ESP32 applications | ESP-IDF validation, build type support, ccache integration |
| **`flash_app.sh`** | Flash and monitor firmware | Port detection, logging, operation-first syntax |
| **`manage_logs.sh`** | Log management and analysis | Search, statistics, cleanup, cross-log analysis |

### **2. Configuration and Setup Scripts**
| Script | Purpose | Key Features |
|--------|---------|--------------|
| **`config_loader.sh`** | Configuration management | YAML parsing, validation, fallback support |
| **`setup_common.sh`** | Environment setup | Cross-platform dependency installation, ESP-IDF setup |
| **`setup_repo.sh`** | Local development setup | Complete environment initialization, user-friendly |
| **`setup_ci.sh`** | CI/CD environment setup | Cache optimization, minimal dependencies |

### **3. Utility and Helper Scripts**
| Script | Purpose | Key Features |
|--------|---------|--------------|
| **`detect_ports.sh`** | Port detection and troubleshooting | Cross-platform detection, connection testing |
| **`get_app_info.py`** | App information extraction | CMake integration, configuration validation |

## 🚀 **Quick Start Guide**

### **1. Initial Setup**
```bash
# Set up complete development environment
./setup_repo.sh

# Verify installation
./build_app.sh list
```

### **2. Build and Flash Workflow**
```bash
# Build application
./build_app.sh gpio_test Release

# Flash and monitor with logging
./flash_app.sh flash_monitor gpio_test Release --log

# View latest logs
./manage_logs.sh latest
```

### **3. Development Commands**
```bash
# Check available options
./build_app.sh list
./flash_app.sh list

# Detect ESP32 devices
./detect_ports.sh --verbose

# Monitor existing firmware
./flash_app.sh monitor --log
```

## ⚙️ **Configuration System**

### **Centralized Configuration**
All scripts read from `app_config.yml` which defines:
- **App Definitions**: Source files, supported build types, ESP-IDF versions
- **Build Configuration**: Debug/Release settings, optimization flags
- **Global Defaults**: Default app, build type, target MCU, ESP-IDF version

### **Environment Variable Overrides**
```bash
# Override default app
export CONFIG_DEFAULT_APP="gpio_test"

# Override default build type
export CONFIG_DEFAULT_BUILD_TYPE="Debug"

# Override default ESP-IDF version
export CONFIG_DEFAULT_IDF_VERSION="release/v5.4"
```

### **Configuration Validation**
Scripts automatically validate:
- ✅ ESP-IDF version compatibility with app
- ✅ Build type support for app
- ✅ App existence in configuration
- ✅ Required dependencies and tools

## �� **Advanced Usage**

### **Build System Features**
```bash
# Clean build with no cache
./build_app.sh gpio_test Release --clean --no-cache

# Specific ESP-IDF version
./build_app.sh adc_test Debug release/v5.4

# List all available configurations
./build_app.sh list
```

### **Flash System Features**
```bash
# Operation-first syntax (recommended)
./flash_app.sh flash gpio_test Release
./flash_app.sh monitor --log debug_session

# Legacy syntax (still supported)
./flash_app.sh gpio_test Release flash_monitor --log
```

### **Logging System Features**
```bash
# Search across all logs
./manage_logs.sh search "ERROR"

# Clean old logs
./manage_logs.sh clean 7

# View log statistics
./manage_logs.sh stats
```

## 🛠️ **Dependencies and Requirements**

### **System Requirements**
- **Operating System**: Linux (Ubuntu 20.04+), macOS (10.15+)
- **Shell**: Bash 4.0+ with advanced features
- **Python**: Python 3.6+ for utility scripts
- **Package Manager**: apt, dnf, yum, or Homebrew

### **Core Dependencies**
- **ESP-IDF**: v5.5+ (automatically installed by setup scripts)
- **yq**: YAML processor for configuration parsing
- **Clang**: C++ toolchain (automatically installed)
- **System Tools**: git, cmake, ninja, ccache

### **Optional Dependencies**
- **ccache**: Build acceleration (enabled by default)
- **screen/tmux**: Terminal multiplexing for monitoring
- **less/more**: Pager for log viewing

## 🔍 **Troubleshooting and Support**

### **Common Issues**

#### **1. ESP-IDF Version Not Supported**
```bash
ERROR: App 'gpio_test' does not support ESP-IDF version 'release/v5.4'
Supported versions for 'gpio_test': release/v5.5
```
**Solution**: Use a supported ESP-IDF version or check app configuration.

#### **2. Build Type Not Supported**
```bash
ERROR: App 'gpio_test' does not support build type 'RelWithDebInfo'
Supported build types for 'gpio_test': Debug Release
```
**Solution**: Use a supported build type or check app configuration.

#### **3. Port Detection Issues**
```bash
# Check available ports
./detect_ports.sh --verbose

# Test port connectivity
./detect_ports.sh --test-connection
```

### **Debug and Verbose Mode**
```bash
# Enable debug output
export DEBUG=1
./flash_app.sh flash_monitor gpio_test Release --log

# Check script help
./flash_app.sh --help
./build_app.sh --help
```

### **Getting Help**
All scripts support comprehensive help:
```bash
# Get help for any script
./build_app.sh --help
./flash_app.sh --help
./manage_logs.sh --help
./detect_ports.sh --help
./setup_repo.sh --help
./setup_ci.sh --help
python3 ./get_app_info.py --help
```

## 📚 **Documentation Structure**

### **Comprehensive Guides**
- **[Scripts Overview](docs/README_SCRIPTS_OVERVIEW.md)**: Complete script reference and integration
- **[Build System](docs/README_BUILD_SYSTEM.md)**: Build configuration and optimization
- **[Flash System](docs/README_FLASH_SYSTEM.md)**: Flashing, monitoring, and port management
- **[Configuration System](docs/README_CONFIG_SYSTEM.md)**: Configuration management and validation
- **[Logging System](docs/README_LOGGING_SYSTEM.md)**: Log management and analysis
- **[Utility Scripts](docs/README_UTILITY_SCRIPTS.md)**: Helper scripts and automation
- **[Port Detection](docs/README_PORT_DETECTION.md)**: Device detection and troubleshooting

### **Quick Reference Cards**
- **Build Commands**: Common build scenarios and options
- **Flash Commands**: Flashing and monitoring workflows
- **Log Management**: Log viewing, search, and cleanup
- **Troubleshooting**: Common issues and solutions

## 🎯 **Best Practices**

### **1. Use Operation-First Syntax**
```bash
# Good: Clear and intuitive
./flash_app.sh flash gpio_test Release
./flash_app.sh monitor --log

# Avoid: Legacy format (less intuitive)
./flash_app.sh gpio_test Release flash
```

### **2. Always Enable Logging**
```bash
# Good: Logging for debugging
./flash_app.sh flash_monitor gpio_test Release --log

# Avoid: No logging makes debugging difficult
./flash_app.sh flash_monitor gpio_test Release
```

### **3. Validate Configuration First**
```bash
# Check available options
./build_app.sh list
./flash_app.sh list

# Validate specific configuration
./build_app.sh gpio_test Release release/v5.5
```

### **4. Regular Maintenance**
```bash
# Clean old logs weekly
./manage_logs.sh clean 7

# Check log statistics monthly
./manage_logs.sh stats
```

## 🔄 **Version Information**

- **Scripts Version**: 2.1.0
- **ESP-IDF Compatibility**: v5.5+
- **Platform Support**: Linux, macOS
- **Last Updated**: January 2025
- **Configuration**: YAML-based with intelligent validation

## 🤝 **Contributing and Support**

### **Development Guidelines**
- Follow existing code style and patterns
- Include comprehensive help text and documentation
- Add proper error handling and validation
- Test across different platforms and configurations

### **Issue Reporting**
- Include script name and version
- Provide complete error messages and output
- Specify platform, OS version, and environment
- Include relevant log files and configuration

### **Getting Help**
1. Check script help: `./script_name.sh --help`
2. Review relevant documentation files
3. Check script source code for detailed comments
4. Use debug mode: `export DEBUG=1`

---

*These scripts provide a robust development environment for ESP32 applications with intelligent configuration validation and comprehensive tooling.*
