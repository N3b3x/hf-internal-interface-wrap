# 🔄 GitHub Workflows Scripts

This directory contains scripts specifically designed for GitHub Actions workflows. These scripts handle CI/CD tasks and are organized separately from project-specific scripts to maintain clean architecture.

## 📁 Scripts Overview

### **`setup_build_directory.sh`** - Complete ESP32 Build Setup
**Purpose**: Handles the complete build directory setup and building process for ESP32 CI builds.

**Usage**:
```bash
./setup_build_directory.sh \
  -p <build_path> \
  -t <target> \
  -b <build_type> \
  -a <app_type>
```

**What it does**:
- Creates ESP-IDF project structure
- Copies source files and components
- Sets up CMakeLists.txt and configuration files
- Configures and builds the project
- Generates size reports and ccache statistics

**Use case**: Full CI build process in GitHub Actions

### **`prepare_build_directory.sh`** - Build Directory Preparation Only
**Purpose**: Prepares build directory structure without building (for separation of concerns).

**Usage**:
```bash
./prepare_build_directory.sh -p <build_path>
```

**What it does**:
- Creates ESP-IDF project structure
- Copies source files and components
- Sets up CMakeLists.txt and configuration files
- Does NOT build the project

**Use case**: Separate build directory preparation from building when needed

## 🔧 Integration with Workflows

These scripts are used by the following GitHub Actions workflows:

- **`esp32-component-ci.yml`** - Main ESP32 CI pipeline
- **`generate_matrix.py`** - Build matrix generation

## 📚 Documentation

- **Clean Principles Refactor**: [../docs/CLEAN_PRINCIPLES_REFACTOR.md](../docs/CLEAN_PRINCIPLES_REFACTOR.md)
- **CI Caching Strategy**: [../../CI_CACHING_STRATEGY.md](../../CI_CACHING_STRATEGY.md)

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
```

## 🔍 Script Dependencies

These scripts require:
- ESP-IDF environment to be sourced
- `idf.py` command available
- Source files in the expected locations relative to the workspace root

## 🤝 Contributing

When modifying these scripts:

1. **Keep CI focus** - these scripts are for CI/CD workflows
2. **Maintain clean interfaces** - use clear command-line arguments
3. **Add comprehensive help** - every script should have `--help`
4. **Follow error handling patterns** - use consistent error handling
5. **Update documentation** - keep this README and related docs current

## 📋 Script Requirements

- **Portability**: Must work in GitHub Actions Ubuntu runners
- **Error handling**: Must fail fast with clear error messages
- **Logging**: Must provide clear status updates for CI logs
- **Configuration**: Must accept parameters via command line
- **Validation**: Must validate inputs and prerequisites

---

*These scripts follow clean architecture principles by separating CI concerns from project logic.*