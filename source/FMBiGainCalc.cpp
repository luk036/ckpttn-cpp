// #include <__config>                    // for std
// #include <__hash_table>                // for __hash_const_iterator, operator!=
#include <array>                           // for array
#include <boost/container/pmr/vector.hpp>  // for vector
#include <boost/container/vector.hpp>      // for operator!=, vec_iterator
#include <ckpttn/FMBiGainCalc.hpp>         // for FMBiGainCalc, part, net
#include <ckpttn/FMPmrConfig.hpp>          // for FM_MAX_DEGREE
#include <cstddef>                         // for size_t
#include <cstdint>                         // for uint8_t
#include <gsl/span>                        // for span
#include <initializer_list>                // for initializer_list
#include <vector>                          // for vector

#include "ckpttn/moveinfo.hpp"  // for MoveInfo
// #include <range/v3/view/remove_if.hpp>
// #include <transrangers.hpp>
// #include <range/v3/view/all.hpp>

using namespace std;

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::_init_gain(const typename Gnl::node_t& net, gsl::span<const uint8_t> part) {
    const auto degree = this->hgr.gr.degree(net);
    if (degree < 2 || degree > FM_MAX_DEGREE)  // [[unlikely]]
    {
        return;  // does not provide any gain when moving
    }
    if (!special_handle_2pin_nets) {
        this->_init_gain_general_net(net, part);
        return;
    }
    switch (degree) {
        case 2:
            this->_init_gain_2pin_net(net, part);
            break;
        case 3:
            this->_init_gain_3pin_net(net, part);
            break;
        default:
            this->_init_gain_general_net(net, part);
    }
}

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl> void FMBiGainCalc<Gnl>::_init_gain_2pin_net(const typename Gnl::node_t& net,
                                                                    gsl::span<const uint8_t> part) {
    auto net_cur = this->hgr.gr[net].begin();
    const auto w = *net_cur;
    const auto v = *++net_cur;

    const auto weight = this->hgr.get_net_weight(net);
    if (part[w] != part[v]) {
        this->totalcost += weight;
        // this->_modify_gain_va(weight, w, v);
        this->_modify_gain(w, weight);
        this->_modify_gain(v, weight);
    } else {
        // this->_modify_gain_va(-weight, w, v);
        this->_modify_gain(w, -weight);
        this->_modify_gain(v, -weight);
    }
}

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl> void FMBiGainCalc<Gnl>::_init_gain_3pin_net(const typename Gnl::node_t& net,
                                                                    gsl::span<const uint8_t> part) {
    auto net_cur = this->hgr.gr[net].begin();
    const auto w = *net_cur;
    const auto v = *++net_cur;
    const auto u = *++net_cur;

    const auto weight = this->hgr.get_net_weight(net);
    if (part[u] == part[v]) {
        if (part[w] == part[v]) {
            // this->_modify_gain_va(-weight, u, v, w);
            this->_modify_gain(u, -weight);
            this->_modify_gain(v, -weight);
            this->_modify_gain(w, -weight);
            return;
        }
        // this->_modify_gain_va(weight, w);
        this->_modify_gain(w, weight);
    } else if (part[w] == part[v]) {
        this->_modify_gain(u, weight);
    } else {
        this->_modify_gain(v, weight);
    }
    this->totalcost += weight;
}

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::_init_gain_general_net(const typename Gnl::node_t& net,
                                               gsl::span<const uint8_t> part) {
    auto num = array<size_t, 2>{0U, 0U};
    for (const auto& w : this->hgr.gr[net]) {
        num[part[w]] += 1;
    }
    const auto weight = this->hgr.get_net_weight(net);

    // #pragma unroll
    for (const auto& k : {0U, 1U}) {
        if (num[k] == 0) {
            for (const auto& w : this->hgr.gr[net]) {
                this->_modify_gain(w, -weight);
            }
        } else if (num[k] == 1) {
            for (const auto& w : this->hgr.gr[net]) {
                if (part[w] == k) {
                    this->_modify_gain(w, weight);
                    break;
                }
            }
        }
    }

    if (num[0] > 0 && num[1] > 0) {
        this->totalcost += weight;
    }
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @param[out] w
 * @return int
 */
template <typename Gnl>
auto FMBiGainCalc<Gnl>::update_move_2pin_net(gsl::span<const uint8_t> part,
                                             const MoveInfo<typename Gnl::node_t>& move_info) ->
    typename Gnl::node_t {
    auto net_cur = this->hgr.gr[move_info.net].begin();
    auto w = (*net_cur != move_info.v) ? *net_cur : *++net_cur;
    const auto weight = this->hgr.get_net_weight(move_info.net);
    const int delta = (part[w] == move_info.from_part) ? weight : -weight;
    this->delta_gain_w = 2 * delta;
    return w;
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::init_IdVec(const typename Gnl::node_t& v, const typename Gnl::node_t& net) {
    // auto rng = this->hgr.gr[net] |
    //         ranges::views::remove_if([&](auto w) { return w == v; });
    // using namespace transrangers;
    // auto rng = filter([&](const auto& w) { return w != v; }, all(this->hgr.gr[net]));
    // this->idx_vec = FMPmr::vector<typename Gnl::node_t>(rng.begin(), rng.end(), &this->rsrc);

    this->idx_vec.clear();
    auto&& rng = this->hgr.gr[net];
    this->idx_vec.reserve(rng.size() - 1);
    for (const auto& w : rng) {
        if (w == v) {
            continue;
        }
        this->idx_vec.push_back(w);
    }
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
template <typename Gnl>
auto FMBiGainCalc<Gnl>::update_move_3pin_net(gsl::span<const uint8_t> part,
                                             const MoveInfo<typename Gnl::node_t>& move_info)
    -> vector<int> {
    // const auto& [net, v, from_part, _] = move_info;
    auto num = array<size_t, 2>{0U, 0U};
    for (const auto& w : this->idx_vec) {
        num[part[w]] += 1;
    }
    // for (const auto& w : this->hgr.gr[move_info.net])
    // {
    //     if (w == move_info.v)
    //     {
    //         continue;
    //     }
    //     num[part[w]] += 1;
    //     idx_vec.push_back(w);
    // }
    auto delta_gain = vector<int>{0, 0};
    auto weight = this->hgr.get_net_weight(move_info.net);
    const auto part_w = part[this->idx_vec[0]];

    if (part_w != move_info.from_part) {
        weight = -weight;
    }
    if (part_w == part[this->idx_vec[1]]) {
        delta_gain[0] += weight;
        delta_gain[1] += weight;
    } else {
        delta_gain[0] += weight;
        delta_gain[1] -= weight;
    }
    return delta_gain;
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
template <typename Gnl>
auto FMBiGainCalc<Gnl>::update_move_general_net(gsl::span<const uint8_t> part,
                                                const MoveInfo<typename Gnl::node_t>& move_info)
    -> vector<int> {
    // const auto& [net, v, from_part, to_part] = move_info;
    auto num = array<uint8_t, 2>{0, 0};
    // auto idx_vec = vector<typename Gnl::node_t> {};
    for (const auto& w : this->idx_vec) {
        num[part[w]] += 1;
    }

    // for (const auto& w : this->hgr.gr[move_info.net])
    // {
    //     if (w == move_info.v)
    //     {
    //         continue;
    //     }
    //     num[part[w]] += 1;
    //     idx_vec.push_back(w);
    // }
    const auto degree = this->idx_vec.size();
    auto delta_gain = vector<int>(degree, 0);
    auto weight = this->hgr.get_net_weight(move_info.net);

    // #pragma unroll
    for (const auto& l_part : {move_info.from_part, move_info.to_part}) {
        if (num[l_part] == 0) {
            for (size_t index = 0U; index != degree; ++index) {
                delta_gain[index] -= weight;
            }
        } else if (num[l_part] == 1) {
            for (size_t index = 0U; index != degree; ++index) {
                auto part_w = part[this->idx_vec[index]];
                if (part_w == l_part) {
                    delta_gain[index] += weight;
                    break;
                }
            }
        }
        weight = -weight;
    }
    return delta_gain;
}

// instantiation

#include <py2cpp/set.hpp>              // for set
#include <xnetwork/classes/graph.hpp>  // for Graph

#include "ckpttn/netlist.hpp"  // for Netlist, SimpleNetlist

template class FMBiGainCalc<SimpleNetlist>;