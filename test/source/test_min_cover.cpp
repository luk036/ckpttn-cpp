#include <doctest/doctest.h> // for ResultBuilder, CHECK, TestCase
// #include <__config>                // for std
#include <boost/utility/string_view.hpp> // for boost::string_view
#include <ckpttn/HierNetlist.hpp>        // for HierNetlist, SimpleHierNetlist
#include <ckpttn/netlist.hpp>            // for Netlist, SimpleNetlist
#include <cstdint>                       // for uint8_t
#include <memory>                        // for unique_ptr
#include <py2cpp/set.hpp>                // for set
#include <vector>                        // for vector, operator==

using namespace std;

extern auto create_test_netlist()
    -> SimpleNetlist;                        // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist; // import create_dwarf
extern auto readNetD(boost::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hgr, boost::string_view areFileName);
// extern tuple<py::set<node_t>, int>
// min_net_cover_pd(SimpleNetlist &, const vector<int> &);

using node_t = SimpleNetlist::node_t;
extern auto create_contraction_subgraph(const SimpleNetlist &,
                                        const py::set<node_t> &)
    -> unique_ptr<SimpleHierNetlist>;

//
// Primal-dual algorithm for minimum vertex cover problem
//

// TEST_CASE("Test min_net_cover_pd dwarf", "[test_min_cover]") {
//     auto hgr = create_dwarf();
//     auto [S, cost] = min_net_cover_pd(hgr, hgr.module_weight);
//     CHECK(cost == 3);
// }

// TEST_CASE("Test min_net_cover_pd ibm01", "[test_min_cover]") {
//     auto hgr = readNetD("../../testcases/ibm01.net");
//     readAre(hgr, "../../testcases/ibm01.are");
//     auto [S, cost] = min_net_cover_pd(hgr, hgr.net_weight);
//     CHECK(cost == 4053);
// }

TEST_CASE("Test contraction subgraph dwarf") {
    const auto hgr = create_dwarf();
    const auto hgr2 = create_contraction_subgraph(hgr, py::set<node_t>{});
    // auto H3 = create_contraction_subgraph(*hgr2, py::set<node_t> {});
    CHECK(hgr2->number_of_modules() < 7);
    CHECK(hgr2->number_of_nets() == 3);
    // CHECK(hgr2->number_of_pins() < 14);
    CHECK(hgr2->get_max_net_degree() <= 3);

    auto part = vector<uint8_t>(hgr.number_of_modules(), 0);
    auto part2 = vector<uint8_t>(hgr2->number_of_modules(), 0);
    auto part3 = vector<uint8_t>(hgr2->number_of_modules(), 0);
    part2[0] = part2[2] = 1;
    part2[1] = 2;
    hgr2->projection_down(part2, part);
    hgr2->projection_up(part, part3);
    CHECK(part2 == part3);
}

TEST_CASE("Test contraction subgraph ibm01") {
    auto hgr = readNetD("../../testcases/ibm01.net");
    readAre(hgr, "../../testcases/ibm01.are");
    auto hgr2 = create_contraction_subgraph(hgr, py::set<node_t>{});
    auto H3 = create_contraction_subgraph(*hgr2, py::set<node_t>{});
    CHECK(hgr2->number_of_modules() < hgr.number_of_modules());
    CHECK(hgr2->number_of_nets() < hgr.number_of_nets());
    // CHECK(hgr2->number_of_pins() < hgr.number_of_pins());
    CHECK(hgr2->get_max_net_degree() <= hgr.get_max_net_degree());

    auto part2 = vector<uint8_t>(hgr2->number_of_modules(), 0);
    auto part3 = vector<uint8_t>(H3->number_of_modules(), 0);
    auto part4 = vector<uint8_t>(H3->number_of_modules(), 0);
    auto i = uint8_t(0);

    for (auto &item : part3) {
        item = ++i % 6;
    }
    H3->projection_down(part3, part2);
    H3->projection_up(part2, part4);
    CHECK(part3 == part4);
}

TEST_CASE("Test contraction subgraph ibm18") {
    auto hgr = readNetD("../../testcases/ibm18.net");
    readAre(hgr, "../../testcases/ibm18.are");
    auto hgr2 = create_contraction_subgraph(hgr, py::set<node_t>{});
    auto H3 = create_contraction_subgraph(*hgr2, py::set<node_t>{});
    CHECK(hgr2->number_of_modules() < hgr.number_of_modules());
    CHECK(hgr2->number_of_nets() < hgr.number_of_nets());
    // CHECK(hgr2->number_of_pins() < hgr.number_of_pins());
    CHECK(hgr2->get_max_net_degree() <= hgr.get_max_net_degree());

    auto part2 = vector<uint8_t>(hgr2->number_of_modules(), 0);
    auto part3 = vector<uint8_t>(H3->number_of_modules(), 0);
    auto part4 = vector<uint8_t>(H3->number_of_modules(), 0);
    for (auto i = 0u; i != H3->number_of_modules(); ++i) {
        part3[i] = uint8_t(i) % 6;
    }
    H3->projection_down(part3, part2);
    H3->projection_up(part2, part4);
    CHECK(part3 == part4);
}
