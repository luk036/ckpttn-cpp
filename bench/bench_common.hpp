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
#include <string_view>
#include <span>  // for span

#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMKWayConstrMgr.hpp>
#include <ckpttn/FMKWayGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/LegalCheck.hpp>
#include <ckpttn/MLPartMgr.hpp>
#include <ckpttn/NNPartMgr.hpp>
#include <netlistx/netlist.hpp>

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
                         std::uint8_t k, bool nn, double bal_tol, std::size_t limitsize) -> int {
    if (k == 2U) {
        MLPartMgr part_mgr{bal_tol};
        part_mgr.set_limitsize(limitsize);
        const auto legal_check = nn ? part_mgr.run_Partition<SimpleNetlist, BiNnPM>(hyprgraph, part)
                                    : part_mgr.run_Partition<SimpleNetlist, BiFmPM>(hyprgraph, part);
        return legal_check == LegalCheck::AllSatisfied ? part_mgr.total_cost : -1;
    }
    MLPartMgr part_mgr{bal_tol, k};
    part_mgr.set_limitsize(limitsize);
    const auto legal_check = nn ? part_mgr.run_Partition<SimpleNetlist, KwNnPM>(hyprgraph, part)
                                : part_mgr.run_Partition<SimpleNetlist, KwFmPM>(hyprgraph, part);
    return legal_check == LegalCheck::AllSatisfied ? part_mgr.total_cost : -1;
}
