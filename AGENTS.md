# AGENTS.md - Agent Coding Guidelines for ckpttn-cpp

## Overview

This is a C++20 circuit partitioning library using CMake as the build system and doctest for testing.

---

## Build Commands

### CMake (Primary)

```bash
# Full build
cmake -S. -B build
cmake --build build

# Build and run tests
cd build/test
CTEST_OUTPUT_ON_FAILURE=1 ctest
# Or run executable directly
./build/test/CkPttnTests
```

### Run a Single Test (doctest)

```bash
# Run specific test case by name (partial match)
./build/test/CkPttnTests -tc="Test FMBiPartMgr"

# List all test cases
./build/test/CkPttnTests --list-test-cases

# Run with strict mode
./build/test/CkPttnTests -s
```

### xmake (Alternative)

```bash
xmake                    # build
xmake run test_ckpttn   # build and run tests
```

### Code Formatting

```bash
# View changes (requires clang-format, cmake-format, pyyaml)
cmake -S . -B build/test
cmake --build build --target format

# Apply changes
cmake --build build --target fix-format
```

### Static Analysis

```bash
# clang-tidy
cmake -S . -B build -DUSE_STATIC_ANALYZER=clang-tidy
cmake --build build

# Include What You Use (iwyu)
cmake -S . -B build -DUSE_STATIC_ANALYZER=iwyu
cmake --build build
```

### Sanitizers

```bash
cmake -S . -B build -DUSE_SANITIZER=Address
cmake --build build
```

---

## Code Style Guidelines

### General

- **C++ Standard**: C++20 (minimum)
- **Header Guard**: Use `#pragma once` (not #ifndef guards)
- **Column Limit**: 100 characters

### Formatting (.clang-format)

Based on Google style with customizations:

```yaml
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 100
AccessModifierOffset: -2
NamespaceIndentation: All
BreakBeforeBraces: Attach
BreakBeforeBinaryOperators: All
IncludeBlocks: Regroup
```

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes/Templates | PascalCase | `FMPartMgr`, `SimpleNetlist` |
| Functions | snake_case | `create_test_netlist()` |
| Variables | snake_case | `hyprgraph`, `total_cost` |
| Member variables | snake_case with `_` suffix | `num_modules_` |
| Constants | kPrefix or UPPER_SNAKE | `kDefaultSize` |
| Headers | lowercase | `FMPartMgr.hpp` |

### Includes

- **Project headers**: `#include <ckpttn/FMPartMgr.hpp>`
- **External headers**: `#include <netlistx/netlist.hpp>`
- **Standard library**: `#include <vector>`, `#include <cstdint>`
- **Group and sort**: Use `IncludeBlocks: Regroup` in clang-format

### Type Aliases

```cpp
template <typename Gnl, typename GainMgr, typename ConstrMgr>
class FMPartMgr : public PartMgrBase<Gnl, GainMgr, ConstrMgr> {
    using Base = PartMgrBase<Gnl, GainMgr, ConstrMgr>;
  public:
    // ...
};
```

### Template Classes

- Define templates in headers (no .cpp implementation)
- Use `// forward declaration` comments for template friends
- Group related templates in the same header

### Error Handling

- **No exceptions** in core algorithms (performance-critical)
- Use `std::optional` or sentinel values for fallible operations
- Validate inputs at API boundaries; assert in internal code

### Testing

- Use **doctest** framework
- Test file location: `test/source/test_*.cpp`
- Test case format:

```cpp
#include "test_common.hpp"

TEST_CASE("Test FMBiPartMgr") {
    const auto hyprgraph = create_test_netlist();
    run_PartMgr<FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>(hyprgraph);
}
```

### Documentation

- Use Doxygen-style comments for public APIs
- Brief description first, then detailed

```cpp
/**
 * @brief Fiduccia-Mattheyses Partitioning Algorithm Manager
 *
 * The `FMPartMgr` class is a subclass of `PartMgrBase`...
 *
 * @tparam Gnl
 * @tparam GainMgr
 * @tparam ConstrMgr
 */
```

---

## Project Structure

```
ckpttn-cpp/
├── include/ckpttn/     # Public headers
├── source/             # Implementation (.cpp)
├── test/source/        # Test files
├── testcases/          # Test data (.net, .are files)
├── cmake/              # CMake utilities
├── CMakeLists.txt      # Main build config
└── xmake.lua           # xmake config
```

---

## Dependencies

- **XNetwork** - Graph library
- **Py2Cpp** - Python to C++ utilities
- **NetlistX** - Netlist data structures
- **MyWheel** - Common utilities
- **Microsoft GSL** - Guidelines Support Library
- **doctest** - Testing framework
- **fmt** - String formatting
- **spdlog** - Logging

---

## Key Files

| File | Purpose |
|------|---------|
| `include/ckpttn/FMPartMgr.hpp` | FM partitioning manager |
| `include/ckpttn/MLPartMgr.hpp` | Multi-level partitioning |
| `include/ckpttn/netlist.hpp` | Netlist data structure |
| `source/FMPartMgr.cpp` | FM implementation |
| `test/source/test_FMBiPartMgr.cpp` | Binary partitioning tests |
| `test/source/test_common.hpp` | Test utilities |
