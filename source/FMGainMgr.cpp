#include <algorithm>  // for all_of, max_element
#include <ckpttn/FMGainMgr.hpp>
#include <ckpttn/FMPmrConfig.hpp>  // for FM_MAX_DEGREE
#include <iterator>                // for distance
#include <transrangers.hpp>
#include <type_traits>  // for is_base_of, integral_const...
#include <vector>       // for vector<>::iterator, vector

#include "ckpttn/bpqueue.hpp"   // for BPQueue
#include "ckpttn/dllist.hpp"    // for Dllink
#include "ckpttn/moveinfo.hpp"  // for MoveInfoV, MoveInfo

// using node_t = typename SimpleNetlist::node_t;
// using namespace ranges;
using namespace std;

/**
 * @brief Construct a new FMGainMgr object
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] hyprgraph
 * @param[in] num_parts
 */
template <typename Gnl, typename GainCalc, class Derived>
FMGainMgr<Gnl, GainCalc, Derived>::FMGainMgr(const Gnl &hyprgraph, uint8_t num_parts)
    : hyprgraph{hyprgraph}, num_parts{num_parts}, gain_calc{hyprgraph, num_parts} {
    static_assert(is_base_of<FMGainMgr<Gnl, GainCalc, Derived>, Derived>::value,
                  "base derived consistence");
    const auto pmax = int(hyprgraph.get_max_degree());
    for (auto _k = 0U; _k != this->num_parts; ++_k) {
        this->gain_bucket.emplace_back(BPQueue<typename Gnl::node_t>(-pmax, pmax));
    }
}

/**
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] part
 * @return int
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::init(std::span<const uint8_t> part) -> int {
    auto total_cost = this->gain_calc.init(part);
    this->waiting_list.clear();
    return total_cost;
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::is_empty() const -> bool {
    using namespace transrangers;
    auto rng = all(this->gain_bucket);
    return rng([](const auto &cursor) { return cursor->is_empty(); });
    // return std::all_of(this->gain_bucket.cbegin(), this->gain_bucket.cend(),
    //                    [](const auto &bckt) { return bckt.is_empty(); });
}

/**
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] part
 * @return pair<MoveInfoV<typename Gnl::node_t>, int>
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::select(std::span<const uint8_t> part)
    -> pair<MoveInfoV<typename Gnl::node_t>, int> {
    const auto it = max_element(
        this->gain_bucket.begin(), this->gain_bucket.end(),
        [](const auto &bckt1, const auto &bckt2) { return bckt1.get_max() < bckt2.get_max(); });

    const auto to_part = static_cast<uint8_t>(distance(this->gain_bucket.begin(), it));
    const auto gainmax = it->get_max();
    auto &vlink = it->popleft();
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
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] to_part
 * @return pair<typename Gnl::node_t, int>
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::select_togo(uint8_t to_part)
    -> pair<typename Gnl::node_t, int> {
    const auto gainmax = this->gain_bucket[to_part].get_max();
    auto &vlink = this->gain_bucket[to_part].popleft();
    this->waiting_list.append(vlink);
    const auto v = vlink.data.first;
    // const auto v =
    //     typename Gnl::node_t(distance(this->gain_calc.start_ptr(to_part),
    //     &vlink));
    return {v, gainmax};
}

/**
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] part
 * @param[in] move_info_v
 */
template <typename Gnl, typename GainCalc, class Derived>
void FMGainMgr<Gnl, GainCalc, Derived>::update_move(
    std::span<const uint8_t> part, const MoveInfoV<typename Gnl::node_t> &move_info_v) {
    this->gain_calc.update_move_init();
    const auto &v = move_info_v.v;
    for (const auto &net : this->hyprgraph.gr[move_info_v.v]) {
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
        } else {
            this->gain_calc.init_idx_vec(v, net);
            if (degree == 3) {
                this->_update_move_3pin_net(part, move_info);
            } else {
                this->_update_move_general_net(part, move_info);
            }
        }
    }
}

/**
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] part
 * @param[in] move_info
 */
template <typename Gnl, typename GainCalc, class Derived>
void FMGainMgr<Gnl, GainCalc, Derived>::_update_move_2pin_net(
    std::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t> &move_info) {
    // const auto [w, delta_gain_w] =
    //     this->gain_calc.update_move_2pin_net(part, move_info);
    const auto w = this->gain_calc.update_move_2pin_net(part, move_info);
    self.modify_key(w, part[w], this->gain_calc.delta_gain_w);
}

/**
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] part
 * @param[in] move_info
 */
template <typename Gnl, typename GainCalc, class Derived>
void FMGainMgr<Gnl, GainCalc, Derived>::_update_move_3pin_net(
    std::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t> &move_info) {
    // uint8_t stack_buf[8192];
    // FMPmr::monotonic_buffer_resource rsrc(stack_buf, sizeof stack_buf);
    // auto idx_vec = FMPmr::vector<typename Gnl::node_t>(&rsrc);

    auto delta_gain = this->gain_calc.update_move_3pin_net(part, move_info);

    // for (const auto& [dGw, w] : views::zip(delta_gain,
    // this->gain_calc.idx_vec))
    auto dGw_it = delta_gain.begin();
    for (const auto &w : this->gain_calc.idx_vec) {
        self.modify_key(w, part[w], *dGw_it);
        ++dGw_it;
    }

    // const auto degree = this->gain_calc.idx_vec.size();
    // for (size_t index = 0U; index != degree; ++index)
    // {
    //     const auto& w = this->gain_calc.idx_vec[index];
    //     self.modify_key(w, part[w], delta_gain[index]);
    // }
}

/**
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] part
 * @param[in] move_info
 */
template <typename Gnl, typename GainCalc, class Derived>
void FMGainMgr<Gnl, GainCalc, Derived>::_update_move_general_net(
    std::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t> &move_info) {
    const auto delta_gain = this->gain_calc.update_move_general_net(part, move_info);

    auto dGw_it = delta_gain.begin();
    for (const auto &w : this->gain_calc.idx_vec) {
        self.modify_key(w, part[w], *dGw_it);
        ++dGw_it;
    }
}

#include <ckpttn/FMBiGainCalc.hpp>     // for FMBiGainCalc
#include <ckpttn/FMBiGainMgr.hpp>      // for FMBiGainMgr
#include <xnetwork/classes/graph.hpp>  // for Graph

#include "ckpttn/netlist.hpp"  // for Netlist, Netlist<>::node_t

template class FMGainMgr<SimpleNetlist, FMBiGainCalc<SimpleNetlist>, FMBiGainMgr<SimpleNetlist>>;

#include <ckpttn/FMKWayGainCalc.hpp>  // for FMKWayGainCalc
#include <ckpttn/FMKWayGainMgr.hpp>   // for FMKWayGainMgr

template class FMGainMgr<SimpleNetlist, FMKWayGainCalc<SimpleNetlist>,
                         FMKWayGainMgr<SimpleNetlist>>;
