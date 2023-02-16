#pragma once

#include <cstdint> // for uint32_t, uint8_t

/**
 * @brief
 *
 * @tparam Node
 */
template <typename Node> struct MoveInfo {
  Node net;
  Node v;
  std::uint8_t from_part;
  std::uint8_t to_part;
};

/**
 * @brief
 *
 * @tparam Node
 */
template <typename Node> struct MoveInfoV {
  Node v;
  std::uint8_t from_part;
  std::uint8_t to_part;
  // node_t v;
};
