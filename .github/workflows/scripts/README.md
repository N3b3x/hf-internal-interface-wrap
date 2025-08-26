# 🔄 GitHub Workflows Scripts

This directory contains scripts specifically designed for GitHub Actions workflows. These scripts handle CI/CD tasks and are organized separately from project-specific scripts to maintain clean architecture.

## 📁 Scripts Overview

### **`setup_build_directory.sh`** - Complete ESP32 Build Setup
**Purpose**: Handles the complete build directory setup and building process for ESP32 CI builds.

### **Build Directory Setup** (`scripts/setup_build_directory.sh`)
Comprehensive build environment setup script that:
- Creates ESP-IDF project structure
- Copies source files and components
- Configures build with ccache
- Generates size reports and ccache statistics
- **NEW**: Intelligently reads `app_config.yml` for app-specific configuration
- **NEW**: Validates ESP-IDF version and build type compatibility

**Usage**:
```bash
./.github/workflows/scripts/setup_build_directory.sh \
  -p <build_path> \
  -t <idf_target> \
  -b <build_type> \
  -a <app_type> \
  -v <idf_version>
```

**Parameters**:
- `-p, --build-path`: Build directory path (default: `ci_project`)
- `-t, --target`: ESP-IDF target (default: `esp32c6`)
- `-b, --build-type`: Build type: Debug, Release, RelWithDebInfo, MinSizeRel (default: `Release`)
- `-a, --app-type`: Application type (default: `hardfoc_interface`)
- `-v, --idf-version`: ESP-IDF version (default: `release/v5.5`)
- `-h, --help`: Show help message

**Intelligent Configuration**:
The script now automatically:
1. **Reads `app_config.yml`** to understand app-specific settings
2. **Validates ESP-IDF version** compatibility with the app
3. **Validates build type** compatibility with the app
4. **Provides detailed feedback** about app configuration
5. **Fails gracefully** if configuration is incompatible

**Example Output**:
```
[INFO] Reading app configuration from examples/esp32/app_config.yml...
[SUCCESS] App configuration validated successfully
App: ascii_art
Description: ASCII art generator example
Category: utility
Source File: AsciiArtComprehensiveTest.cpp
IDF Versions: ['release/v5.5']
Build Types: ['Debug', 'Release']
```

### **`prepare_build_directory.sh`** - Build Directory Preparation Only
**Purpose**: Prepares build directory structure without building (for separation of concerns).

**Usage**:
```bash
./prepare_build_directory.sh -p <build_path>
```

**What it does**:
- Creates ESP-IDF project structure
- Copies source files and components (excluding scripts directory to prevent conflicts)
- Sets up CMakeLists.txt and configuration files
- Does NOT build the project (use setup_build_directory.sh for that)
- Provides clear next steps for manual building

**Advanced Features**:
- **Smart File Handling**: Avoids copying scripts directory to prevent conflicts
- **Prerequisites Checking**: Verifies all required files exist before proceeding
- **Clear Guidance**: Provides specific commands for next steps

**Use case**: Separate build directory preparation from building when needed

## 🔧 Integration with Workflows

These scripts are used by the following GitHub Actions workflows:

- **`esp32-component-ci.yml`** - Main ESP32 CI pipeline
- **`generate_matrix.py`** - Build matrix generation

## 📚 Documentation

- **CI Caching Strategy**: [../../README_CI_CACHING_STRATEGY.md](../../README_CI_CACHING_STRATEGY.md)

## 🚀 Local Development

While these scripts are designed for CI, they can be used locally for testing:

```bash
# Test the scripts
cd .github/workflows/scripts
./setup_build_directory.sh --help
./prepare_build_directory.sh --help

# Run with test parameters
./setup_build_directory.sh \
  -p test_build \
  -t esp32c6 \
  -b Debug \
  -a test_app

# Test preparation only
./prepare_build_directory.sh -p test_prep
```

## 🔍 Script Dependencies

