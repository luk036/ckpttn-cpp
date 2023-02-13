#pragma once

#include <cstdint>   // for uint8_t
#include <gsl/span>  // for span
#include <vector>    // for vector

#include "FMConstrMgr.hpp"  // for FMConstrMgr, FMConstrMgr::node_t, Lega...
// #include "moveinfo.hpp"     // for MoveInfo

// forward declare
template <typename Node> struct MoveInfo;
template <typename Node> struct MoveInfoV;

/**
 * @brief FM num_parts-Way Partition Constraint Manager
 *
 * @tparam Gnl
 */
template <typename Gnl> class FMKWayConstrMgr : public FMConstrMgr<Gnl> {
  private:
    std::vector<int> illegal;

  public:
    /**
     * @brief Construct a new FMKWayConstrMgr object
     *
     * @param[in] hgr
     * @param[in] bal_tol
     * @param[in] num_parts
     */
    FMKWayConstrMgr(const Gnl& hgr, double bal_tol, std::uint8_t num_parts)
        : FMConstrMgr<Gnl>{hgr, bal_tol, num_parts}, illegal(num_parts, 1) {}

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
