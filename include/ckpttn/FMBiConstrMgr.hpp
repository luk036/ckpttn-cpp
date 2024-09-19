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
     * @brief Construct a new FMBiConstrMgr object (for general framework)
     *
     * @param[in] hyprgraph The hypergraph to use
     * @param[in] bal_tol The balance tolerance to use
     */
    FMBiConstrMgr(const Gnl &hyprgraph, double bal_tol) : FMConstrMgr<Gnl>{hyprgraph, bal_tol, 2} {}

    /**
     * @brief Constructs a new FMBiConstrMgr object for the general framework.
     *
     * @param[in] hyprgraph The hypergraph to use.
     * @param[in] bal_tol The balance tolerance to use.
     */
    FMBiConstrMgr(const Gnl &hyprgraph, double bal_tol, std::uint8_t /*num_parts*/)
        : FMConstrMgr<Gnl>{hyprgraph, bal_tol, 2} {}

    /**
     * @brief Selects the partition to move a vertex to based on the difference in partition sizes.
     *
     * @return std::uint8_t The index of the partition to move the vertex to (0 or 1).
     */
    auto select_togo() const -> std::uint8_t { return this->diff[0] < this->diff[1] ? 0 : 1; }
};