These scripts require:
- ESP-IDF environment to be sourced
- `idf.py` command available
- Source files in the expected locations relative to the workspace root
- ccache available for optimal build performance

## 🎯 Script Capabilities

### **Build Optimization**
- **ccache Support**: Automatic ccache integration for faster rebuilds
- **Build Type Validation**: Ensures only valid build types are used
- **Parameter Validation**: Validates all input parameters before execution

### **File Management**
- **Smart Copying**: Avoids conflicts by excluding scripts directory
- **Prerequisites Checking**: Verifies all required files exist
- **Clean Setup**: Removes existing build directories before setup

### **Reporting & Analysis**
- **Size Reports**: Generates both human-readable (.txt) and machine-readable (.json) size reports
- **ccache Statistics**: Provides detailed ccache performance metrics
- **Build Artifacts**: Lists all generated files for easy identification

### **Error Handling**
- **Fail Fast**: Exits immediately on any error with clear messages
- **Validation**: Checks inputs and prerequisites before proceeding
- **Clear Feedback**: Provides detailed status updates throughout execution

## 🤝 Contributing

When modifying these scripts:

1. **Keep CI focus** - these scripts are for CI/CD workflows
2. **Maintain clean interfaces** - use clear command-line arguments
3. **Add comprehensive help** - every script should have `--help`
4. **Follow error handling patterns** - use consistent error handling
5. **Update documentation** - keep this README and related docs current
6. **Preserve ccache integration** - maintain build optimization features
7. **Maintain smart file handling** - avoid conflicts in file copying

## 📋 Script Requirements

- **Portability**: Must work in GitHub Actions Ubuntu runners
- **Error handling**: Must fail fast with clear error messages
- **Logging**: Must provide clear status updates for CI logs
- **Configuration**: Must accept parameters via command line
- **Validation**: Must validate inputs and prerequisites
- **Optimization**: Must support ccache for build performance
- **Conflict Prevention**: Must avoid file conflicts during setup

## 🔧 Technical Details

### **Build Process Flow**
1. **Parameter Validation** - Check all inputs are valid
2. **Prerequisites Check** - Verify required files exist
3. **Directory Setup** - Create and configure build directory
4. **File Copying** - Copy source files (excluding conflicts)
5. **Build Configuration** - Set up ESP-IDF build parameters
6. **Compilation** - Execute build with ccache support
7. **Reporting** - Generate size reports and ccache statistics
8. **Artifact Discovery** - List and categorize build outputs

### **File Handling Strategy**
- **Included**: `main/`, `components/`, `src/`, `inc/`, `CMakeLists.txt`, `app_config.yml`, `sdkconfig`
- **Excluded**: `scripts/` directory (to prevent conflicts with CI scripts)
- **Generated**: `build/` directory with all compilation outputs

### **ccache Integration**
- **Build Flag**: Uses `--ccache` for all ESP-IDF builds
- **Statistics**: Generates `ccache_stats.txt` for performance analysis
- **Optimization**: Enables incremental builds for faster CI execution

---

*These scripts follow clean architecture principles by separating CI concerns from project logic while providing comprehensive build optimization and reporting capabilities.*

# CI Matrix Generation Scripts

This directory contains scripts for generating GitHub Actions CI matrices and managing ESP32 application configurations.

## 📁 Scripts Overview

### `generate_matrix.py`
Dynamic CI matrix generator that creates build combinations based on `app_config.yml` configuration.

## 🚀 Generate Matrix Script

### **Purpose**
The `generate_matrix.py` script dynamically generates GitHub Actions CI matrices by reading application configurations from `examples/esp32/app_config.yml`. It supports hierarchical configuration, per-app overrides, and flexible build type specifications.

### **Features**
- **Dynamic Matrix Generation**: Creates build combinations on-the-fly
- **Hierarchical Configuration**: Global defaults with per-app overrides
- **Flexible Build Types**: Support for flat and nested build type arrays
- **Multiple IDF Versions**: Per-app ESP-IDF version specification
- **Smart Overrides**: App-specific settings take precedence over global defaults
- **Configuration Source Tracking**: Identifies whether settings come from global or app-specific configs

