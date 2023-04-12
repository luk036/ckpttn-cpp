#pragma once

#include <cinttypes> // for uint8_t
#include <gsl/span>  // for span
#include <vector>    // for vector

// #include "moveinfo.hpp"  // for MoveInfo

// forward declare
template <typename Node> struct MoveInfo;
template <typename Node> struct MoveInfoV;

/**
 * @brief Check if the move of v can satisfied, GetBetter, or NotSatisfied
 *
 */
enum class LegalCheck { NotSatisfied, GetBetter, AllSatisfied };

/**
 * @brief FM Partition Constraint Manager
 *
 * @tparam Gnl
 */
template <typename Gnl> class FMConstrMgr {
private:
  const Gnl &hgr;
  double bal_tol;
  unsigned int total_weight{0};
  unsigned int weight{}; // cache value

protected:
  std::vector<unsigned int> diff;
  unsigned int lowerbound{};
  std::uint8_t num_parts;

  using node_t = typename Gnl::node_t;

  /**
   * @brief Construct a new FMConstrMgr object
   *
   * @param[in] hgr
   * @param[in] bal_tol
   */
  FMConstrMgr(const Gnl &hgr, double bal_tol) : FMConstrMgr(hgr, bal_tol, 2) {}

  /**
   * @brief Construct a new FMConstrMgr object
   *
   * @param[in] hgr
   * @param[in] bal_tol
   * @param[in] num_parts
   */
  FMConstrMgr(const Gnl &hgr, double bal_tol, std::uint8_t num_parts);

public:
  /**
   * @brief
   *
   * @param[in] part
   */
  auto init(gsl::span<const std::uint8_t> part) -> void;

  /**
   * @brief
   *
   * @param[in] move_info_v
   * @return LegalCheck
   */
  auto check_legal(const MoveInfoV<node_t> &move_info_v) -> LegalCheck;

  /**
   * @brief
   *
   * @param[in] move_info_v
   * @return true
   * @return false
   */
  auto check_constraints(const MoveInfoV<node_t> &move_info_v) -> bool;

  /**
   * @brief
   *
   * @param[in] move_info_v
   */
  auto update_move(const MoveInfoV<node_t> &move_info_v) -> void;
};
