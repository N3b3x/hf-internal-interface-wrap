# CI Caching Strategy for ESP32 Component CI

## Overview

This document describes the comprehensive caching strategy implemented for the ESP32 CI pipeline in `.github/workflows/esp32-component-ci.yml` to significantly reduce build times and improve CI efficiency.

## Cache Layers

### 1. Development Tools Cache (`~/.cache/apt`, `/usr/local/bin`)

**Purpose**: Cache development tools setup including clang-20 and other build dependencies.

**Cache Key**: 
```
dev-tools-{runner_os}-{hash_of_setup_ci.sh}
```

**Contents Cached**:
- APT package cache
- Development tools in `/usr/local/bin`
- LLVM/Clang installations

**Cache Invalidation**:
- Changes to `.github/workflows/setup_ci.sh`
- Different runner OS

### 2. ESP-IDF and Tools Cache (`~/.espressif`, `~/esp`)

**Purpose**: Cache the complete ESP-IDF installation including all tools, compilers, and Python environments.

**Cache Key**: 
```
esp-idf-{idf_version_docker}-{runner_os}-{hash_of_setup_common.sh}
```

**Contents Cached**:
- ESP-IDF source code (without git history for space optimization)
- All ESP-IDF tools (xtensa-esp32-elf-gcc, riscv32-esp-elf-gcc, etc.)
- Python virtual environments for ESP-IDF tools
- ESP-IDF configuration files

**Cache Invalidation**:
- Changes to `examples/esp32/scripts/setup_common.sh`
- Different ESP-IDF version (from matrix)
- Different runner OS

**Expected Cache Hit Rate**: 90%+ after first successful run

### 3. Python Dependencies Cache (`~/.cache/pip`, `~/.local/lib`)

**Purpose**: Cache all Python packages and dependencies to avoid re-downloading.

**Cache Key**:
```
python-deps-{idf_version_docker}-{runner_os}-{hash_of_setup_common.sh_and_requirements.txt}
```

**Contents Cached**:
- Pip package cache
- Installed Python packages in user site-packages
- Virtual environment packages

**Cache Invalidation**:
- Changes to `examples/esp32/scripts/setup_common.sh`
- Changes to `examples/esp32/requirements.txt`
- Different ESP-IDF version (from matrix)
- Different runner OS

**Expected Cache Hit Rate**: 95%+ after first successful run

### 4. ccache Cache (`~/.ccache`)

**Purpose**: Cache compiled object files to avoid recompilation of unchanged source code.

**Cache Key**:
```
ccache-{idf_version_docker}-{build_type}-{hash_of_source_files}
```

**Contents Cached**:
- Compiled object files
- Preprocessor output
- Compiler cache metadata

**Cache Invalidation**:
- Changes to source files (`src/**`, `inc/**`, `examples/**`)
- Different ESP-IDF version (from matrix)
- Different build type (Debug/Release)

**Expected Cache Hit Rate**: 80-90% for incremental builds

### 5. Docker Buildx Cache (`/tmp/.buildx-cache`)

**Purpose**: Cache Docker layers for containerized builds using `espressif/esp-idf-ci-action@v1`.

**Cache Key**:
```
{runner_os}-buildx-{idf_version_docker}
```

**Contents Cached**:
- Docker image layers
- Build context cache

**Expected Cache Hit Rate**: 70-80% for similar builds

### 6. Static Analysis Tools Cache

**Purpose**: Cache static analysis tools and their outputs.

**Cache Key**:
```
static-analysis-{runner_os}-{hash_of_source_and_config_files}
```

**Contents Cached**:
- APT package cache for analysis tools
- Clang-tidy cache
- cppcheck installations

## Cache Control Features

### Clean Build Option

The workflow supports a `clean_build` input parameter that bypasses ALL caches:

```yaml
workflow_dispatch:
  inputs:
    clean_build:
      description: 'Force clean build (ignore all caches)'
      required: false
      default: false
      type: boolean
```

When `clean_build` is set to `true`, all cache steps are skipped using:
```yaml
if: ${{ !inputs.clean_build }}
```

### Cache Logging and Monitoring

The workflow includes comprehensive cache hit logging:

```yaml
- name: Log cache results
  run: |
    if [ "${{ inputs.clean_build }}" = "true" ]; then
      echo "🧹 Clean build requested - all caches skipped"
    else
      echo "ESP-IDF cache hit - ${{ steps.esp-idf-cache.outputs.cache-hit }}"
      echo "Python deps cache hit - ${{ steps.python-cache.outputs.cache-hit }}"
      echo "ccache cache hit - ${{ steps.ccache-cache.outputs.cache-hit }}"
    fi
```

## Matrix Strategy Integration

The caching strategy integrates with the dynamic matrix generation system:

- **Matrix Variables**: `idf_version_docker`, `build_type`, `example_type`
- **Cache Scope**: Different matrix combinations get separate caches
- **Optimization**: Related matrix builds can share base caches (ESP-IDF, Python deps)

## Docker Integration

### ESP-IDF CI Action Caching

The workflow uses the official `espressif/esp-idf-ci-action@v1` with ccache integration:

```yaml
extra_docker_args: >-
  -v $HOME/.ccache:/root/.ccache
  -e CCACHE_DIR=/root/.ccache
  -e IDF_CCACHE_ENABLE=1
```

### ccache Statistics

The workflow collects and uploads ccache statistics:

```bash
ccache -s &&
ccache -s > ${{ env.BUILD_PATH }}/build/ccache_stats.txt
```

## Cache Performance Metrics

### Expected Time Savings

