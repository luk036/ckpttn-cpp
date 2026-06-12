#include <algorithm>  // for all_of, max_element
#include <ckpttn/FMGainMgr.hpp>
#include <ckpttn/FMPmrConfig.hpp>  // for FM_MAX_DEGREE
#include <iterator>                // for distance
#include <mywheel/bpqueue.hpp>     // for BPQueue
#include <mywheel/dllist.hpp>      // for Dllink
#include <transrangers.hpp>
#include <type_traits>  // for is_base_of, integral_const...

#include "ckpttn/moveinfo.hpp"  // for MoveInfoV, MoveInfo

// using node_t = typename SimpleNetlist::node_t;
// using namespace ranges;
using namespace std;

/**
 * @brief Constructs a new FMGainMgr object.
 *
 * Initializes the gain buckets for each partition based on the maximum
 * degree of the hypergraph and the number of partitions.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @param[in] hyprgraph The hypergraph to manage gains for
 * @param[in] num_parts The number of partitions
 */
template <typename Gnl, typename GainCalc, class Derived>
FMGainMgr<Gnl, GainCalc, Derived>::FMGainMgr(const Gnl& hyprgraph, uint8_t num_parts)
    : hyprgraph{hyprgraph}, num_parts{num_parts}, gain_calc{hyprgraph, num_parts} {
    static_assert(is_base_of_v<FMGainMgr<Gnl, GainCalc, Derived>, Derived>,
                  "base derived consistence");
    const auto pmax = int(hyprgraph.get_max_degree());
    const auto range = static_cast<int>(this->num_parts - 1) * pmax;
    for (auto part_idx = 0U; part_idx != this->num_parts; ++part_idx) {
        this->gain_bucket.emplace_back(BPQueue<typename Gnl::node_t>(-range, range));
    }
}

/**
 * @brief Initializes the gain manager with the given partition.
 *
 * Delegates initialization to the gain calculator and clears the waiting list.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @param[in] part The partition assignment to initialize from
 * @return The total cost of the initial partition
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::init(std::span<const uint8_t> part) -> int {
    auto total_cost = this->gain_calc.init(part);
    this->waiting_list.clear();
    return total_cost;
}

/**
 * @brief Checks if all gain buckets are empty.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @return true if all gain buckets have no candidates
 * @return false if at least one bucket has candidates
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::is_empty() const -> bool {
    using namespace transrangers;
    auto rng = all(this->gain_bucket);
    return rng([](const auto& cursor) { return cursor->is_empty(); });
    // return std::all_of(this->gain_bucket.cbegin(), this->gain_bucket.cend(),
    //                    [](const auto &bckt) { return bckt.is_empty(); });
}

/**
 * @brief Selects the best move across all partitions.
 *
 * Finds the partition with the maximum gain value and returns the
 * corresponding vertex and gain.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @param[in] part The current partition assignment
 * @return A pair containing the move info and the gain of the selected move
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::select(std::span<const uint8_t> part)
    -> pair<MoveInfoV<typename Gnl::node_t>, int> {
    const auto it = max_element(
        this->gain_bucket.begin(), this->gain_bucket.end(),
        [](const auto& bckt1, const auto& bckt2) { return bckt1.get_max() < bckt2.get_max(); });

    const auto to_part = static_cast<uint8_t>(distance(this->gain_bucket.begin(), it));
    const auto gainmax = it->get_max();
    auto& vlink = it->popleft();
    this->waiting_list.append(vlink);
    // typename Gnl::node_t v = &vlink - this->gain_calc.start_ptr(to_part);
    const auto v = vlink.data.first;
    // const auto v =
    //     typename Gnl::node_t(distance(this->gain_calc.start_ptr(to_part),
    //     &vlink));
    // auto move_info_v = MoveInfoV<typename Gnl::node_t> {v, part[v], to_part};
    return {{v, part[v], to_part}, gainmax};
}

/**
 * @brief Selects the best move to a specific partition.
 *
 * Pops the vertex with the highest gain from the specified partition's
 * bucket and appends it to the waiting list.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @param[in] to_part The target partition index
 * @return A pair containing the selected vertex and its gain
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::select_togo(uint8_t to_part)
    -> pair<typename Gnl::node_t, int> {
    const auto gainmax = this->gain_bucket[to_part].get_max();
    auto& vlink = this->gain_bucket[to_part].popleft();
    this->waiting_list.append(vlink);
    const auto v = vlink.data.first;
    // const auto v =
    //     typename Gnl::node_t(distance(this->gain_calc.start_ptr(to_part),
    //     &vlink));
    return {v, gainmax};
}

/**
 * @brief Updates gain values for all nets affected by a vertex move.
 *
 * Iterates over all nets connected to the moved vertex and dispatches
 * to specialized handlers based on net degree (2-pin, 3-pin, or general).
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @param[in] part The current partition assignment
 * @param[in] move_info_v Information about the performed vertex move
 */
