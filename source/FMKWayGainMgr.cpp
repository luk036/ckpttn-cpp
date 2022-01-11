#include <stdint.h>  // for uint8_t

#include <__config>                        // for std
#include <__hash_table>                    // for __hash_const_iterator, ope...
#include <boost/container/pmr/vector.hpp>  // for vector
#include <ckpttn/FMKWayGainCalc.hpp>       // for FMKWayGainCalc
#include <ckpttn/FMKWayGainMgr.hpp>        // for FMKWayGainMgr, FMKWayGainM...
#include <gsl/span>                        // for span
#include <py2cpp/range.hpp>                // for _iterator
#include <py2cpp/set.hpp>                  // for set
#include <vector>                          // for vector, __vector_base<>::v...

#include "ckpttn/FMGainMgr.hpp"  // for FMGainMgr::Item
#include "ckpttn/bpqueue.hpp"    // for bpqueue
#include "ckpttn/netlist.hpp"    // for MoveInfoV, Netlist, Simple...
#include "ckpttn/robin.hpp"      // for robin<>::iterable_wrapper

using namespace std;

/**
 * @brief
 *
 * @param[in] part
 * @return int
 */
auto FMKWayGainMgr::init(gsl::span<const uint8_t> part) -> int {
    auto totalcost = Base::init(part);

    for (auto& bckt : this->gainbucket) {
        bckt.clear();
    }
    for (const auto& v : this->H) {
        const auto pv = part[v];
        for (const auto& k : this->RR.exclude(pv)) {
            auto& vlink = this->gainCalc.vertex_list[k][v];
            this->gainbucket[k].append_direct(vlink);
        }
        auto& vlink = this->gainCalc.vertex_list[pv][v];
        this->gainbucket[pv].set_key(vlink, 0);
        this->waitinglist.append(vlink);
    }
    for (const auto& v : this->H.module_fixed) {
        this->lock_all(part[v], v);
    }
    return totalcost;
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info_v
 * @param[in] gain
 */
void FMKWayGainMgr::update_move_v(const MoveInfoV<node_t>& move_info_v, int gain) {
    // const auto& [v, fromPart, toPart] = move_info_v;

    for (auto k = 0U; k != this->K; ++k) {
        if (move_info_v.fromPart == k || move_info_v.toPart == k) {
            continue;
        }
        this->gainbucket[k].modify_key(this->gainCalc.vertex_list[k][move_info_v.v],
                                       this->gainCalc.deltaGainV[k]);
    }
    this->_set_key(move_info_v.fromPart, move_info_v.v, -gain);
    // this->_set_key(toPart, v, -2*this->pmax);
}
