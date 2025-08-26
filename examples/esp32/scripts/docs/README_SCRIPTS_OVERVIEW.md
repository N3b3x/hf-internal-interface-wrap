# ESP32 HardFOC Interface Wrapper - Scripts Overview

This document provides a comprehensive overview of all available scripts in the ESP32 scripts directory, their purposes, capabilities, dependencies, and how they work together to provide a professional development environment.

---

**Navigation**: [← Previous: Port Detection](README_PORT_DETECTION.md) | [Back to Scripts](../README.md) | [Next: Build System →](README_BUILD_SYSTEM.md)

---

## 📋 **Table of Contents**

- [📋 Overview](#-overview)
- [🏗️ Architecture and Design](#️-architecture-and-design)
- [📁 Script Categories and Capabilities](#️-script-categories-and-capabilities)
- [🔗 Dependencies and Integration](#️-dependencies-and-integration)
- [⚙️ Configuration Management](#️-configuration-management)
- [🚀 Usage Patterns and Workflows](#️-usage-patterns-and-workflows)
- [🛠️ Development and Maintenance](#️-development-and-maintenance)
- [🔍 Troubleshooting and Support](#️-troubleshooting-and-support)

## 📋 **Overview**

The ESP32 scripts directory contains a comprehensive suite of scripts designed to streamline ESP32 development workflows. These scripts provide intelligent configuration management, cross-platform compatibility, and robust error handling to ensure reliable development operations.

### **Core Design Principles**
- **Configuration-Driven**: All behavior controlled through centralized YAML configuration
- **Intelligent Validation**: Automatic compatibility checking and error prevention
- **Cross-Platform**: Consistent behavior across Linux and macOS
- **Professional Quality**: Comprehensive error handling, logging, and user feedback
- **Modular Design**: Scripts can be used independently or as part of integrated workflows

### **Key Capabilities**
- **Build Management**: Intelligent ESP-IDF integration with validation
- **Flash Operations**: Port detection, firmware flashing, and monitoring
- **Log Management**: Comprehensive logging with search and analysis
- **Environment Setup**: Automated dependency installation and configuration
- **Port Detection**: Cross-platform ESP32 device identification
- **Configuration Validation**: YAML-based configuration with intelligent fallbacks

## 🏗️ **Architecture and Design**

### **Script Organization Structure**
```
examples/esp32/scripts/
├── docs/                           # Comprehensive documentation
│   ├── README_SCRIPTS_OVERVIEW.md  # This file - complete overview
│   ├── README_BUILD_SYSTEM.md      # Build system architecture
│   ├── README_FLASH_SYSTEM.md      # Flash and monitor system
│   ├── README_CONFIG_SYSTEM.md     # Configuration management
│   ├── README_LOGGING_SYSTEM.md    # Log management system
│   ├── README_UTILITY_SCRIPTS.md   # Utility and helper scripts
│   └── README_PORT_DETECTION.md    # Port detection system
├── flash_app.sh                    # Main flash and monitor script
├── build_app.sh                    # Build system management
├── manage_logs.sh                  # Log management and analysis
├── config_loader.sh                # Configuration loading and validation
├── detect_ports.sh                 # Port detection and troubleshooting
├── setup_common.sh                 # Common setup and environment
├── setup_ci.sh                     # CI/CD environment setup
├── setup_repo.sh                   # Repository setup and initialization
└── get_app_info.py                 # Python script for app information
```

### **Architecture Patterns**
- **Configuration-First**: All scripts read from centralized `app_config.yml`
- **Validation Layer**: `config_loader.sh` provides validation and fallbacks
- **Modular Functions**: Common functionality shared through `setup_common.sh`
- **Error Handling**: Consistent error reporting and troubleshooting guidance
- **Logging Integration**: Unified logging system across all scripts

### **Data Flow Architecture**
```
app_config.yml → config_loader.sh → Individual Scripts → User Interface
     ↓                    ↓              ↓              ↓
Configuration    Validation &      Script Logic    User Commands
Definitions      Fallbacks        & Execution      & Output
```

## 📁 **Script Categories and Capabilities**

### **1. Core Development Scripts**

#### **`build_app.sh` - Build System Management**
**Purpose**: Intelligent ESP32 application building with configuration validation

**Key Capabilities**:
- ESP-IDF version compatibility validation
- Build type support verification
- ccache integration for build acceleration
- Clean build options and cache management
- Cross-platform build environment setup

**Dependencies**:
- ESP-IDF v5.5+ installation
- `config_loader.sh` for configuration
- `yq` for YAML parsing (with fallback)
- System build tools (cmake, ninja, ccache)

**Use Cases**:
- Development builds with debug symbols
- Production builds with optimization
- CI/CD automated builds
- Multi-ESP-IDF version testing

#### **`flash_app.sh` - Flash and Monitor Operations**
**Purpose**: Comprehensive firmware flashing and device monitoring

**Key Capabilities**:
- Automatic ESP32 port detection
- Operation-first command syntax
- Comprehensive logging integration
- Port connectivity testing
- Cross-platform port management

**Dependencies**:
- ESP-IDF tools (esptool, idf.py)
- `detect_ports.sh` for port detection
- `manage_logs.sh` for logging
- Serial port access permissions

**Use Cases**:
- Firmware deployment to devices
- Development debugging and monitoring
- Production firmware updates
- Automated testing and validation

#### **`manage_logs.sh` - Log Management System**
**Purpose**: Comprehensive log file management and analysis

**Key Capabilities**:
- Log file listing and statistics
- Cross-log search and pattern matching
- Automatic log rotation and cleanup
- Log file viewing and analysis
- Storage optimization and management

**Dependencies**:
- System tools (find, grep, stat, less)
- Log directory structure
- File system permissions
- Text processing utilities

**Use Cases**:
- Development debugging and analysis
- Production issue investigation
- Log maintenance and cleanup
- Performance monitoring and analysis

### **2. Configuration and Setup Scripts**

#### **`config_loader.sh` - Configuration Management**
**Purpose**: Centralized configuration loading and validation

**Key Capabilities**:
- YAML configuration parsing with `yq`
- Fallback parsing for systems without `yq`
- Configuration validation and error checking
- Environment variable overrides
- Cross-platform path handling

**Dependencies**:
- `yq` YAML processor (preferred)
- Bash 4.0+ with advanced features
- YAML configuration file structure
- Environment variable support

**Use Cases**:
- Script configuration loading
- Configuration validation
- Default value management
- Environment-specific overrides

#### **`setup_common.sh` - Environment Setup Functions**
**Purpose**: Shared setup and environment configuration functions

**Key Capabilities**:
- Cross-platform dependency installation
- ESP-IDF installation and configuration
- Clang toolchain setup
- Python dependency management
- Cache optimization functions

**Dependencies**:
- System package managers (apt, dnf, yum, brew)
- Internet connectivity for downloads
- Sudo access for system packages
- Git for repository cloning

**Use Cases**:
- Local development environment setup
- CI/CD environment preparation
- Dependency installation automation
- Environment configuration management

#### **`setup_repo.sh` - Local Development Setup**
**Purpose**: Complete local development environment initialization

**Key Capabilities**:
- User-friendly setup process
- Interactive confirmation and guidance
- Complete dependency installation
- Environment variable configuration
- Development alias setup

**Dependencies**:
- `setup_common.sh` functions
- User account (non-root)
- Sudo access for packages
- Interactive terminal

**Use Cases**:
- Initial development environment setup
- New developer onboarding
- Environment refresh and updates
- Dependency troubleshooting

#### **`setup_ci.sh` - CI/CD Environment Setup**
**Purpose**: Optimized CI/CD environment preparation

**Key Capabilities**:
- Cache-aware installation
- Minimal dependency installation
- CI-specific optimizations
- Cache statistics and reporting
- Non-interactive operation

**Dependencies**:
- `setup_common.sh` functions
- CI environment variables
- Cache directory access
- Non-interactive terminal

**Use Cases**:
- GitHub Actions workflow setup
- GitLab CI environment preparation
- Jenkins build environment setup
- Automated testing environment

### **3. Utility and Helper Scripts**

#### **`detect_ports.sh` - Port Detection and Troubleshooting**
**Purpose**: Cross-platform ESP32 device detection and port management

**Key Capabilities**:
- Automatic ESP32 device detection
- Cross-platform port identification
- Port connectivity testing
- Permission and access troubleshooting
- Detailed device information

**Dependencies**:
- System USB drivers
- Serial port access permissions
- System tools (lsusb, system_profiler)
- Bash shell with advanced features

**Use Cases**:
- Device connection troubleshooting
- Port identification for flashing
- Development environment setup
- Production deployment verification

#### **`get_app_info.py` - App Information Extraction**
**Purpose**: Python-based app configuration information extraction

**Key Capabilities**:
- CMake integration support
- Configuration validation
- App information extraction
- Cross-platform Python execution
- YAML configuration parsing

**Dependencies**:
- Python 3.6+
- PyYAML library
- YAML configuration file
- File system access

**Use Cases**:
- CMake build system integration
- Configuration validation
- App information extraction
- Automated build processes

## 🔗 **Dependencies and Integration**

### **Core Dependencies**

#### **System-Level Dependencies**
- **Operating System**: Linux (Ubuntu 20.04+), macOS (10.15+)
- **Shell Environment**: Bash 4.0+ with advanced features
- **Package Managers**: apt, dnf, yum, or Homebrew
- **System Tools**: git, cmake, ninja, ccache, stat, find, grep

#### **Development Dependencies**
- **ESP-IDF**: v5.5+ (automatically installed by setup scripts)
- **Clang Toolchain**: C++ compiler, formatter, and analyzer
- **Python**: Python 3.6+ with PyYAML support
- **yq**: YAML processor for configuration parsing

#### **Optional Dependencies**
- **ccache**: Build acceleration and caching
- **screen/tmux**: Terminal multiplexing for monitoring
- **less/more**: Pager for log viewing and navigation

### **Integration Points**

#### **ESP-IDF Integration**
- Direct integration with ESP-IDF build system
- Automatic ESP-IDF environment sourcing
- Target-specific configuration support
- Build type optimization integration

#### **Configuration Integration**
- Centralized YAML configuration management
- Environment variable override support
- Cross-script configuration sharing
- Validation and error prevention

#### **Logging Integration**
- Unified logging system across all scripts
- Automatic log rotation and management
- Cross-log search and analysis
- Performance monitoring and debugging

## ⚙️ **Configuration Management**

### **Configuration File Structure**
The `app_config.yml` file provides centralized configuration for all scripts:

```yaml
# Global metadata and defaults
metadata:
  default_app: "ascii_art"
  default_build_type: "Release"
  target: "esp32c6"
  idf_versions: ["release/v5.5", "release/v5.4"]

# App-specific configurations
apps:
  gpio_test:
    description: "GPIO peripheral testing app"
    source_file: "GpioComprehensiveTest.cpp"
    category: "peripheral"
    build_types: ["Debug", "Release"]
    idf_versions: ["release/v5.5"]
    ci_enabled: true
    featured: true

# Build configuration
build_config:
  build_types:
    Debug:
      cmake_build_type: "Debug"
      optimization: "-O0"
      debug_level: "-g3"
    Release:
      cmake_build_type: "Release"
      optimization: "-O2"
      debug_level: "-g"
```

### **Configuration Loading Process**
1. **Primary Method**: Uses `yq` for reliable YAML parsing
2. **Fallback Method**: Basic parsing with grep/sed for systems without `yq`
3. **Validation**: Checks configuration integrity and compatibility
4. **Environment Overrides**: Allows environment variable customization
5. **Error Handling**: Provides clear error messages for configuration issues

### **Configuration Validation**
Scripts automatically validate:
- ESP-IDF version compatibility with app
- Build type support for app
- App existence in configuration
- Required dependencies and tools
- Configuration file integrity

## 🚀 **Usage Patterns and Workflows**

### **Development Workflow**
```bash
# 1. Initial setup
./setup_repo.sh

# 2. Build application
./build_app.sh gpio_test Release

# 3. Flash and monitor with logging
./flash_app.sh flash_monitor gpio_test Release --log

# 4. View and analyze logs
./manage_logs.sh latest
./manage_logs.sh search "ERROR"
```

### **Debugging Workflow**
```bash
# 1. Check available ports
./detect_ports.sh --verbose

# 2. Monitor existing firmware
./flash_app.sh monitor --log

# 3. Search logs for issues
./manage_logs.sh search "ERROR"
./manage_logs.sh search "WARNING"
```

### **CI/CD Workflow**
```bash
# 1. Setup CI environment
./setup_ci.sh

# 2. Build and test
./build_app.sh gpio_test Release
./flash_app.sh flash gpio_test Release --log

# 3. Analyze results
./manage_logs.sh stats
./manage_logs.sh latest
```

### **Production Deployment Workflow**
```bash
# 1. Build production firmware
./build_app.sh gpio_test Release

# 2. Verify target devices
./detect_ports.sh --test-connection

# 3. Deploy firmware
./flash_app.sh flash gpio_test Release --log production_deploy

# 4. Verify deployment
./flash_app.sh monitor --log production_verify
```

## 🛠️ **Development and Maintenance**

### **Script Development Guidelines**

#### **Code Quality Standards**
- **Error Handling**: Comprehensive error checking and user feedback
- **Help Documentation**: Detailed help text with examples
- **Cross-Platform**: Consistent behavior across operating systems
- **Performance**: Efficient execution and resource usage
- **Maintainability**: Clear code structure and documentation

#### **Adding New Scripts**
1. Follow existing naming conventions and patterns
2. Include comprehensive help text and documentation
3. Add proper error handling and validation
4. Document in appropriate README files
5. Test across different platforms and configurations

#### **Modifying Existing Scripts**
1. Maintain backward compatibility where possible
2. Update documentation and help text
3. Test with existing workflows and configurations
4. Consider impact on other scripts and dependencies

### **Testing and Validation**

#### **Testing Requirements**
- **Cross-Platform Testing**: Linux, macOS
- **Configuration Testing**: Various app_config.yml configurations
- **Error Handling Testing**: Invalid inputs and error conditions
- **Integration Testing**: Script interaction and dependencies
- **Performance Testing**: Resource usage and execution time

#### **Validation Procedures**
- **Syntax Checking**: Bash syntax validation
- **Function Testing**: Individual function verification
- **Integration Testing**: End-to-end workflow testing
- **Documentation Review**: Help text and documentation accuracy
- **User Experience Testing**: Usability and error message clarity

## 🔍 **Troubleshooting and Support**

### **Common Issues and Solutions**

#### **1. Configuration Issues**
**Problem**: Scripts fail to load configuration
**Symptoms**: "Configuration file not found" or "Invalid configuration" errors
**Solutions**:
- Verify `app_config.yml` exists and is accessible
- Check file permissions and ownership
- Validate YAML syntax with `yq eval app_config.yml`
- Use `./config_loader.sh` for debugging

#### **2. ESP-IDF Issues**
**Problem**: Build or flash operations fail
**Symptoms**: "ESP-IDF not found" or "idf.py command not found" errors
**Solutions**:
- Run `./setup_repo.sh` to install ESP-IDF
- Source ESP-IDF environment: `source ~/esp/esp-idf/export.sh`
- Check ESP-IDF version compatibility
- Verify target MCU configuration

#### **3. Port Detection Issues**
**Problem**: Scripts cannot find ESP32 devices
**Symptoms**: "No ports detected" or "Port not accessible" errors
**Solutions**:
- Run `./detect_ports.sh --verbose` for detailed information
- Check USB cable connections and drivers
- Verify device permissions and udev rules
- Test with `./detect_ports.sh --test-connection`

#### **4. Permission Issues**
**Problem**: Scripts fail due to insufficient permissions
**Symptoms**: "Permission denied" or "Access denied" errors
**Solutions**:
- Check file and directory permissions
- Verify user group membership (dialout, tty)
- Add udev rules for ESP32 devices
- Use `sudo` for system-level operations when appropriate

### **Debug and Verbose Mode**

#### **Enabling Debug Output**
```bash
# Enable debug mode for all scripts
export DEBUG=1

# Enable verbose output for specific scripts
./detect_ports.sh --verbose
./flash_app.sh flash_monitor gpio_test Release --log
```

#### **Debug Information Available**
- Configuration loading and validation details
- Port detection and connection testing results
- Build process and dependency information
- Error context and troubleshooting suggestions

### **Getting Help and Support**

#### **Script Help System**
All scripts provide comprehensive help:
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

#### **Documentation Resources**
- **Script Help**: Built-in help text with examples
- **README Files**: Comprehensive documentation in docs/ directory
- **Source Code**: Detailed comments and error handling
- **Configuration Examples**: Sample configurations and use cases

#### **Issue Reporting Guidelines**
When reporting issues, include:
1. **Script Name and Version**: Which script and version
2. **Error Messages**: Complete error output and context
3. **Environment**: OS, version, and configuration details
4. **Reproduction Steps**: Clear steps to reproduce the issue
5. **Log Files**: Relevant log files and configuration
6. **Expected Behavior**: What you expected to happen

## 📊 **Performance and Optimization**

### **Build Performance**
- **ccache Integration**: Automatic build acceleration
- **Incremental Builds**: Smart dependency tracking
- **Parallel Compilation**: Multi-core build optimization
- **Cache Management**: Intelligent cache cleanup and optimization

### **Execution Performance**
- **Efficient Parsing**: Optimized YAML and configuration parsing
- **Minimal Dependencies**: Reduced external tool requirements
- **Smart Fallbacks**: Efficient fallback mechanisms
- **Resource Management**: Optimized memory and CPU usage

### **Cache Optimization**
- **ESP-IDF Cache**: Toolchain and component caching
- **Build Cache**: ccache integration and management
- **Python Cache**: pip and dependency caching
- **System Cache**: Package manager and system cache utilization

## 🔄 **Version Information and Compatibility**

### **Current Version**
- **Scripts Version**: 2.1.0
- **ESP-IDF Compatibility**: v5.5+
- **Platform Support**: Linux, macOS
- **Last Updated**: January 2025

### **Version Compatibility Matrix**
| Script Category | ESP-IDF v5.5 | ESP-IDF v5.4 | ESP-IDF v5.3 |
|-----------------|---------------|---------------|---------------|
| Build System    | ✅ Full       | ⚠️ Limited    | ❌ Not Supported |
| Flash System    | ✅ Full       | ⚠️ Limited    | ❌ Not Supported |
| Configuration   | ✅ Full       | ⚠️ Limited    | ❌ Not Supported |
| Setup Scripts   | ✅ Full       | ⚠️ Limited    | ❌ Not Supported |

**Legend:**
- ✅ **Full**: Supports Debug and Release builds
- ⚠️ **Limited**: Supports Debug builds only
- ❌ **Not Supported**: Version not configured in the system

**Current ESP-IDF Version Support:**
- **ESP-IDF v5.5**: Full support with Debug and Release builds for all apps
- **ESP-IDF v5.4**: Limited support with Debug builds only (Release builds not configured)
- **ESP-IDF v5.3**: Not currently configured or supported

**Available ESP-IDF Versions:**
- **`release/v5.5`**: Current stable release (✅ Full support)
- **`release/v5.4`**: Previous stable release (⚠️ Limited support)
- **`release/v5.3`**: Older stable release (❌ Not configured)
- **`master`**: Development branch (❌ Not configured)

**Note**: To add support for ESP-IDF v5.4 (Release builds) or v5.3, update the `examples/esp32/app_config.yml` file in the `metadata.idf_versions` and `metadata.default_build_types` sections.

### **Backward Compatibility**
- **Configuration Format**: Backward compatible with v2.0.x
- **Command Syntax**: Legacy syntax still supported
- **Environment Variables**: Compatible with existing setups
- **Dependencies**: Maintains compatibility with older ESP-IDF versions

## 🚀 **Future Development and Roadmap**

### **Planned Enhancements**
- **Enhanced Port Detection**: Improved device identification and troubleshooting
- **Advanced Logging**: Structured logging and log analysis tools
- **Configuration Validation**: Enhanced YAML schema validation
- **Performance Monitoring**: Build and execution performance metrics
- **Plugin System**: Extensible script functionality

### **Community Contributions**
- **Code Contributions**: Guidelines for contributing improvements
- **Documentation**: Help with documentation and examples
- **Testing**: Cross-platform testing and validation
- **Feature Requests**: Community-driven feature development

---

**Navigation**: [← Previous: Port Detection](README_PORT_DETECTION.md) | [Back to Scripts](../README.md) | [Next: Build System →](README_BUILD_SYSTEM.md)
