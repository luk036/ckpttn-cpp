#pragma once

#include <cstdint>
#include <netlistx/netlist.hpp>
#include <span>

/**
 * @brief Mid-Level K-Way Partition Manager
 *
 * The `MidLvlKWayPartMgr` class performs mid-level exhaustive partitioning
 * for small hypergraphs with arbitrary number of partitions (k-way).
 * It iterates over partition pairs and uses the middle-levels Gray code
 * algorithm to find optimal partitioning for small instances.
 */
class MidLvlKWayPartMgr {
  public:
    /// @brief Total cost of the current partitioning solution
    int total_cost{};

    /**
     * @brief Constructs a new MidLvlKWayPartMgr object
     *
     * @param[in] bal_tol The balance tolerance for the partitioning
     * @param[in] num_parts The number of partitions to create
     */
    MidLvlKWayPartMgr(double bal_tol, std::uint8_t num_parts);

    /**
     * @brief Optimizes the partition using exhaustive mid-level k-way search
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
    /// @brief Maximum number of FM passes to perform
    static constexpr int max_passes = 5;
    /// @brief Maximum number of modules for pair-wise exhaustive search
    static constexpr size_t max_pair_modules = 15;
};
