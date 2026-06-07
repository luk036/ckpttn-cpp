#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMConstrMgr.hpp>
#include <ckpttn/MLMidLvlPartMgr.hpp>
#include <ckpttn/MidLvlPartMgr.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <netlistx/netlist.hpp>
#include <netlistx/readwrite.hpp>
#include <string_view>
#include <vector>
#include <xnetwork/classes/graph.hpp>
#include "test_common.hpp"

TEST_CASE("Test MLMidLvl dwarf") {
    const auto hyprgraph = create_dwarf();
    MLMidLvlPartMgr part_mgr{0.3};
    std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);

    auto lc = part_mgr.run_Partition<SimpleNetlist>(hyprgraph, part);
    CHECK_EQ(lc, LegalCheck::AllSatisfied);
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MLMidLvl p1") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    for (auto sz : {10U, 20U}) {
        auto begin = std::chrono::steady_clock::now();
        MLMidLvlPartMgr part_mgr{0.45};
        part_mgr.set_limitsize(sz);
        std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);

        auto lc = part_mgr.run_Partition<SimpleNetlist>(hyprgraph, part);
        auto sec = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - begin).count();
        CHECK_EQ(lc, LegalCheck::AllSatisfied);

        std::cout << "limitsize=" << sz << "  time=" << sec << "s  cost="
                  << part_mgr.total_cost << "\n";
    }
}

TEST_CASE("Test MLMidLvl p1") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    for (auto sz : {10U, 20U}) {
        auto begin = std::chrono::steady_clock::now();
        MLMidLvlPartMgr part_mgr{0.45};
        part_mgr.set_limitsize(sz);
        std::vector<uint8_t> part(hyprgraph.number_of_modules(), 0);

        auto lc = part_mgr.run_Partition<SimpleNetlist>(hyprgraph, part);
        auto sec = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - begin).count();
        CHECK_EQ(lc, LegalCheck::AllSatisfied);

        std::cout << "limitsize=" << sz << "  time=" << sec << "s  cost="
                  << part_mgr.total_cost << "\n";
    }
}

TEST_CASE("Test MLMidLvl n11 small") {
    constexpr auto M = 11U;
    const auto total = M + 1U;
    xnetwork::SimpleGraph g(total);
    for (auto i = 0U; i < M; ++i) { g.add_edge(i, M); }
    SimpleNetlist hl(std::move(g), M, 1);

    MLMidLvlPartMgr part_mgr{0.45};
    std::vector<uint8_t> part(M, 0);
    for (auto i = 0U; i < M / 2; ++i) { part[i] = 1; }

    auto lc = part_mgr.run_Partition<SimpleNetlist>(hl, part);
    CHECK_EQ(lc, LegalCheck::AllSatisfied);
    CHECK_EQ(part_mgr.total_cost, 1);
}
