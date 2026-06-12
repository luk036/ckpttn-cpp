#pragma once

#include <cstdint>
#include <span>

#include "FMBiConstrMgr.hpp"
#include "FMBiGainCalc.hpp"

/**
 * @brief Mid-Level Partition Manager (2-way)
 *
 * The `MidLvlPartMgr` class performs mid-level (exhaustive) partitioning
 * for small hypergraphs using the middle-levels Gray code algorithm.
 * It enumerates balanced partitions via Hamiltonian cycles to find
 * the optimal 2-way partitioning for small instances.
 *
 * @tparam Gnl The hypergraph type
 */
template <typename Gnl> class MidLvlPartMgr {
  public:
    using node_t = typename Gnl::node_t;

    /// @brief Total cost of the current partitioning solution
    int total_cost{};

    /**
     * @brief Constructs a new MidLvlPartMgr object
     *
     * @param[in] hyprgraph The hypergraph to partition
     * @param[in] bal_tol The balance tolerance for the partitioning
     */
    MidLvlPartMgr(const Gnl& hyprgraph, double bal_tol);

    /**
     * @brief Optimizes the partition using exhaustive mid-level search
     *
     * @param[in,out] part The partition vector to optimize
     */
    void optimize(std::span<std::uint8_t> part);

  private:
    /// @brief Reference to the hypergraph being partitioned
    const Gnl& hyprgraph;
    /// @brief Gain calculator for 2-way FM partitioning
    FMBiGainCalc<Gnl> gain_calc;
    /// @brief Constraint manager for balance checking
    FMBiConstrMgr<Gnl> constr_mgr;
};
