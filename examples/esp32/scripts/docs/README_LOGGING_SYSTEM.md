# ESP32 HardFOC Interface Wrapper - Logging System Documentation

This document provides comprehensive documentation for the logging system, which includes both the logging capabilities built into `flash_app.sh` and the dedicated log management tool `manage_logs.sh`.

---

**Navigation**: [← Previous: Flash System](README_FLASH_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Configuration System →](README_CONFIG_SYSTEM.md)

---

## Overview

The logging system provides comprehensive capture and management of all ESP32 development activities:
- **Automatic log creation** - Timestamped log files for each session
- **Rich metadata** - Complete context for each log entry
- **Log management** - Tools for viewing, searching, and maintaining logs
- **Automatic rotation** - Prevents disk space issues
- **Git integration** - Logs are automatically excluded from version control

## Logging Architecture

### Components
1. **`flash_app.sh`** - Creates logs during flash/monitor operations
2. **`manage_logs.sh`** - Manages and analyzes existing logs
3. **Log directory** - Centralized storage location
4. **Log rotation** - Automatic cleanup system

### Log File Structure
```
examples/esp32/logs/
├── gpio_test_Release_20250825_143022.log
├── i2c_test_Debug_20250825_150145.log
├── test_run_001_20250825_151200.log
└── ...
```

## Enabling Logging

### Basic Logging
Enable logging with auto-generated filenames:

```bash
# Enable logging for any operation
./scripts/flash_app.sh gpio_test Release flash_monitor --log

# Enable logging for specific operations
./scripts/flash_app.sh gpio_test Release flash --log
./scripts/flash_app.sh gpio_test Release monitor --log
```

### Custom Log Names
Use descriptive names for better organization:

```bash
# Development sessions
./scripts/flash_app.sh gpio_test Release flash_monitor --log gpio_debug_session

# Test runs
./scripts/flash_app.sh i2c_test Release flash_monitor --log i2c_test_run_001

# CI/CD builds
./scripts/flash_app.sh all Release flash --log ci_build_$(date +%Y%m%d_%H%M%S)
```

### Log File Naming Convention

#### Auto-generated Names
Format: `{app_type}_{build_type}_{timestamp}.log`

**Examples:**
```
gpio_test_Release_20250825_143022.log
i2c_test_Debug_20250825_150145.log
uart_test_Release_20250825_151200.log
```

#### Custom Names
Format: `{custom_name}_{timestamp}.log`

**Examples:**
```
debug_session_20250825_143022.log
test_run_001_20250825_150145.log
ci_build_20250825_151200.log
```

#### Timestamp Format
- **Format**: `YYYYMMDD_HHMMSS`
- **Example**: `20250825_143022` (August 25, 2025, 14:30:22)

## Log File Contents

### Header Information
Each log file begins with comprehensive metadata:

```
======================================================
ESP32 HardFOC Interface Wrapper - Monitor Log
======================================================
Date: 2025-08-25 14:30:22
App Type: gpio_test
Build Type: Release
Operation: flash_monitor
Target: esp32c6
Port: /dev/ttyACM0
Build Directory: build_gpio_test_Release
Project Name: esp32_iid_gpio_test_app
======================================================
```

### Content Sections
1. **Build Output** - Complete build process (if auto-built)
2. **Flash Output** - ESP-IDF flash operation details
3. **Monitor Output** - Complete ESP32 serial output
4. **Error Messages** - Any errors or warnings encountered
5. **System Information** - Platform, versions, configurations

### Log File Size
- **Small logs** (4-10KB): Flash operations, build summaries
- **Medium logs** (10-100KB): Short monitor sessions
- **Large logs** (100KB-1MB+): Extended monitor sessions, debugging

## Log Management with `manage_logs.sh`

### Command Overview
```bash
./scripts/manage_logs.sh [command] [options]
```

### Available Commands

#### 1. List Logs (`list`)
Display all available log files with details:

```bash
./scripts/manage_logs.sh list
```

