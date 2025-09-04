# 🚀 HardFOC ESP32 CI/CD Pipeline Documentation

<div align="center">

![CI/CD](https://img.shields.io/badge/CI%2FCD-Advanced%20Workflows-blue?style=for-the-badge&logo=github-actions)
![ESP32](https://img.shields.io/badge/ESP32-C6%20Support-green?style=for-the-badge&logo=espressif)
![Security](https://img.shields.io/badge/Security-First%20Approach-orange?style=for-the-badge&logo=security)

**📋 Comprehensive CI/CD Pipeline for HardFOC ESP32 Development**

*Advanced automated workflows leveraging custom ESP-IDF project tools*

</div>

---

## 📋 Table of Contents

- [🎯 Overview](#-overview)
- [🏗️ Workflow Architecture](#️-workflow-architecture)
- [🚀 Available Workflows](#-available-workflows)
- [🔧 Configuration](#-configuration)
- [📊 Workflow Selection Guide](#-workflow-selection-guide)
- [🛡️ Security Features](#️-security-features)
- [⚡ Performance Optimization](#-performance-optimization)
- [🔍 Troubleshooting](#-troubleshooting)

---

## 🎯 Overview

The HardFOC ESP32 CI/CD pipeline provides comprehensive automated testing, building, and deployment capabilities using custom ESP-IDF project tools. The pipeline is designed for maximum efficiency, security, and reliability.

### **Key Features**
- 🔄 **Parallel Execution** - Multiple jobs run simultaneously for maximum efficiency
- 🛡️ **Security-First** - Comprehensive security scanning and vulnerability detection
- 📊 **Smart Caching** - Optimized for fast builds and minimal resource usage
- 🎯 **HardFOC Optimized** - Specifically designed for ESP32 development workflows
- 🔧 **Custom Tools Integration** - Leverages [hf-espidf-project-tools](https://github.com/N3b3x/hf-espidf-project-tools)

---

## 🏗️ Workflow Architecture

### **Custom Tools Integration**

All workflows use the custom ESP-IDF project tools as a Git submodule:

```
examples/esp32/scripts/  # Git submodule: hf-espidf-project-tools
├── build_app.sh         # Build automation
├── generate_matrix.py   # Matrix generation
├── setup_ci.sh         # CI environment setup
└── ...                 # Additional tools
```

### **Workflow Dependencies**

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           🚀 CI Workflows Layer                                 │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                              │
│  │ Advanced    │  │ Release     │  │ Docs        │                              │
│  │ CI          │  │ CI          │  │ CI          │                              │
│  │             │  │             │  │             │                              │
│  │ • Matrix    │  │ • Clean     │  │ • Doxygen   │                              │
│  │   Builds    │  │   Builds    │  │ • Link      │                              │
│  │ • Full      │  │ • Artifacts │  │   Check     │                              │
│  │   Linting   │  │ • Releases  │  │ • Markdown  │                              │
│  │ • Security  │  │ • Waits for │  │ • Spell     │                              │
│  │ • Static    │  │   Advanced  │  │   Check     │                              │
│  │   Analysis  │  │   CI        │  │             │                              │
│  └─────────────┘  └─────────────┘  └─────────────┘                              │
└─────────────────────────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                        🛡️ External Actions Layer                                │
├─────────────────────────────────────────────────────────────────────────────────┤
│                 ┌───────────────────────────┐                                   │
│  ┌─────────────────┐    ┌─────────────────┐ │  ┌─────────────────┐              │
│  │ N3b3x/          │    │ espressif/      │ └─▶│ Security        │              │
│  │ hf-espidf-      │───▶│ esp-idf-ci-     │    │ Scanners        │              │
│  │ ci-tools        │    │ action          │    │                 │              │
│  │                 │    │                 │    │ • CodeQL        │              │
│  │ • Build         │    │ • ESP-IDF       │    │ • Trivy         │              │
│  │ • Lint          │    │   Environment   │    │ • Bandit        │              │
│  │ • Security      │    │ • Docker        │    │ • GitLeaks      │              │
│  │ • Docs          │    │   Containers    │    │ • TruffleHog    │              │
│  │ • Static        │    │ • Toolchain     │    │ • detect-secrets│              │
│  │   Analysis      │    │   Setup         │    │                 │              │
│  └─────────────────┘    └─────────────────┘    └─────────────────┘              │
└─────────────────────────────────────────────────────────────────────────────────┘
            │
            ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           🔧 Custom Tools Layer                                 │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐              │
│  │ hf-espidf-      │    │ Matrix          │    │ Build           │              │
│  │ project-tools   │───▶│ Generation      │    │ Automation      │              │
│  │ (Git Submodule) │    │                 │    │                 │              │
│  └─────────────────┘    └─────────────────┘    └─────────────────┘              │
│           │                       │                       │                     │
│           └───────────────────────┼───────────────────────┘                     │
│                                   │                                             │
│  ┌────────────────────────────────▼──────────────────────────────────┐          │
│  │                    Environment Setup                              │          │
│  └───────────────────────────────────────────────────────────────────┘          │
└─────────────────────────────────────────────────────────────────────────────────┘

Data Flow:
CI Workflows → External Actions → Custom Tools → Hardware/Artifacts
       │              │                │               │
       ▼              ▼                ▼               ▼
┌─────────────┐  ┌─────────────┐  ┌─────────┐ ┌─────────────┐
│ Parallel    │  │ ESP-IDF     │  │ Matrix  │ │ Firmware    │
│ Execution   │  │ Build       │  │ Config  │ │ Binaries    │
│             │  │             │  │         │ │             │
│ Security    │  │ Docker      │  │ Build   │ │ Artifacts   │
│ Scanning    │  │ Containers  │  │ Scripts │ │             │
│             │  │             │  │         │ │             │
│ Docs        │  │ Toolchain   │  │ Setup   │ │ Reports     │
│ Generation  │  │ Management  │  │ Scripts │ │             │
└─────────────┘  └─────────────┘  └─────────┘ └─────────────┘
```

---

## 🚀 Available Workflows

### **1. 🚀 Advanced ESP32 CI** ([`esp32-advanced-ci.yml`](esp32-advanced-ci.yml))

**Purpose**: Comprehensive CI/CD pipeline for all development work

**Triggers**:
- Push to `main`, `develop`, `feature/*`, `bugfix/*`, `release/*` branches
- Pull requests to `main`, `develop`
- Manual dispatch with options

**Features**:
- ✅ Matrix-based parallel builds
- ✅ Comprehensive linting and formatting
- ✅ Full security audit
- ✅ Static analysis
- ✅ Link checking
- ✅ Build summary and notifications

**Usage**:
```yaml
# Manual trigger with options
workflow_dispatch:
  inputs:
    clean_build: false
    run_security: true
```

### **2. 📦 Release ESP32 CI** ([`esp32-release-ci.yml`](esp32-release-ci.yml))

**Purpose**: Production release pipeline with artifact creation

**Triggers**:
- Push to `release/*` branches
- Tags matching `v*` pattern
- Manual dispatch with version input

**Features**:
- ✅ Waits for Advanced CI to complete
- ✅ Clean builds for release
- ✅ Release artifact creation
- ✅ GitHub release creation

**Usage**:
```yaml
# Manual trigger with version
workflow_dispatch:
  inputs:
    release_version: "v1.0.0"
```

### **3. 📚 Documentation CI** ([`docs.yml`](docs.yml))

**Purpose**: Repository-wide documentation generation and validation

**Triggers**:
- Push to `main`, `release/*` branches
- Pull requests to `main` (docs changes only)

**Features**:
- ✅ Doxygen documentation generation
- ✅ Comprehensive link checking
- ✅ Markdown linting
- ✅ Spell checking
- ✅ GitHub Pages deployment

---

## 🔧 Configuration

### **Environment Variables**

All workflows use consistent environment variables:

```yaml
env:
  PROJECT_DIR: examples/esp32
  TOOLS_DIR: examples/esp32/scripts
  CACHE_VERSION: v1
```

### **Custom Tools Configuration**

The workflows automatically use the custom tools from the Git submodule:

```yaml
# Matrix generation
python3 ${{ env.TOOLS_DIR }}/generate_matrix.py --validate --verbose

# Build automation
./${{ env.TOOLS_DIR }}/build_app.sh "${{ matrix.app_name }}" "${{ matrix.build_type }}"
```

### **External Actions Integration**

All workflows leverage the custom ESP-IDF CI tools:

```yaml
# Build workflow
uses: N3b3x/hf-espidf-ci-tools/.github/workflows/build.yml@v1

# Lint workflow
uses: N3b3x/hf-espidf-ci-tools/.github/workflows/lint.yml@v1

# Security workflow
uses: N3b3x/hf-espidf-ci-tools/.github/workflows/security.yml@v1
```

---

## 📊 Workflow Selection Guide

| Use Case | Recommended Workflow | Key Features |
|----------|---------------------|--------------|
| **Production Projects** | Advanced CI | All checks in parallel |
| **Development** | Advanced CI | Comprehensive validation |
| **Releases** | Release CI | Waits for Advanced CI + artifacts |
| **Documentation Heavy** | Documentation CI | Doc-focused checks |

### **Workflow Triggers Summary**

| Workflow | Main | Develop | Feature/* | Release/* | Tags | Manual |
|----------|------|---------|-----------|-----------|------|--------|
| Advanced | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ |
| Release | ❌ | ❌ | ❌ | ✅* | ✅ | ✅ |
| Documentation | ✅ | ❌ | ❌ | ✅ | ❌ | ❌ |

*Release CI waits for Advanced CI to complete on release branches

### **🔄 Workflow Dependencies**

**Development Flow:**
```
Push to develop/feature/*/bugfix/* branch
    ↓
Advanced CI runs (comprehensive validation)
```

**Main Branch Flow:**
```
Push to main branch
    ↓
Advanced CI runs (validation)
    ↓
Documentation CI runs (doc generation)
```

**Release Branch Flow:**
```
Push to release/* branch
    ↓
Advanced CI runs (validation)
    ↓
Documentation CI runs (doc generation)
    ↓
Release CI waits for Advanced CI to pass
    ↓
Release CI creates artifacts + GitHub release
```

**Tag Flow:**
```
Create tag (v1.0.0)
    ↓
Release CI runs directly (no wait needed)
    ↓
Creates artifacts + GitHub release
```

---

## 🛡️ Security Features

### **Comprehensive Security Scanning**

- **Dependency Scanning**: pip-audit, Safety
- **Code Analysis**: Bandit, CodeQL
- **Secret Detection**: GitLeaks, TruffleHog, detect-secrets
- **Vulnerability Scanning**: Trivy
- **ESP-IDF Security**: Component analysis and best practices

### **Security Workflow Features**

```yaml
# Security audit configuration
security:
  scan_type: "all"
  run_codeql: true
  codeql_languages: "cpp,python"
  tools_repo_sha: ${{ github.sha }}
```

### **Security Best Practices**

- ✅ Weekly scheduled security scans
- ✅ Security-first workflow for critical branches
- ✅ Comprehensive vulnerability reporting
- ✅ Automated security artifact generation
- ✅ PR security comments and annotations

---

## ⚡ Performance Optimization

### **Caching Strategy**

- **ESP-IDF Cache**: Handled by ESP-IDF CI action
- **Python Dependencies**: Cached based on requirements files
- **ccache**: Compilation cache for faster builds
- **Essential Tools**: Cached for faster setup

### **Parallel Execution**

- **Matrix Builds**: Multiple configurations in parallel
- **Independent Jobs**: No unnecessary dependencies
- **Fail-Fast**: Configurable failure handling
- **Resource Optimization**: Efficient runner usage

### **Performance Monitoring**

```yaml
# Performance metrics
performance-check:
  steps:
    - name: Check build times
      run: |
        echo "Build completed in: ${{ github.run_duration }}"
        echo "Cache hit rate: High (optimized configuration)"
```

---

## 🔍 Troubleshooting

### **Common Issues**

#### **Submodule Issues**
```bash
# Update submodule
git submodule update --init --recursive

# Check submodule status
git submodule status
```

#### **Matrix Generation Issues**
```bash
# Validate configuration
cd examples/esp32
python3 scripts/generate_matrix.py --validate --verbose

# Check app_config.yml
cat app_config.yml
```

#### **Build Failures**
```bash
# Clean build
CLEAN=1 ./scripts/build_app.sh gpio_test Release

# Check build logs
cat build-*/log/build.log
```

#### **Permission Issues**
```bash
# Make scripts executable
chmod +x examples/esp32/scripts/*.sh
```

### **Debug Mode**

Enable verbose output for debugging:

```yaml
# Verbose build
./scripts/build_app.sh --verbose gpio_test Release

# Clean rebuild
CLEAN=1 ./scripts/build_app.sh gpio_test Release
```

### **Cache Issues**

```bash
# Clear caches
rm -rf ~/.ccache
rm -rf ~/.cache/pip

# Force clean build
CLEAN=1 ./scripts/build_app.sh gpio_test Release
```

---

## 📚 Related Documentation

- [Custom ESP-IDF Project Tools](https://github.com/N3b3x/hf-espidf-project-tools)
- [ESP-IDF CI Tools](https://github.com/N3b3x/hf-espidf-ci-tools)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)

---

## 🤝 Contributing

### **Adding New Workflows**

1. **Create Workflow File**
   ```bash
   touch .github/workflows/new-workflow.yml
   ```

2. **Follow Naming Convention**
   - Use descriptive names: `esp32-{purpose}-ci.yml`
   - Include emoji for visual identification

3. **Use Custom Tools**
   - Always reference `${{ env.TOOLS_DIR }}`
   - Use external actions from `N3b3x/hf-espidf-ci-tools`

4. **Test Integration**
   - Test with existing workflows
   - Verify custom tools integration
   - Check error handling

### **Modifying Existing Workflows**

1. **Backup Original**
```bash
   cp .github/workflows/workflow.yml .github/workflows/workflow.yml.backup
   ```

2. **Make Changes**
   - Maintain backward compatibility
   - Update documentation
   - Test thoroughly

3. **Validate Changes**
   ```bash
   # Check YAML syntax
   yamllint .github/workflows/workflow.yml
   
   # Check GitHub Actions syntax
   actionlint .github/workflows/workflow.yml
   ```

---

<div align="center">

**🚀 HardFOC ESP32 CI/CD Pipeline**

*Advanced automated workflows for professional ESP32 development*

---

**🔗 Quick Links**

[📋 Workflow Files](.) | [🔧 Custom Tools](../../examples/esp32/scripts/) | [📚 Documentation](../../docs/) | [🤝 Contributing](../../CONTRIBUTING.md)

</div>