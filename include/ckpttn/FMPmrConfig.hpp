#pragma once

#if __has_include(<memory_resource>)
#include <memory_resource>
#include <vector>
namespace FMPmr = std::pmr;
#elif __has_include(<experimental/memory_resource>)
#include <experimental/memory_resource>
#include <experimental/vector>
namespace FMPmr = std::experimental::pmr;
#endif

const auto FM_MAX_NUM_PARTITIONS = 255U;
const auto FM_MAX_DEGREE = 65536U;

// workaround clang