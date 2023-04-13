// -*- coding: utf-8 -*-
#include <doctest/doctest.h> // for ResultBuilder, CHECK, TestCase

#include <ckpttn/netlist.hpp>         // for Netlist, SimpleNetlist, graph_t
#include <xnetwork/classes/graph.hpp> // for SimpleGraph, Graph
// #include <py2cpp/py2cpp.hpp>
// #include <__config>     // for std
#include <cstdint>     // for uint32_t
#include <type_traits> // for move
#include <utility>     // for pair
#include <vector>      // for vector

using namespace std;

// using graph_t =
//     boost::adjacency_list<boost::hash_setS, boost::vecS, boost::undirectedS>;
// using node_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
// using edge_t = typename boost::graph_traits<graph_t>::edge_iterator;

/**
 * @brief Create a test netlist object
 *
 * @return Netlist
 */
auto create_dwarf() -> SimpleNetlist {
  using Edge = pair<uint32_t, uint32_t>;
  const auto num_nodes = 13U;
  enum nodes { a0, a1, a2, a3, p1, p2, p3, n1, n2, n3, n4, n5, n6 };
  // static vector<nodes> module_name_list = {a1, a2, a3};
  // static vector<nodes> net__name_list = {n1, n2, n3};

  // char name[] = "ABCDE";
  vector<Edge> edge_array{
      Edge(p1, n1), Edge(a0, n1), Edge(a1, n1), Edge(a0, n2), Edge(a2, n2),
      Edge(a3, n2), Edge(a1, n3), Edge(a2, n3), Edge(a3, n3), Edge(a2, n4),
      Edge(p2, n4), Edge(a3, n5), Edge(p3, n5), Edge(a0, n6)};
  // index_t indices[] = {0, 1, 2, 3, 4, 5};
  // int num_arcs = sizeof(edge_array) / sizeof(Edge);
  // auto R = py::range(num_nodes);
  // graph_t g{R, R};
  xnetwork::SimpleGraph g(num_nodes);
  for (const auto &e : edge_array) {
    g.add_edge(e.first, e.second);
  }
  // using node_t = typename boost::graph_traits<graph_t>::vertex_descriptor;
  // using IndexMap =
  //     typename boost::property_map<graph_t, boost::vertex_index_t>::type;
  // IndexMap index = boost::get(boost::vertex_index, g);
  // auto gr = py::GraphAdaptor<graph_t>(std::move(g));

  // vector<node_t> module_list(7);
  // vector<node_t> net_list(5);
  vector<unsigned int> module_weight = {1, 3, 4, 2, 0, 0, 0};
  // auto hgr = Netlist{std::move(g), py::range(7), py::range(7, 13),
  // py::range(7),
  //                  py::range(-7, 6)};
  SimpleNetlist hgr(std::move(g), 7, 6);

  hgr.module_weight = module_weight;
  hgr.num_pads = 3;
  return hgr;
}

/**
 * @brief Create a test netlist object
 *
 * @return Netlist
 */
auto create_test_netlist() -> SimpleNetlist {
  using Edge = pair<uint32_t, uint32_t>;
  auto num_nodes = 6U;
  enum nodes { a1, a2, a3, n1, n2, n3 };

  // char name[] = "ABCDE";
  auto edge_array = vector<Edge>{Edge(a1, n1), Edge(a1, n2), Edge(a2, n1),
                                 Edge(a2, n2), Edge(a3, n2), Edge(a1, n3)};
  // index_t indices[] = {0, 1, 2, 3, 4, 5};
  // auto num_arcs = sizeof(edge_array) / sizeof(Edge);
  // auto g = graph_t{edge_array, edge_array + num_arcs, num_nodes};
  // auto gr = py::GraphAdaptor<graph_t>{std::move(g)};
  // const auto R = py::range(num_nodes);
  graph_t g(num_nodes);
  for (const auto &e : edge_array) {
    g.add_edge(e.first, e.second);
  }

  auto module_weight = vector<unsigned int>{3, 4, 2};
  auto hgr = SimpleNetlist{std::move(g), 3, 3};
  hgr.module_weight = std::move(module_weight);
  return hgr;
}

TEST_CASE("Test Netlist") {
  const auto hgr = create_test_netlist();

  CHECK(hgr.number_of_modules() == 3);
  CHECK(hgr.number_of_nets() == 3);
  // CHECK(hgr.number_of_pins() == 6);
  CHECK(hgr.get_max_degree() == 3);
  CHECK(hgr.get_max_net_degree() == 3);
  CHECK(!hgr.has_fixed_modules);
}

TEST_CASE("Test dwarf") {
  // static_assert(sizeof(double*) == 8);
  const auto hgr = create_dwarf();

  CHECK(hgr.number_of_modules() == 7);
  CHECK(hgr.number_of_nets() == 6);
  // CHECK(hgr.number_of_pins() == 14);
  CHECK(hgr.get_max_degree() == 3);
  CHECK(hgr.get_max_net_degree() == 3);
  CHECK(!hgr.has_fixed_modules);
  CHECK(hgr.get_module_weight(1) == 3U);
}
