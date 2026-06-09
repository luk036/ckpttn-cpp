#include <ckpttn/MLMidLvlPartMgr.hpp>

#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/HierNetlist.hpp>
#include <ckpttn/MidLvlPartMgr.hpp>
#include <cstdint>
#include <iostream>
#include <memory>
#include <netlistx/netlist.hpp>
#include <new>
#include <py2cpp/set.hpp>
#include <span>
#include <vector>

using node_t = SimpleNetlist::node_t;
extern auto create_contracted_subgraph(const SimpleNetlist&, py::set<node_t>)
    -> std::unique_ptr<SimpleHierNetlist>;

MLMidLvlPartMgr::MLMidLvlPartMgr(double bal_tol, std::uint8_t num_parts)
    : bal_tol{bal_tol}, num_parts{num_parts} {}

template <typename Gnl>
auto MLMidLvlPartMgr::run_Partition(const Gnl& hyprgraph,
                                     std::span<std::uint8_t> part) -> LegalCheck {
    using GainMgr = FMBiGainMgr<Gnl>;
    using ConstrMgr = FMBiConstrMgr<Gnl>;
    using PartMgr = FMPartMgr<Gnl, GainMgr, ConstrMgr>;

    if (hyprgraph.number_of_modules() <= exhaustive_limit) {
        MidLvlPartMgr<Gnl> mid_mgr(hyprgraph, this->bal_tol);
        mid_mgr.optimize(part);
        this->total_cost = mid_mgr.total_cost;
        if (auto constr_mgr = FMBiConstrMgr<Gnl>(hyprgraph, this->bal_tol);
            constr_mgr.final_check(part)) {
            return LegalCheck::AllSatisfied;
        }
        return LegalCheck::GetBetter;
    }

    GainMgr legal_gain_mgr(hyprgraph);
    ConstrMgr legal_constr_mgr(hyprgraph, this->bal_tol);
    PartMgr legal_part_mgr(hyprgraph, legal_gain_mgr, legal_constr_mgr);
    auto lc = legal_part_mgr.legalize(part);
    if (lc != LegalCheck::AllSatisfied) {
        this->total_cost = legal_part_mgr.total_cost;
        return lc;
    }

    if (hyprgraph.number_of_modules() >= this->limitsize) {
        try {
            const auto hgr2
                = create_contracted_subgraph(hyprgraph, py::set<typename Gnl::node_t>{});
            if (hgr2->number_of_modules() * 3 / 2 < hyprgraph.number_of_modules()) {
                auto part2 = std::vector<std::uint8_t>(hgr2->number_of_modules(), 0);
                hgr2->projection_up(part, part2);
                auto lc_recur = this->run_Partition<Gnl>(*hgr2, part2);
                if (lc_recur != LegalCheck::NotSatisfied) {
                    hgr2->projection_down(part2, part);
                }
            }
        } catch (const std::bad_alloc& e) {
            std::cerr << "Out of Memory: " << e.what() << '\n';
        }
    }

    GainMgr gain_mgr(hyprgraph);
    ConstrMgr constr_mgr(hyprgraph, this->bal_tol);
    PartMgr part_mgr(hyprgraph, gain_mgr, constr_mgr);
    part_mgr.optimize(part);
    this->total_cost = part_mgr.total_cost;

    return LegalCheck::AllSatisfied;
}

template auto MLMidLvlPartMgr::run_Partition<SimpleNetlist>(
    const SimpleNetlist& hyprgraph, std::span<std::uint8_t> part) -> LegalCheck;
