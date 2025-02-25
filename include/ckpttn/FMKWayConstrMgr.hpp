#pragma once

#include <cstdint>   // for uint8_t
#include <span>  // for span
#include <vector>    // for vector

#include "FMConstrMgr.hpp"  // for FMConstrMgr, FMConstrMgr::node_t, Lega...
// #include "moveinfo.hpp"     // for MoveInfo

// forward declare
template <typename Node> struct MoveInfo;
template <typename Node> struct MoveInfoV;

/**
 * @brief Fiduccia-Mattheyses num_parts-Way Partition Constraint Manager
 *
 * @tparam Gnl
 */
template <typename Gnl> class FMKWayConstrMgr : public FMConstrMgr<Gnl> {
  private:
    std::vector<int> illegal;

  public:
    /**
     * @brief Constructs a new FMKWayConstrMgr object.
     *
     * @param[in] hyprgraph The hypergraph to use.
     * @param[in] bal_tol The balance tolerance to use.
     * @param[in] num_parts The number of partitions.
     */
    FMKWayConstrMgr(const Gnl &hyprgraph, double bal_tol, std::uint8_t num_parts)
        : FMConstrMgr<Gnl>{hyprgraph, bal_tol, num_parts}, illegal(num_parts, 1) {}

    /**
     * @brief Selects the next partition to move a node to.
     *
     * @return The index of the partition to move the node to.
     */
    auto select_togo() const -> std::uint8_t;

    /**
     * @brief Initializes the FMKWayConstrMgr object with the given partition information.
     *
     * @param[in] part A span of the partition indices for each node.
     */
    auto init(std::span<const std::uint8_t> part) -> void {
        FMConstrMgr<Gnl>::init(part);
        auto it = this->diff.begin();
        for (auto &il : this->illegal) {
            il = (*it < this->lowerbound);
            ++it;
        }
    }

    /**
     * @brief Checks if the given move information is legal according to the constraints.
     *
     * @param[in] move_info_v The move information to check.
     * @return LegalCheck The result of the legality check.
     */
    auto check_legal(const MoveInfoV<typename Gnl::node_t> &move_info_v) -> LegalCheck;
};
