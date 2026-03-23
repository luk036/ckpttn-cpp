#pragma once

#include <cstdint>  // for uint32_t, uint8_t

/**
 * @brief Move information for a single vertex in a net
 *
 * This struct stores information about moving a single vertex from one partition to another.
 * Used in the FM algorithm to track gain calculations for individual net-pin relationships.
 *
 * @tparam Node The node type (typically representing a module or net)
 */
template <typename Node> struct MoveInfo {
    /// @brief The net being processed
    Node net;
    /// @brief The vertex being moved
    Node v;
    /// @brief The partition the vertex is moving from
    std::uint8_t from_part;
    /// @brief The partition the vertex is moving to
    std::uint8_t to_part;
};

/**
 * @brief Move information for a single vertex (without net reference)
 *
 * This struct stores simplified information about moving a vertex from one partition to another.
 * Used in the FM algorithm for gain calculation and constraint checking.
 *
 * @tparam Node The node type (typically representing a module)
 */
template <typename Node> struct MoveInfoV {
    /// @brief The vertex being moved
    Node v;
    /// @brief The partition the vertex is moving from
    std::uint8_t from_part;
    /// @brief The partition the vertex is moving to
    std::uint8_t to_part;
    // node_t v;
};
