#pragma once

#include <stdint.h>  // for uint8_t

#include <algorithm>  // for min_element
#include <gsl/span>   // for span
#include <iterator>   // for distance
#include <vector>     // for vector
// #include <range/v3/algorithm/min_element.hpp>

#include "FMConstrMgr.hpp"     // for FMConstrMgr, FMConstrMgr::node_t, Lega...
#include "ckpttn/netlist.hpp"  // for MoveInfoV (ptr only), SimpleNetlist
// #include <range/v3/view/zip.hpp>
// Check if (the move of v can satisfied, makebetter, or notsatisfied

/**
 * @brief FM K-Way Partition Constraint Manager
 *
 */
class FMKWayConstrMgr : public FMConstrMgr {
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
    FMKWayConstrMgr(const SimpleNetlist& H, double BalTol, std::uint8_t K)
        : FMConstrMgr{H, BalTol, K}, illegal(K, 1) {}

    /**
     * @brief
     *
     * @return std::uint8_t
     */
    [[nodiscard]] auto select_togo() const -> std::uint8_t {
        auto it = std::min_element(this->diff.cbegin(), this->diff.cend());
        return std::uint8_t(std::distance(this->diff.cbegin(), it));
    }

    /**
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> void {
        FMConstrMgr::init(part);
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
    auto check_legal(const MoveInfoV<node_t>& move_info_v) -> LegalCheck;
};
