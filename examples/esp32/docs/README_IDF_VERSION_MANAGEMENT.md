# ESP-IDF Version Management System

This document describes the ESP-IDF version management system that allows each app to specify its preferred ESP-IDF version and build type preferences.

## Overview

The system provides:
- **Per-app ESP-IDF version preferences** - Each app can specify its preferred ESP-IDF version
- **Per-app default build type preferences** - Each app can specify its preferred default build type
- **Automatic version switching** - Build and flash scripts automatically switch to the correct ESP-IDF version
- **CI pipeline integration** - CI builds respect per-app preferences
- **Backward compatibility** - Existing workflows continue to work

## Configuration

### Global Defaults

In `app_config.yml`, global defaults are specified in the `metadata` section:

```yaml
metadata:
  default_idf_version: "release/v5.5"      # Global default ESP-IDF version
  default_build_type: "Release"            # Global default build type
  idf_versions: ["release/v5.5"]          # Supported versions (legacy)
```

### Per-App Preferences

Each app can specify its own preferences:

```yaml
apps:
  gpio_test:
    description: "GPIO peripheral comprehensive testing"
    source_file: "GpioComprehensiveTest.cpp"
    category: "peripheral"
    build_types: ["Debug", "Release"]
    preferred_idf_version: "release/v5.5"           # Per-app ESP-IDF version
    preferred_default_build_type: "Debug"           # Per-app default build type
    ci_enabled: true
    featured: true
```

### Fallback Behavior

- If an app doesn't specify `preferred_idf_version`, it uses the global `default_idf_version`
- If an app doesn't specify `preferred_default_build_type`, it uses the global `default_build_type`
- If no global defaults are specified, the system falls back to hardcoded defaults

## Scripts

### 1. ESP-IDF Version Management (`manage_idf_versions.sh`)

Primary script for managing ESP-IDF versions.

```bash
# Install/update ESP-IDF version
./scripts/manage_idf_versions.sh install release/v5.5

# Switch to specific version
./scripts/manage_idf_versions.sh switch release/v5.4

# Source environment for specific version
./scripts/manage_idf_versions.sh source release/v5.5

# Source environment for app's preferred version
./scripts/manage_idf_versions.sh app gpio_test

# Show current version
./scripts/manage_idf_versions.sh current

# List available versions
./scripts/manage_idf_versions.sh list

# Clean up old versions
./scripts/manage_idf_versions.sh cleanup
```

### 2. Developer Setup (`dev_setup.sh`)

Convenience script for developers to quickly set up their environment.

```bash
# Setup environment for specific app
./scripts/dev_setup.sh setup gpio_test

# Setup with specific build type
./scripts/dev_setup.sh setup ascii_art Debug

# Show app information
./scripts/dev_setup.sh info pio_test

# List all apps with preferences
./scripts/dev_setup.sh list

# Show current environment status
./scripts/dev_setup.sh status

# Manage ESP-IDF versions
./scripts/dev_setup.sh idf list
./scripts/dev_setup.sh idf app gpio_test
```

### 3. Build and Flash Scripts

The build and flash scripts automatically handle ESP-IDF version switching:

```bash
# Build script automatically switches to correct ESP-IDF version
./scripts/build_app.sh gpio_test Debug

# Flash script automatically switches to correct ESP-IDF version
./scripts/flash_app.sh flash gpio_test Debug
```

## Workflow Examples

### Example 1: Working with GPIO Test App

```bash
# 1. Setup environment for GPIO test (automatically switches to preferred ESP-IDF version)
./scripts/dev_setup.sh setup gpio_test

# 2. Build the app (uses correct ESP-IDF version)
./scripts/build_app.sh gpio_test Debug

# 3. Flash and monitor (uses correct ESP-IDF version)
./scripts/flash_app.sh flash_monitor gpio_test Debug
```

### Example 2: Switching Between Different Apps

```bash
# Work with GPIO test (ESP-IDF v5.5)
./scripts/dev_setup.sh setup gpio_test
./scripts/build_app.sh gpio_test Debug

# Switch to different app (automatically switches ESP-IDF version if needed)
./scripts/dev_setup.sh setup ascii_art
./scripts/build_app.sh ascii_art Release
```