**Output:**
```
Found 3 log file(s):

Filename                                           Date                 Time            Size      
---------                                          ----                 ----            ----      
gpio_test_Release_20250825_143022.log              2025-08-25           14:30:22        712K      
i2c_test_Debug_20250825_150145.log                 2025-08-25           15:01:45        156K      
test_run_001_20250825_151200.log                   2025-08-25           15:12:00        89K      
```

#### 2. View Log (`view`)
View a specific log file with context:

```bash
# View by filename
./scripts/manage_logs.sh view gpio_test_Release_20250825_143022.log

# View by custom name
./scripts/manage_logs.sh view debug_session_20250825_143022.log
```

**Features:**
- File information (size, modification date)
- Uses `less` for better viewing experience
- Falls back to `cat` if `less` unavailable

#### 3. Search Logs (`search`)
Search across all log files for specific patterns:

```bash
# Search for errors
./scripts/manage_logs.sh search "ERROR"

# Search for specific patterns
./scripts/manage_logs.sh search "GPIO.*failed"
./scripts/manage_logs.sh search "I2C.*timeout"

# Search for build information
./scripts/manage_logs.sh search "Build completed"
```

**Output:**
```
Searching for pattern: 'ERROR'
Searching across 3 log file(s)...

Found 2 match(es) in: gpio_test_Release_20250825_143022.log
Matches:
  1: E (12345) GPIO_Test: [ERROR] GPIO initialization failed
  2: E (12346) GPIO_Test: [ERROR] Port configuration error
  ... and 0 more matches

Found 1 match(es) in: i2c_test_Debug_20250825_150145.log
Matches:
  1: E (12347) I2C_Test: [ERROR] I2C bus error
```

#### 4. Log Statistics (`stats`)
Display comprehensive log statistics:

```bash
./scripts/manage_logs.sh stats
```

**Output:**
```
Log Statistics:
======================================================
Total log files: 3
Total size: 957.25 KB
Oldest log: gpio_test_Release_20250825_143022.log (2025-08-25)
Newest log: test_run_001_20250825_151200.log (2025-08-25)

Examples breakdown:
  2 gpio
  1 i2c
```

#### 5. Latest Log (`latest`)
Show information about the most recent log file:

```bash
./scripts/manage_logs.sh latest
```

**Output:**
```
Latest log file: test_run_001_20250825_151200.log
Path: /home/user/project/examples/esp32/logs/test_run_001_20250825_151200.log
Size: 89K
Modified: 2025-08-25 15:12:00.123456789 -0600

Last 20 lines:
======================================================
I (12345) GPIO_Test: Test completed successfully
I (12346) GPIO_Test: All tests passed
...
```

#### 6. Clean Logs (`clean`)
Remove old log files to free disk space:

```bash
# Clean logs older than 7 days
./scripts/manage_logs.sh clean 7

# Clean logs older than 30 days (default)
./scripts/manage_logs.sh clean 30

# Clean logs older than 1 day
./scripts/manage_logs.sh clean 1
```

**Features:**
- Interactive confirmation before deletion
- Shows which files will be deleted
- Safe operation with user confirmation

## Log Rotation and Maintenance

### Automatic Rotation
The system automatically manages log files:
- **Maximum logs**: 50 files
- **Rotation trigger**: When limit exceeded
- **Cleanup method**: Removes oldest logs first
- **Cleanup timing**: During new log creation

### Manual Cleanup
Use the clean command for proactive maintenance:

```bash
# Weekly cleanup (recommended)
./scripts/manage_logs.sh clean 7

# Monthly cleanup
./scripts/manage_logs.sh clean 30

# Aggressive cleanup
./scripts/manage_logs.sh clean 1
```

### Log Directory Management
```bash
# Check log directory status
ls -la examples/esp32/logs/

# Create log directory if missing
mkdir -p examples/esp32/logs/

# Check disk usage
du -sh examples/esp32/logs/
```

## Advanced Logging Features

### Environment Variables
Control logging behavior with environment variables:

```bash
# Enable debug logging
export DEBUG=1

# Custom log directory
export LOG_DIR="/custom/log/path"

# Custom log retention
export MAX_LOGS=100
```

### Log Filtering
Filter logs by various criteria:

