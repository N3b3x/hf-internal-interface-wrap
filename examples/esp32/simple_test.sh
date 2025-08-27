#!/bin/bash
echo "Testing simple parsing..."

# Test the grep approach directly
echo "Testing grep approach:"
echo 'build_types: [["Debug", "Release"], ["Debug"]]' | grep -o '"[^"]*"' | sed 's/"//g' | sort -u | tr '\n' ' '
echo ""

# Test with the actual config file
echo "Testing with actual config file:"
grep -A 10 "metadata:" app_config.yml | grep "build_types:" | grep -o '"[^"]*"' | sed 's/"//g' | sort -u | tr '\n' ' '
echo ""