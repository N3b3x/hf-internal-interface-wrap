# 📱 ESP32 HardFOC Interface Wrapper - Flash System Documentation

This document provides comprehensive documentation for the `flash_app.sh` script, which is the main development workflow script for flashing and monitoring ESP32 apps.

---

**Navigation**: [← Previous: Build System](README_BUILD_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Logging System →](README_LOGGING_SYSTEM.md)

---

## 📋 Table of Contents

- [📋 Overview](#-overview)
- [🚀 Basic Usage](#️-basic-usage)
- [⚙️ Advanced Features](#️-advanced-features)
- [🔍 Troubleshooting](#️-troubleshooting)
- [📚 Getting Help](#️-getting-help)

## 📋 Overview

The `flash_app.sh` script is a feature-rich tool that handles the complete ESP32 development workflow:
- **Automatic port detection** - Finds ESP32 devices automatically
- **Smart build management** - Builds apps if needed
- **Comprehensive logging** - Captures all output for debugging
- **Error handling** - Provides detailed error messages and solutions
- **Cross-platform support** - Works on Linux and macOS

## Basic Usage

### Command Syntax
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

### Parameters
- **`operation`** - Operation to perform (`flash`, `monitor`, `flash_monitor`) - **Required first argument**
- **`app_type`** - Type of app to flash (e.g., `gpio_test`, `i2c_test`) - **Required for flash operations**
- **`build_type`** - Build configuration (`Debug` or `Release`) - **Required for flash operations**
- **`--log [log_name]`** - Enable logging with optional custom name

### Default Values
- **`app_type`**: Loaded from `app_config.yml` (for flash operations)
- **`build_type`**: Loaded from `app_config.yml` (for flash operations)
- **`operation`**: `flash_monitor` (flash then monitor)
- **`logging`**: Disabled by default

### Special Cases
- **`monitor` command**: Can be used standalone without app type or build type
- **`monitor --log`**: Enables logging for monitor-only operations
- **`monitor --log custom_name`**: Enables logging with custom name for monitor-only operations

## Operations

### 1. Flash Only (`flash`)
Flashes the firmware to the ESP32 device without starting the monitor.

```bash
./scripts/flash_app.sh flash gpio_test Release
```

**Use cases:**
- Quick firmware updates
- Batch flashing multiple devices
- CI/CD pipelines
- When you only need to update firmware

### 2. Monitor Only (`monitor`)
Starts the serial monitor for an already-flashed device. This command can be used standalone without specifying app type or build type.

```bash
# Monitor existing firmware (no app/build type needed)
./scripts/flash_app.sh monitor

# Monitor with logging
./scripts/flash_app.sh monitor --log

# Monitor with custom log name
./scripts/flash_app.sh monitor --log debug_session
```

**Use cases:**
- Debugging existing firmware
- Monitoring device output
- Testing without reflashing
- Development iteration
- Quick debugging sessions

### 3. Flash and Monitor (`flash_monitor`)
Flashes the firmware and then starts the monitor (default operation).

```bash
./scripts/flash_app.sh flash_monitor gpio_test Release
# or simply:
./scripts/flash_app.sh flash_monitor gpio_test Release
```

**Use cases:**
- Development workflow
- Testing new firmware
- Debugging with fresh code
- Most common development scenario

## Logging System

### Enabling Logging
The script includes a comprehensive logging system that captures all output for later analysis.

#### Basic Logging
```bash
# Enable logging with auto-generated filename
./scripts/flash_app.sh flash_monitor gpio_test Release --log
```

#### Custom Log Names
```bash
# Enable logging with custom name
./scripts/flash_app.sh flash_monitor gpio_test Release --log test_run_001
./scripts/flash_app.sh flash_monitor gpio_test Release --log debug_session
```

### Log File Naming Convention
- **Auto-generated**: `{app_type}_{build_type}_{timestamp}.log`
- **Custom name**: `{custom_name}_{timestamp}.log`
- **Timestamp format**: `YYYYMMDD_HHMMSS`

**Examples:**
```
gpio_test_Release_20250825_143022.log
test_run_001_20250825_143022.log
debug_session_20250825_143022.log
```

### Log File Contents
Each log file contains:
- **Header information** - Date, app type, build type, target, port
- **Build output** - Complete build process output
- **Flash output** - ESP-IDF flash operation output
- **Monitor output** - Complete ESP32 serial output
- **Error messages** - Any errors or warnings encountered

### Log Directory
Logs are stored in `examples/esp32/logs/` and are automatically:
- **Created** when logging is enabled
- **Rotated** (keeps last 50 logs)
- **Ignored by git** (via `.gitignore`)

## Port Detection and Management

### Automatic Port Detection
The script automatically detects ESP32 devices using platform-specific methods:

#### Linux
- Searches `/dev/ttyACM*` (ESP32-C6)
- Searches `/dev/ttyUSB*` (older ESP32)
- Uses `lsusb` for device identification
- Automatically fixes permissions

#### macOS
- Searches `/dev/cu.*` (callout devices, preferred)
- Searches `/dev/tty.*` (terminal devices)
- Identifies common ESP32 device patterns
- Handles permission issues

### Manual Port Specification
You can manually specify a port using the `ESPPORT` environment variable:

```bash
export ESPPORT="/dev/ttyACM0"
./scripts/flash_app.sh flash_monitor gpio_test Release
```

### Port Validation
The script validates ports by:
- Checking if the port exists
- Verifying port readability
- Testing port format compatibility
- Providing detailed error messages

## Build Management

### Automatic Building
If no valid build exists, the script automatically builds the app:

```bash
./scripts/flash_app.sh flash_monitor gpio_test Release
# If build doesn't exist:
# 1. Automatically calls build_app.sh
# 2. Builds the app
# 3. Proceeds with flashing
```

### Build Validation
The script validates builds by checking for:
- Main binary file (`.bin`)
- Bootloader binary
- Build artifacts (`.ninja`, `CMakeCache.txt`)
- Build timestamp files

### Build Directory Naming
Builds follow the pattern: `build_{app_type}_{build_type}`

**Examples:**
```
build_gpio_test_Release/
build_i2c_test_Debug/
build_ascii_art_Release/
```

## Error Handling and Troubleshooting

### Common Error Scenarios

#### 1. ESP-IDF Not Found
```bash
ERROR: ESP-IDF environment not found, attempting to source...
ERROR: ESP-IDF export.sh not found at $HOME/esp/esp-idf/export.sh
```

**Solutions:**
- Install ESP-IDF v5.5+
- Source ESP-IDF manually: `source ~/esp/esp-idf/export.sh`
- Use `setup_common.sh` script

#### 2. No Suitable Ports Found
```bash
ERROR: No suitable serial ports found!
```

**Solutions:**
- Connect ESP32 device via USB
- Check device enumeration: `./scripts/detect_ports.sh --verbose`
- Install USB-to-UART drivers
- Try different USB cable/port

#### 3. Port Not Accessible
```bash
ERROR: Port /dev/ttyACM0 is not readable
```

**Solutions:**
- **Linux**: `sudo chmod 666 /dev/ttyACM0`
- **Linux**: Add user to dialout group: `sudo usermod -a -G dialout $USER`
- **macOS**: Check System Preferences > Security & Privacy

#### 4. Build Validation Failed
```bash
NO VALID BUILD FOUND - STARTING AUTO-BUILD
```

**Solutions:**
- Let the script auto-build (recommended)
- Manually build: `./scripts/build_app.sh gpio_test Release`
- Check build configuration

### Debug Mode
Enable debug output for troubleshooting:

```bash
export DEBUG=1
./scripts/flash_app.sh flash_monitor gpio_test Release --log
```

## Advanced Features

### Environment Variables
The script respects several environment variables:

```bash
# ESP-IDF configuration
export IDF_PATH="/path/to/esp-idf"
export IDF_TARGET="esp32c6"

# Port specification
export ESPPORT="/dev/ttyACM0"

# Debug mode
export DEBUG=1
```

### Configuration Integration
The script integrates with the centralized configuration system:

- **`app_config.yml`** - App definitions and descriptions
- **`config_loader.sh`** - Configuration loading and validation
- **Build types** - Debug/Release configurations
- **Target settings** - MCU-specific configurations

### Cross-Platform Compatibility
The script automatically adapts to different platforms:

- **Operating system detection**
- **Platform-specific port detection**
- **Platform-specific permission handling**
- **Platform-specific error messages**

## Examples and Use Cases

### Development Workflow
```bash
# 1. Build the app
./scripts/build_app.sh gpio_test Release

# 2. Flash and monitor with logging
./scripts/flash_app.sh flash_monitor gpio_test Release --log

# 3. View the logs
./scripts/manage_logs.sh latest
```

### Monitor-Only Workflow
```bash
# 1. Monitor existing firmware
./scripts/flash_app.sh monitor

# 2. Monitor with logging
./scripts/flash_app.sh monitor --log

# 3. Monitor with custom log name
./scripts/flash_app.sh monitor --log debug_session

# 4. View the logs
./scripts/manage_logs.sh latest
```

### Debugging Session
```bash
# 1. Monitor existing firmware with logging
./scripts/flash_app.sh monitor --log debug_session

# 2. Search logs for errors
./scripts/manage_logs.sh search "ERROR"

# 3. View specific log file
./scripts/manage_logs.sh view debug_session_20250825_143022.log
```

### Batch Operations
```bash
# Flash multiple apps
for app in gpio_test i2c_test uart_test; do
    ./scripts/flash_app.sh flash $app Release --log batch_$(date +%Y%m%d)
done
```

### CI/CD Integration
```bash
# Automated testing with logging
./scripts/flash_app.sh flash_monitor gpio_test Release --log ci_test_$(date +%Y%m%d_%H%M%S)
```

## Integration with Other Scripts

### Build System
- **`build_app.sh`** - Called automatically when needed
- **`config_loader.sh`** - Provides configuration and validation
- **`app_config.yml`** - Defines available apps

### Log Management
- **`manage_logs.sh`** - Manages generated log files
- **Log rotation** - Automatically manages log file count
- **Log cleanup** - Prevents disk space issues

### Port Detection
- **`detect_ports.sh`** - Provides detailed port information
- **Port testing** - Validates port connectivity
- **Troubleshooting** - Helps resolve port issues

## Performance and Optimization

### Build Caching
- **ccache integration** - Faster rebuilds
- **Build validation** - Prevents unnecessary rebuilds
- **Smart detection** - Uses existing builds when possible

### Logging Performance
- **Efficient capture** - Minimal performance impact
- **Automatic rotation** - Prevents disk space issues
- **Compressed storage** - Efficient log file storage

## Security Considerations

### Permission Handling
- **Automatic permission fixing** - Linux port permissions
- **User group management** - dialout group for Linux
- **Secure defaults** - Minimal required permissions

### Log File Security
- **Git exclusion** - Logs are not committed to version control
- **Local storage** - Logs remain on local machine
- **Permission control** - Log files have appropriate permissions

## Troubleshooting Guide

### Quick Diagnostics
```bash
# 1. Check available apps
./scripts/flash_app.sh list

# 2. Check available ports
./scripts/detect_ports.sh --verbose

# 3. Test port connectivity
./scripts/detect_ports.sh --test-connection

# 4. Check ESP-IDF environment
echo $IDF_PATH
which idf.py
```

### Common Issues and Solutions

#### Issue: Script fails with "command not found"
**Solution**: Ensure ESP-IDF is sourced:
```bash
source ~/esp/esp-idf/export.sh
```

#### Issue: Permission denied on port
**Solution**: Fix port permissions:
```bash
sudo chmod 666 /dev/ttyACM0
sudo usermod -a -G dialout $USER
```

#### Issue: Build always fails
**Solution**: Check configuration:
```bash
./scripts/config_loader.sh --validate
```

#### Issue: Logs not being created
**Solution**: Check log directory permissions:
```bash
ls -la examples/esp32/logs/
mkdir -p examples/esp32/logs/
```

## Best Practices

### 1. Always Use Logging
```bash
# Good: Always enable logging for debugging
./scripts/flash_app.sh flash_monitor gpio_test Release --log

# Bad: No logging makes debugging difficult
./scripts/flash_app.sh flash_monitor gpio_test Release
```

### 2. Use Descriptive Log Names
```bash
# Good: Descriptive log names
./scripts/flash_app.sh flash_monitor gpio_test Release --log gpio_debug_session

# Bad: Generic log names
./scripts/flash_app.sh flash_monitor gpio_test Release --log test
```

### 3. Regular Log Maintenance
```bash
# Clean old logs weekly
./scripts/manage_logs.sh clean 7

# Check log statistics monthly
./scripts/manage_logs.sh stats
```

### 4. Port Validation
```bash
# Always verify ports before flashing
./scripts/detect_ports.sh --verbose

# Test port connectivity
./scripts/detect_ports.sh --test-connection
```

### 5. Use Monitor-Only for Quick Debugging
```bash
# Good: Quick debugging without rebuilding
./scripts/flash_app.sh monitor --log debug_session

# Bad: Rebuilding when you just want to monitor
./scripts/flash_app.sh monitor gpio_test Release --log debug_session
```

## Support and Maintenance

### Getting Help
1. **Check script help**: `./scripts/flash_app.sh --help`
2. **Review this documentation**
3. **Check script source code** for detailed comments
4. **Use debug mode**: `export DEBUG=1`

### Reporting Issues
1. **Include script name and version**
2. **Provide error messages and output**
3. **Specify platform and environment**
4. **Include relevant log files**

### Contributing
1. **Follow existing code style**
2. **Add comprehensive documentation**
3. **Include error handling**
4. **Test across platforms**

## Version Information

- **Script Version**: 2.0.0
- **ESP-IDF Compatibility**: v5.5+
- **Platform Support**: Linux, macOS
- **Last Updated**: August 2025

For additional information, see:
- [Scripts Overview](README_SCRIPTS_OVERVIEW.md)
- [Build System](README_BUILD_SYSTEM.md)
- [Port Detection](README_PORT_DETECTION.md)
- [Logging System](README_LOGGING_SYSTEM.md)

---

**Navigation**: [← Previous: Build System](README_BUILD_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Logging System →](README_LOGGING_SYSTEM.md)
