/**
 * @file bench_common.hpp
 * @brief Shared driver for the partitioning benchmarks.
 *
 * Centralises the (k x FM/NN) partition-manager selection that the flat and
 * multi-level benchmarks otherwise repeat. `partition_flat` and `partition_ml`
 * return the total cut cost, or -1 when legalization is not satisfied.
 */

#pragma once

#include <cstddef>  // for size_t
#include <cstdint>  // for uint8_t
#include <future>   // for future
#include <limits>   // for numeric_limits
#include <optional>  // for optional
#include <string_view>
#include <vector>  // for vector
#include <span>     // for span

#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMKWayConstrMgr.hpp>
#include <ckpttn/FMKWayGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/LegalCheck.hpp>
#include <ckpttn/MLPartMgr.hpp>
#include <ckpttn/NNPartMgr.hpp>
#include <netlistx/netlist.hpp>
#include <xnetwork/thread_pool.hpp>  // for thread_pool

using BiFmPM = FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>;
using BiNnPM = NNPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>;
using KwFmPM = FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>;
using KwNnPM = NNPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>;

extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& hyprgraph, std::string_view areFileName);

/**
 * @brief Run the single-level partitioner for the given algorithm and k.
 *
 * @return the total cut cost, or -1 if legalization is not satisfied.
 */
inline auto partition_flat(const SimpleNetlist& hyprgraph, std::span<std::uint8_t> part,
                           std::uint8_t k, bool nn, double bal_tol) -> int {
    if (k == 2U) {
        FMBiGainMgr<SimpleNetlist> gain_mgr{hyprgraph};
        FMBiConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, bal_tol};
        if (nn) {
            BiNnPM part_mgr{hyprgraph, gain_mgr, constr_mgr, 2U};
            if (part_mgr.legalize(part) != LegalCheck::AllSatisfied) {
                return -1;
            }
            part_mgr.optimize(part);
            return part_mgr.total_cost;
        }
        BiFmPM part_mgr{hyprgraph, gain_mgr, constr_mgr};
        if (part_mgr.legalize(part) != LegalCheck::AllSatisfied) {
            return -1;
        }
        part_mgr.optimize(part);
        return part_mgr.total_cost;
    }
    FMKWayGainMgr<SimpleNetlist> gain_mgr{hyprgraph, k};
    FMKWayConstrMgr<SimpleNetlist> constr_mgr{hyprgraph, bal_tol, k};
    if (nn) {
        KwNnPM part_mgr{hyprgraph, gain_mgr, constr_mgr, k};
        if (part_mgr.legalize(part) != LegalCheck::AllSatisfied) {
            return -1;
        }
        part_mgr.optimize(part);
        return part_mgr.total_cost;
    }
    KwFmPM part_mgr{hyprgraph, gain_mgr, constr_mgr, k};
    if (part_mgr.legalize(part) != LegalCheck::AllSatisfied) {
        return -1;
    }
    part_mgr.optimize(part);
    return part_mgr.total_cost;
}

/**
 * @brief Run the multi-level partitioner for the given algorithm and k.
 *
 * @return the total cut cost, or -1 if legalization is not satisfied.
 */
inline auto partition_ml(const SimpleNetlist& hyprgraph, std::span<std::uint8_t> part,
                         std::uint8_t k, bool nn, double bal_tol, std::size_t limitsize,
                         std::optional<double> contraction_ratio = std::nullopt) -> int {
    if (k == 2U) {
        MLPartMgr part_mgr{bal_tol};
        part_mgr.set_limitsize(limitsize);
        if (contraction_ratio) part_mgr.set_contraction_ratio(*contraction_ratio);
        const auto legal_check = nn ? part_mgr.run_Partition<SimpleNetlist, BiNnPM>(hyprgraph, part)
                                    : part_mgr.run_Partition<SimpleNetlist, BiFmPM>(hyprgraph, part);
        return legal_check == LegalCheck::AllSatisfied ? part_mgr.total_cost : -1;
    }
    MLPartMgr part_mgr{bal_tol, k};
    part_mgr.set_limitsize(limitsize);
    if (contraction_ratio) part_mgr.set_contraction_ratio(*contraction_ratio);
    const auto legal_check = nn ? part_mgr.run_Partition<SimpleNetlist, KwNnPM>(hyprgraph, part)
                                : part_mgr.run_Partition<SimpleNetlist, KwFmPM>(hyprgraph, part);
    return legal_check == LegalCheck::AllSatisfied ? part_mgr.total_cost : -1;
}

/**
 * @brief Deterministic SplitMix64-based initial partition (same scheme as BenchFMNN).
 *
 * @return a vector of length n with each entry in [0, k).
 */
inline auto bench_make_init(std::size_t n, std::uint8_t k, std::uint64_t seed)
    -> std::vector<std::uint8_t> {
    std::uint64_t state = seed;
    auto next = [&state]() -> std::uint64_t {
        state += 0x9E3779B97F4A7C15ULL;
        std::uint64_t z = state;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    };
    auto part = std::vector<std::uint8_t>(n, 0U);
    for (std::size_t i = 0; i != n; ++i) {
        part[i] = static_cast<std::uint8_t>(next() % k);
    }
    return part;
}

/// @brief Seed spacing between consecutive multi-start trials.
inline constexpr std::uint64_t kStartSeedStride = 1000003ULL;

/**
 * @brief Run `num_starts` independent partition starts on a thread pool, returning the best cost.
 *
 * Start `i` uses the deterministic initial partition derived from
 * `base_seed + i * kStartSeedStride`, so results are reproducible across runs. The tasks share
 * only the read-only hypergraph; each builds its own partition-manager instance.
 *
 * @return the minimum cut cost over the starts, or -1 when every start fails legalization.
 */
inline auto multi_start_cost(const SimpleNetlist& hyprgraph, std::uint8_t k, bool nn, bool ml,
                             double bal_tol, std::size_t limitsize,
                             std::optional<double> contraction_ratio, unsigned num_starts,
                             std::uint32_t base_seed, xnetwork::thread_pool& pool) -> int {
    const auto n = hyprgraph.number_of_modules();
    auto futures = std::vector<std::future<int>>{};
    futures.reserve(num_starts);
    for (auto i = 0U; i < num_starts; ++i) {
        const auto start_seed = static_cast<std::uint64_t>(base_seed) + i * kStartSeedStride;
        futures.push_back(pool.enqueue([&hyprgraph, k, nn, ml, bal_tol, limitsize, contraction_ratio,
                                        start_seed, n]() -> int {
            auto part = bench_make_init(n, k, start_seed);
            return ml ? partition_ml(hyprgraph, part, k, nn, bal_tol, limitsize, contraction_ratio)
                      : partition_flat(hyprgraph, part, k, nn, bal_tol);
        }));
    }
    auto best = std::numeric_limits<int>::max();
    for (auto& future : futures) {
        const auto cost = future.get();
        if (cost >= 0 && cost < best) {
            best = cost;
        }
    }
    return best == std::numeric_limits<int>::max() ? -1 : best;
}
