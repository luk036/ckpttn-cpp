#pragma once

#include <cinttypes>  // for uint8_t
#include <gsl/span>   // for span
#include <vector>     // for vector

#include "moveinfo.hpp"  // for MoveInfo

// forward declare
// template <typename graph_t> struct Netlist;
// using SimpleNetlist = Netlist<xnetwork::SimpleGraph>;

/**
 * @brief Check if the move of v can satisfied, getbetter, or notsatisfied
 *
 */
enum class LegalCheck { notsatisfied, getbetter, allsatisfied };

/**
 * @brief FM Partition Constraint Manager
 *
 */
template <typename Gnl> class FMConstrMgr {
  private:
    const Gnl& H;
    double BalTol;
    unsigned int totalweight{0};
    unsigned int weight{};  // cache value

  protected:
    std::vector<unsigned int> diff;
    unsigned int lowerbound{};
    std::uint8_t K;

    using node_t = typename Gnl::node_t;

    /**
     * @brief Construct a new FMConstrMgr object
     *
     * @param[in] H
     * @param[in] BalTol
     */
    FMConstrMgr(const Gnl& H, double BalTol) : FMConstrMgr(H, BalTol, 2) {}

    /**
     * @brief Construct a new FMConstrMgr object
     *
     * @param[in] H
     * @param[in] BalTol
     * @param[in] K
     */
    FMConstrMgr(const Gnl& H, double BalTol, std::uint8_t K);

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
    auto check_legal(const MoveInfoV<node_t>& move_info_v) -> LegalCheck;

    /**
     * @brief
     *
     * @param[in] move_info_v
     * @return true
     * @return false
     */
    auto check_constraints(const MoveInfoV<node_t>& move_info_v) -> bool;

    /**
     * @brief
     *
     * @param[in] move_info_v
     */
    auto update_move(const MoveInfoV<node_t>& move_info_v) -> void;
};
