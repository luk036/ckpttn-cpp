#pragma once

// #if __has_include(<memeory_resource>)
#include <memory_resource>
#include <vector>
namespace FMPmr = std::pmr;
// #else
// #include <boost/container/pmr/memory_resource.hpp>
// #include <boost/container/pmr/monotonic_buffer_resource.hpp>
// #include <boost/container/pmr/vector.hpp>
// namespace FMPmr = boost::container::pmr;
// #endif

const auto FM_MAX_NUM_PARTITIONS = 255U;
const auto FM_MAX_DEGREE = 65536U;

// workaround clang