### **Usage**

#### **Basic Matrix Generation**
```bash
# Generate matrix for GitHub Actions (compact format)
python3 generate_matrix.py

# Generate matrix with pretty printing (for debugging)
python3 generate_matrix.py --pretty

# Generate matrix with hierarchical configuration analysis
python3 generate_matrix.py --hierarchical-info
```

#### **Command Line Options**

| Option | Description | Output |
|--------|-------------|---------|
| `--pretty` | Pretty-printed JSON matrix for debugging | Formatted matrix with indentation |
| `--hierarchical-info` | Hierarchical configuration analysis | Human-readable configuration breakdown |
| `--apps-only` | List of CI-enabled apps only | Array of app names |
| `--build-types-only` | List of all build types used | Array of build type names |
| `--metadata` | Display metadata configuration | Metadata section from config |

#### **Output Examples**

**Standard Matrix (GitHub Actions)**
```json
{
  "include": [
    {
      "idf_version": "release/v5.5",
      "idf_version_docker": "release-v5.5",
      "build_type": "Debug",
      "app_name": "ascii_art",
      "config_source": "app"
    }
  ]
}
```

**Hierarchical Analysis**
```
=== Hierarchical Configuration Analysis ===
Global IDF versions: ['release/v5.5', 'release/v5.4']
Global build types per IDF: [['Debug', 'Release'], ['Debug']]

IDF Version to Build Types Mapping:
  release/v5.5: ['Debug', 'Release']
  release/v5.4: ['Debug']

Per-app overrides:
  ascii_art:
    IDF versions: ['release/v5.5']
    Build types: ['Debug', 'Release']
```

### **Configuration Patterns**

#### **Pattern 1: Single IDF Version (Standard)**
```yaml
idf_versions: ["release/v5.5"]
build_types: ["Debug", "Release"]
# Result: 2 builds (Debug + Release for v5.5)
```

#### **Pattern 2: Multiple IDF Versions with Same Build Types**
```yaml
idf_versions: ["release/v5.5", "release/v5.4"]
build_types: ["Debug", "Release"]
# Result: 4 builds (Debug + Release for both v5.5 and v5.4)
```

#### **Pattern 3: Multiple IDF Versions with Different Build Types**
```yaml
idf_versions: ["release/v5.5", "release/v5.4"]
build_types: [["Debug", "Release"], ["Debug"]]
# Result: 3 builds (Debug+Release for v5.5, Debug only for v5.4)
```

### **Matrix Generation Logic**

#### **Hierarchical Precedence**
1. **App-specific overrides** take highest precedence
2. **Global defaults** are used when no app-specific settings exist
3. **Fallback defaults** are used if neither exists

#### **Build Type Detection**
```python
# Check if build_types is nested (array of arrays) or flat (single array)
if isinstance(app_build_types[0], list):
    # Nested format: build_types: [["Debug", "Release"], ["Debug"]]
    if i < len(app_build_types):
        # Use the build types for this specific IDF version index
        for build_type in app_build_types[i]:
            effective_combinations.append((idf_version, build_type))
else:
    # Flat format: build_types: ["Debug", "Release"] (same for all IDF versions)
    for build_type in app_build_types:
        effective_combinations.append((idf_version, build_type))
```

#### **Configuration Source Tracking**
- **`"app"`**: Settings come from app-specific configuration
- **`"global"`**: Settings come from global metadata defaults

### **Integration with GitHub Actions**

#### **Workflow Usage**
```yaml
jobs:
  build:
    strategy:
      matrix:
        include: ${{ fromJSON(steps.matrix.outputs.matrix) }}
    steps:
      - name: Generate Matrix
        id: matrix
        run: |
          python3 examples/esp32/scripts/generate_matrix.py
        shell: bash
```