```bash
# Filter by app type
find examples/esp32/logs/ -name "*gpio*" -type f

# Filter by date
find examples/esp32/logs/ -name "*.log" -mtime -7

# Filter by size
find examples/esp32/logs/ -name "*.log" -size +100k
```

### Log Analysis Scripts
Create custom analysis scripts:

```bash
#!/bin/bash
# Custom log analysis script

# Find all logs from today
today=$(date +%Y%m%d)
logs=$(find examples/esp32/logs/ -name "*${today}*.log" -type f)

# Analyze each log
for log in $logs; do
    echo "Analyzing: $log"
    
    # Count errors
    error_count=$(grep -c "ERROR" "$log" 2>/dev/null || echo "0")
    echo "  Errors: $error_count"
    
    # Count warnings
    warning_count=$(grep -c "WARN" "$log" 2>/dev/null || echo "0")
    echo "  Warnings: $warning_count"
    
    echo ""
done
```

## Integration with Development Workflow

### Development Session
```bash
# 1. Start development with logging
./scripts/flash_app.sh gpio_test Release flash_monitor --log dev_session_$(date +%Y%m%d)

# 2. Work with the device...

# 3. Check logs for issues
./scripts/manage_logs.sh search "ERROR"

# 4. View latest log
./scripts/manage_logs.sh latest
```

### Debugging Session
```bash
# 1. Monitor with logging
./scripts/flash_app.sh gpio_test Release monitor --log debug_$(date +%H%M)

# 2. Reproduce issue...

# 3. Search logs for specific patterns
./scripts/manage_logs.sh search "GPIO.*interrupt"
./scripts/manage_logs.sh search "I2C.*timeout"
```

### Testing Session
```bash
# 1. Run tests with logging
./scripts/flash_app.sh gpio_test Release flash_monitor --log test_run_$(date +%Y%m%d_%H%M)

# 2. Analyze test results
./scripts/manage_logs.sh search "PASSED"
./scripts/manage_logs.sh search "FAILED"
./scripts/manage_logs.sh search "SUCCESS"
```

### CI/CD Integration
```bash
# 1. Automated testing with logging
./scripts/flash_app.sh all Release flash --log ci_$(date +%Y%m%d_%H%M%S)

# 2. Analyze results
./scripts/manage_logs.sh stats
./scripts/manage_logs.sh search "ERROR"
./scripts/manage_logs.sh search "FAILED"

# 3. Archive important logs
cp examples/esp32/logs/ci_*.log /path/to/archive/
```

## Log File Formats and Compatibility

### Text Format
- **Encoding**: UTF-8
- **Line endings**: Unix (LF) or Windows (CRLF)
- **Viewers**: Any text editor, `less`, `cat`, `grep`

### Binary Compatibility
- **ESP-IDF output**: Text-based, fully compatible
- **Serial monitor**: Text-based, fully compatible
- **Build output**: Text-based, fully compatible

### Cross-Platform Support
- **Linux**: Full support, automatic permission handling
- **macOS**: Full support, system permission handling
- **Windows**: Limited support (WSL recommended)

## Performance Considerations

### Log File Size Impact
- **Small logs** (< 100KB): Minimal impact
- **Medium logs** (100KB-1MB): Moderate impact
- **Large logs** (> 1MB): May affect system performance

### Optimization Strategies
```bash
# Regular cleanup
./scripts/manage_logs.sh clean 7

# Monitor disk usage
du -sh examples/esp32/logs/

# Use descriptive names to avoid confusion
./scripts/flash_app.sh gpio_test Release flash_monitor --log gpio_debug_$(date +%H%M)
```

### Storage Requirements
- **Typical session**: 50-500KB
- **Extended debugging**: 1-5MB
- **Long-term storage**: 10-100MB per week
- **Recommended cleanup**: Weekly

## Troubleshooting

### Common Issues

#### 1. Logs Not Created
**Symptoms**: No log files in `examples/esp32/logs/`

**Solutions:**
```bash
# Check if logging is enabled
./scripts/flash_app.sh gpio_test Release flash_monitor --log

# Check log directory permissions
ls -la examples/esp32/logs/

# Create log directory if missing
mkdir -p examples/esp32/logs/
```

#### 2. Permission Denied
**Symptoms**: Cannot create or access log files

