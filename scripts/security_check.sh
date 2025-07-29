#!/bin/bash

# Security Check Script for ESP32 Codebase
# This script helps identify and fix the security issues mentioned in the guidelines

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(dirname "$SCRIPT_DIR")"
SECURITY_CONFIG="$WORKSPACE_DIR/.clang-tidy-security"

echo "🔒 ESP32 Security Check and Fix Script"
echo "====================================="

# Check if clang-tidy is available (only needed for full analysis)
CLANG_TIDY_AVAILABLE=false
if command -v clang-tidy &> /dev/null; then
    CLANG_TIDY_AVAILABLE=true
    echo "✅ clang-tidy found - full analysis available"
else
    echo "ℹ️  clang-tidy not installed - running basic checks only"
    echo "   For full analysis: sudo apt install clang-tidy (Ubuntu/Debian) or brew install llvm (macOS)"
fi

# Check if the security config exists
if [ ! -f "$SECURITY_CONFIG" ]; then
    echo "⚠️  Security configuration file not found: $SECURITY_CONFIG"
    echo "   Full clang-tidy analysis will not be available"
fi

echo "📁 Workspace: $WORKSPACE_DIR"
echo "⚙️  Config: $SECURITY_CONFIG"

# Function to run security analysis
run_security_analysis() {
    echo ""
    echo "🔍 Running security analysis..."
    
    if [ "$CLANG_TIDY_AVAILABLE" = false ]; then
        echo "⚠️  clang-tidy not available - skipping full analysis"
        echo "   Install clang-tidy for comprehensive security analysis"
        return 0
    fi
    
    # Find all C++ source files
    CPP_FILES=$(find "$WORKSPACE_DIR/src" "$WORKSPACE_DIR/examples" -name "*.cpp" -o -name "*.c" 2>/dev/null || true)
    
    if [ -z "$CPP_FILES" ]; then
        echo "⚠️  No C++ source files found for analysis"
        return 1
    fi
    
    echo "📄 Found $(echo "$CPP_FILES" | wc -l) source files to analyze"
    
    # Create output directory
    mkdir -p "$WORKSPACE_DIR/security_analysis"
    
    # Run clang-tidy with security configuration
    echo "$CPP_FILES" | xargs clang-tidy \
        --config-file="$SECURITY_CONFIG" \
        --header-filter="$WORKSPACE_DIR/(src|inc|examples)/.*\.(h|hpp)$" \
        --export-fixes="$WORKSPACE_DIR/security_analysis/fixes.yaml" \
        > "$WORKSPACE_DIR/security_analysis/security_report.txt" 2>&1 || true
    
    echo "📊 Security analysis complete. Reports saved to:"
    echo "   - $WORKSPACE_DIR/security_analysis/security_report.txt"
    echo "   - $WORKSPACE_DIR/security_analysis/fixes.yaml"
}

