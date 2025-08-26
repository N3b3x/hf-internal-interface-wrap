# GitHub Actions Workflows

This directory contains GitHub Actions workflows for continuous integration and deployment of the ESP32 HardFOC Interface Wrapper project.

## 📁 Directory Structure

```
.github/workflows/
├── README.md                           # This file - workflows overview
├── esp32-component-ci.yml             # Main ESP32 CI workflow
├── security-audit.yml                 # Security vulnerability scanning workflow
├── secrets-management-guide.yml       # Secure secrets handling workflow
├── docs.yml                           # Documentation build workflow
├── scripts/                           # CI-specific scripts
│   ├── README.md                      # Scripts documentation
│   ├── setup_build_directory.sh       # Build directory setup script
│   ├── prepare_build_directory.sh     # Build directory preparation script
│   └── generate_matrix.py             # Dynamic CI matrix generator
└── docs/                              # Workflow documentation
    └── CLEAN_PRINCIPLES_REFACTOR.md   # Refactoring documentation
```

## 🚀 Available Workflows

## 📍 **Centralized Project Location**

All workflows use a centralized variable `ESP32_PROJECT_PATH` to specify the ESP32 project location. This makes it easy to change the project location in the future without updating multiple files.

**Current Configuration**:
```yaml
env:
  ESP32_PROJECT_PATH: examples/esp32  # Centralized ESP32 project location
```

**Benefits**:
- **Easy Maintenance**: Change one variable to update all paths across the entire CI system
- **Future-Proof**: Move the ESP32 project to any location by updating the variable
- **Consistency**: All workflows and scripts use the same path reference
- **Reduced Errors**: No more mismatched paths between different workflow files

**How to Change Project Location**:
If you need to move the ESP32 project to a different location (e.g., `projects/esp32` or `embedded/esp32`), simply update the `ESP32_PROJECT_PATH` variable in the workflow files:

```yaml
env:
  ESP32_PROJECT_PATH: projects/esp32  # New location
```

### **ESP32 Component CI** (`esp32-component-ci.yml`)
**Purpose**: Continuous integration for ESP32 components and applications

**Features**:
- **Dynamic Matrix Generation**: Uses `generate_matrix.py` to create build combinations
- **Multi-IDF Support**: Builds against multiple ESP-IDF versions
- **Comprehensive Testing**: Builds all enabled applications with multiple build types
- **Artifact Management**: Generates and uploads build outputs, size reports, and ccache statistics
- **Smart Caching**: Uses ccache for faster incremental builds

**Matrix Generation**:
```yaml
strategy:
  matrix:
    include: ${{ fromJSON(steps.matrix.outputs.matrix) }}
```

**Matrix Source**:
```yaml
- name: Generate Matrix
  id: matrix
  run: |
    python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py
```

### **Security Audit** (`security-audit.yml`)
**Purpose**: Automated security vulnerability scanning and analysis

**Features**:
- **CodeQL Analysis**: Advanced semantic code analysis for security vulnerabilities
- **Dependency Scanning**: Automated scanning of dependencies for known vulnerabilities
- **SAST (Static Application Security Testing)**: Static code analysis for security issues
- **Security Reporting**: Comprehensive security reports and alerts
- **Automated Remediation**: Suggestions for fixing identified vulnerabilities

**Triggers**: Push to main, PRs, scheduled runs

### **Secrets Management Guide** (`secrets-management-guide.yml`)
**Purpose**: Demonstrates secure secrets handling and management practices

**Features**:
- **Environment-Specific Secrets**: Different secrets for different environments
- **Secrets Rotation**: Automated secrets rotation strategies
- **Secure Storage**: Best practices for storing and accessing secrets
- **Access Control**: Role-based access to sensitive information
- **Audit Logging**: Comprehensive logging of secrets access

**Triggers**: Manual dispatch, scheduled runs

### **Documentation Build** (`docs.yml`)
**Purpose**: Automated documentation building and deployment

