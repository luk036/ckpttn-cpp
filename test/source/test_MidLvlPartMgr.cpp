#include <ckpttn/MidLvlPartMgr.hpp>
#include <cstdint>
#include <netlistx/netlist.hpp>
#include <netlistx/readwrite.hpp>
#include <string_view>
#include <utility>
#include <vector>
#include <xnetwork/classes/graph.hpp>

#include "test_common.hpp"

TEST_CASE("Test MidLvlPartMgr default") {
    const auto hyprgraph = create_test_netlist();
    MidLvlPartMgr<SimpleNetlist> part_mgr{hyprgraph};
    const auto N = hyprgraph.number_of_modules();
    std::vector<std::uint8_t> part(N, 0);
    const auto half = N / 2;
    for (auto i = 0U; i < half; ++i) part[i] = 1;
    part_mgr.optimize(part);
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MidLvlPartMgr n5") {
    const auto M = 5U;
    const auto total = M + 1U;
    xnetwork::SimpleGraph g(total);
    for (auto i = 0U; i < M; ++i) {
        g.add_edge(i, M);
    }
    SimpleNetlist hl(std::move(g), M, 1);

    MidLvlPartMgr<SimpleNetlist> part_mgr{hl};
    std::vector<std::uint8_t> part(M, 0);
    for (auto i = 0U; i < M / 2; ++i) {
        part[i] = 1;
    }

    part_mgr.optimize(part);
    CHECK_GE(part_mgr.total_cost, 0);
}

TEST_CASE("Test MidLvlPartMgr optimize n8") {
    constexpr auto M = 8U;
    const auto total = M + 1U;
    xnetwork::SimpleGraph g(total);
    for (auto i = 0U; i < M; ++i) {
        g.add_edge(i, M);
    }
    SimpleNetlist hl(std::move(g), M, 1);

    MidLvlPartMgr<SimpleNetlist> part_mgr{hl};
    std::vector<std::uint8_t> part(M, 0);
    for (auto i = 0U; i < M / 2; ++i) {
        part[i] = 1;
    }

    part_mgr.optimize(part);
    CHECK_EQ(part_mgr.total_cost, 1);
}

TEST_CASE("Test MidLvlPartMgr optimize n11") {
    constexpr auto M = 11U;
    const auto total = M + 1U;
    xnetwork::SimpleGraph g(total);
    for (auto i = 0U; i < M; ++i) {
        g.add_edge(i, M);
    }
    SimpleNetlist hl(std::move(g), M, 1);

    MidLvlPartMgr<SimpleNetlist> part_mgr{hl};
    std::vector<std::uint8_t> part(M, 0);
    for (auto i = 0U; i < M / 2; ++i) {
        part[i] = 1;
    }

    part_mgr.optimize(part);
    CHECK_EQ(part_mgr.total_cost, 1);
}

TEST_CASE("Test MidLvlPartMgr p1 cost") {
    const auto hyprgraph = readNetD("../../testcases/p1.net");
    MidLvlPartMgr<SimpleNetlist> part_mgr{hyprgraph};
    const auto N = hyprgraph.number_of_modules();

    std::vector<std::uint8_t> part(N, 0);
    const auto half = N / 2;
    for (auto i = 0U; i < half; ++i) {
        part[i] = 1;
    }

    const auto cost = part_mgr.compute_total_cost(part);
    CHECK_GE(cost, 0);
    CHECK_GT(cost, 0);
}

