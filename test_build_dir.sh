#!/bin/bash

# Test script to verify build directory functionality
echo "Testing build directory functionality..."

# Change to project directory
cd examples/esp32

# Source the config loader
source scripts/config_loader.sh

echo "Project directory: $PROJECT_DIR"
echo ""

# Test get_build_directory function
echo "Testing get_build_directory function:"
build_dir=$(get_build_directory "gpio_test" "Release")
echo "Build directory: $build_dir"
echo ""

# Test if it's an absolute path
if [[ "$build_dir" == /* ]]; then
    echo "✅ Build directory is an absolute path"
else
    echo "❌ Build directory is not an absolute path"
fi

# Test if it's in the project directory
if [[ "$build_dir" == "$PROJECT_DIR"* ]]; then
    echo "✅ Build directory is in project directory"
else
    echo "❌ Build directory is not in project directory"
fi

echo ""
echo "Test completed."
