#include <doctest/doctest.h>  // for TestCase, TEST_CASE
// #include <__config>                 // for std
// #include <__hash_table>             // for operator!=
#include <ckpttn/netlist.hpp>       // for Netlist, Netlist<>::nodeview_t
#include <ckpttn/netlist_algo.hpp>  // for min_maximal_matching, min_vertex_...
#include <py2cpp/dict.hpp>          // for dict
#include <py2cpp/range.hpp>         // for _iterator, iterable_wrapper
#include <py2cpp/set.hpp>           // for set
#include <string_view>              // for std::string_view

using namespace std;

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(std::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hyprgraph, std::string_view areFileName);
// extern pair<py::set<node_t>, int>
// min_net_cover_pd(SimpleNetlist &, const vector<int> &);

using node_t = SimpleNetlist::node_t;

TEST_CASE("Test min_vertex_cover dwarf") {
    const auto hyprgraph = create_dwarf();
    py::dict<node_t, int> weight{};
    py::set<node_t> covset{};
    for (auto node : hyprgraph) {
        weight[node] = 1;
        // covset[node] = false;
    }
    min_vertex_cover(hyprgraph, weight, covset);
}

//
// Primal-dual algorithm for minimum vertex cover problem
//

TEST_CASE("Test min_maximal_matching dwarf") {
    const auto hyprgraph = create_dwarf();
    // const auto N = hyprgraph.number_of_nets();
    py::dict<node_t, int> weight{};
    py::set<node_t> matchset{};
    py::set<node_t> dep{};
    for (auto net : hyprgraph.nets) {
        // matchset[net] = false;
        weight[net] = 1;
    }
    // for (auto v : hyprgraph)
    // {
    //     dep[v] = false;
    // }
    min_maximal_matching(hyprgraph, weight, matchset, dep);
}
