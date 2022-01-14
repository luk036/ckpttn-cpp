#pragma once

#include <stdint.h>  // for uint32_t, uint8_t

/**
 * @brief
 *
 * @tparam Node
 */
template <typename Node> struct MoveInfo {
    Node net;
    Node v;
    std::uint8_t fromPart;
    std::uint8_t toPart;
};

/**
 * @brief
 *
 * @tparam Node
 */
template <typename Node> struct MoveInfoV {
    Node v;
    std::uint8_t fromPart;
    std::uint8_t toPart;
    // node_t v;
};