**Features**:
- **Documentation Generation**: Builds project documentation from source
- **Format Validation**: Ensures documentation meets quality standards
- **Deployment**: Automatically deploys updated documentation
- **Link Checking**: Validates internal and external links
- **Version Control**: Tracks documentation changes and versions

**Triggers**: Documentation changes, manual dispatch

## 🔧 Scripts

### **Matrix Generation** (`scripts/generate_matrix.py`)
Dynamic CI matrix generator that creates build combinations based on `${{ env.ESP32_PROJECT_PATH }}/app_config.yml` configuration.

**Key Features**:
- **Hierarchical Configuration**: Global defaults with per-app overrides
- **Flexible Build Types**: Support for flat and nested build type arrays
- **Multiple IDF Versions**: Per-app ESP-IDF version specification
- **Configuration Source Tracking**: Identifies settings source (global vs app-specific)

**Usage**:
```bash
# Generate matrix for GitHub Actions
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py

# Pretty-printed output for debugging
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --pretty

# Hierarchical configuration analysis
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --hierarchical-info
```

**For detailed documentation, see**: [`scripts/README.md`](scripts/README.md)

### **Build Directory Setup** (`scripts/setup_build_directory.sh`)
Comprehensive build environment setup script that:
- Creates ESP-IDF project structure
- Copies source files and components
- Configures build with ccache
- Generates size reports and ccache statistics
- Intelligently reads `app_config.yml` for app-specific configuration
- Validates ESP-IDF version and build type compatibility

**Usage**:
```bash
./.github/workflows/scripts/setup_build_directory.sh \
  -p <build_path> \
  -t <idf_target> \
  -b <build_type> \
  -a <app_type> \
  -v <idf_version>
```

**Intelligent Configuration**:
The script now automatically:
1. **Reads `app_config.yml`** to understand app-specific settings
2. **Validates ESP-IDF version** compatibility with the app
3. **Validates build type** compatibility with the app
4. **Provides detailed feedback** about app configuration
5. **Fails gracefully** if configuration is incompatible

### **Build Directory Preparation** (`scripts/prepare_build_directory.sh`)
Simplified script that only prepares the build directory structure and copies files.

**Usage**:
```bash
./.github/workflows/scripts/prepare_build_directory.sh \
  -p <build_path> \
  -t <idf_target> \
  -b <build_type> \
  -a <app_type>
```

## 📋 Configuration

### **App Configuration** (`${{ env.ESP32_PROJECT_PATH }}/app_config.yml`)
Central configuration file that defines:
- **Application metadata**: Names, descriptions, categories
- **Build configurations**: Build types, optimization levels
- **CI settings**: Timeouts, exclusions, special handling
- **Global defaults**: Default build types and IDF versions
- **Per-app overrides**: App-specific IDF versions and build types

**Configuration Patterns**:
```yaml
# Global defaults
metadata:
  default_build_types: [["Debug", "Release"], ["Debug"]]
  idf_versions: ["release/v5.5", "release/v5.4"]

# App with overrides
apps:
  my_app:
    idf_versions: ["release/v5.5"]  # Override global
    build_types: ["Debug", "Release"]  # Override global
```

## 🔄 Workflow Execution

### **Workflow Triggers**

| Workflow | Triggers | Purpose |
|----------|----------|---------|
| **ESP32 Component CI** | Push to main, PRs, manual dispatch | Build and test ESP32 applications |
| **Security Audit** | Push to main, PRs, scheduled runs | Security vulnerability scanning |
| **Secrets Management** | Manual dispatch, scheduled runs | Secure secrets handling practices |
| **Documentation Build** | Documentation changes, manual dispatch | Build and deploy documentation |

### **Build Job**
1. **Matrix Generation**: Creates build combinations dynamically
2. **Environment Setup**: Sources ESP-IDF environment
3. **Build Directory Setup**: Prepares project structure
4. **Build Execution**: Compiles with specified parameters
5. **Artifact Generation**: Creates size reports and ccache stats
6. **Artifact Upload**: Uploads build outputs and reports

### **Matrix Structure**
Each matrix entry contains:
- `idf_version`: ESP-IDF version for cloning
- `idf_version_docker`: ESP-IDF version for artifacts
- `build_type`: Build type (Debug, Release, etc.)
- `app_name`: Application name
- `config_source`: Configuration source (app or global)

