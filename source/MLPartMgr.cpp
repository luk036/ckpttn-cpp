#include <ckpttn/FMConstrMgr.hpp>  // for LegalCheck, LegalCheck::allsatisfied
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
extern auto create_contraction_subgraph(const SimpleNetlist&, const py::set<node_t>&)
    -> std::unique_ptr<SimpleHierNetlist>;

/**
 * @brief run_Partition
 *
 * @tparam Gnl
 * @tparam PartMgr
 * @param H
 * @param[in] H
 * @param[in,out] part
 * @return LegalCheck
 */
template <typename Gnl, typename PartMgr>
auto MLPartMgr::run_FMPartition(const Gnl& H, gsl::span<std::uint8_t> part) -> LegalCheck {
    using GainMgr = typename PartMgr::GainMgr_;
    using ConstrMgr = typename PartMgr::ConstrMgr_;

    auto legalcheck_fn = [&]() {
        GainMgr gainMgr(H, this->K);
        ConstrMgr constrMgr(H, this->BalTol, this->K);
        PartMgr partMgr(H, gainMgr, constrMgr, this->K);
        auto legalcheck = partMgr.legalize(part);
        return std::make_pair(legalcheck, partMgr.totalcost);
        // release memory resource all memory saving
    };

    auto optimize_fn = [&]() {
        GainMgr gainMgr(H, this->K);
        ConstrMgr constrMgr(H, this->BalTol, this->K);
        PartMgr partMgr(H, gainMgr, constrMgr, this->K);
        partMgr.optimize(part);
        return partMgr.totalcost;
        // release memory resource all memory saving
    };

    auto legalcheck_cost = legalcheck_fn();
    if (legalcheck_cost.first != LegalCheck::allsatisfied) {
        this->totalcost = legalcheck_cost.second;
        return legalcheck_cost.first;
    }

    if (H.number_of_modules() >= this->limitsize) {  // OK
        const auto H2 = create_contraction_subgraph(H, py::set<typename Gnl::node_t>{});
        if (H2->number_of_modules() <= H.number_of_modules()) {
            auto part2 = std::vector<std::uint8_t>(H2->number_of_modules(), 0);
            H2->projection_up(part, part2);
            auto legalcheck_recur = this->run_FMPartition<Gnl, PartMgr>(*H2, part2);
            if (legalcheck_recur == LegalCheck::allsatisfied) {
                H2->projection_down(part2, part);
            }
        }
    }

    this->totalcost = optimize_fn();
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
    const SimpleNetlist& H, gsl::span<std::uint8_t> part) -> LegalCheck;

template auto MLPartMgr::run_FMPartition<
    SimpleNetlist,
    FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>, FMKWayConstrMgr<SimpleNetlist>>>(
    const SimpleNetlist& H, gsl::span<std::uint8_t> part) -> LegalCheck;
