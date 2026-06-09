#include <ckpttn/FMConstrMgr.hpp>
#include <ckpttn/FMKWayConstrMgr.hpp>
#include <ckpttn/FMKWayGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/HierNetlist.hpp>
#include <ckpttn/MLMidLvlKWayPartMgr.hpp>
#include <ckpttn/MidLvlKWayPartMgr.hpp>
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

MLMidLvlKWayPartMgr::MLMidLvlKWayPartMgr(double bal_tol, std::uint8_t num_parts)
    : bal_tol_{bal_tol}, num_parts_{num_parts} {}

void MLMidLvlKWayPartMgr::optimize(std::span<std::uint8_t> part, const SimpleNetlist& hyprgraph) {
    using GainMgr = FMKWayGainMgr<SimpleNetlist>;
    using ConstrMgr = FMKWayConstrMgr<SimpleNetlist>;
    using PartMgr = FMPartMgr<SimpleNetlist, GainMgr, ConstrMgr>;

    const auto exhaustive_limit = (base_exhaustive * this->num_parts_) / 2U;

    if (hyprgraph.number_of_modules() <= exhaustive_limit) {
        MidLvlKWayPartMgr kway_mgr{this->bal_tol_, this->num_parts_};
        kway_mgr.optimize(part, hyprgraph);
        this->total_cost = kway_mgr.total_cost;
        return;
    }

    auto legalcheck_fn = [&]() {
        GainMgr gain_mgr(hyprgraph, this->num_parts_);
        ConstrMgr constr_mgr(hyprgraph, this->bal_tol_, this->num_parts_);
        PartMgr part_mgr(hyprgraph, gain_mgr, constr_mgr, this->num_parts_);
        return part_mgr.legalize(part);
    };

    auto optimize_fn = [&]() {
        GainMgr gain_mgr(hyprgraph, this->num_parts_);
        ConstrMgr constr_mgr(hyprgraph, this->bal_tol_, this->num_parts_);
        PartMgr part_mgr(hyprgraph, gain_mgr, constr_mgr, this->num_parts_);
        part_mgr.optimize(part);
        return part_mgr.total_cost;
    };

    auto lc = legalcheck_fn();
    if (lc != LegalCheck::AllSatisfied) {
        return;
    }

    if (hyprgraph.number_of_modules() >= this->limitsize_) {
        try {
            const auto hgr2 = create_contracted_subgraph(hyprgraph, py::set<node_t>{});
            if (hgr2->number_of_modules() * 3 / 2 < hyprgraph.number_of_modules()) {
                auto part2 = std::vector<std::uint8_t>(hgr2->number_of_modules(), 0);
                hgr2->projection_up(part, part2);
                this->optimize(part2, *hgr2);
                hgr2->projection_down(part2, part);
            }
        } catch (const std::bad_alloc& e) {
            std::cerr << "Out of Memory: " << e.what() << '\n';
        }
    }

    this->total_cost = optimize_fn();
}