## 📊 Monitoring and Debugging

### **Matrix Analysis**
```bash
# Check total build count
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --pretty | jq '.include | length'

# Analyze specific app builds
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --pretty | jq '.include[] | select(.app_name == "app_name")'

# Hierarchical configuration analysis
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --hierarchical-info
```

### **Build Artifacts**
- **Binary files**: `.bin`, `.elf`, `.map`
- **Size reports**: Component and overall size analysis
- **Ccache statistics**: Build cache performance metrics
- **Build logs**: Detailed compilation output

## 🎯 Best Practices

### **Matrix Optimization**
1. **Limit IDF versions** to those actually needed
2. **Use Debug + Release** as standard build types
3. **Consider CI timeouts** when adding builds
4. **Test configurations** before committing to CI

### **Configuration Management**
1. **Use global defaults** for common settings
2. **Use app-specific overrides** for special requirements
3. **Document complex configurations** with clear comments
4. **Version control** all configuration changes

### **Performance**
1. **Enable ccache** for faster incremental builds
2. **Monitor build times** and adjust as needed
3. **Use parallel builds** when possible
4. **Optimize artifact uploads** for large files

## 🔗 Related Documentation

- [Scripts Documentation](scripts/README.md) - Detailed script usage and configuration
- [ESP32 Examples](../../${{ env.ESP32_PROJECT_PATH }}/README.md) - Application examples and testing
- [Clean Principles Refactor](docs/CLEAN_PRINCIPLES_REFACTOR.md) - Architecture refactoring details
- [App Configuration](../../${{ env.ESP32_PROJECT_PATH }}/app_config.yml) - Application configuration file

## 🚨 Troubleshooting

### **Common Issues**

**1. Matrix Generation Fails**
- Verify `${{ env.ESP32_PROJECT_PATH }}/app_config.yml` syntax
- Check file paths and permissions
- Run with `--hierarchical-info` for analysis

**2. Build Failures**
- Check ESP-IDF version compatibility
- Verify component dependencies
- Review build logs for specific errors

**3. Security Scan Failures**
- Check CodeQL database setup
- Verify dependency scanning configuration
- Review security policy settings

**4. Secrets Management Issues**
- Verify secrets are properly configured
- Check environment access permissions
- Review secrets rotation policies

**5. Documentation Build Failures**
- Check documentation source format
- Verify link validation settings
- Review deployment configuration

**6. Performance Issues**
- Monitor ccache hit rates
- Check build timeouts
- Optimize parallel build configuration

### **Debug Commands**
```bash
# Validate configuration
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --metadata

# Check matrix structure
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --pretty

# Analyze configuration hierarchy
python3 ${{ env.ESP32_PROJECT_PATH }}/scripts/generate_matrix.py --hierarchical-info
```

---

*This workflow system follows clean architecture principles by separating CI concerns from project logic while providing comprehensive build optimization and reporting capabilities.*

## 🔗 Workflow Dependencies and Relationships

### **Primary CI Pipeline**
The **ESP32 Component CI** workflow is the main CI pipeline that:
- Builds and tests all ESP32 applications
- Generates build artifacts and reports
- Provides the foundation for quality assurance

### **Security and Compliance**
The **Security Audit** workflow complements the CI pipeline by:
- Scanning code for security vulnerabilities
- Checking dependencies for known issues
- Ensuring security compliance standards

### **Infrastructure Management**
The **Secrets Management** workflow provides:
- Secure handling of sensitive information
- Best practices for CI/CD security
- Audit trails for secrets access

### **Documentation Quality**
The **Documentation Build** workflow ensures:
- Documentation stays current with code changes
- Quality standards are maintained
- Links and references remain valid

### **Integration Points**
- **ESP32 CI** → **Security Audit**: Code changes trigger security scans
- **ESP32 CI** → **Documentation**: Build artifacts may update documentation
- **All Workflows** → **Secrets Management**: Secure access to required credentials
