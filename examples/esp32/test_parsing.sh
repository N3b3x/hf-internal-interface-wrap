#!/bin/bash

# Test script to verify build types parsing
echo "Testing build types parsing..."

# Source the config loader
source scripts/config_loader.sh

echo "=== Testing get_build_types() ==="
echo "All available build types: '$(get_build_types)'"

echo ""
echo "=== Testing get_build_types_for_idf_version() ==="
echo "Build types for ESP-IDF v5.5: '$(get_build_types_for_idf_version "release/v5.5")'"
echo "Build types for ESP-IDF v5.4: '$(get_build_types_for_idf_version "release/v5.4")'"

echo ""
echo "=== Testing validation ==="
echo "Is 'Release' valid for v5.5? $(is_valid_build_type "Release" "release/v5.5" && echo "YES" || echo "NO")"
echo "Is 'Release' valid for v5.4? $(is_valid_build_type "Release" "release/v5.4" && echo "YES" || echo "NO")"
echo "Is 'Debug' valid for v5.5? $(is_valid_build_type "Debug" "release/v5.5" && echo "YES" || echo "NO")"
echo "Is 'Debug' valid for v5.4? $(is_valid_build_type "Debug" "release/v5.4" && echo "YES" || echo "NO")"