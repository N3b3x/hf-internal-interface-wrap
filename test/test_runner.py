#!/usr/bin/env python3
"""
ESP32 Unit Test Runner using pytest-embedded

This script runs unit tests for the HardFOC IID project using pytest-embedded
framework, which allows automated testing on ESP32 hardware or simulators.

Usage:
    python test_runner.py [--target esp32c6] [--build-dir build]

Author: HardFOC Team
Date: 2025
"""

import os
import sys
import re
import json
import argparse
from pathlib import Path
from typing import Dict, List, Optional, Tuple

import pytest
from pytest_embedded import Dut


class UnityTestParser:
    """Parser for Unity test framework output"""
    
    def __init__(self):
        self.test_results = []
        self.test_stats = {
            'total': 0,
            'passed': 0,
            'failed': 0,
            'ignored': 0
        }
    
    def parse_unity_output(self, output: str) -> Dict:
        """Parse Unity test output and extract results"""
        lines = output.split('\n')
        
        for line in lines:
            line = line.strip()
            
            # Parse individual test results
            if ':PASS' in line or ':FAIL' in line or ':IGNORE' in line:
                self._parse_test_line(line)
            
            # Parse summary statistics
            elif 'Tests:' in line and 'Failures:' in line:
                self._parse_summary_line(line)
        
        return {
            'results': self.test_results,
            'stats': self.test_stats
        }
    
    def _parse_test_line(self, line: str):
        """Parse individual test result line"""
        # Unity format: filename:line:test_name:PASS/FAIL/IGNORE[:message]
        parts = line.split(':')
        if len(parts) >= 4:
            filename = parts[0]
            line_num = parts[1] if parts[1].isdigit() else 'unknown'
            test_name = parts[2]
            result = parts[3]
            message = ':'.join(parts[4:]) if len(parts) > 4 else ''
            
            self.test_results.append({
                'file': filename,
                'line': line_num,
                'name': test_name,
                'result': result,
                'message': message
            })
            
            if result == 'PASS':
                self.test_stats['passed'] += 1
            elif result == 'FAIL':
                self.test_stats['failed'] += 1
            elif result == 'IGNORE':
                self.test_stats['ignored'] += 1
            
            self.test_stats['total'] += 1
    
    def _parse_summary_line(self, line: str):
        """Parse Unity summary line"""
        # Extract numbers from summary
        numbers = re.findall(r'\d+', line)
        if len(numbers) >= 2:
            self.test_stats['total'] = int(numbers[0])
            self.test_stats['failed'] = int(numbers[1])
            self.test_stats['passed'] = self.test_stats['total'] - self.test_stats['failed']


def generate_junit_xml(test_results: Dict, output_file: str):
    """Generate JUnit XML from Unity test results"""
    from xml.etree.ElementTree import Element, SubElement, tostring
    from xml.dom import minidom
    
    root = Element('testsuites')
    root.set('name', 'HardFOC_IID_Unit_Tests')
    root.set('tests', str(test_results['stats']['total']))
    root.set('failures', str(test_results['stats']['failed']))
    root.set('time', '0')  # Unity doesn't provide timing info
    
    testsuite = SubElement(root, 'testsuite')
    testsuite.set('name', 'ESP32_Unit_Tests')
    testsuite.set('tests', str(test_results['stats']['total']))
    testsuite.set('failures', str(test_results['stats']['failed']))
    testsuite.set('time', '0')
    
    for test in test_results['results']:
        testcase = SubElement(testsuite, 'testcase')
        testcase.set('classname', test['file'])
        testcase.set('name', test['name'])
        testcase.set('time', '0')
        
        if test['result'] == 'FAIL':
            failure = SubElement(testcase, 'failure')
            failure.set('message', test['message'])
            failure.text = f"Test failed at line {test['line']}: {test['message']}"
        elif test['result'] == 'IGNORE':
            skipped = SubElement(testcase, 'skipped')
            skipped.set('message', 'Test ignored')
    
    # Pretty print XML
    rough_string = tostring(root, 'unicode')
    reparsed = minidom.parseString(rough_string)
    
    with open(output_file, 'w') as f:
        f.write(reparsed.toprettyxml(indent="  "))


