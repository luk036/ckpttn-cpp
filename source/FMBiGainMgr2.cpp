#include <stdint.h>  // for uint8_t

#include <__config>                 // for std
#include <__hash_table>             // for __hash_const_iterator, operator!=
#include <ckpttn/FMBiGainCalc.hpp>  // for FMBiGainCalc, FMBiGainCalc::Item
#include <ckpttn/FMBiGainMgr.hpp>   // for FMBiGainMgr, FMBiGainMgr::Base
#include <gsl/span>                 // for span
#include <py2cpp/range.hpp>         // for _iterator
#include <py2cpp/set.hpp>           // for set
#include <vector>                   // for vector

#include "ckpttn/bpqueue.hpp"  // for bpqueue
#include "ckpttn/netlist.hpp"  // for Netlist, SimpleNetlist

using namespace std;

/**
 * @brief
 *
 * @param[in] part
 */
auto FMBiGainMgr::init(gsl::span<const uint8_t> part) -> int {
    auto totalcost = Base::init(part);
    for (auto& bckt : this->gainbucket) {
        bckt.clear();
    }

    for (const auto& v : this->H) {
        auto& vlink = this->gainCalc.vertex_list[v];
        // auto toPart = 1 - part[v];
        this->gainbucket[1 - part[v]].append_direct(vlink);
    }
    for (const auto& v : this->H.module_fixed) {
        this->lock_all(part[v], v);
    }
    return totalcost;
}
