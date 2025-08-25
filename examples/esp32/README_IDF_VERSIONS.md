# ESP-IDF Version Management - Quick Reference

## 🚀 What's New

This system allows **each app to specify its own ESP-IDF version and default build type preferences**, while maintaining backward compatibility.

## 📋 Key Features

- ✅ **Per-app ESP-IDF version preferences**
- ✅ **Per-app default build type preferences** 
- ✅ **Automatic version switching** in build/flash scripts
- ✅ **CI pipeline integration** respecting per-app preferences
- ✅ **Backward compatibility** with existing workflows

## 🛠️ Quick Start

### 1. Setup Environment for an App
```bash
# Setup environment for GPIO test (automatically switches to preferred ESP-IDF version)
./scripts/dev_setup.sh setup gpio_test

# Setup with specific build type
./scripts/dev_setup.sh setup ascii_art Debug
```

### 2. Build and Flash (Automatic Version Switching)
```bash
# Build script automatically switches to correct ESP-IDF version
./scripts/build_app.sh gpio_test Debug

# Flash script automatically switches to correct ESP-IDF version  
./scripts/flash_app.sh flash gpio_test Debug
```

### 3. Manage ESP-IDF Versions
```bash
# Install new ESP-IDF version
./scripts/manage_idf_versions.sh install release/v5.4

# Switch to specific version
./scripts/manage_idf_versions.sh switch release/v5.4

# Source environment for app's preferred version
./scripts/manage_idf_versions.sh app gpio_test
```

## 📁 Configuration

### Global Defaults (app_config.yml)
```yaml
metadata:
  default_idf_version: "release/v5.5"      # Global default ESP-IDF version
  default_build_type: "Release"            # Global default build type
```

### Per-App Preferences (app_config.yml)
```yaml
apps:
  gpio_test:
    preferred_idf_version: "release/v5.5"           # Per-app ESP-IDF version
    preferred_default_build_type: "Debug"           # Per-app default build type
```

## 🔧 Scripts Overview

| Script | Purpose | Key Commands |
|--------|---------|--------------|
| `dev_setup.sh` | Developer convenience | `setup`, `info`, `list`, `status` |
| `manage_idf_versions.sh` | ESP-IDF version management | `install`, `switch`, `source`, `app` |
| `build_app.sh` | Building apps | Automatically switches ESP-IDF version |
| `flash_app.sh` | Flashing apps | Automatically switches ESP-IDF version |

## 📊 Current App Preferences

| App | ESP-IDF Version | Default Build Type | Notes |
|-----|----------------|-------------------|-------|
| gpio_test | release/v5.5 | Debug | ⭐ Featured, prefers Debug for debugging |
| ascii_art | release/v5.5 | Release | ⭐ Featured, global defaults |
| adc_test | release/v5.5 | Release | ⭐ Featured, global defaults |
| pio_test | release/v5.5 | Release | ⭐ Featured, global defaults |
| bluetooth_test | release/v5.5 | Release | ⭐ Featured, global defaults |
| utils_test | release/v5.5 | Release | ⭐ Featured, global defaults |
| All others | release/v5.5 | Release | Global defaults |

## 🔄 Workflow Examples

### Example 1: GPIO Test App
```bash
# 1. Setup environment (switches to ESP-IDF v5.5)
./scripts/dev_setup.sh setup gpio_test

# 2. Build (uses correct ESP-IDF version)
./scripts/build_app.sh gpio_test Debug

# 3. Flash and monitor (uses correct ESP-IDF version)
./scripts/flash_app.sh flash_monitor gpio_test Debug
```

### Example 2: Switch Between Apps
```bash
# Work with GPIO test (ESP-IDF v5.5)
./scripts/dev_setup.sh setup gpio_test
./scripts/build_app.sh gpio_test Debug

# Switch to different app (automatically switches ESP-IDF version if needed)
./scripts/dev_setup.sh setup ascii_art
./scripts/build_app.sh ascii_art Release
```

## 🚨 Troubleshooting

### Common Issues
```bash
# Check current environment status
./scripts/dev_setup.sh status

# Show app-specific information
./scripts/dev_setup.sh info <app_type>

# List available ESP-IDF versions
./scripts/manage_idf_versions.sh list

# Check current ESP-IDF version
./scripts/manage_idf_versions.sh current
```

### Quick Fixes
```bash
# ESP-IDF not found
./scripts/manage_idf_versions.sh install release/v5.5

# Version mismatch
./scripts/manage_idf_versions.sh switch release/v5.5

# Environment not loaded
./scripts/dev_setup.sh setup <app_type>
```

## 📚 Documentation

- **Full Documentation**: `docs/README_IDF_VERSION_MANAGEMENT.md`
- **Scripts Help**: `./scripts/dev_setup.sh --help`
- **IDF Management Help**: `./scripts/manage_idf_versions.sh --help`

## 🔮 Future Enhancements

The system is designed to support:
- Multiple ESP-IDF versions for different apps
- Custom ESP-IDF branches/commits
- Batch operations across multiple apps
- Advanced CI matrix configurations

## 💡 Best Practices

1. **Use semantic versioning** for ESP-IDF versions
2. **Test compatibility** before changing versions
3. **Document version requirements** in app descriptions
4. **Leverage CI pipeline** to test multiple versions
5. **Use consistent naming** across similar apps

---

**Note**: This system is fully backward compatible. Existing apps and workflows continue to work unchanged.