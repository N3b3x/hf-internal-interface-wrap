# ESP32 HardFOC Interface Wrapper - Scripts Overview

This document provides a comprehensive overview of all available scripts in the ESP32 scripts directory, their purposes, and how they work together.

---

**Navigation**: [← Previous: Port Detection](README_PORT_DETECTION.md) | [Back to Scripts](../README.md) | [Next: Build System →](README_BUILD_SYSTEM.md)

---

## Scripts Directory Structure

```
examples/esp32/scripts/
├── docs/                           # Documentation files
│   ├── README_SCRIPTS_OVERVIEW.md  # This file
│   ├── README_BUILD_SYSTEM.md      # Build system documentation
│   ├── README_PORT_DETECTION.md    # Port detection and troubleshooting
│   ├── README_FLASH_SYSTEM.md      # Flash and monitor system
│   ├── README_LOGGING_SYSTEM.md    # Logging and log management
│   ├── README_CONFIG_SYSTEM.md     # Configuration management
│   └── README_UTILITY_SCRIPTS.md  # Utility and helper scripts
├── flash_app.sh                    # Main flash and monitor script
├── build_app.sh                    # Build system script
├── manage_logs.sh                  # Log management and analysis
├── config_loader.sh                # Configuration loading and validation
├── detect_ports.sh                 # Port detection and troubleshooting
├── setup_common.sh                 # Common setup and environment
├── setup_ci.sh                     # CI/CD environment setup
├── setup_repo.sh                   # Repository setup and initialization
└── get_app_info.py                 # Python script for app information
```

## Script Categories

### 1. Core Development Scripts
- **`flash_app.sh`** - Main development workflow script
- **`build_app.sh`** - Build system management
- **`manage_logs.sh`** - Log file management and analysis

### 2. Configuration and Setup Scripts
- **`config_loader.sh`** - Configuration management
- **`setup_common.sh`** - Environment setup
- **`setup_ci.sh`** - CI/CD setup
- **`setup_repo.sh`** - Repository initialization

### 3. Utility and Helper Scripts
- **`detect_ports.sh`** - Port detection and troubleshooting
- **`get_app_info.py`** - App information extraction

## Quick Reference

### Current Command Syntax (Updated)

The scripts now use an **operation-first** approach for better usability:

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

### Essential Commands for Development

```bash
# Build an app
./scripts/build_app.sh <app_type> <build_type>

# Flash and monitor with logging (operation-first format - recommended)
./scripts/flash_app.sh flash_monitor <app_type> <build_type> --log

# Monitor existing firmware (standalone command)
./scripts/flash_app.sh monitor --log

# Manage logs
./scripts/manage_logs.sh list
./scripts/manage_logs.sh latest
./scripts/manage_logs.sh search "ERROR"

# Detect available ports
./scripts/detect_ports.sh --verbose

# Get help for any script
./scripts/flash_app.sh --help
./scripts/build_app.sh --help
./scripts/manage_logs.sh --help
```

### Common Workflows

#### 1. Development Workflow
```bash
# 1. Build the app
./scripts/build_app.sh gpio_test Release

# 2. Flash and monitor with logging (operation-first format)
./scripts/flash_app.sh flash_monitor gpio_test Release --log

# 3. View logs
./scripts/manage_logs.sh latest
```

#### 2. Debugging Workflow
```bash
# 1. Check available ports
./scripts/detect_ports.sh --test-connection

# 2. Monitor existing firmware (standalone command)
./scripts/flash_app.sh monitor --log

# 3. Search logs for errors
./scripts/manage_logs.sh search "ERROR"
```

#### 3. CI/CD Workflow
```bash
# 1. Setup CI environment
./scripts/setup_ci.sh

# 2. Build and test (operation-first format)
./scripts/build_app.sh gpio_test Release
./scripts/flash_app.sh flash gpio_test Release --log

# 3. Analyze results
./scripts/manage_logs.sh stats
```

## Script Dependencies

### Configuration Dependencies
- **`app_config.yml`** - Main configuration file
- **`config_loader.sh`** - Loads and validates configuration
- **Environment variables** - ESP-IDF, target, etc.

### Runtime Dependencies
- **ESP-IDF v5.5+** - Required for building and flashing
- **Python 3.6+** - Required for some utility scripts
- **Bash 4.0+** - Required for shell scripts
- **System tools** - `stat`, `find`, `grep`, `less`, etc.

## Error Handling and Troubleshooting

