#pragma once

#include "FMConstrMgr.hpp" // import FMConstrMgr

/*!
 * @brief Constraint Manager
 *
 * Check if (the move of v can satisfied, makebetter, or notsatisfied
 */
class FMBiConstrMgr : public FMConstrMgr
{
  public:
    /*!
     * @brief Construct a new FMBiConstrMgr object
     *
     * @param[in] H
     * @param[in] BalTol
     */
    FMBiConstrMgr(const SimpleNetlist& H, double BalTol)
        : FMConstrMgr {H, BalTol, 2}
    {
    }

    /*!
     * @brief Construct a new FMBiConstrMgr object (for general framework)
     *
     * @param[in] H
     * @param[in] BalTol
     * @param[in] K (for compatability only)
     */
    FMBiConstrMgr(const SimpleNetlist& H, double BalTol, std::uint8_t /*K*/)
        : FMConstrMgr {H, BalTol, 2}
    {
    }

    /*!
     * @brief
     *
     * @return std::uint8_t
     */
    [[nodiscard]] auto select_togo() const -> std::uint8_t
    {
        return this->diff[0] < this->diff[1] ? 0 : 1;
    }
};
