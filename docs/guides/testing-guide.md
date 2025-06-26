# 🧪 Testing Guide

Unit tests validate the correctness of each driver and utility class. Follow these instructions to build and execute the tests.

## Building

```bash
cd tests
mkdir build && cd build
cmake ..
make
```

Set up your toolchain as described in the README. The CMake project will compile mock implementations for host-based testing.

## Running

```bash
./test_runner
```

The test runner outputs a summary of passed and failed cases.

## Writing Tests

- Use the provided mock classes for hardware independent verification.
- Keep tests deterministic and free of timing dependencies.
- Group related tests into separate files under `tests/`.

See the existing tests for examples of mocking and expectation setup.

## 🔄 Continuous Integration
- Integrate the tests into your CI system using `ctest`
- Fail the pipeline if any unit tests fail

## 🛠️ Troubleshooting
- Ensure mocks cover all hardware interactions
- Use verbose output (`./test_runner -v`) to diagnose failures

## 🔗 Related Examples
- [📝 Data Logger](../examples/data-logger.md)
