# 🏗️ Hierarchical Configuration System

This document explains how the ESP32 CI pipeline uses a hierarchical configuration system to manage IDF versions and build types across different apps.

## 🎯 Overview

The hierarchical configuration system allows you to:
- **Set global defaults** for IDF versions and build types in the `metadata` section
- **Override per-app** when specific apps need different configurations
- **Maintain backward compatibility** for existing configurations
- **Flexibly manage** complex CI requirements

## 🔧 How It Works

### **1. Global Configuration (Metadata Section)**
```yaml
metadata:
  idf_versions: ["release/v5.5"]  # Default for all apps
  target: "esp32c6"
  # ... other global settings
```

### **2. Per-App Overrides**
```yaml
apps:
  # App using global settings (no overrides)
  ascii_art:
    description: "ASCII art generator example"
    source_file: "AsciiArtComprehensiveTest.cpp"
    # Uses global idf_versions and build_types automatically
    
  # App with custom IDF versions
  demo_legacy_test:
    description: "Legacy compatibility testing"
    source_file: "DemoLegacyTest.cpp"
    idf_versions: ["release/v4.4", "release/v5.0"]  # Override global
    build_types: ["Release"]  # Override global
    
  # App with custom build types only
  demo_performance_test:
    description: "Performance-critical testing"
    source_file: "DemoPerformanceTest.cpp"
    # Uses global idf_versions (no override)
    build_types: ["Release", "RelWithDebInfo"]  # Override global
```

## 📊 Configuration Precedence Rules

### **Priority Order (Highest to Lowest)**
1. **App-specific overrides** - If an app defines `idf_versions` or `build_types`
2. **Global defaults** - From `metadata` section
3. **Hardcoded fallbacks** - Built-in safety defaults

### **Override Detection Logic**
```python
# Per-app overrides (app-specific settings take precedence)
per_app_build_types = app_config.get('build_types', global_build_types)
per_app_idf_versions = app_config.get('idf_versions', global_idf_versions)

# Use app-specific settings if defined, otherwise fall back to global
effective_build_types = per_app_build_types if 'build_types' in app_config else global_build_types
effective_idf_versions = per_app_idf_versions if 'idf_versions' in app_config else global_idf_versions
```

## 🚀 Real-World Examples

### **Example 1: Standard App (Uses Global)**
```yaml
gpio_test:
  description: "GPIO peripheral testing"
  source_file: "GpioComprehensiveTest.cpp"
  # No overrides - uses global settings
  # Result: Builds with release/v5.5 in Debug, Release, RelWithDebInfo, MinSizeRel
```

**Generated Matrix Entries:**
```json
{
  "idf_version": "release/v5.5",
  "build_type": "Debug",
  "app_name": "gpio_test",
  "config_source": "global"
},
{
  "idf_version": "release/v5.5", 
  "build_type": "Release",
  "app_name": "gpio_test",
  "config_source": "global"
}
// ... 2 more entries for RelWithDebInfo and MinSizeRel
```

### **Example 2: Legacy Compatibility App (Full Override)**
```yaml
demo_legacy_test:
  description: "Legacy compatibility testing"
  source_file: "DemoLegacyTest.cpp"
  idf_versions: ["release/v4.4", "release/v5.0"]  # Override global
  build_types: ["Release"]  # Override global
```

**Generated Matrix Entries:**
```json
{
  "idf_version": "release/v4.4",
  "build_type": "Release", 
  "app_name": "demo_legacy_test",
  "config_source": "app"
},
{
  "idf_version": "release/v5.0",
  "build_type": "Release",
  "app_name": "demo_legacy_test", 
  "config_source": "app"
}
```

### **Example 3: Performance App (Partial Override)**
```yaml
demo_performance_test:
  description: "Performance-critical testing"
  source_file: "DemoPerformanceTest.cpp"
  # Uses global idf_versions (no override)
  build_types: ["Release", "RelWithDebInfo"]  # Override global
```

**Generated Matrix Entries:**
```json
{
  "idf_version": "release/v5.5",  # From global
  "build_type": "Release",
  "app_name": "demo_performance_test",
  "config_source": "app"
},
{
  "idf_version": "release/v5.5",  # From global
  "build_type": "RelWithDebInfo", 
  "app_name": "demo_performance_test",
  "config_source": "app"
}
```

