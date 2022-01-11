#include <boost/container/pmr/vector.hpp>  // for vector
#include <boost/container/vector.hpp>      // for operator!=, vec_iterator
#include <ckpttn/FMGainMgr.hpp>
#include <ckpttn/FMPmrConfig.hpp>      // for FM_MAX_DEGREE
#include <py2cpp/set.hpp>              // for set
#include <xnetwork/classes/graph.hpp>  // for Graph
// #include <range/v3/view/zip.hpp>
// #include <__config>      // for std
// #include <__hash_table>  // for __hash_const_iterator, ope...
#include <iterator>     // for distance
#include <type_traits>  // for is_base_of, integral_const...
#include <vector>       // for vector<>::iterator, vector

#include "ckpttn/bpqueue.hpp"  // for bpqueue
#include "ckpttn/dllist.hpp"   // for dllink
#include "ckpttn/netlist.hpp"  // for MoveInfoV, SimpleNetlist

using node_t = typename SimpleNetlist::node_t;
// using namespace ranges;
using namespace std;

/**
 * @brief Construct a new FMGainMgr object
 *
 * @param[in] H
 * @param[in] K
 */
template <typename GainCalc, class Derived>
FMGainMgr<GainCalc, Derived>::FMGainMgr(const SimpleNetlist& H, uint8_t K)
    : H{H}  // , pmax {H.get_max_degree()}
      ,
      K{K},
      gainCalc{H, K} {
    static_assert(is_base_of<FMGainMgr<GainCalc, Derived>, Derived>::value,
                  "base derived consistence");
    const auto pmax = int(H.get_max_degree());
    for (auto k = 0U; k != this->K; ++k) {
        this->gainbucket.emplace_back(bpqueue<node_t>(-pmax, pmax));
    }
}

/**
 * @brief
 *
 * @param[in] part
 */
template <typename GainCalc, class Derived>
auto FMGainMgr<GainCalc, Derived>::init(gsl::span<const uint8_t> part) -> int {
    auto totalcost = this->gainCalc.init(part);
    // this->totalcost = this->gainCalc.totalcost;
    this->waitinglist.clear();
    return totalcost;
}

/**
 * @brief
 *
 * @param[in] part
 * @return tuple<MoveInfoV<node_t>, int>
 */
template <typename GainCalc, class Derived>
auto FMGainMgr<GainCalc, Derived>::select(gsl::span<const uint8_t> part)
    -> tuple<MoveInfoV<node_t>, int> {
    const auto it = max_element(
        this->gainbucket.begin(), this->gainbucket.end(),
        [](const auto& bckt1, const auto& bckt2) { return bckt1.get_max() < bckt2.get_max(); });

    const auto toPart = uint8_t(distance(this->gainbucket.begin(), it));
    const auto gainmax = it->get_max();
    auto& vlink = it->popleft();
    this->waitinglist.append(vlink);
    // node_t v = &vlink - this->gainCalc.start_ptr(toPart);
    const auto v = vlink.data.first;
    // const auto v =
    //     node_t(distance(this->gainCalc.start_ptr(toPart), &vlink));
    // auto move_info_v = MoveInfoV<node_t> {v, part[v], toPart};
    return {{v, part[v], toPart}, gainmax};
}

/**
 * @brief
 *
 * @param[in] toPart
 * @return tuple<node_t, int>
 */
template <typename GainCalc, class Derived>
auto FMGainMgr<GainCalc, Derived>::select_togo(uint8_t toPart) -> tuple<node_t, int> {
    const auto gainmax = this->gainbucket[toPart].get_max();
    auto& vlink = this->gainbucket[toPart].popleft();
    this->waitinglist.append(vlink);
    const auto v = vlink.data.first;
    // const auto v =
    //     node_t(distance(this->gainCalc.start_ptr(toPart), &vlink));
    return {v, gainmax};
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info_v
 * @param[in] gain
 */
template <typename GainCalc, class Derived>
void FMGainMgr<GainCalc, Derived>::update_move(gsl::span<const uint8_t> part,
                                               const MoveInfoV<node_t>& move_info_v) {
    this->gainCalc.update_move_init();
    const auto& v = move_info_v.v;
    for (const node_t& net : this->H.G[move_info_v.v]) {
        const auto degree = this->H.G.degree(net);
        if (degree < 2 || degree > FM_MAX_DEGREE)  // [[unlikely]]
        {
            continue;  // does not provide any gain change when
                       // moving
        }
        const auto move_info = MoveInfo<node_t>{net, v, move_info_v.fromPart, move_info_v.toPart};
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
 * @param[in] part
 * @param[in] move_info
 */
template <typename GainCalc, class Derived>
void FMGainMgr<GainCalc, Derived>::_update_move_2pin_net(gsl::span<const uint8_t> part,
                                                         const MoveInfo<node_t>& move_info) {
    // const auto [w, deltaGainW] =
    //     this->gainCalc.update_move_2pin_net(part, move_info);
    const auto w = this->gainCalc.update_move_2pin_net(part, move_info);
    self.modify_key(w, part[w], this->gainCalc.deltaGainW);
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 */
template <typename GainCalc, class Derived>
void FMGainMgr<GainCalc, Derived>::_update_move_3pin_net(gsl::span<const uint8_t> part,
                                                         const MoveInfo<node_t>& move_info) {
    // byte StackBuf[8192];
    // FMPmr::monotonic_buffer_resource rsrc(StackBuf, sizeof StackBuf);
    // auto IdVec = FMPmr::vector<node_t>(&rsrc);

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
 * @param[in] part
 * @param[in] move_info
 */
template <typename GainCalc, class Derived>
void FMGainMgr<GainCalc, Derived>::_update_move_general_net(gsl::span<const uint8_t> part,
                                                            const MoveInfo<node_t>& move_info) {
    const auto deltaGain = this->gainCalc.update_move_general_net(part, move_info);
    // Why not auto&& ?
    // for (const auto& [dGw, w] : views::zip(deltaGain, this->gainCalc.IdVec))
    // {
    //     self.modify_key(w, part[w], dGw);
    // }

    auto dGw_it = deltaGain.begin();
    for (const auto& w : this->gainCalc.IdVec) {
        self.modify_key(w, part[w], *dGw_it);
        ++dGw_it;
    }
}

#include <ckpttn/FMBiGainMgr.hpp>  // for FMBiGainMgr

template class FMGainMgr<FMBiGainCalc, FMBiGainMgr>;

#include <ckpttn/FMKWayGainMgr.hpp>  // for FMKWayGainMgr

template class FMGainMgr<FMKWayGainCalc, FMKWayGainMgr>;
