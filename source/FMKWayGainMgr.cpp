#include <cstdint>  // for uint8_t
// #include <__config>                        // for std
// #include <__hash_table>                    // for __hash_const_iterator,
// ope...
#include <ckpttn/FMKWayGainCalc.hpp>  // for FMKWayGainCalc
#include <ckpttn/FMKWayGainMgr.hpp>   // for FMKWayGainMgr, move_info_v
#include <ckpttn/FMPmrConfig.hpp>     // for pmr...
#include <span>                       // for span
#include <vector>                     // for vector, __vector_base<>::v...

#include "ckpttn/moveinfo.hpp"  // for MoveInfoV
#include <mywheel/bpqueue.hpp>   // for BPQueue
#include <mywheel/dllist.hpp>    // for Dllink
#include <mywheel/robin.hpp>     // for fun::Robin<>::iterable_wrapper

using namespace std;

/**
 * @brief
 *
 * @param[in] part
 * @return int
 */
template <typename Gnl> auto FMKWayGainMgr<Gnl>::init(std::span<const uint8_t> part) -> int {
    auto total_cost = Base::init(part);

    for (auto &bckt : this->gain_bucket) {
        bckt.clear();
    }
    for (const auto &v : this->hyprgraph) {
        const auto pv = part[v];
        for (const auto &k : this->rr.exclude(pv)) {
            auto &vlink = this->gain_calc.vertex_list[k][v];
            this->gain_bucket[k].append(vlink, this->gain_calc.init_gain_list[k][v]);
        }
        auto &vlink = this->gain_calc.vertex_list[pv][v];
        this->gain_bucket[pv].set_key(vlink, 0);
        this->waiting_list.append(vlink);
    }
    for (const auto &v : this->hyprgraph.module_fixed) {
        this->lock_all(part[v], v);
    }
    return total_cost;
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info_v
 * @param[in] gain
 */
template <typename Gnl>
void FMKWayGainMgr<Gnl>::update_move_v(const MoveInfoV<typename Gnl::node_t> &move_info_v,
                                       int gain) {
    // const auto& [v, from_part, to_part] = move_info_v;

    for (auto k = 0U; k != this->num_parts; ++k) {
        if (move_info_v.from_part == k || move_info_v.to_part == k) {
            continue;
        }
        this->gain_bucket[k].modify_key(this->gain_calc.vertex_list[k][move_info_v.v],
                                        this->gain_calc.delta_gain_v[k]);
    }
    this->_set_key(move_info_v.from_part, move_info_v.v, -gain);
    // this->_set_key(to_part, v, -2*this->pmax);
}

// instantiation

#include <py2cpp/range.hpp>  // for _iterator
#include <py2cpp/set.hpp>    // for set

#include "ckpttn/netlist.hpp"  // for Netlist, SimpleNetlist

template class FMKWayGainMgr<SimpleNetlist>;