## 🔍 Matrix Analysis Tools

### **1. Hierarchical Configuration Analysis**
```bash
cd .github/workflows
python3 generate_matrix.py --hierarchical-info
```

**Output:**
```
=== Hierarchical Configuration Analysis ===
Global IDF versions: ['release/v5.5']
Global build types: ['Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel']

Per-app overrides:
  ascii_art:
    IDF versions: uses global
    Build types: uses global
  demo_legacy_test:
    IDF versions: ['release/v4.4', 'release/v5.0']
    Build types: ['Release']
  demo_performance_test:
    IDF versions: uses global
    Build types: ['Release', 'RelWithDebInfo']
```

### **2. Full Matrix Generation**
```bash
python3 generate_matrix.py --pretty
```

### **3. Matrix Statistics**
```bash
python3 generate_matrix.py --pretty | jq '.include | length'
# Shows total number of build combinations
```

## 📈 Matrix Size Calculation

### **Formula**
```
Total Builds = Sum(per_app_idf_versions × per_app_build_types)
```

### **Example Calculation**
- **Standard Apps (16 apps)**: 16 × 4 × 1 = 64 builds
- **Legacy App**: 1 × 2 × 1 = 2 builds  
- **Performance App**: 1 × 1 × 2 = 2 builds
- **Total**: 64 + 2 + 2 = 68 builds

## 🎛️ Configuration Options

### **Available Build Types**
- `Debug` - Full debugging symbols, optimization disabled
- `Release` - Optimized for performance
- `RelWithDebInfo` - Release with debug info
- `MinSizeRel` - Optimized for size

### **Available IDF Versions**
- `release/v4.4` - Legacy ESP-IDF v4.4
- `release/v5.0` - ESP-IDF v5.0
- `release/v5.5` - Current stable ESP-IDF v5.5
- Custom versions as needed

## 🔧 Advanced Configuration

### **Excluding Specific Combinations**
```yaml
ci_config:
  exclude_combinations:
    - app_name: "wifi_test"
      build_type: "Debug"
      reason: "WiFi debug builds are unstable in CI"
```

### **Conditional Overrides**
```yaml
apps:
  conditional_test:
    description: "Conditional testing"
    source_file: "ConditionalTest.cpp"
    # Override based on conditions
    idf_versions: ["release/v5.5"]  # Only latest for this app
    build_types: ["Release", "MinSizeRel"]  # Only optimized builds
```

## 🚨 Best Practices

### **1. Use Global Defaults When Possible**
- Only override when absolutely necessary
- Keeps configuration simple and maintainable

### **2. Document Override Reasons**
- Add comments explaining why overrides are needed
- Helps future maintainers understand decisions

### **3. Test Override Combinations**
- Use `--hierarchical-info` to verify configuration
- Ensure matrix generation works correctly

### **4. Keep Overrides Minimal**
- Override only what's different from global
- Don't duplicate global settings unnecessarily

## 🔮 Future Enhancements

### **Potential Features**
- **Environment-based overrides** - Different configs for different CI environments
- **Conditional overrides** - Override based on app properties or conditions
- **Template inheritance** - Reuse common override patterns
- **Validation rules** - Ensure override combinations are valid

### **Example Future Syntax**
```yaml
apps:
  future_test:
    description: "Future enhancement example"
    source_file: "FutureTest.cpp"
    # Environment-based overrides
    overrides:
      production:
        build_types: ["Release"]
        idf_versions: ["release/v5.5"]
      development:
        build_types: ["Debug", "Release"]
        idf_versions: ["release/v5.5", "master"]
```

## 📚 Related Documentation

- [CI Pipeline Overview](../README.md)
- [Scripts Documentation](../scripts/README.md)
- [Clean Principles Refactor](CLEAN_PRINCIPLES_REFACTOR.md)
- [CI Caching Strategy](../../CI_CACHING_STRATEGY.md)

---

*The hierarchical configuration system provides flexible, maintainable CI configuration while preserving backward compatibility and clear precedence rules.*