### Example 3: Manual ESP-IDF Version Management

```bash
# Install new ESP-IDF version
./scripts/manage_idf_versions.sh install release/v5.4

# Switch to it
./scripts/manage_idf_versions.sh switch release/v5.4

# Source environment
./scripts/manage_idf_versions.sh source release/v5.4
```

## CI Pipeline Integration

The CI pipeline automatically respects per-app preferences:

1. **Matrix Generation**: Each app is built with its preferred ESP-IDF version
2. **Docker Images**: Uses the appropriate ESP-IDF Docker image for each app
3. **Build Types**: Respects per-app build type preferences
4. **Artifacts**: Named with app-specific ESP-IDF version information

### CI Matrix Example

```yaml
# Generated matrix includes per-app preferences
matrix:
  include:
    - idf_version: "release/v5.5"
      idf_version_docker: "release-v5.5"
      build_type: "Debug"
      app_name: "gpio_test"
      app_default_build_type: "Debug"
    
    - idf_version: "release/v5.5"
      idf_version_docker: "release-v5.5"
      build_type: "Release"
      app_name: "ascii_art"
      app_default_build_type: "Release"
```

## Environment Variables

The system sets these environment variables:

- `IDF_PATH`: Path to ESP-IDF installation
- `IDF_TARGET`: Target chip (e.g., esp32c6)
- `IDF_CCACHE_ENABLE`: Compiler cache setting

## Troubleshooting

### Common Issues

1. **ESP-IDF not found**
   ```bash
   # Install ESP-IDF
   ./scripts/manage_idf_versions.sh install release/v5.5
   ```

2. **Version mismatch**
   ```bash
   # Check current version
   ./scripts/manage_idf_versions.sh current
   
   # Switch to required version
   ./scripts/manage_idf_versions.sh switch release/v5.5
   ```

3. **Environment not loaded**
   ```bash
   # Source environment for app
   ./scripts/dev_setup.sh setup <app_type>
   ```

### Debug Information

```bash
# Show detailed environment status
./scripts/dev_setup.sh status

# Show app-specific information
./scripts/dev_setup.sh info <app_type>

# List available ESP-IDF versions
./scripts/manage_idf_versions.sh list
```

## Migration from Old System

The new system is backward compatible:

1. **Existing apps** continue to work with global defaults
2. **New apps** can specify per-app preferences
3. **Build scripts** automatically detect and use preferences
4. **CI pipeline** respects both global and per-app settings

### Adding Per-App Preferences

To add preferences to an existing app:

```yaml
apps:
  existing_app:
    # ... existing configuration ...
    preferred_idf_version: "release/v5.5"           # Add this line
    preferred_default_build_type: "Release"         # Add this line
```

## Best Practices

1. **Use semantic versioning** for ESP-IDF versions (e.g., `release/v5.5`)
2. **Test compatibility** before changing ESP-IDF versions
3. **Document version requirements** in app descriptions
4. **Use consistent naming** across similar apps
5. **Leverage CI pipeline** to test multiple ESP-IDF versions

## Advanced Usage

### Multiple ESP-IDF Versions

```bash
# Install multiple versions
./scripts/manage_idf_versions.sh install release/v5.4
./scripts/manage_idf_versions.sh install release/v5.5

# Switch between them
./scripts/manage_idf_versions.sh switch release/v5.4
./scripts/manage_idf_versions.sh switch release/v5.5
```

### Custom ESP-IDF Branches

```bash
# Install custom branch
./scripts/manage_idf_versions.sh install feature/new-feature

# Install specific commit
./scripts/manage_idf_versions.sh install abc1234
```

### Batch Operations

```bash
# Setup multiple apps
for app in gpio_test adc_test uart_test; do
    ./scripts/dev_setup.sh setup "$app"
    ./scripts/build_app.sh "$app" Release
done
```

## Support

For issues or questions:

1. Check the troubleshooting section above
2. Review app configuration in `app_config.yml`
3. Check ESP-IDF installation status
4. Verify environment variables are set correctly
5. Review CI pipeline logs for version information