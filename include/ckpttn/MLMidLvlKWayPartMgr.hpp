/**
 * @file MLMidLvlKWayPartMgr.hpp
 * @brief Multi-level mid-level k-way partition manager
 */

#pragma once

#include <cstdint>
#include <netlistx/netlist.hpp>
#include <span>

/**
 * @brief Multi-Level Mid-Level K-Way Partition Manager
 *
 * The `MLMidLvlKWayPartMgr` class combines multi-level partitioning with
 * mid-level (exhaustive) refinement for k-way partitioning. For small
 * hypergraphs it uses the exhaustive mid-level k-way search directly;
 * for larger instances it applies multi-level coarsening followed by
 * FM-based refinement on the coarsened graph.
 */
class MLMidLvlKWayPartMgr {
  public:
    /// @brief Total cost of the current partitioning solution
    int total_cost{};

    /**
     * @brief Constructs a new MLMidLvlKWayPartMgr object
     *
     * @param[in] bal_tol The balance tolerance for the partitioning
     * @param[in] num_parts The number of partitions to create
     */
    MLMidLvlKWayPartMgr(double bal_tol, std::uint8_t num_parts);

    /**
     * @brief Sets the size limit for multi-level coarsening
     *
     * @param[in] limit The minimum module count to trigger coarsening
     */
    void set_limitsize(size_t limit) { this->limitsize_ = limit; }

    /**
     * @brief Optimizes the partition using multi-level mid-level k-way algorithm
     *
     * @param[in,out] part The partition vector to optimize
     * @param[in] hyprgraph The hypergraph to partition
     */
    void optimize(std::span<std::uint8_t> part, const SimpleNetlist& hyprgraph);

  private:
    /// @brief Balance tolerance for partition constraints
    double bal_tol_;
    /// @brief Number of partitions
    std::uint8_t num_parts_;
    /// @brief Module count threshold to trigger multi-level coarsening
    size_t limitsize_{50U};
    /// @brief Base module count threshold for exhaustive search (scaled by num_parts)
    static constexpr size_t base_exhaustive{25U};
};
