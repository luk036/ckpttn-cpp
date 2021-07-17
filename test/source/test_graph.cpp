// -*- coding: utf-8 -*-
#include <cinttypes>
#include <doctest/doctest.h>
#include <py2cpp/py2cpp.hpp>
#include <xnetwork/classes/graph.hpp>

/**
 * @brief Create a test netlist object
 *
 * @return Netlist
 */
TEST_CASE("Test xnetwork")
{
    constexpr auto num_nodes = 6;
    enum nodes
    {
        a1,
        a2,
        a3,
        n1,
        n2,
        n3
    };
    // const auto R = py::range<std::uint8_t>(0, num_nodes);
    auto G = xn::SimpleGraph {num_nodes};
    G.add_edge(a1, n1);
    G.add_edge(a1, n1);
    G.add_edge(a1, n2);
    G.add_edge(a2, n2);

    auto count = 0;
    for ([[maybe_unused]] auto _ : G)
    {
        ++count;
    }

    CHECK(G.number_of_nodes() == count);

    auto deg = 0;
    for ([[maybe_unused]] auto _ : G[a1])
    {
        ++deg;
    }

    CHECK(G.degree(a1) == deg);
}