### Common Issues
1. **ESP-IDF not found** - Use `setup_common.sh` or source manually
2. **Port not accessible** - Use `detect_ports.sh --verbose`
3. **Build failures** - Check configuration with `config_loader.sh`
4. **Logging issues** - Verify log directory permissions
5. **Command syntax errors** - Use operation-first format: `./scripts/flash_app.sh flash gpio_test Release`

### Help and Documentation
All scripts support `--help` or `-h` for usage information:
```bash
# Get help for any script
./scripts/flash_app.sh --help
./scripts/build_app.sh --help
./scripts/manage_logs.sh --help
./scripts/detect_ports.sh --help
./scripts/setup_repo.sh --help
./scripts/setup_ci.sh --help
python3 ./scripts/get_app_info.py --help

# Quick help examples
./scripts/flash_app.sh -h
./scripts/build_app.sh -h
```

### Debug Mode
Most scripts support verbose output:
```bash
# Enable debug output
export DEBUG=1
./scripts/flash_app.sh flash_monitor gpio_test Release --log

# Check script help
./scripts/flash_app.sh --help
./scripts/manage_logs.sh --help
```

## Current Script Features and Improvements

### Recent Updates (August 2025)
- **Operation-first syntax** - Commands now start with the operation for better usability
- **Standalone monitor command** - `./scripts/flash_app.sh monitor` works without app/build type
- **Smart port detection** - Automatic ESP32 device detection on macOS and Linux
- **Comprehensive logging** - Timestamped logs with automatic rotation
- **Flexible parameter handling** - Both operation-first and legacy formats supported

### Key Improvements
- **Better UX**: Operation-first syntax is more intuitive
- **Port detection**: No more manual port specification needed
- **Logging system**: Comprehensive capture and management of all output
- **Error handling**: Detailed error messages with troubleshooting steps
- **Cross-platform**: Works consistently on Linux and macOS

## Integration with ESP-IDF

### ESP-IDF Commands Used
- `idf.py build` - Building projects
- `idf.py flash` - Flashing firmware
- `idf.py monitor` - Serial monitoring
- `idf.py set-target` - Setting target MCU

### Environment Variables
- `IDF_PATH` - ESP-IDF installation path
- `IDF_TARGET` - Target MCU (esp32c6)
- `ESPPORT` - Serial port for flashing/monitoring

## Best Practices

### 1. Always Use Logging
```bash
# Good: Always enable logging for debugging
./scripts/flash_app.sh gpio_test Release flash_monitor --log

# Bad: No logging makes debugging difficult
./scripts/flash_app.sh gpio_test Release flash_monitor
```

### 2. Use Appropriate Build Types
```bash
# Development: Use Debug for detailed logging
./scripts/build_app.sh gpio_test Debug

# Production: Use Release for performance
./scripts/build_app.sh gpio_test Release
```

### 3. Regular Log Maintenance
```bash
# Clean old logs weekly
./scripts/manage_logs.sh clean 7

# Check log statistics monthly
./scripts/manage_logs.sh stats
```

### 4. Port Detection
```bash
# Always verify ports before flashing
./scripts/detect_ports.sh --verbose

# Test port connectivity
./scripts/detect_ports.sh --test-connection
```

### 5. Use Help When Unsure
```bash
# Get help for any script
./scripts/flash_app.sh --help
./scripts/build_app.sh --help
./scripts/manage_logs.sh --help

# Quick help with -h
./scripts/detect_ports.sh -h
```

## Script Development

### Adding New Scripts
1. Follow the existing naming convention
2. Include comprehensive help text
3. Add error handling and validation
4. Document in appropriate README files
5. Test across different platforms

### Modifying Existing Scripts
1. Maintain backward compatibility
2. Update documentation
3. Test with existing workflows
4. Consider impact on other scripts

## Support and Maintenance

### Getting Help
1. Check script help: `./script_name.sh --help`
2. Review relevant README files
3. Check script source code for comments
4. Use debug mode: `export DEBUG=1`

### Reporting Issues
1. Include script name and version
2. Provide error messages and output
3. Specify platform and environment
4. Include relevant log files

### Contributing
1. Follow existing code style
2. Add comprehensive documentation
3. Include error handling
4. Test across platforms

## Version Information

- **Scripts Version**: 2.0.0
- **ESP-IDF Compatibility**: v5.5+
- **Platform Support**: Linux, macOS
- **Last Updated**: August 2025

For detailed information about specific scripts, see the individual README files in the `docs/` directory.

---

**Navigation**: [← Previous: Port Detection](README_PORT_DETECTION.md) | [Back to Scripts](../README.md) | [Next: Build System →](README_BUILD_SYSTEM.md)
