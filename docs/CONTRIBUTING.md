# Contributing

Thank you for considering a contribution to the HardFOC Internal Interface Wrapper! This document outlines the process for submitting patches and keeping the codebase healthy.

## Development Setup

1. Install **ESP-IDF v5.5+** and export the environment using `source $IDF_PATH/export.sh`.
2. Clone this repository and initialize submodules if present.
3. Build the tests: `cd tests && mkdir build && cd build && cmake .. && make`.

## Coding Style

- Format C++ code with `clang-format` using the provided style file.
- Document all public functions in Doxygen style.
- Keep commits focused and include descriptive messages.

## Pull Requests

1. Fork the repository and create a feature branch.
2. Ensure `scripts/check_docs.py docs/index.md` reports no broken links.
3. Run all unit tests and fix any failures.
4. Open a pull request describing your changes.

We appreciate improvements to documentation, bug fixes, and new platform implementations.
