// -*- coding: utf-8 -*-
#include <doctest/doctest.h> // for ResultBuilder, CHECK, TestCase
// #include <__hash_table>                // for __hash_const_iterator,
// operator!=
#include <py2cpp/range.hpp>           // for _iterator
#include <py2cpp/set.hpp>             // for set
#include <xnetwork/classes/graph.hpp> // for Graph, SimpleGraph

/**
 * @brief Create a test netlist object
 *
 * @return Netlist
 */
TEST_CASE("Test xnetwork") {
    constexpr auto num_nodes = 6;
    enum nodes { a1, a2, a3, n1, n2, n3 };
    // const auto R = py::range<std::uint8_t>(0, num_nodes);
    xnetwork::SimpleGraph gr{num_nodes};
    gr.add_edge(a1, n1);
    gr.add_edge(a1, n1);
    gr.add_edge(a1, n2);
    gr.add_edge(a2, n2);

    auto count = 0;
    for (auto _v : gr) {
        static_assert(sizeof _v >= 0, "make compiler happy");
        ++count;
    }

    CHECK(gr.number_of_nodes() == count);

    auto deg = 0;
    for (auto _v : gr[a1]) {
        static_assert(sizeof _v >= 0, "make compiler happy");
        ++deg;
    }

    CHECK(gr.degree(a1) == deg);
}
