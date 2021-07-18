#include <ckpttn/FMConstrMgr.hpp>  // import LegalCheck
#include <ckpttn/MLPartMgr.hpp>
// #include <iostream>

using node_t = typename SimpleNetlist::node_t;

/*!
 * @brief run_Partition
 *
 * @tparam GainMgr
 * @tparam ConstrMgr
 * @param[in] H
 * @param[in,out] part
 * @return LegalCheck
 */
template <typename PartMgr>
auto MLPartMgr::run_FMPartition(const SimpleNetlist& H, gsl::span<std::uint8_t> part)
    -> LegalCheck {
    using GainMgr = typename PartMgr::GainMgr_;
    using ConstrMgr = typename PartMgr::ConstrMgr_;

    auto legalcheck_fn = [&]() {
        auto gainMgr = GainMgr(H, this->K);
        auto constrMgr = ConstrMgr(H, this->BalTol, this->K);
        auto partMgr = PartMgr(H, gainMgr, constrMgr, this->K);
        auto legalcheck = partMgr.legalize(part);
        return std::tuple{legalcheck, partMgr.totalcost};
        // release memory resource all memory saving
    };

    auto optimize_fn = [&]() {
        auto gainMgr = GainMgr(H, this->K);
        auto constrMgr = ConstrMgr(H, this->BalTol, this->K);
        auto partMgr = PartMgr(H, gainMgr, constrMgr, this->K);
        partMgr.optimize(part);
        return partMgr.totalcost;
        // release memory resource all memory saving
    };

    auto [legalcheck, cost] = legalcheck_fn();
    if (legalcheck != LegalCheck::allsatisfied) {
        this->totalcost = cost;
        return legalcheck;
    }

    if (H.number_of_modules() >= this->limitsize) {  // OK
        const auto H2 = create_contraction_subgraph(H, py::set<node_t>{});
        if (H2->number_of_modules() <= H.number_of_modules()) {
            auto part2 = std::vector<std::uint8_t>(H2->number_of_modules(), 0);
            H2->projection_up(part, part2);
            auto legalcheck_recur = this->run_FMPartition<PartMgr>(*H2, part2);
            if (legalcheck_recur == LegalCheck::allsatisfied) {
                H2->projection_down(part2, part);
            }
        }
    }

    this->totalcost = optimize_fn();
    return legalcheck;
}

#include <ckpttn/FMBiConstrMgr.hpp>  // import FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>    // import FMBiGainMgr
#include <ckpttn/FMPartMgr.hpp>      // import FMBiPartMgr

template auto MLPartMgr::run_FMPartition<FMPartMgr<FMBiGainMgr, FMBiConstrMgr>>(
    const SimpleNetlist& H, gsl::span<std::uint8_t> part) -> LegalCheck;

#include <ckpttn/FMKWayConstrMgr.hpp>  // import FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>    // import FMKWayGainMgr

template auto MLPartMgr::run_FMPartition<FMPartMgr<FMKWayGainMgr, FMKWayConstrMgr>>(
    const SimpleNetlist& H, gsl::span<std::uint8_t> part) -> LegalCheck;
