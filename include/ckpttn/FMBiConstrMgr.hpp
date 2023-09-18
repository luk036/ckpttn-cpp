#pragma once

#include "FMConstrMgr.hpp"  // import FMConstrMgr

/**
 * @brief Constraint Manager
 *
 * Check if (the move of v can satisfied, makebetter, or NotSatisfied
 *
 * @tparam Gnl
 */
template <typename Gnl> class FMBiConstrMgr : public FMConstrMgr<Gnl> {
  public:
    /**
     * @brief Construct a new FMBiConstrMgr object
     *
     * @param[in] hyprgraph
     * @param[in] bal_tol
     */
    FMBiConstrMgr(const Gnl &hyprgraph, double bal_tol) : FMConstrMgr<Gnl>{hyprgraph, bal_tol, 2} {}

    /**
     * @brief Construct a new FMBiConstrMgr object (for general framework)
     *
     * @param[in] hyprgraph
     * @param[in] bal_tol
     */
    FMBiConstrMgr(const Gnl &hyprgraph, double bal_tol, std::uint8_t /*num_parts*/)
        : FMConstrMgr<Gnl>{hyprgraph, bal_tol, 2} {}

    /**
     * @brief
     *
     * @return std::uint8_t
     */
    auto select_togo() const -> std::uint8_t { return this->diff[0] < this->diff[1] ? 0 : 1; }
};
