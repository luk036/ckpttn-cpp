#pragma once

#include <cinttypes>  // for uint8_t
#include <span>       // for span
#include <vector>     // for vector

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
 * @brief Fiduccia-Mattheyses Partition Constraint Manager
 *
 * @tparam Gnl
 */
template <typename Gnl> class FMConstrMgr {
  private:
    const Gnl &hyprgraph;
    double bal_tol;
    unsigned int total_weight{0};
    unsigned int weight{};  // cache value

  protected:
    std::vector<unsigned int> diff;
    unsigned int lowerbound{};
    std::uint8_t num_parts;

    using node_t = typename Gnl::node_t;

    /**
     * @brief Constructs a new FMConstrMgr object with the given hypergraph and balance tolerance,
     * using a default of 2 partitions.
     *
     * @param[in] hyprgraph The hypergraph to use for the FMConstrMgr.
     * @param[in] bal_tol The balance tolerance to use for the FMConstrMgr.
     */
    FMConstrMgr(const Gnl &hyprgraph, double bal_tol) : FMConstrMgr(hyprgraph, bal_tol, 2) {}

    /**
     * @brief Constructs a new FMConstrMgr object with the given hypergraph, balance tolerance, and
     * number of partitions.
     *
     * @param[in] hyprgraph The hypergraph to use for the FMConstrMgr.
     * @param[in] bal_tol The balance tolerance to use for the FMConstrMgr.
     * @param[in] num_parts The number of partitions to use for the FMConstrMgr.
     */
    FMConstrMgr(const Gnl &hyprgraph, double bal_tol, std::uint8_t num_parts);

  public:
    /**
     * @brief Initializes the FMConstrMgr with the given partition information.
     *
     * @param[in] part The partition information to initialize the FMConstrMgr with.
     */
    auto init(std::span<const std::uint8_t> part) -> void;

    /**
     * @brief Check if the proposed move of the given nodes can be legally performed, and if so,
     * whether it would improve the current partitioning.
     *
     * @param[in] move_info_v A vector of information about the proposed node moves.
     * @return LegalCheck Indicates whether the move is not satisfied, would get better, or is fully
     * satisfied.
     */
    auto check_legal(const MoveInfoV<node_t> &move_info_v) -> LegalCheck;

    /**
     * @brief Check if the proposed moves in the given vector of move information can be legally
     * performed while satisfying the constraints.
     *
     * @param[in] move_info_v A vector of information about the proposed node moves.
     * @return true If the proposed moves can be legally performed while satisfying the constraints.
     * @return false If the proposed moves cannot be legally performed or would violate the
     * constraints.
     */
    auto check_constraints(const MoveInfoV<node_t> &move_info_v) -> bool;

    /**
     * @brief Update the partitioning based on the proposed node moves.
     *
     * @param[in] move_info_v A vector of information about the proposed node moves.
     */
    auto update_move(const MoveInfoV<node_t> &move_info_v) -> void;
};
