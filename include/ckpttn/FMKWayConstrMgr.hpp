#pragma once

#include "FMConstrMgr.hpp"
#include <gsl/span>
#include <range/v3/algorithm/min_element.hpp>
// #include <range/v3/view/zip.hpp>
// Check if (the move of v can satisfied, makebetter, or notsatisfied

/*!
 * @brief FM K-Way Partition Constraint Manager
 *
 */
class FMKWayConstrMgr : public FMConstrMgr
{
  private:
    std::vector<int> illegal;

  public:
    /*!
     * @brief Construct a new FMKWayConstrMgr object
     *
     * @param[in] H
     * @param[in] BalTol
     * @param[in] K
     */
    FMKWayConstrMgr(const SimpleNetlist& H, double BalTol, std::uint8_t K)
        : FMConstrMgr {H, BalTol, K}
        , illegal(K, 1)
    {
    }

    /*!
     * @brief
     *
     * @return std::uint8_t
     */
    [[nodiscard]] auto select_togo() const -> std::uint8_t
    {
        auto it = ranges::min_element(this->diff);
        return std::uint8_t(std::distance(this->diff.cbegin(), it));
    }

    /*!
     * @brief
     *
     * @param[in] part
     */
    auto init(gsl::span<const std::uint8_t> part) -> void
    {
        FMConstrMgr::init(part);
        auto it = this->diff.begin();
        for (auto& il : this->illegal)
        {
            il = (*it < this->lowerbound);
            ++it;
        }
    }

    /*!
     * @brief
     *
     * @param[in] move_info_v
     * @return LegalCheck
     */
    auto check_legal(const MoveInfoV<node_t>& move_info_v) -> LegalCheck;
};
