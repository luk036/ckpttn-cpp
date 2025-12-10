# ckpttn-cpp 项目上下文

## 项目概述

ckpttn-cpp 是一个用于现代 C++ 的电路分区库，专为超图（hypergraph）分区设计。主要功能包括多级分区管理，可将复杂的超图划分为多个部分，同时满足特定的平衡性和优化准则。支持二分（binary）和 k 路（k-way）分区。

该项目使用现代 C++（C++20）开发，依赖 XNetwork、Boost 等库，采用多级分区算法实现优化。算法从初始分区开始，通过模块在不同部分之间移动迭代改进，直到达到稳定解或满足停止条件。

## 核心组件

- **MLPartMgr**: 多级分区管理器，实现主分区算法 `run_FMPartition`。
- **FMPartMgr**: Fiduccia-Mattheyses 分区算法管理器，继承自 `PartMgrBase`。
- **Netlist**: 网表结构，基于 XNetwork 的图实现。
- **HierNetlist**: 分层网表，用于多级算法中的图收缩和投影。
- **GainCalc/GainMgr**: 增益计算和管理器，评估模块移动带来的收益。
- **ConstrMgr**: 约束管理器，确保分区满足平衡条件。

## 编译和运行

### 构建项目

```bash
cmake -S. -B build
cmake --build build
```

### 运行独立程序

```bash
./build/standalone/CkPttn --help
```

### 运行测试

```bash
cmake -S. -B build
cmake --build build
cd build/test
CTEST_OUTPUT_ON_FAILURE=1 ctest
```

### 代码格式化

```bash
# 检查格式
cmake --build build --target format

# 应用格式
cmake --build build --target fix-format
```

### 构建文档

```bash
cmake -S . -B build
cmake --build build --target GenerateDocs
```

## 依赖管理

项目使用 CPM.cmake (现代 C++ 包管理器) 管理依赖，主要依赖包括：

- XNetwork (版本 1.6.2)
- Boost
- C++20 标准库 (特别是 `<memory_resource>`)

## 开发规范

- 使用 C++20 标准
- 代码格式化通过 clang-format 和 cmake-format 维护
- 包含集成测试套件
- 使用 GitHub Actions 实现持续集成
- 使用 Codecov 进行代码覆盖率分析
- 模块化设计，采用模板和组件化设计模式

## 重要文件结构

- `include/ckpttn/`: 主要头文件
- `source/`: 源文件实现
- `test/`: 测试代码
- `standalone/`: 独立可执行文件
- `CMakeLists.txt`: CMake 构建配置
- `cmake/`: CMake 模块和工具