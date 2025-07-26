# GitHub Actions CI

This directory contains the GitHub Actions workflow for the fet library.

## CI Workflow (ci.yml)

The CI workflow runs on every push and pull request to main/master branches and includes:

### Build Matrix
- **Operating Systems**: Ubuntu, Windows, macOS
- **Compilers**: GCC, Clang, MSVC
- **Language Standard**: C++14

### Build Steps
1. **Environment Setup**: Installs compilers and dependencies (Boost)
2. **Missing Dependencies**: Creates bind_front.hpp for C++14 compatibility
3. **Compilation Test**: Verifies all headers compile without errors
4. **Execution Test**: Runs simple test to verify basic functionality

### Code Quality Checks
1. **Format Check**: Validates code formatting using uncrustify
2. **Header Independence**: Ensures each header can be compiled standalone

### Dependencies
- **Boost**: Required for some headers (logic/tribool, fusion)
- **bind_front.hpp**: Custom C++14 implementation (auto-generated)

The workflow is designed specifically for header-only libraries and focuses on compilation validation across multiple platforms and compilers.