**Solutions:**
```bash
# Check directory permissions
ls -la examples/esp32/logs/

# Fix permissions
chmod 755 examples/esp32/logs/
chmod 644 examples/esp32/logs/*.log
```

#### 3. Disk Space Issues
**Symptoms**: System running out of disk space

**Solutions:**
```bash
# Check disk usage
df -h
du -sh examples/esp32/logs/

# Clean old logs
./scripts/manage_logs.sh clean 1

# Check for large log files
find examples/esp32/logs/ -name "*.log" -size +10M
```

#### 4. Search Not Working
**Symptoms**: Search command returns no results

**Solutions:**
```bash
# Check if logs exist
./scripts/manage_logs.sh list

# Verify search pattern
./scripts/manage_logs.sh search "ERROR"

# Check log file content
./scripts/manage_logs.sh view <log_filename>
```

### Debug Mode
Enable debug output for troubleshooting:

```bash
# Enable debug mode
export DEBUG=1

# Run commands with debug output
./scripts/manage_logs.sh list
./scripts/flash_app.sh gpio_test Release flash_monitor --log
```

## Best Practices

### 1. Always Use Logging
```bash
# Good: Always enable logging for debugging
./scripts/flash_app.sh gpio_test Release flash_monitor --log

# Bad: No logging makes debugging difficult
./scripts/flash_app.sh gpio_test Release flash_monitor
```

### 2. Use Descriptive Log Names
```bash
# Good: Descriptive names for easy identification
./scripts/flash_app.sh gpio_test Release flash_monitor --log gpio_debug_session_$(date +%H%M)

# Bad: Generic names that don't help
./scripts/flash_app.sh gpio_test Release flash_monitor --log test
```

### 3. Regular Log Maintenance
```bash
# Weekly cleanup (recommended)
./scripts/manage_logs.sh clean 7

# Monthly statistics check
./scripts/manage_logs.sh stats

# Monitor disk usage
du -sh examples/esp32/logs/
```

### 4. Organized Log Structure
```bash
# Development sessions
./scripts/flash_app.sh gpio_test Release flash_monitor --log dev_$(date +%Y%m%d)

# Test runs
./scripts/flash_app.sh gpio_test Release flash_monitor --log test_$(date +%Y%m%d_%H%M)

# CI/CD builds
./scripts/flash_app.sh gpio_test Release flash --log ci_$(date +%Y%m%d_%H%M%S)
```

### 5. Log Analysis Workflow
```bash
# 1. Create logs with descriptive names
./scripts/flash_app.sh gpio_test Release flash_monitor --log gpio_debug_$(date +%H%M)

# 2. Work with the device...

# 3. Search for specific issues
./scripts/manage_logs.sh search "ERROR"
./scripts/manage_logs.sh search "GPIO.*failed"

# 4. View relevant log sections
./scripts/manage_logs.sh view <log_filename>
```

## Security Considerations

### Log File Privacy
- **Local storage**: Logs remain on local machine
- **Git exclusion**: Logs are not committed to version control
- **Permission control**: Log files have appropriate permissions

### Sensitive Information
- **Serial output**: May contain device information
- **Build output**: May contain paths and configurations
- **Error messages**: May contain system details

### Best Practices
```bash
# Don't share log files with sensitive information
# Review logs before sharing
# Use log cleanup to remove old logs
./scripts/manage_logs.sh clean 7
```

## Support and Maintenance

### Getting Help
1. **Check script help**: `./scripts/manage_logs.sh --help`
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

- **Logging System Version**: 2.0.0
- **Script Compatibility**: All ESP32 scripts
- **Platform Support**: Linux, macOS
- **Last Updated**: August 2025

For additional information, see:
- [Scripts Overview](README_SCRIPTS_OVERVIEW.md)
- [Flash System](README_FLASH_SYSTEM.md)
- [Build System](README_BUILD_SYSTEM.md)
- [Port Detection](README_PORT_DETECTION.md)

---

**Navigation**: [← Previous: Flash System](README_FLASH_SYSTEM.md) | [Back to Scripts](../README.md) | [Next: Configuration System →](README_CONFIG_SYSTEM.md)
