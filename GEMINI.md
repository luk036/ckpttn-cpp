
# GEMINI.md

## Project Overview

This is a C++ project for circuit partitioning, specifically for hypergraphs. The library, named `ckpttn-cpp`, provides a system for multi-level partitioning to divide complex problems into smaller parts. It uses a multi-level approach, starting with an initial partition and iteratively refining it to improve balance and optimization. The core logic is in the `MLPartMgr` class, which uses helper classes for gain calculation, constraint management, and partitioning.

The project is built with modern C++ (C++20 or above) and uses CMake for its build system. It also includes support for `xmake`. It has a comprehensive test suite using `doctest` and continuous integration configured with GitHub Actions. Code formatting is enforced using `clang-format` and `cmake-format`.

## Building and Running

### Using CMake

**Build and run the standalone target:**

```bash
cmake -S. -B build
cmake --build build
./build/standalone/CkPttn --help
```

**Build and run the test suite:**

```bash
cmake -S. -B build
cmake --build build
cd build/test
CTEST_OUTPUT_ON_FAILURE=1 ctest
```

### Using xmake

**Build the project:**

```bash
xmake
```

**Run tests:**

```bash
xmake run test_ckpttn
```

## Development Conventions

*   **Coding Style:** The project uses Google's C++ style guide, enforced by `.clang-format`.
*   **Testing:** Tests are written using the `doctest` framework and are located in the `test` directory. The CI pipeline runs these tests automatically.
*   **Continuous Integration:** GitHub Actions is used for CI. The workflow for Ubuntu is defined in `.github/workflows/ubuntu.yml`. It builds the project, runs tests, and collects code coverage information using `lcov` and Codecov.
*   **Documentation:** Documentation is generated using Doxygen. The configuration is in `documentation/Doxyfile`.
*   **Dependencies:** Dependencies are managed using `CPM.cmake`. The main dependencies are XNetwork and Boost.
*   **License:** The project is released into the public domain under the Unlicense.
