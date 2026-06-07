#pragma once

#include <cstdint>
#include <span>
#include <vector>

/**
 * @brief Exhaustive Partition Manager using Middle-Levels Gray Code
 *
 * `MidLvlPartMgr` enumerates all balanced (or near-balanced) 2-way partitions
 * of a hypergraph's modules using the Gray code order of the middle levels
 * graph. Each step flips a single bit — moving one module to the opposite
 * partition — exactly like the FM algorithm's move operation, but exhaustively
 * visiting all possible assignments instead of using local search.
 *
 * For N modules:
 *  - If N is odd: the middle levels bitstring has length N, visiting all states
 *    with floor(N/2) or ceil(N/2) ones.
 *  - If N is even: one dummy module is added (bitstring length N+1). The dummy
 *    module's bit may float (0 or 1), so all balanced and near-balanced
 *    partitions of the N real modules are visited.
 *
 * The algorithm starts from the initial partition, traverses the full Hamilton
 * cycle, snapshots the lowest-cost solution, and restores it.
 *
 * @tparam Gnl The hypergraph type (must match SimpleNetlist-like interface)
 */
template <typename Gnl>
class MidLvlPartMgr {
  public:
    using node_t = typename Gnl::node_t;

    /// @brief Total cost of the best partition found
    int total_cost{};

    explicit MidLvlPartMgr(const Gnl& hyprgraph);

    void optimize(std::span<std::uint8_t> part);

    auto compute_total_cost(std::span<const std::uint8_t> part) const -> int;

  private:
    const Gnl& hyprgraph;
};
