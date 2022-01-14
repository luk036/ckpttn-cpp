#include <doctest/doctest.h>  // for TestCase, TEST_CASE
// #include <__config>                 // for std
// #include <__hash_table>             // for operator!=
#include <ckpttn/netlist.hpp>       // for Netlist, Netlist<>::nodeview_t
#include <ckpttn/netlist_algo.hpp>  // for min_maximal_matching, min_vertex_...
#include <py2cpp/dict.hpp>          // for dict
#include <py2cpp/range.hpp>         // for _iterator, iterable_wrapper
#include <py2cpp/set.hpp>           // for set
#include <string_view>              // for string_view

using namespace std;

extern auto create_test_netlist() -> SimpleNetlist;  // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist;         // import create_dwarf
extern auto readNetD(string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist& H, string_view areFileName);
// extern tuple<py::set<node_t>, int>
// min_net_cover_pd(SimpleNetlist &, const vector<int> &);

using node_t = SimpleNetlist::node_t;

TEST_CASE("Test min_vertex_cover dwarf") {
    const auto H = create_dwarf();
    auto weight = py::dict<node_t, int>{};
    auto covset = py::set<node_t>{};
    for (auto node : H) {
        weight[node] = 1;
        // covset[node] = false;
    }
    min_vertex_cover(H, weight, covset);
}

//
// Primal-dual algorithm for minimum vertex cover problem
//

TEST_CASE("Test min_maximal_matching dwarf") {
    const auto H = create_dwarf();
    // const auto N = H.number_of_nets();
    auto weight = py::dict<node_t, int>{};
    auto matchset = py::set<node_t>{};
    auto dep = py::set<node_t>{};
    for (auto net : H.nets) {
        // matchset[net] = false;
        weight[net] = 1;
    }
    // for (auto v : H)
    // {
    //     dep[v] = false;
    // }
    min_maximal_matching(H, weight, matchset, dep);
}
