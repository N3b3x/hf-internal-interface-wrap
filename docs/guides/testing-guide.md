# 🧪 Testing Guide

This project currently does not include unit tests. The instructions below are kept for reference in case a future release adds them.

## Building

```bash
# (Tests directory not present in this repository)
```

Set up your toolchain as described in the README. The CMake project will compile mock implementations for host-based testing.

## Running

Test binaries are not provided. When tests become available, this section will describe how to execute them.

## Writing Tests

- Use the provided mock classes for hardware independent verification.
- Keep tests deterministic and free of timing dependencies.
- Group related tests into separate files when a testing framework is added.

See the existing tests for examples of mocking and expectation setup.

## 🔄 Continuous Integration
- Integrate the tests into your CI system using `ctest`
- Fail the pipeline if any unit tests fail

## 🛠️ Troubleshooting
- Ensure mocks cover all hardware interactions
- Use verbose output (`./test_runner -v`) to diagnose failures

## 🔗 Related Examples
- [📝 Data Logger](../examples/data-logger.md)
