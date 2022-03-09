// #include <__config>                        // for std
// #include <__hash_table>                    // for __hash_const_iterator, ope...
// #include <boost/container/pmr/vector.hpp>  // for vector
// #include <boost/container/vector.hpp>      // for operator!=, vec_iterator
#include <ckpttn/FMGainMgr.hpp>
#include <ckpttn/FMPmrConfig.hpp>  // for FM_MAX_DEGREE
#include <gsl/gsl_util>            // for narrow_cast
#include <iterator>                // for distance
#include <py2cpp/set.hpp>          // for set
#include <type_traits>             // for is_base_of, integral_const...
#include <vector>                  // for vector<>::iterator, vector

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
 * @param[in] hgr
 * @param[in] num_parts
 */
template <typename Gnl, typename GainCalc, class Derived>
FMGainMgr<Gnl, GainCalc, Derived>::FMGainMgr(const Gnl& hgr, uint8_t num_parts)
    : hgr{hgr}, num_parts{num_parts}, gainCalc{hgr, num_parts} {
    static_assert(is_base_of<FMGainMgr<Gnl, GainCalc, Derived>, Derived>::value,
                  "base derived consistence");
    const auto pmax = int(hgr.get_max_degree());
    for (auto k = 0U; k != this->num_parts; ++k) {
        this->gainbucket.emplace_back(BPQueue<typename Gnl::node_t>(-pmax, pmax));
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
auto FMGainMgr<Gnl, GainCalc, Derived>::init(gsl::span<const uint8_t> part) -> int {
    auto totalcost = this->gainCalc.init(part);
    // this->totalcost = this->gainCalc.totalcost;
    this->waitinglist.clear();
    return totalcost;
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
auto FMGainMgr<Gnl, GainCalc, Derived>::select(gsl::span<const uint8_t> part)
    -> pair<MoveInfoV<typename Gnl::node_t>, int> {
    const auto it = max_element(
        this->gainbucket.begin(), this->gainbucket.end(),
        [](const auto& bckt1, const auto& bckt2) { return bckt1.get_max() < bckt2.get_max(); });

    const auto toPart = gsl::narrow_cast<uint8_t>(distance(this->gainbucket.begin(), it));
    const auto gainmax = it->get_max();
    auto& vlink = it->popleft();
    this->waitinglist.append(vlink);
    // typename Gnl::node_t v = &vlink - this->gainCalc.start_ptr(toPart);
    const auto v = vlink.data.first;
    // const auto v =
    //     typename Gnl::node_t(distance(this->gainCalc.start_ptr(toPart), &vlink));
    // auto move_info_v = MoveInfoV<typename Gnl::node_t> {v, part[v], toPart};
    return {{v, part[v], toPart}, gainmax};
}

/**
 * @brief
 *
 * @tparam GainCalc
 * @tparam Derived
 * @param[in] toPart
 * @return pair<typename Gnl::node_t, int>
 */
template <typename Gnl, typename GainCalc, class Derived>
auto FMGainMgr<Gnl, GainCalc, Derived>::select_togo(uint8_t toPart)
    -> pair<typename Gnl::node_t, int> {
    const auto gainmax = this->gainbucket[toPart].get_max();
    auto& vlink = this->gainbucket[toPart].popleft();
    this->waitinglist.append(vlink);
    const auto v = vlink.data.first;
    // const auto v =
    //     typename Gnl::node_t(distance(this->gainCalc.start_ptr(toPart), &vlink));
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
    gsl::span<const uint8_t> part, const MoveInfoV<typename Gnl::node_t>& move_info_v) {
    this->gainCalc.update_move_init();
    const auto& v = move_info_v.v;
    for (const auto& net : this->hgr.gr[move_info_v.v]) {
        const auto degree = this->hgr.gr.degree(net);
        if (degree < 2 || degree > FM_MAX_DEGREE)  // [[unlikely]]
        {
            continue;  // does not provide any gain change when
                       // moving
        }
        const auto move_info
            = MoveInfo<typename Gnl::node_t>{net, v, move_info_v.fromPart, move_info_v.toPart};
        if (!this->gainCalc.special_handle_2pin_nets) {
            this->gainCalc.init_IdVec(v, net);
            this->_update_move_general_net(part, move_info);
            continue;
        }
        if (degree == 2) {
            this->_update_move_2pin_net(part, move_info);
        } else {
            this->gainCalc.init_IdVec(v, net);
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
    gsl::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t>& move_info) {
    // const auto [w, deltaGainW] =
    //     this->gainCalc.update_move_2pin_net(part, move_info);
    const auto w = this->gainCalc.update_move_2pin_net(part, move_info);
    self.modify_key(w, part[w], this->gainCalc.deltaGainW);
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
    gsl::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t>& move_info) {
    // uint8_t StackBuf[8192];
    // FMPmr::monotonic_buffer_resource rsrc(StackBuf, sizeof StackBuf);
    // auto IdVec = FMPmr::vector<typename Gnl::node_t>(&rsrc);

    auto deltaGain = this->gainCalc.update_move_3pin_net(part, move_info);

    // for (const auto& [dGw, w] : views::zip(deltaGain, this->gainCalc.IdVec))
    auto dGw_it = deltaGain.begin();
    for (const auto& w : this->gainCalc.IdVec) {
        self.modify_key(w, part[w], *dGw_it);
        ++dGw_it;
    }

    // const auto degree = this->gainCalc.IdVec.size();
    // for (size_t index = 0U; index != degree; ++index)
    // {
    //     const auto& w = this->gainCalc.IdVec[index];
    //     self.modify_key(w, part[w], deltaGain[index]);
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
    gsl::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t>& move_info) {
    const auto deltaGain = this->gainCalc.update_move_general_net(part, move_info);

    auto dGw_it = deltaGain.begin();
    for (const auto& w : this->gainCalc.IdVec) {
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
