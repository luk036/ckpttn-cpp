#include <cstdint>  // for uint8_t
// #include <__config>                 // for std
// #include <__hash_table>             // for __hash_const_iterator, operator!=
#include <ckpttn/FMBiGainCalc.hpp>  // for FMBiGainCalc, FMBiGainCalc<>::Item
#include <ckpttn/FMBiGainMgr.hpp>   // for FMBiGainMgr, part, FMBiGainMgr::Base
#include <mywheel/bpqueue.hpp>      // for BPQueue
#include <py2cpp/range.hpp>         // for _iterator
#include <py2cpp/set.hpp>           // for set
#include <span>                     // for span

using namespace std;

/**
 * @brief Initializes the binary gain manager with the given partition.
 *
 * Clears both gain buckets and populates them with initial gain values
 * from the gain calculator. Locks any fixed modules in their partitions.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] part The partition assignment to initialize from
 * @return The total cost of the initial partition
 */
template <typename Gnl> auto FMBiGainMgr<Gnl>::init(std::span<const uint8_t> part) -> int {
    auto total_cost = Base::init(part);
    for (auto& bckt : this->gain_bucket) {
        bckt.clear();
    }

    for (const auto& v : this->hyprgraph) {
        auto& vlink = this->gain_calc.vertex_list[v];
        // auto to_part = 1 - part[v];
        // this->gain_bucket[1 - part[v]].append_direct(vlink);
        this->gain_bucket[1 - part[v]].append(vlink, this->gain_calc.init_gain_list[v]);
    }
    for (const auto& v : this->hyprgraph.module_fixed) {
        this->lock_all(part[v], v);
    }
    return total_cost;
}

// instantiation

#include <netlistx/netlist.hpp>  // for Netlist, SimpleNetlist

template class FMBiGainMgr<SimpleNetlist>;
