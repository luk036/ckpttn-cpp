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
    CHECK_EQ(lc, LegalCheck::GetBetter);
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MLMidLvl p1") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    for (auto sz : {10U, 20U}) {
        auto begin = std::chrono::steady_clock::now();
    MLMidLvlPartMgr part_mgr{0.45};
    part_mgr.set_limitsize(200);
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

TEST_CASE("Test MLMidLvl ibm01") {
    auto hyprgraph = readNetD("../../testcases/ibm01.net");
    readAre(hyprgraph, "../../testcases/ibm01.are");

    auto begin = std::chrono::steady_clock::now();
    MLMidLvlPartMgr part_mgr{0.45};
    auto part = std::vector<uint8_t>(hyprgraph.number_of_modules(), 0);
    const auto half = hyprgraph.number_of_modules() / 2;
    for (auto i = 0U; i < half; ++i) {
        part[i] = 1;
    }

    auto lc = part_mgr.run_Partition<SimpleNetlist>(hyprgraph, part);
    auto sec = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - begin).count();
    CHECK_EQ(lc, LegalCheck::AllSatisfied);

    std::cout << "ibm01  time=" << sec << "s  cost="
              << part_mgr.total_cost << "\n";
    CHECK_GE(part_mgr.total_cost, 200);
}

TEST_CASE("Test MLMidLvl n8 even") {
    constexpr auto M = 8U;
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

TEST_CASE("Test MLMidLvl sphere_netlist") {
    const auto hyprgraph = read_yosys_json("../../yosys_testcases/sphere_netlist.json");
    const auto N = hyprgraph.number_of_modules();

    auto begin = std::chrono::steady_clock::now();
    MLMidLvlPartMgr part_mgr{0.3};
    std::vector<uint8_t> part(N, 0);
    for (auto i = 0U; i < N / 2; ++i) { part[i] = 1; }

    auto lc = part_mgr.run_Partition<SimpleNetlist>(hyprgraph, part);
    auto sec = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - begin).count();
    CHECK_EQ(lc, LegalCheck::AllSatisfied);
    std::cout << "sphere 2-way  time=" << sec << "s  cost="
              << part_mgr.total_cost << "  modules=" << N << "\n";
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MLMidLvl n12 even") {
    constexpr auto M = 12U;
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