# Function to check for specific security issues
check_specific_issues() {
    echo ""
    echo "🎯 Checking for specific security issues..."
    
    ISSUES_FOUND=0
    
    # Check for sscanf usage
    echo "🔍 Checking for unsafe sscanf usage..."
    if grep -r "sscanf" "$WORKSPACE_DIR/src" "$WORKSPACE_DIR/examples" 2>/dev/null; then
        echo "⚠️  Found sscanf usage - replace with SafeParsing::ParseInteger"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    else
        echo "✅ No unsafe sscanf usage found"
    fi
    
    # Check for magic numbers
    echo "🔍 Checking for WEP key magic numbers..."
    if grep -rE "\b(5|13|16|29)\b.*[Ww][Ee][Pp]" "$WORKSPACE_DIR/src" "$WORKSPACE_DIR/examples" 2>/dev/null; then
        echo "⚠️  Found WEP key magic numbers - use named constants"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    else
        echo "✅ No WEP key magic numbers found"
    fi
    
    # Check for UUID magic numbers
    echo "🔍 Checking for UUID magic number (16)..."
    if grep -rE "\b16\b.*[Uu][Uu][Ii][Dd]" "$WORKSPACE_DIR/src" "$WORKSPACE_DIR/examples" 2>/dev/null; then
        echo "⚠️  Found UUID magic number - use UUID_128_BYTE_LENGTH"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    else
        echo "✅ No UUID magic numbers found"
    fi
    
    # Check for std::this_thread::sleep_for
    echo "🔍 Checking for std::this_thread::sleep_for usage..."
    if grep -r "std::this_thread::sleep_for\|sleep_for" "$WORKSPACE_DIR/src" "$WORKSPACE_DIR/examples" 2>/dev/null; then
        echo "⚠️  Found std::this_thread::sleep_for - replace with FreeRtosUtils::DelayMs"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    else
        echo "✅ No std::this_thread::sleep_for usage found"
    fi
    
    # Check for inconsistent logging
    echo "🔍 Checking for inconsistent logging (std::cout/cerr mixed with ESP_LOG)..."
    if grep -rE "std::(cout|cerr)" "$WORKSPACE_DIR/src" "$WORKSPACE_DIR/examples" 2>/dev/null | grep -v "example\|demo"; then
        echo "⚠️  Found std::cout/cerr usage - use ESP_LOG macros consistently"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    else
        echo "✅ No inconsistent logging found"
    fi
    
    # Check for missing thread/chrono includes
    echo "🔍 Checking for missing thread/chrono includes..."
    if grep -l "sleep_for\|this_thread" "$WORKSPACE_DIR/src"/*.cpp "$WORKSPACE_DIR/examples"/*.cpp 2>/dev/null | \
       xargs grep -L "#include.*thread\|#include.*chrono" 2>/dev/null; then
        echo "⚠️  Found sleep_for usage without proper includes"
        ISSUES_FOUND=$((ISSUES_FOUND + 1))
    else
        echo "✅ No missing thread/chrono includes found"
    fi
    
    echo ""
    if [ $ISSUES_FOUND -eq 0 ]; then
        echo "🎉 No specific security issues found!"
    else
        echo "⚠️  Found $ISSUES_FOUND types of security issues"
        echo "📖 See SECURITY_IMPROVEMENTS.md for detailed fixes"
    fi
}

# Function to show summary and recommendations
show_recommendations() {
    echo ""
    echo "📋 Security Improvement Recommendations"
    echo "======================================"
    echo ""
    echo "1. 🔧 Include SecurityGuidelines.h in your source files:"
    echo "   #include \"SecurityGuidelines.h\""
    echo ""
    echo "2. 🛡️  Replace unsafe sscanf with safe parsing:"
    echo "   SafeParsing::ParseInteger(input, output)"
    echo ""
    echo "3. 📝 Use named constants for magic numbers:"
    echo "   Security::WEP_KEY_LENGTH_128_BIT instead of 13"
    echo "   Security::UUID_128_BYTE_LENGTH instead of 16"
    echo ""
    echo "4. ⏱️  Use FreeRTOS delays:"
    echo "   FreeRtosUtils::DelayMs(1000)"
    echo ""
    echo "5. 📄 Use consistent ESP_LOG macros:"
    echo "   ESP_LOGI(TAG, \"message\") instead of std::cout"
    echo ""
    echo "📖 For detailed examples, see:"
    echo "   - $WORKSPACE_DIR/SECURITY_IMPROVEMENTS.md"
    echo "   - $WORKSPACE_DIR/examples/esp32/main/security_improvements_example.cpp"
}

# Function to test compilation
test_compilation() {
    echo ""
    echo "🔨 Testing compilation of security examples..."
    
    if [ -f "$WORKSPACE_DIR/examples/esp32/main/security_improvements_example.cpp" ]; then
        # Simple syntax check (not full compilation due to ESP-IDF dependencies)
        if command -v g++ &> /dev/null; then
            g++ -fsyntax-only -std=c++17 \
                -I"$WORKSPACE_DIR/src/mcu/esp32" \
                -DHF_MCU_FAMILY_ESP32 \
                "$WORKSPACE_DIR/examples/esp32/main/security_improvements_example.cpp" 2>/dev/null && \
            echo "✅ Security example syntax check passed" || \
            echo "⚠️  Security example has syntax issues - check ESP-IDF setup"
        else
            echo "ℹ️  g++ not available for syntax checking"
        fi
    else
        echo "⚠️  Security example file not found"
    fi
}

# Main execution
main() {
    case "${1:-all}" in
        "analysis"|"analyze")
            run_security_analysis
            ;;
        "check")
            check_specific_issues
            ;;
        "test")
            test_compilation
            ;;
        "all"|"")
            check_specific_issues
            run_security_analysis
            show_recommendations
            test_compilation
            ;;
        "help"|"-h"|"--help")
            echo "Usage: $0 [command]"
            echo ""
            echo "Commands:"
            echo "  all       - Run all security checks (default)"
            echo "  check     - Check for specific security issues"
            echo "  analysis  - Run full clang-tidy security analysis"
            echo "  test      - Test compilation of security examples"
            echo "  help      - Show this help message"
            ;;
        *)
            echo "❌ Unknown command: $1"
            echo "Run '$0 help' for usage information"
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@"