| Component | Without Cache | With Cache | Time Saved |
|-----------|---------------|------------|------------|
| Development Tools | 2-5 minutes | 0-30 seconds | 85-95% |
| ESP-IDF Installation | 5-10 minutes | 0-30 seconds | 90-95% |
| Python Dependencies | 2-5 minutes | 0-10 seconds | 95-98% |
| Docker Layers | 3-8 minutes | 30 seconds-2 minutes | 60-85% |
| First Build | 15-25 minutes | 5-10 minutes | 60-70% |
| Incremental Build | 5-10 minutes | 1-3 minutes | 70-80% |
| Static Analysis | 3-8 minutes | 1-3 minutes | 60-75% |

### Cache Size Estimates

| Cache Type | Estimated Size | Optimization Applied |
|------------|----------------|---------------------|
| ESP-IDF Tools | 2-3 GB | Git history removal |
| ESP-IDF Source | 500-800 MB | Git history removal |
| Python Dependencies | 200-500 MB | Automatic cleanup |
| ccache | 100-500 MB | Size-based cleanup |
| Docker Layers | 1-2 GB | Layer optimization |
| Static Analysis Tools | 100-300 MB | APT cache management |

## Workflow-Specific Configuration

### Environment Variables

```yaml
env:
  IDF_TARGET: esp32c6
  BUILD_PATH: ci_project
  IDF_CCACHE_ENABLE: "1"  # Enables ccache inside ESP-IDF
```

### Concurrency Control

```yaml
concurrency:
  group: ci-${{ github.ref }}
  cancel-in-progress: true
```

This ensures that overlapping CI runs don't interfere with each other's caches.

### Artifact Uploads

Build artifacts are cached separately from the main cache system:

```yaml
- name: Upload artifacts
  uses: actions/upload-artifact@v4
  with:
    name: fw-${{ matrix.example_type }}-${{ matrix.idf_version_docker }}-${{ matrix.build_type }}
    retention-days: 7
    path: |
      ${{ env.BUILD_PATH }}/build/*.bin
      ${{ env.BUILD_PATH }}/build/*.elf
      ${{ env.BUILD_PATH }}/build/*.map
      ${{ env.BUILD_PATH }}/build/size.*
      ${{ env.BUILD_PATH }}/build/ccache_stats.txt
```

## Cache Troubleshooting

### Common Issues

1. **Cache Miss on Every Run**
   - Check if cache keys are changing unexpectedly
   - Verify file hashes used in cache keys
   - Check for environment variable changes in matrix

2. **Large Cache Sizes**
   - Monitor GitHub Actions cache storage limits (10GB per repository)
   - Check for unnecessary files in cached directories
   - Verify cleanup operations are working

3. **Cache Corruption**
   - Use `clean_build: true` workflow input to bypass cache
   - Check GitHub Actions cache storage limits
   - Verify cache restore operations in workflow logs

### Debug Commands

To debug caching issues, you can:

1. **Check cache status in workflow logs**:
   - Look for cache hit/miss indicators
   - Review cache key generation
   - Check file hash outputs

2. **Manual cache investigation**:
   ```bash
   # Check cache status (in setup scripts)
   ./examples/esp32/scripts/setup_ci.sh
   
   # View cache statistics (in Docker container)
   du -sh ~/.espressif ~/esp ~/.cache/pip ~/.ccache
   
   # ccache statistics
   ccache -s
   ```

3. **Force clean build**:
   - Use the workflow dispatch with `clean_build: true`
   - This bypasses all caches for debugging

## Best Practices

### For Developers

1. **Minimize Setup Changes**: Changes to setup scripts invalidate caches
2. **Use Requirements Files**: Keep Python dependencies in `examples/esp32/requirements.txt`
3. **Avoid Large Files**: Large files in source directories increase cache size
4. **Test Cache Behavior**: Verify cache hits in CI logs before merging

### For CI Maintenance

1. **Monitor Cache Hit Rates**: Track cache performance over time in workflow logs
2. **Optimize Cache Keys**: Balance specificity with cache reuse
3. **Clean Old Caches**: GitHub automatically manages cache cleanup, but monitor storage usage
4. **Update Dependencies**: Keep requirements files and setup scripts current
5. **Matrix Optimization**: Consider cache sharing between related matrix builds

### For Workflow Updates

1. **Test Cache Impact**: When updating the workflow, verify cache behavior
2. **Document Key Changes**: Update this document when cache strategies change
3. **Monitor Build Times**: Track build time improvements/regressions
4. **Version Pinning**: Pin action versions to ensure cache stability

## GitHub Actions Cache Limitations

- **Size Limit**: 10GB per repository
- **Retention**: 7 days for unused caches
- **Access**: Caches are branch-scoped with fallback to default branch
- **Concurrency**: Multiple jobs can read from the same cache

## Future Improvements

1. **Cross-Architecture Caching**: Optimize caches for different runner architectures
2. **Cache Warming**: Pre-populate caches for new ESP-IDF versions
3. **Selective Invalidation**: More granular cache invalidation strategies
4. **Cache Analytics**: Better monitoring and reporting of cache effectiveness
5. **Multi-Repository Sharing**: Explore cache sharing across related repositories

## Related Files

- **Main Workflow**: `.github/workflows/esp32-component-ci.yml`
- **Matrix Generation**: `.github/workflows/generate_matrix.py`
- **Setup Scripts**: `examples/esp32/scripts/setup_*.sh`
- **Requirements**: `examples/esp32/requirements.txt`
- **Docker Action**: Uses `espressif/esp-idf-ci-action@v1`