template <typename Gnl, typename GainCalc, class Derived>
void FMGainMgr<Gnl, GainCalc, Derived>::update_move(
    std::span<const uint8_t> part, const MoveInfoV<typename Gnl::node_t>& move_info_v) {
    this->gain_calc.update_move_init();
    const auto& v = move_info_v.v;
    for (const auto& net : this->hyprgraph.gr[move_info_v.v]) {
        const auto degree = this->hyprgraph.gr.degree(net);
        if (degree < 2 || degree > FM_MAX_DEGREE)  // [[unlikely]]
        {
            continue;  // does not provide any gain change when
                       // moving
        }
        const auto move_info
            = MoveInfo<typename Gnl::node_t>{net, v, move_info_v.from_part, move_info_v.to_part};
        if (!this->gain_calc.special_handle_2pin_nets) {
            this->gain_calc.init_idx_vec(v, net);
            this->_update_move_general_net(part, move_info);
            continue;
        }
        if (degree == 2) {
            this->_update_move_2pin_net(part, move_info);
            continue;
        }
        this->gain_calc.init_idx_vec(v, net);
        if (degree == 3) {
            this->_update_move_3pin_net(part, move_info);
        } else {
            this->_update_move_general_net(part, move_info);
        }
    }
}

/**
 * @brief Updates gain values for a 2-pin net after a vertex move.
 *
 * Computes the delta gain for the other vertex and applies the key change.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @param[in] part The current partition assignment
 * @param[in] move_info Information about the performed move
 */
template <typename Gnl, typename GainCalc, class Derived>
void FMGainMgr<Gnl, GainCalc, Derived>::_update_move_2pin_net(
    std::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t>& move_info) {
    // const auto [w, delta_gain_w] =
    //     this->gain_calc.update_move_2pin_net(part, move_info);
    const auto w = this->gain_calc.update_move_2pin_net(part, move_info);
    self.modify_key(w, part[w], this->gain_calc.delta_gain_w);
}

/**
 * @brief Updates gain values for a 3-pin net after a vertex move.
 *
 * Computes delta gains for the two remaining vertices and applies
 * the corresponding key changes.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @param[in] part The current partition assignment
 * @param[in] move_info Information about the performed move
 */
template <typename Gnl, typename GainCalc, class Derived>
void FMGainMgr<Gnl, GainCalc, Derived>::_update_move_3pin_net(
    std::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t>& move_info) {
    // uint8_t stack_buf[8192];
    // FMPmr::monotonic_buffer_resource rsrc(stack_buf, sizeof stack_buf);
    // auto idx_vec = FMPmr::vector<typename Gnl::node_t>(&rsrc);

    const auto delta_gain = this->gain_calc.update_move_3pin_net(part, move_info);

    auto dGw_it = delta_gain.begin();
    for (const auto& w : this->gain_calc.idx_vec) {
        if constexpr (std::is_same_v<std::decay_t<decltype(*dGw_it)>, int>) {
            if (*dGw_it != 0) {
                self.modify_key(w, part[w], *dGw_it);
            }
        } else {
            self.modify_key(w, part[w], *dGw_it);
        }
        ++dGw_it;
    }
}

/**
 * @brief Updates gain values for a general net (degree > 3) after a vertex move.
 *
 * Computes delta gains for all remaining vertices in the net and applies
 * the corresponding key changes.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainCalc The gain calculator type
 * @tparam Derived The derived gain manager type (CRTP)
 * @param[in] part The current partition assignment
 * @param[in] move_info Information about the performed move
 */
template <typename Gnl, typename GainCalc, class Derived>
void FMGainMgr<Gnl, GainCalc, Derived>::_update_move_general_net(
    std::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t>& move_info) {
    const auto delta_gain = this->gain_calc.update_move_general_net(part, move_info);

    auto dGw_it = delta_gain.begin();
    for (const auto& w : this->gain_calc.idx_vec) {
        if constexpr (std::is_same_v<std::decay_t<decltype(*dGw_it)>, int>) {
            if (*dGw_it != 0) {
                self.modify_key(w, part[w], *dGw_it);
            }
        } else {
            self.modify_key(w, part[w], *dGw_it);
        }
        ++dGw_it;
    }
}

#include <ckpttn/FMBiGainCalc.hpp>     // for FMBiGainCalc
#include <ckpttn/FMBiGainMgr.hpp>      // for FMBiGainMgr
#include <netlistx/netlist.hpp>        // for Netlist, Netlist<>::node_t
#include <xnetwork/classes/graph.hpp>  // for Graph

template class FMGainMgr<SimpleNetlist, FMBiGainCalc<SimpleNetlist>, FMBiGainMgr<SimpleNetlist>>;

#include <ckpttn/FMKWayGainCalc.hpp>  // for FMKWayGainCalc
#include <ckpttn/FMKWayGainMgr.hpp>   // for FMKWayGainMgr

template class FMGainMgr<SimpleNetlist, FMKWayGainCalc<SimpleNetlist>,
                         FMKWayGainMgr<SimpleNetlist>>;
