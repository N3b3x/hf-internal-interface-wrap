# CI Caching Strategy for ESP32 Examples

## Overview

This document describes the comprehensive caching strategy implemented for the ESP32 CI pipeline to significantly reduce build times and improve CI efficiency.

## Cache Layers

### 1. ESP-IDF and Tools Cache (`~/.espressif`, `~/esp`)

**Purpose**: Cache the complete ESP-IDF installation including all tools, compilers, and Python environments.

**Cache Key**: 
```
esp-idf-{idf_version}-{runner_os}-{hash_of_setup_common.sh}
```

**Contents Cached**:
- ESP-IDF source code (without git history for space optimization)
- All ESP-IDF tools (xtensa-esp32-elf-gcc, riscv32-esp-elf-gcc, etc.)
- Python virtual environments for ESP-IDF tools
- ESP-IDF configuration files

**Cache Invalidation**:
- Changes to `examples/esp32/scripts/setup_common.sh`
- Different ESP-IDF version
- Different runner OS

**Expected Cache Hit Rate**: 90%+ after first successful run

### 2. Python Dependencies Cache (`~/.cache/pip`, `~/.local/lib`)

**Purpose**: Cache all Python packages and dependencies to avoid re-downloading.

**Cache Key**:
```
python-deps-{idf_version}-{runner_os}-{hash_of_setup_common.sh_and_requirements.txt}
```

**Contents Cached**:
- Pip package cache
- Installed Python packages in user site-packages
- Virtual environment packages

**Cache Invalidation**:
- Changes to `examples/esp32/scripts/setup_common.sh`
- Changes to `examples/esp32/requirements.txt`
- Different ESP-IDF version
- Different runner OS

**Expected Cache Hit Rate**: 95%+ after first successful run

### 3. ccache Cache (`~/.ccache`)

**Purpose**: Cache compiled object files to avoid recompilation of unchanged source code.

**Cache Key**:
```
ccache-{idf_version}-{build_type}-{hash_of_source_files}
```

**Contents Cached**:
- Compiled object files
- Preprocessor output
- Compiler cache metadata

**Cache Invalidation**:
- Changes to source files (`src/**`, `inc/**`, `examples/**`)
- Different ESP-IDF version
- Different build type (Debug/Release)

**Expected Cache Hit Rate**: 80-90% for incremental builds

### 4. Docker Buildx Cache (`/tmp/.buildx-cache`)

**Purpose**: Cache Docker layers for containerized builds.

**Cache Key**:
```
{runner_os}-buildx-{idf_version}
```

**Contents Cached**:
- Docker image layers
- Build context cache

**Expected Cache Hit Rate**: 70-80% for similar builds

## Cache Optimization Strategies

### 1. Smart Installation Logic

The setup scripts now check for existing cached components before installing:

```bash
# Cache-aware ESP-IDF installation
if [[ -d "$HOME/.espressif" && -d "$HOME/esp/esp-idf" ]]; then
    echo "✅ ESP-IDF found in cache, skipping installation"
else
    echo "🔄 ESP-IDF not found in cache, installing..."
    install_esp_idf
fi
```

### 2. Space Optimization

- **Git History Removal**: ESP-IDF git history is removed after installation to save 100-200MB
- **Build File Cleanup**: Temporary build files are cleaned up
- **Pip Cache Management**: Large pip caches (>500MB) are automatically cleaned

### 3. Cache-Aware Verification

The setup process verifies that cached components are fully functional:

```bash
# Verify critical tools are available
for tool in "idf.py" "esptool.py" "idf-size"; do
    if command_exists "$tool" || [[ -f "$HOME/.espressif/python_env/idf5.5_py3.10_env/bin/$tool" ]]; then
        tools_available=$((tools_available + 1))
    fi
done
```

## Cache Performance Metrics

### Expected Time Savings

| Component | Without Cache | With Cache | Time Saved |
|-----------|---------------|------------|------------|
| ESP-IDF Installation | 5-10 minutes | 0-30 seconds | 90-95% |
| Python Dependencies | 2-5 minutes | 0-10 seconds | 95-98% |
| First Build | 15-25 minutes | 5-10 minutes | 60-70% |
| Incremental Build | 5-10 minutes | 1-3 minutes | 70-80% |

### Cache Size Estimates

| Cache Type | Estimated Size | Optimization Applied |
|------------|----------------|---------------------|
| ESP-IDF Tools | 2-3 GB | Git history removal |
| ESP-IDF Source | 500-800 MB | Git history removal |
| Python Dependencies | 200-500 MB | Automatic cleanup |
| ccache | 100-500 MB | Size-based cleanup |

## Cache Troubleshooting

### Common Issues

1. **Cache Miss on Every Run**
   - Check if cache keys are changing unexpectedly
   - Verify file hashes used in cache keys
   - Check for environment variable changes

2. **Large Cache Sizes**
   - Run `optimize_cache_for_ci` function
   - Check for unnecessary files in cached directories
   - Verify cleanup operations are working

3. **Cache Corruption**
   - Use `clean_build: true` input to bypass cache
   - Check GitHub Actions cache storage limits
   - Verify cache restore operations

### Debug Commands

```bash
# Check cache status
./scripts/setup_ci.sh

# View cache statistics
du -sh ~/.espressif ~/esp ~/.cache/pip ~/.ccache

# Manual cache cleanup
rm -rf ~/.espressif ~/esp ~/.cache/pip ~/.ccache
```

## Best Practices

### For Developers

1. **Minimize Setup Changes**: Changes to setup scripts invalidate caches
2. **Use Requirements Files**: Keep Python dependencies in `requirements.txt`
3. **Avoid Large Files**: Large files in source directories increase cache size
4. **Test Cache Behavior**: Verify cache hits in CI logs

### For CI Maintenance

1. **Monitor Cache Hit Rates**: Track cache performance over time
2. **Optimize Cache Keys**: Balance specificity with cache reuse
3. **Clean Old Caches**: Remove outdated cache entries
4. **Update Dependencies**: Keep requirements files current

## Future Improvements

1. **Multi-version Caching**: Cache multiple ESP-IDF versions simultaneously
2. **Incremental ESP-IDF Updates**: Only download changed components
3. **Cross-runner Cache Sharing**: Share caches between different runner types
4. **Cache Compression**: Compress large cache entries for faster upload/download
