#include <doctest/doctest.h> // for ResultBuilder, TestCase, CHECK
// #include <__config>                    // for std
#include <boost/utility/string_view.hpp> // for boost::string_view
#include <ckpttn/FMKWayConstrMgr.hpp>    // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>      // for FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>          // for FMPartMgr
#include <cstdint>                       // for uint8_t
#include <vector>                        // for vector

#include "ckpttn/netlist.hpp" // for SimpleNetlist

using namespace std;

extern auto create_test_netlist()
    -> SimpleNetlist;                        // import create_test_netlist
extern auto create_dwarf() -> SimpleNetlist; // import create_dwarf
extern auto readNetD(boost::string_view netDFileName) -> SimpleNetlist;
extern void readAre(SimpleNetlist &hgr, boost::string_view areFileName);

/**
 * @brief Run test cases
 *
 * @param[in] hgr
 * @param[in] num_parts
 */
void run_FMKWayPartMgr(const SimpleNetlist &hgr, uint8_t num_parts) {
  FMKWayGainMgr<SimpleNetlist> gain_mgr{hgr, num_parts};
  FMKWayConstrMgr<SimpleNetlist> constr_mgr{hgr, 0.4, num_parts};
  FMPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>,
            FMKWayConstrMgr<SimpleNetlist>>
      part_mgr{hgr, gain_mgr, constr_mgr, num_parts};
  vector<uint8_t> part(hgr.number_of_modules(), 0);

  part_mgr.legalize(part);
  auto totalcostbefore = part_mgr.total_cost;
  part_mgr.optimize(part);
  CHECK(totalcostbefore >= 0);
  CHECK(part_mgr.total_cost <= totalcostbefore);
  CHECK(part_mgr.total_cost >= 0);
  totalcostbefore = part_mgr.total_cost;
  part_mgr.init(part);
  CHECK(part_mgr.total_cost == totalcostbefore);
}

TEST_CASE("Test FMKWayPartMgr") {
  const auto hgr = create_dwarf();
  run_FMKWayPartMgr(hgr, 3);
}

TEST_CASE("Test FMKWayPartMgr p1") {
  const auto hgr = readNetD("../../testcases/p1.net");
  run_FMKWayPartMgr(hgr, 3);
}

TEST_CASE("Test FMKWayPartMgr ibm01") {
  auto hgr = readNetD("../../testcases/ibm01.net");
  readAre(hgr, "../../testcases/ibm01.are");
  run_FMKWayPartMgr(hgr, 3);
}

// TEST_CASE("Test FMKWayPartMgr ibm18")
// {
//     auto hgr = readNetD("../../testcases/ibm18.net");
//     readAre(hgr, "../../testcases/ibm18.are");
//     run_FMKWayPartMgr(hgr, 3);
// }
