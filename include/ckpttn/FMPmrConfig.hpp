#pragma once

// #include <boost/container/pmr/memory_resource.hpp>
// #include <boost/container/pmr/monotonic_buffer_resource.hpp>
// #include <boost/container/pmr/vector.hpp>
// namespace FMPmr = boost::container::pmr;

#include <memory_resource>
namespace FMPmr = std::pmr;

const auto FM_MAX_NUM_PARTITIONS = 255U;
const auto FM_MAX_DEGREE = 65536U;

// workaround clang