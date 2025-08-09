#!/bin/bash

# PIO Test Build Validation Script
# This script validates that the PIO comprehensive test builds correctly

set -e

echo "╔═══════════════════════════════════════════════════════════════════════════════╗"
echo "║                     PIO Test Build Validation Script                         ║"
echo "║                                                                               ║"
echo "║  This script validates the PIO comprehensive test build configuration        ║"
echo "║  and ensures all dependencies are correctly set up.                          ║"
echo "╚═══════════════════════════════════════════════════════════════════════════════╝"
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: Run this script from the examples/esp32 directory"
    echo "   Current directory: $(pwd)"
    exit 1
fi

# Check if ESP-IDF is set up
if [ -z "$IDF_PATH" ]; then
    echo "❌ Error: ESP-IDF environment not set up"
    echo "   Please source the ESP-IDF setup script first:"
    echo "   source \$IDF_PATH/export.sh"
    exit 1
fi

echo "🔍 Checking ESP-IDF environment..."
echo "   IDF_PATH: $IDF_PATH"
echo "   IDF_TARGET: ${IDF_TARGET:-esp32c6}"

# Check if source files exist
echo ""
echo "🔍 Checking PIO test source files..."

if [ ! -f "main/PioComprehensiveTest.cpp" ]; then
    echo "❌ Error: PioComprehensiveTest.cpp not found"
    exit 1
fi
echo "   ✅ PioComprehensiveTest.cpp found"

if [ ! -f "main/TestFramework.h" ]; then
    echo "❌ Error: TestFramework.h not found"
    exit 1
fi
echo "   ✅ TestFramework.h found"

# Check CMakeLists.txt configuration
echo ""
echo "🔍 Checking CMakeLists.txt configuration..."

if ! grep -q "pio_test" CMakeLists.txt; then
    echo "❌ Error: pio_test not found in CMakeLists.txt"
    exit 1
fi
echo "   ✅ pio_test found in CMakeLists.txt"

if ! grep -q "PioComprehensiveTest.cpp" main/CMakeLists.txt; then
    echo "❌ Error: PioComprehensiveTest.cpp not configured in main/CMakeLists.txt"
    exit 1
fi
echo "   ✅ PioComprehensiveTest.cpp configured in main/CMakeLists.txt"

# Set target for ESP32-C6
export IDF_TARGET=esp32c6

# Test build configuration
echo ""
echo "🔧 Testing build configuration..."
echo "   Configuring for pio_test..."

# Clean any previous build
if [ -d "build" ]; then
    echo "   Cleaning previous build..."
    rm -rf build
fi

# Configure the build
echo "   Running idf.py reconfigure..."
if ! idf.py reconfigure -DEXAMPLE_TYPE=pio_test >/dev/null 2>&1; then
    echo "❌ Error: Build configuration failed"
    echo "   Try running: idf.py reconfigure -DEXAMPLE_TYPE=pio_test"
    exit 1
fi
echo "   ✅ Build configuration successful"

# Test compilation (just configuration, not full build)
echo ""
echo "🔧 Testing compilation setup..."
echo "   Generating build files..."

if ! idf.py build -DEXAMPLE_TYPE=pio_test --cmake-warn-uninitialized 2>&1 | tee build.log; then
    echo "❌ Error: Build failed"
    echo "   Check build.log for details"
    exit 1
fi

# Check if binary was created
if [ ! -f "build/esp32_iid_pio_test_example.bin" ]; then
    echo "❌ Error: Binary not created"
    echo "   Expected: build/esp32_iid_pio_test_example.bin"
    exit 1
fi

echo "   ✅ Build successful"

# Get build statistics
BUILD_SIZE=$(ls -lh build/esp32_iid_pio_test_example.bin | awk '{print $5}')
echo "   📊 Binary size: $BUILD_SIZE"

if [ -f "build/bootloader/bootloader.bin" ]; then
    BOOTLOADER_SIZE=$(ls -lh build/bootloader/bootloader.bin | awk '{print $5}')
    echo "   📊 Bootloader size: $BOOTLOADER_SIZE"
fi

# Check for memory usage information
if [ -f "build/esp32_iid_pio_test_example.map" ]; then
    echo ""
    echo "📊 Memory usage summary:"
    # Extract memory usage from map file if available
    if grep -q "Memory region" build/esp32_iid_pio_test_example.map; then
        grep -A 10 "Memory region" build/esp32_iid_pio_test_example.map | head -15
    fi
fi

echo ""
echo "╔═══════════════════════════════════════════════════════════════════════════════╗"
echo "║                            BUILD VALIDATION COMPLETE                         ║"
echo "║                                                                               ║"
echo "║  ✅ PIO test builds successfully                                              ║"
echo "║  ✅ All dependencies are correctly configured                                 ║"
echo "║  ✅ Binary created: build/esp32_iid_pio_test_example.bin                      ║"
echo "║                                                                               ║"
echo "║  To flash and run:                                                            ║"
echo "║    idf.py flash monitor                                                       ║"
echo "║                                                                               ║"
echo "║  To test with WS2812 LEDs:                                                    ║"
echo "║    - Connect LEDs to GPIO2                                                    ║"
echo "║    - Provide 5V power to LED strip                                           ║"
echo "║    - Connect common ground                                                    ║"
echo "║                                                                               ║"
echo "║  To test with logic analyzer:                                                 ║"
echo "║    - Connect probe to GPIO2                                                   ║"
echo "║    - Set sample rate to 20MHz+                                               ║"
echo "║    - Trigger on rising edge                                                   ║"
echo "╚═══════════════════════════════════════════════════════════════════════════════╝"

# Clean up build log
rm -f build.log

echo ""
echo "🎉 PIO test is ready to use!"