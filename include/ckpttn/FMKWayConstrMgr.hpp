#pragma once

#include <stdint.h>  // for uint8_t

#include <gsl/span>  // for span
#include <vector>    // for vector

#include "FMConstrMgr.hpp"  // for FMConstrMgr, FMConstrMgr::node_t, Lega...
#include "moveinfo.hpp"     // for MoveInfo

// forward declare
// template <typename graph_t> struct Netlist;
// using SimpleNetlist = Netlist<xnetwork::SimpleGraph>;

// #include <range/v3/view/zip.hpp>
// Check if (the move of v can satisfied, makebetter, or notsatisfied

/**
 * @brief FM K-Way Partition Constraint Manager
 *
 */
template <typename Gnl> class FMKWayConstrMgr : public FMConstrMgr<Gnl> {
  private:
    std::vector<int> illegal;

  public:
    /**
     * @brief Construct a new FMKWayConstrMgr object
     *
     * @param[in] H
     * @param[in] BalTol
     * @param[in] K
     */
    FMKWayConstrMgr(const Gnl& H, double BalTol, std::uint8_t K)
        : FMConstrMgr<Gnl>{H, BalTol, K}, illegal(K, 1) {}

    /**
     * @brief
     *
     * @return std::uint8_t
     */
    [[nodiscard]] auto select_togo() const -> std::uint8_t;

    /**
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> void {
        FMConstrMgr<Gnl>::init(part);
        auto it = this->diff.begin();
        for (auto& il : this->illegal) {
            il = (*it < this->lowerbound);
            ++it;
        }
    }

    /**
     * @brief
     *
     * @param[in] move_info_v
     * @return LegalCheck
     */
    auto check_legal(const MoveInfoV<typename Gnl::node_t>& move_info_v) -> LegalCheck;
};
