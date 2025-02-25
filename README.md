[![Actions Status](https://github.com/luk036/ckpttn-cpp/workflows/MacOS/badge.svg)](https://github.com/luk036/ckpttn-cpp/actions)
[![Actions Status](https://github.com/luk036/ckpttn-cpp/workflows/Windows/badge.svg)](https://github.com/luk036/ckpttn-cpp/actions)
[![Actions Status](https://github.com/luk036/ckpttn-cpp/workflows/Ubuntu/badge.svg)](https://github.com/luk036/ckpttn-cpp/actions)
[![Actions Status](https://github.com/luk036/ckpttn-cpp/workflows/Install/badge.svg)](https://github.com/luk036/ckpttn-cpp/actions)
[![codecov](https://codecov.io/gh/luk036/ckpttn-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/luk036/ckpttn-cpp)

<p align="center">
  <img src="https://repository-images.githubusercontent.com/254842585/4dfa7580-7ffb-11ea-99d0-46b8fe2f4170" height="175" width="auto" />
</p>

# ckpttn-cpp ✂️

> Circuit Partitioning for modern C++

This library defines a system for the management of multi-level partitioning, which is a technique used to divide a complex problem into smaller, more manageable parts. The primary objective of this library is to provide a framework for partitioning a hypergraph (a type of graph where edges can connect more than two vertices) into multiple parts while maintaining specific balance and optimization criteria.

The primary algorithmic approach utilized in this system is that of multi-level partitioning. The algorithm commences with an initial partitioning of the modules, subsequently undergoing iterative refinements through the transfer of modules between parts, with the objective of enhancing the balance and optimization criteria. The algorithm continues until a stable solution is reached or a predefined stopping criterion is met.

The primary inputs for this system are a hypergraph, module weights (which represent the relative importance or size of each module in the graph), and an initial partitioning of the modules. The output is an optimized partitioning of the modules that satisfies the specified balance constraints and minimizes the total cost of the partitioning.

The fundamental component of the algorithm is the run_FMPartition method, which is contained within the MLPartMgr class. This method accepts the hypergraph, module weights, and initial partitioning as inputs. The initial partitioning is first evaluated to ascertain its legality, that is, whether it satisfies the balance constraints. In the event that the initial partitioning is not found to be legal, the algorithm will return without implementing any changes. If the initial partitioning is found to be legal, the algorithm proceeds to optimize the partitioning.

The optimization process comprises two principal stages.

1. In the event that the hypergraph is sufficiently large (as determined by the limitsize property), the hypergraph is initially contracted into a smaller one. This represents a method of reducing the complexity of the problem. Subsequently, the process is invoked recursively on the aforementioned smaller hypergraph.

2. Subsequently, a recursive call is made (or alternatively, if the hypergraph is of a sufficiently small size, the process may commence directly). At this juncture, an optimize function is invoked with the objective of enhancing the partitioning.

## Clustering Algorithm

A clustering algorithm for graph contraction is employed. The primary objective is to reduce the complexity of a graph (referred to as a hypergraph) by consolidating specific nodes (termed modules) into clusters. This process serves to reduce the complexity of the graph while maintaining its essential structure.

The algorithm accepts as input a hypergraph (represented by the Netlist class), weights for modules and clusters, and a set of forbidden nets (connections that should not be grouped). The output is a new, simplified graph, referred to as a hierarchical netlist, with updated weights for the modules.

The algorithm operates through a series of steps to achieve its objective. Initially, it identifies a minimum maximal matching in the graph, which involves pairing nodes that are connected.

2. Subsequently, the initial clusters, nets, and cell list are established based on the aforementioned matching.

3. Subsequently, a new graph is constructed based on the aforementioned clusters and the remaining nodes.

4. Subsequently, the algorithm eliminates any duplicate nets, which are connections that essentially represent the same entity. This step serves to further simplify the graph.

5. Following the removal of duplicate data points, the graph is reconstructed with the updated information.

6. Ultimately, the subgraph is contracted, whereby the clustered nodes are merged into a single unit within the new graph.

Throughout the process, the algorithm maintains a record of the weights associated with modules and nets, updating them in accordance with the formation of clusters comprising nodes. This is a significant aspect of the process, as the assigned weights represent the relative importance or size of each module.

The primary logic flow entails transforming the original complex graph into a simplified one through the grouping of connected nodes, the removal of redundant connections, and the updating of weights in accordance with these changes. This is accomplished through a sequence of graph operations and data structure manipulations.

It is crucial for novice programmers to grasp that this algorithmic process is grounded in graph theory, which is employed to depict intricate connections between entities. The objective of the algorithm is to simplify these relationships while preserving the most important information. This kind of algorithm is useful in a number of fields, including circuit design, network analysis, and data compression.

## Additional Information

The algorithm employs a number of helper classes (GainCalc, GainMgr, ConstrMgr, PartMgr) to facilitate the management of disparate aspects of the partitioning process. These classes are responsible for calculating the gain of moving a module from one partition to another, managing the constraints of the partitioning process, and performing the actual optimization.

A significant concept within this algorithmic framework is that of "gain." The term "gain" is used to quantify the extent to which the overall cost of the partitioning would be reduced if a specific alteration were to be implemented. The algorithm attempts to implement changes that result in a positive gain, thereby enhancing the overall quality of the partitioning.

Furthermore, the library incorporates a mechanism for capturing "snapshots" of the partitioning process in instances where a relocation results in a negative gain. This enables the algorithm to potentially reverse a series of moves if they prove to be disadvantageous overall.

In conclusion, this library offers a versatile framework for addressing intricate partitioning challenges, encompassing the capacity to handle diverse partitioning types (binary or k-way) and to tackle problems of varying scales through its multi-level approach.

Requirements:
- This library requires C++20 or above.
- This library requires standard library `<meomory_resource>`.
- This library depends on XNetwork, Boost.

## ✨ Features

- Lazy evaluation of Cholesky/LDLT decomposition
- Suport semi- and strictlu positive definite.
- [Modern CMake practices](https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/)
- Integrated test suite
- Continuous integration via [GitHub Actions](https://help.github.com/en/actions/)
- Code coverage via [codecov](https://codecov.io)
- Code formatting enforced by [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and [cmake-format](https://github.com/cheshirekow/cmake_format) via [Format.cmake](https://github.com/TheLartians/Format.cmake)
- Reproducible dependency management via [CPM.cmake](https://github.com/TheLartians/CPM.cmake)
- Installable target with automatic versioning information and header generation via [PackageProject.cmake](https://github.com/TheLartians/PackageProject.cmake)
- Automatic [documentation](https://thelartians.github.io/ModernCppStarter) and deployment with [Doxygen](https://www.doxygen.nl) and [GitHub Pages](https://pages.github.com)
- Support for [sanitizer tools, and more](#additional-tools)

## Usage

### Adjust the template to your needs

- Use this repo [as a template](https://help.github.com/en/github/creating-cloning-and-archiving-repositories/creating-a-repository-from-a-template).
- Replace all occurrences of "CkPttn" in the relevant CMakeLists.txt with the name of your project
  - Capitalization matters here: `CkPttn` means the name of the project, while `ckpttn` is used in file names.
  - Remember to rename the `include/ckpttn` directory to use your project's lowercase name and update all relevant `#include`s accordingly.
- Replace the source files with your own
- For header-only libraries: see the comments in [CMakeLists.txt](CMakeLists.txt)
- Add [your project's codecov token](https://docs.codecov.io/docs/quick-start) to your project's github secrets under `CODECOV_TOKEN`
- Happy coding!

Eventually, you can remove any unused files, such as the standalone directory or irrelevant github workflows for your project.
Feel free to replace the License with one suited for your project.

To cleanly separate the library and subproject code, the outer `CMakeList.txt` only defines the library itself while the tests and other subprojects are self-contained in their own directories.
During development it is usually convenient to [build all subprojects at once](#build-everything-at-once).

### Build and run the standalone target

Use the following command to build and run the executable target.

```bash
cmake -S. -B build
cmake --build build
./build/standalone/CkPttn --help
```

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S. -B build
cmake --build build
cd build/test
CTEST_OUTPUT_ON_FAILURE=1 ctest

# or maybe simply call the executable:
./build/test/CkPttnTests
```

To collect code coverage information, run CMake with the `-DENABLE_TEST_COVERAGE=1` option.

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system.

```bash
cmake -S . -B build/test

# view changes
cmake --build build --target format

# apply changes
cmake --build build --target fix-format
```

See [Format.cmake](https://github.com/TheLartians/Format.cmake) for details.

### Build the documentation

The documentation is automatically built and [published](https://luk036.github.io/ckpttn-cpp) whenever a [GitHub Release](https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository) is created.
To manually build documentation, call the following command.

```bash
cmake -S . -B build
cmake --build build --target GenerateDocs
# view the docs
open build/documentation/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments on installed your system.

### Additional tools

The test and standalone subprojects include the [tools.cmake](cmake/tools.cmake) file which is used to import additional tools on-demand through CMake configuration arguments.
The following are currently supported.

#### Sanitizers

Sanitizers can be enabled by configuring CMake with `-DUSE_SANITIZER=<Address | Memory | MemoryWithOrigins | Undefined | Thread | Leak | 'Address;Undefined'>`.

#### Static Analyzers

Static Analyzers can be enabled by setting `-DUSE_STATIC_ANALYZER=<clang-tidy | iwyu | cppcheck>`, or a combination of those in quotation marks, separated by semicolons.
By default, analyzers will automatically find configuration files such as `.clang-format`.
Additional arguments can be passed to the analyzers by setting the `CLANG_TIDY_ARGS`, `IWYU_ARGS` or `CPPCHECK_ARGS` variables.

#### Ccache

Ccache can be enabled by configuring with `-DUSE_CCACHE=<ON | OFF>`.

## Related projects and alternatives

- [**ModernCppStarter & PVS-Studio Static Code Analyzer**](https://github.com/viva64/pvs-studio-cmake-examples/tree/master/modern-cpp-starter): Official instructions on how to use the ModernCppStarter with the PVS-Studio Static Code Analyzer.
- [**lefticus/cpp_starter_project**](https://github.com/lefticus/cpp_starter_project/): A popular C++ starter project, created in 2017.
- [**filipdutescu/modern-cpp-template**](https://github.com/filipdutescu/modern-cpp-template): A recent starter using a more traditional approach for CMake structure and dependency management.
- [**vector-of-bool/pitchfork**](https://github.com/vector-of-bool/pitchfork/): Pitchfork is a Set of C++ Project Conventions.
