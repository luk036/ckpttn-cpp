#include <ckpttn/FMConstrMgr.hpp>  // for LegalCheck, LegalCheck::AllSatisfied
#include <ckpttn/MLPartMgr.hpp>    // for MLPartMgr
#include <cstdint>                 // for uint8_t
#include <gsl/span>                // for span
#include <memory>                  // for unique_ptr
#include <py2cpp/set.hpp>          // for set
#include <utility>                 // for pair
#include <vector>                  // for vector

#include "ckpttn/HierNetlist.hpp"  // for HierNetlist, SimpleHierNetlist
#include "ckpttn/netlist.hpp"      // for SimpleNetlist
// #include <iostream>

using node_t = typename SimpleNetlist::node_t;
extern auto create_contracted_subgraph(const SimpleNetlist &, const py::set<node_t> &)
    -> std::unique_ptr<SimpleHierNetlist>;

/**
 * @brief run_Partition
 *
 * @tparam Gnl
 * @tparam PartMgr
 * @param[in] hyprgraph
 * @param[in] hyprgraph
 * @param[in,out] part
 * @return LegalCheck
 */
template <typename Gnl, typename PartMgr>
auto MLPartMgr::run_FMPartition(const Gnl &hyprgraph, gsl::span<std::uint8_t> part) -> LegalCheck {
    using GainMgr = typename PartMgr::GainMgr_;
    using ConstrMgr = typename PartMgr::ConstrMgr_;

    auto legalcheck_fn = [&]() {
        GainMgr gain_mgr(hyprgraph, this->num_parts);
        ConstrMgr constr_mgr(hyprgraph, this->bal_tol, this->num_parts);
        PartMgr part_mgr(hyprgraph, gain_mgr, constr_mgr, this->num_parts);
        auto legalcheck = part_mgr.legalize(part);
        return std::make_pair(legalcheck, part_mgr.total_cost);
        // release memory resource all memory saving
    };

    auto optimize_fn = [&]() {
        GainMgr gain_mgr(hyprgraph, this->num_parts);
        ConstrMgr constr_mgr(hyprgraph, this->bal_tol, this->num_parts);
        PartMgr part_mgr(hyprgraph, gain_mgr, constr_mgr, this->num_parts);
        part_mgr.optimize(part);
        return part_mgr.total_cost;
        // release memory resource all memory saving
    };

    auto legalcheck_cost = legalcheck_fn();
    if (legalcheck_cost.first != LegalCheck::AllSatisfied) {
        this->total_cost = legalcheck_cost.second;
        return legalcheck_cost.first;
    }

    if (hyprgraph.number_of_modules() >= this->limitsize) {  // OK
        const auto hgr2 = create_contracted_subgraph(hyprgraph, py::set<typename Gnl::node_t>{});
        if (hgr2->number_of_modules() <= hyprgraph.number_of_modules()) {
            auto part2 = std::vector<std::uint8_t>(hgr2->number_of_modules(), 0);
            hgr2->projection_up(part, part2);
            auto legalcheck_recur = this->run_FMPartition<Gnl, PartMgr>(*hgr2, part2);
            if (legalcheck_recur == LegalCheck::AllSatisfied) {
                hgr2->projection_down(part2, part);
            }
        }
    }

    this->total_cost = optimize_fn();
    return legalcheck_cost.first;
}

#include <ckpttn/FMBiConstrMgr.hpp>    // for FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>      // for FMBiGainMgr
#include <ckpttn/FMKWayConstrMgr.hpp>  // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>    // for FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>        // for FMPartMgr

template auto MLPartMgr::run_FMPartition<
    SimpleNetlist,
    FMPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>>(
    const SimpleNetlist &hyprgraph, gsl::span<std::uint8_t> part) -> LegalCheck;

template auto MLPartMgr::run_FMPartition<
    SimpleNetlist,
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>>(
    const SimpleNetlist &hyprgraph, gsl::span<std::uint8_t> part) -> LegalCheck;
