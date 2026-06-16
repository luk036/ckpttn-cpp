/**
 * @file FMPmrConfig.hpp
 * @brief PMR configuration and constants for FM algorithm
 */

#pragma once

// #include <boost/container/pmr/memory_resource.hpp>
// #include <boost/container/pmr/monotonic_buffer_resource.hpp>
// #include <boost/container/pmr/vector.hpp>
// namespace FMPmr = boost::container::pmr;

#include <memory_resource>  // for std::pmr

/**
 * @brief PMR namespace alias
 *
 * By default, `FMPmr` aliases `std::pmr`. To use Boost.Container's PMR
 * instead of the standard library version, uncomment the Boost includes
 * and alias above and comment out the std::pmr line below.
 */
namespace FMPmr = std::pmr;

/// @brief Maximum number of partitions supported by the FM algorithm
const auto FM_MAX_NUM_PARTITIONS = 255U;
/// @brief Maximum degree (net size) supported by the FM algorithm
const auto FM_MAX_DEGREE = 500U;