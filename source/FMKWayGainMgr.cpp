#include <cstdint>  // for uint8_t
// #include <__config>                        // for std
// #include <__hash_table>                    // for __hash_const_iterator,
// ope...
#include <ckpttn/FMKWayGainCalc.hpp>  // for FMKWayGainCalc
#include <ckpttn/FMKWayGainMgr.hpp>   // for FMKWayGainMgr, move_info_v
#include <ckpttn/FMPmrConfig.hpp>     // for pmr...
#include <mywheel/bpqueue.hpp>        // for BPQueue
#include <mywheel/dllist.hpp>         // for Dllink
#include <mywheel/robin.hpp>          // for fun::Robin<>::iterable_wrapper
#include <span>                       // for span

#include "ckpttn/moveinfo.hpp"  // for MoveInfoV

using namespace std;

/**
 * @brief Initializes the k-way gain manager with the given partition.
 *
 * Clears all gain buckets, populates them with initial gain values for
 * each vertex and partition, and locks fixed modules.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] part The partition assignment to initialize from
 * @return The total cost of the initial partition
 */
template <typename Gnl> auto FMKWayGainMgr<Gnl>::init(std::span<const uint8_t> part) -> int {
    auto total_cost = Base::init(part);

    for (auto& bckt : this->gain_bucket) {
        bckt.clear();
    }
    for (const auto& v : this->hyprgraph) {
        const auto pv = part[v];
        for (const auto& k : this->rr.exclude(pv)) {
            auto& vlink = this->gain_calc.vertex_list[k][v];
            this->gain_bucket[k].append(vlink, this->gain_calc.init_gain_list[k][v]);
        }
        auto& vlink = this->gain_calc.vertex_list[pv][v];
        this->gain_bucket[pv].set_key(vlink, 0);
        this->waiting_list.append(vlink);
    }
    for (const auto& v : this->hyprgraph.module_fixed) {
        this->lock_all(part[v], v);
    }
    return total_cost;
}

/**
 * @brief Updates vertex gains after a move in k-way partitioning.
 *
 * Adjusts the gain keys for the moved vertex in all partitions except
 * the source and destination, and updates the key in the source partition.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] move_info_v Information about the performed vertex move
 * @param[in] gain The gain of the performed move
 */
template <typename Gnl>
void FMKWayGainMgr<Gnl>::update_move_v(const MoveInfoV<typename Gnl::node_t>& move_info_v,
                                       int gain) {
    // const auto& [v, from_part, to_part] = move_info_v;

    for (auto part_idx = 0U; part_idx != this->num_parts; ++part_idx) {
        if (move_info_v.from_part == part_idx || move_info_v.to_part == part_idx) {
            continue;
        }
        this->gain_bucket[part_idx].modify_key(this->gain_calc.vertex_list[part_idx][move_info_v.v],
                                               this->gain_calc.delta_gain_v[part_idx]);
    }
    this->_set_key(move_info_v.from_part, move_info_v.v, -gain);
    // this->_set_key(to_part, v, -2*this->pmax);
}

// instantiation

#include <netlistx/netlist.hpp>  // for Netlist, SimpleNetlist
#include <py2cpp/range.hpp>      // for _iterator
#include <py2cpp/set.hpp>        // for set

template class FMKWayGainMgr<SimpleNetlist>;