@pytest.mark.esp32c6
@pytest.mark.esp32
@pytest.mark.esp32s3
def test_hardfoc_iid_unit_tests(dut: Dut) -> None:
    """
    Main test function that runs all HardFOC IID unit tests
    
    This function:
    1. Flashes the test firmware to the device
    2. Monitors test execution
    3. Parses Unity test output
    4. Generates test reports
    """
    
    # Create output directories
    test_output_dir = Path('build/test_results')
    test_output_dir.mkdir(parents=True, exist_ok=True)
    
    # Configuration
    test_timeout = 300  # 5 minutes
    
    print("🚀 Starting HardFOC IID Unit Tests")
    print(f"📱 Target: {dut.target}")
    print(f"🔧 Build directory: {dut.build_dir}")
    
    # Capture all output
    all_output = []
    
    try:
        # Wait for test startup
        dut.expect("HardFOC IID Unit Test Application Starting", timeout=30)
        print("✅ Test application started successfully")
        
        # Wait for Unity to begin
        dut.expect("Starting HardFOC IID Unit Tests", timeout=10)
        print("🧪 Unity testing framework initialized")
        
        # Monitor test execution with detailed patterns
        test_patterns = [
            # Test execution patterns
            r"=== Running .* Tests ===",
            r"--- .* Tests ---",
            r"TEST\([^)]+\)",
            r".*:PASS",
            r".*:FAIL",
            r".*:IGNORE",
            
            # Unity summary patterns
            r"\d+ Tests \d+ Failures \d+ Ignored",
            r"OK \(\d+ tests\)",
            r"FAIL \(\d+ failures?, \d+ tests\)",
            
            # Test completion
            r"All tests completed!",
            r"ALL TESTS PASSED!",
            r"tests FAILED out of",
            
            # Memory and system info
            r"Free heap: \d+ bytes",
            
            # Error patterns
            r"ASSERTION FAILED",
            r"abort\(\) was called",
            r"Guru Meditation Error",
        ]
        
        # Collect test output
        while True:
            try:
                output = dut.expect(test_patterns, timeout=test_timeout)
                all_output.append(output.group())
                print(f"📝 {output.group()}")
                
                # Check for test completion
                if any(completion in output.group() for completion in [
                    "All tests completed!",
                    "ALL TESTS PASSED!",
                    "tests FAILED out of"
                ]):
                    print("🏁 Test execution completed")
                    break
                    
                # Check for critical errors
                if any(error in output.group() for error in [
                    "abort() was called",
                    "Guru Meditation Error"
                ]):
                    print("❌ Critical error detected during test execution")
                    break
                    
            except Exception as e:
                print(f"⏰ Test monitoring ended: {e}")
                break
        
        # Give a moment for any final output
        try:
            dut.expect(r".*", timeout=2)
        except:
            pass
        
    except Exception as e:
        print(f"❌ Error during test execution: {e}")
        all_output.append(f"ERROR: {e}")
    
    # Parse results
    full_output = '\n'.join(all_output)
    parser = UnityTestParser()
    test_results = parser.parse_unity_output(full_output)
    
    # Save raw output
    with open(test_output_dir / 'test_output.log', 'w') as f:
        f.write(full_output)
    
    # Generate reports
    results_file = test_output_dir / 'test_results.json'
    with open(results_file, 'w') as f:
        json.dump(test_results, f, indent=2)
    
    # Generate JUnit XML
    junit_file = test_output_dir / 'junit.xml'
    generate_junit_xml(test_results, str(junit_file))
    
    # Print summary
    stats = test_results['stats']
    print("\n📊 Test Summary:")
    print(f"   Total tests: {stats['total']}")
    print(f"   Passed: {stats['passed']}")
    print(f"   Failed: {stats['failed']}")
    print(f"   Ignored: {stats['ignored']}")
    
    if stats['failed'] > 0:
        print("\n❌ Failed tests:")
        for test in test_results['results']:
            if test['result'] == 'FAIL':
                print(f"   - {test['name']}: {test['message']}")
    
    # Assert test success
    assert stats['failed'] == 0, f"{stats['failed']} tests failed out of {stats['total']} total tests"
    assert stats['total'] > 0, "No tests were executed"
    
    print(f"\n✅ All {stats['passed']} tests passed successfully!")


if __name__ == "__main__":
    # Parse command line arguments
    parser = argparse.ArgumentParser(description="Run HardFOC IID unit tests")
    parser.add_argument("--target", default="esp32c6", 
                       help="ESP32 target (esp32c6, esp32, esp32s3)")
    parser.add_argument("--build-dir", default="build",
                       help="Build directory path")
    parser.add_argument("--port", default=None,
                       help="Serial port (auto-detect if not specified)")
    parser.add_argument("--junit-xml", default="build/test_results/junit.xml",
                       help="JUnit XML output file")
    parser.add_argument("--verbose", "-v", action="store_true",
                       help="Verbose output")
    
    args = parser.parse_args()
    
    # Set pytest arguments
    pytest_args = [
        __file__,
        f"--target={args.target}",
        f"--build-dir={args.build_dir}",
        f"--junit-xml={args.junit_xml}",
        "--tb=short",
        "--capture=no",
    ]
    
    if args.verbose:
        pytest_args.append("-v")
    
    if args.port:
        pytest_args.append(f"--port={args.port}")
    
    # Run pytest
    exit_code = pytest.main(pytest_args)
    sys.exit(exit_code)