#### **Matrix Structure**
Each matrix entry contains:
- `idf_version`: ESP-IDF version for cloning (Git format)
- `idf_version_docker`: ESP-IDF version for artifacts (Docker-safe format)
- `build_type`: Build type (Debug, Release, etc.)
- `app_name`: Application name
- `config_source`: Source of configuration (app or global)

### **Error Handling**

#### **Validation**
- **Configuration loading**: YAML parsing and structure validation
- **Required fields**: Ensures essential configuration elements exist
- **Type checking**: Validates build type array structures

#### **Fallbacks**
- **Missing build types**: Defaults to `["Debug", "Release"]`
- **Missing IDF versions**: Defaults to `["release/v5.5"]`
- **Index mismatches**: Graceful handling of array length differences

### **Performance Considerations**

#### **Matrix Size**
- **Small matrices** (< 100 builds): Fast generation
- **Large matrices** (> 100 builds): Consider build timeouts and resource limits
- **Optimization**: Use app-specific overrides to reduce unnecessary builds

#### **Caching**
- **Configuration parsing**: YAML loaded once per execution
- **Matrix generation**: Single pass through configuration
- **Output formatting**: Minimal string processing

### **Debugging and Troubleshooting**

#### **Common Issues**

**1. Matrix Too Large**
```bash
# Check total build count
python3 generate_matrix.py --pretty | jq '.include | length'

# Analyze per-app builds
python3 generate_matrix.py --pretty | jq '.include[] | {app_name, idf_version, build_type}'
```

**2. Configuration Errors**
```bash
# Validate YAML structure
python3 generate_matrix.py --metadata

# Check hierarchical configuration
python3 generate_matrix.py --hierarchical-info
```

**3. Build Type Mismatches**
```bash
# Verify build types per app
python3 generate_matrix.py --build-types-only

# Check specific app configuration
python3 generate_matrix.py --pretty | jq '.include[] | select(.app_name == "app_name")'
```

#### **Debug Commands**
```bash
# Full matrix with pretty printing
python3 generate_matrix.py --pretty

# Configuration analysis
python3 generate_matrix.py --hierarchical-info

# App list only
python3 generate_matrix.py --apps-only

# Build types only
python3 generate_matrix.py --build-types-only
```

### **Best Practices**

#### **Configuration Design**
1. **Use global defaults** for common settings across all apps
2. **Use app-specific overrides** for special requirements
3. **Keep build types minimal** unless specific testing requires more
4. **Document complex configurations** with clear comments

#### **Matrix Optimization**
1. **Limit IDF versions** to those actually needed
2. **Use Debug + Release** as standard build types
3. **Consider CI timeouts** when adding builds
4. **Test configurations** before committing to CI

#### **Maintenance**
1. **Regular validation** of matrix generation
2. **Monitor build times** and adjust as needed
3. **Update documentation** when adding new features
4. **Version control** all configuration changes

### **Future Enhancements**

#### **Planned Features**
- **Conditional builds**: Build only when certain conditions are met
- **Dependency tracking**: Build apps in dependency order
- **Parallel optimization**: Group builds for optimal parallelization
- **Artifact management**: Automatic artifact naming and organization

#### **Extensibility**
- **Plugin system**: Custom matrix generation rules
- **External configuration**: Support for remote configuration sources
- **Validation rules**: Custom validation for configuration files
- **Reporting**: Build matrix analysis and statistics

---

## 📚 Related Documentation

- [App Configuration Guide](../../../examples/esp32/README.md)
- [CI Workflow Documentation](../README.md)
- [ESP32 Examples](../../../examples/esp32/README.md)
- [GitHub Actions Workflows](../README.md)

## 🔧 Technical Details

- **Language**: Python 3.8+
- **Dependencies**: Standard library only (json, sys, yaml)
- **Input**: `examples/esp32/app_config.yml`
- **Output**: JSON matrix for GitHub Actions
- **License**: Project-specific license
