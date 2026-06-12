#pragma once

#include <cstdint>
#include <span>

enum class LegalCheck;

/**
 * @brief Multi-Level Mid-Level Partition Manager (2-way)
 *
 * The `MLMidLvlPartMgr` class combines multi-level partitioning with
 * mid-level (exhaustive) refinement for 2-way partitioning. For small
 * hypergraphs it uses the exhaustive mid-level search directly; for
 * larger instances it applies multi-level coarsening followed by
 * mid-level refinement on the coarsened graph.
 */
class MLMidLvlPartMgr {
  public:
    /// @brief Total cost of the current partitioning solution
    int total_cost{};

    /**
     * @brief Constructs a new MLMidLvlPartMgr object (default 2 partitions)
     *
     * @param[in] bal_tol The balance tolerance for the partitioning
     */
    explicit MLMidLvlPartMgr(double bal_tol);

    /**
     * @brief Constructs a new MLMidLvlPartMgr object
     *
     * @param[in] bal_tol The balance tolerance for the partitioning
     * @param[in] num_parts The number of partitions (must be 2 for this type)
     */
    MLMidLvlPartMgr(double bal_tol, std::uint8_t num_parts);

    /**
     * @brief Sets the size limit for multi-level coarsening
     *
     * @param[in] limit The minimum module count to trigger coarsening
     */
    void set_limitsize(size_t limit) { this->limitsize = limit; }

    /**
     * @brief Runs the multi-level mid-level partitioning algorithm
     *
     * @tparam Gnl The hypergraph type
     * @param[in] hyprgraph The input hypergraph
     * @param[in,out] part The partition vector
     * @return LegalCheck The legality check result
     */
    template <typename Gnl> auto run_Partition(const Gnl& hyprgraph, std::span<std::uint8_t> part)
        -> LegalCheck;

  private:
    /// @brief Balance tolerance for partition constraints
    double bal_tol;
    /// @brief Module count threshold to trigger multi-level coarsening
    size_t limitsize{50U};
    /// @brief Module count threshold below which exhaustive search is used
    static constexpr size_t exhaustive_limit{25U};
};
