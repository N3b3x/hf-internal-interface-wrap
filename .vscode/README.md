# VS Code Configuration for ESP32 HardFOC Interface Wrapper

This directory contains VS Code configuration files that have been updated to be **computer agnostic** and work with ESP-IDF toolchain paths.

## Files Overview

### 1. `settings.json`
- **Purpose**: Global VS Code settings for C/C++ IntelliSense
- **Status**: ✅ Computer agnostic - uses environment variables
- **Key Features**:
  - Uses `${env:IDF_PATH}` for ESP-IDF components
  - Uses `${env:HOME}` for toolchain paths
  - Supports multiple toolchain versions

### 2. `c_cpp_properties.json`
- **Purpose**: C/C++ IntelliSense configuration
- **Status**: ✅ Computer agnostic - uses environment variables
- **Key Features**:
  - ESP32-C6 specific configuration
  - RISC-V architecture support
  - Uses environment variables for paths

### 3. `tasks.json`
- **Purpose**: Build and flash tasks for all examples
- **Status**: ✅ Complete coverage of all tests from `examples_config.yml`
- **Key Features**:
  - All 16 examples supported
  - Debug and Release build variants
  - Organized by category (Peripheral, Sensor, Utility, etc.)
  - Uses existing build/flash scripts

## Setup Instructions

### Prerequisites
1. **ESP-IDF installed** and sourced:
   ```bash
   source ~/esp/esp-idf/export.sh
   ```

2. **VS Code C/C++ extension** installed

### Initial Setup
1. **Source ESP-IDF**:
   ```bash
   source ~/esp/esp-idf/export.sh
   ```

2. **Open project in VS Code**:
   ```bash
   code .  # or open VS Code and File -> Open Folder
   ```

3. **Reload VS Code window**:
   - Press `Ctrl+Shift+P`
   - Type "Developer: Reload Window"
   - Press Enter

### Verification
After setup, you should see:
- ✅ IntelliSense working for ESP-IDF components
- ✅ All example tasks available in Command Palette
- ✅ No hardcoded paths in configuration files

## Available Tasks

### Build Tasks
- **Build All Examples (Release)** - Default build task
- **Build All Examples (Debug)**
- Individual build tasks for each example in both Debug/Release

### Flash Tasks
- **Flash All Examples (Release)**
- **Flash All Examples (Debug)**
- Individual flash tasks for each example

### Utility Tasks
- **Regenerate compile_commands.json**
- **Clean All Builds**

## Example Usage

### Building an Example
1. Press `Ctrl+Shift+P`
2. Type "Tasks: Run Task"
3. Select "Build GPIO Test (Release)"
4. Task will run in the integrated terminal

### Flashing an Example
1. Press `Ctrl+Shift+P`
2. Type "Tasks: Run Task"
3. Select "Flash PWM Test (Release)"
4. Task will flash and start monitoring

## Troubleshooting

### IntelliSense Issues
1. **Check ESP-IDF is sourced**:
   ```bash
   echo $IDF_PATH
   # Should show: /home/username/esp/esp-idf
   ```

2. **Verify toolchain paths exist**:
   ```bash
   ls -la ~/.espressif/tools/riscv32-esp-elf/
   ```

3. **Reload VS Code window**

### Task Execution Issues
1. **Check working directory**:
   - Tasks run from `examples/esp32/` directory
   - Ensure you're in the workspace root

2. **Verify scripts exist**:
   ```bash
   ls -la examples/esp32/scripts/
   ```

3. **Check script permissions**:
   ```bash
   chmod +x examples/esp32/scripts/*.sh
   ```

## Maintenance

### When to Update
- **ESP-IDF version changes**
- **Toolchain updates**
- **New examples added**

### Update Process
1. Source new ESP-IDF version
2. Check if toolchain paths have changed
3. Update configuration files manually if needed
4. Reload VS Code window

## File Structure
```
.vscode/
├── settings.json              # Global VS Code settings
├── c_cpp_properties.json     # C/C++ IntelliSense config
├── tasks.json                # Build and flash tasks
└── README.md                 # This file
```

## Environment Variables Used

- `${env:IDF_PATH}` - ESP-IDF installation directory
- `${env:HOME}` - User home directory for toolchain paths
- `${workspaceFolder}` - Current workspace directory

## Notes

- **Computer agnostic**: No hardcoded usernames or paths
- **Environment-based**: Uses environment variables for dynamic paths
- **Consistent**: All configuration files use the same approach
- **Manual updates**: Configuration files may need manual updates when toolchain versions change
