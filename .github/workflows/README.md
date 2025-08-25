# 🔄 GitHub Actions Workflows

This directory contains all GitHub Actions workflows and their associated resources for the ESP32 HardFOC Interface Wrapper project.

## 📁 Directory Structure

```
.github/workflows/
├── README.md                           # This file - workflows overview
├── esp32-component-ci.yml             # Main ESP32 CI pipeline
├── generate_matrix.py                  # Build matrix generation script
├── secrets-management-guide.yml       # Secrets management workflow
├── security-audit.yml                 # Security scanning workflow
├── docs.yml                           # Documentation build workflow
├── scripts/                           # CI-specific scripts
│   ├── README.md                      # Scripts documentation
│   ├── setup_build_directory.sh       # ESP32 build directory setup
│   └── prepare_build_directory.sh     # Build directory preparation only
└── docs/                              # Workflow documentation
    └── CLEAN_PRINCIPLES_REFACTOR.md   # CI refactoring documentation
```

## 🚀 Workflows Overview

### **`esp32-component-ci.yml`** - Main ESP32 CI Pipeline
- **Triggers**: Push to main, PRs, manual dispatch
- **Purpose**: Build, test, and analyze ESP32 components
- **Features**: Multi-ESP-IDF version support, caching, static analysis
- **Scripts used**: `setup_build_directory.sh`

### **`generate_matrix.py`** - Build Matrix Generation
- **Purpose**: Dynamically generates build matrices for CI
- **Input**: Project configuration and ESP-IDF versions
- **Output**: JSON matrix for GitHub Actions

### **`secrets-management-guide.yml`** - Secrets Management
- **Purpose**: Demonstrates secure secrets handling
- **Features**: Environment-specific secrets, rotation strategies

### **`security-audit.yml`** - Security Scanning
- **Purpose**: Automated security vulnerability scanning
- **Tools**: CodeQL, dependency scanning, SAST

### **`docs.yml`** - Documentation Build
- **Purpose**: Build and deploy project documentation
- **Triggers**: Documentation changes

## 🔧 CI Scripts

The `scripts/` directory contains scripts specifically designed for CI workflows:

- **`setup_build_directory.sh`** - Complete ESP32 build setup and building
- **`prepare_build_directory.sh`** - Build directory preparation only

These scripts follow clean architecture principles by separating CI concerns from project logic.

## 📚 Documentation

- **Scripts**: [scripts/README.md](scripts/README.md)
- **Clean Principles**: [docs/CLEAN_PRINCIPLES_REFACTOR.md](docs/CLEAN_PRINCIPLES_REFACTOR.md)
- **CI Strategy**: [../CI_CACHING_STRATEGY.md](../CI_CACHING_STRATEGY.md)

## 🎯 Design Principles

### **Clean Architecture**
- Workflows focus on CI orchestration
- Business logic is in dedicated scripts
- Clear separation of concerns

### **Reusability**
- Scripts can be used in multiple workflows
- Scripts can be tested locally
- Scripts can be used in other CI systems

### **Maintainability**
- Each workflow has a single responsibility
- Scripts are version-controlled and testable
- Clear documentation and examples

## 🚨 Troubleshooting

### **Common Issues**

#### **Script Not Found**
```bash
# Error: Script not found
ERROR: .github/workflows/scripts/setup_build_directory.sh not found

# Solution: Ensure scripts are in the correct location
ls -la .github/workflows/scripts/
```

#### **Permission Denied**
```bash
# Error: Permission denied
ERROR: Permission denied

# Solution: Ensure scripts are executable
chmod +x .github/workflows/scripts/*.sh
```

#### **Build Failures**
```bash
# Check script help for correct usage
./.github/workflows/scripts/setup_build_directory.sh --help

# Verify prerequisites
./.github/workflows/scripts/setup_build_directory.sh \
  -p test_build \
  -t esp32c6 \
  -b Debug \
  -a test_app
```

## 🔮 Future Enhancements

### **Additional Workflows**
- Performance benchmarking
- Release automation
- Dependency updates

### **Script Improvements**
- More granular build options
- Better error reporting
- Integration with testing frameworks

### **Documentation**
- Workflow diagrams
- Performance metrics
- Best practices guide

## 🤝 Contributing

When adding new workflows or modifying existing ones:

1. **Follow single responsibility** - each workflow should do one thing well
2. **Use existing scripts** - leverage the scripts in the `scripts/` directory
3. **Add documentation** - update this README and related docs
4. **Test locally** - verify scripts work before committing
5. **Follow naming conventions** - use descriptive names and consistent structure

## 📋 Requirements

### **Workflow Requirements**
- Clear purpose and triggers
- Proper error handling
- Comprehensive logging
- Efficient caching strategies

### **Script Requirements**
- Command-line interface
- Help documentation
- Error validation
- CI-friendly output

---

*This workflows directory demonstrates clean CI/CD architecture with separated concerns, reusable components, and comprehensive documentation.*