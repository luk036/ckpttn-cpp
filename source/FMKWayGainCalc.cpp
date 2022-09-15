#include <cassert>  // for assert
#include <cstdint>  // for uint8_t

// #include <__config>                                           // for std
// #include <__hash_table>                                       // for __hash_...
#include <algorithm>                  // for fill
#include <ckpttn/FMKWayGainCalc.hpp>  // for FMKWayG...
#include <ckpttn/FMPmrConfig.hpp>     // for FM_MAX_...
#include <cstddef>                    // for byte
#include <gsl/span>                   // for span
#include <initializer_list>           // for initial...
#include <type_traits>                // for swap
#include <utility>                    // for pair
#include <vector>                     // for vector

#include "ckpttn/dllist.hpp"    // for Dllink
#include "ckpttn/moveinfo.hpp"  // for MoveInfo
#include "ckpttn/robin.hpp"     // for Robin<>...

// using namespace ranges;
using namespace std;

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 * @param[in] vertex_list
 */
template <typename Gnl> void FMKWayGainCalc<Gnl>::_init_gain(const typename Gnl::node_t& net,
                                                             gsl::span<const uint8_t> part) {
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
template <typename Gnl>
void FMKWayGainCalc<Gnl>::_init_gain_2pin_net(const typename Gnl::node_t& net,
                                              gsl::span<const uint8_t> part) {
    auto net_cur = this->hgr.gr[net].begin();
    const auto w = *net_cur;
    const auto v = *++net_cur;
    const auto part_w = part[w];
    const auto part_v = part[v];
    const auto weight = this->hgr.get_net_weight(net);
    if (part_v == part_w) {
        this->_modify_gain(w, part_v, -weight);
        this->_modify_gain(v, part_v, -weight);
        // this->_modify_gain_va(-weight, part_v, w, v);
    } else {
        this->totalcost += weight;
        this->vertex_list[part_v][w].data.second += weight;
        this->vertex_list[part_w][v].data.second += weight;
    }
}

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMKWayGainCalc<Gnl>::_init_gain_3pin_net(const typename Gnl::node_t& net,
                                              gsl::span<const uint8_t> part) {
    auto net_cur = this->hgr.gr[net].begin();
    const auto w = *net_cur;
    const auto v = *++net_cur;
    const auto u = *++net_cur;
    const auto part_w = part[w];
    const auto part_v = part[v];
    const auto part_u = part[u];
    const auto weight = this->hgr.get_net_weight(net);
    auto a = w;
    auto b = v;
    auto c = u;

    if (part_u == part_v) {
        if (part_w == part_v) {
            this->_modify_gain(u, part_v, -weight);
            this->_modify_gain(v, part_v, -weight);
            this->_modify_gain(w, part_v, -weight);
            // this->_modify_gain_va(-weight, part_v, u, v, w);
            return;
        }
    } else if (part_w == part_v) {
        a = u, b = w, c = v;
    } else if (part_w == part_u) {
        a = v, b = u, c = w;
    } else {
        this->totalcost += 2 * weight;
        // this->_modify_vertex_va(weight, part_v, u, w);
        // this->_modify_vertex_va(weight, part_w, u, v);
        // this->_modify_vertex_va(weight, part_u, v, w);
        this->_modify_gain(u, part_v, weight);
        this->_modify_gain(w, part_v, weight);
        this->_modify_gain(u, part_w, weight);
        this->_modify_gain(v, part_w, weight);
        this->_modify_gain(v, part_u, weight);
        this->_modify_gain(w, part_u, weight);
        return;
    }

    for (const auto& e : {b, c}) {
        this->_modify_gain(e, part[b], -weight);
        this->vertex_list[part[a]][e].data.second += weight;
    }
    this->vertex_list[part[b]][a].data.second += weight;

    // this->_modify_gain_va(-weight, part[b], b, c);
    // this->_modify_vertex_va(weight, part[a], b, c);
    // this->_modify_vertex_va(weight, part[b], a);

    this->totalcost += weight;
}

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMKWayGainCalc<Gnl>::_init_gain_general_net(const typename Gnl::node_t& net,
                                                 gsl::span<const uint8_t> part) {
    uint8_t StackBufLocal[2048];
    FMPmr::monotonic_buffer_resource rsrcLocal(StackBufLocal, sizeof StackBufLocal);
    auto num = FMPmr::vector<uint8_t>(this->num_parts, 0, &rsrcLocal);
    // auto idx_vec = FMPmr::vector<typename Gnl::node_t>(&rsrc);

    for (const auto& w : this->hgr.gr[net]) {
        num[part[w]] += 1;
        // idx_vec.push_back(w);
    }
    const auto weight = this->hgr.get_net_weight(net);
    for (const auto& c : num) {
        if (c > 0) {
            this->totalcost += weight;
        }
    }
    this->totalcost -= weight;

    // for (const auto& [k, c] : views::enumerate(num))
    auto k = 0U;
    for (const auto& c : num) {
        if (c == 0) {
            for (const auto& w : this->hgr.gr[net]) {
                vertex_list[k][w].data.second -= weight;
            }
        } else if (c == 1) {
            for (const auto& w : this->hgr.gr[net]) {
                if (part[w] == k) {
                    this->_modify_gain(w, part[w], weight);
                    break;
                }
            }
        }
        ++k;
    }
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @param[out] w
 * @return ret_2pin_info
 */
template <typename Gnl>
auto FMKWayGainCalc<Gnl>::update_move_2pin_net(gsl::span<const uint8_t> part,
                                               const MoveInfo<typename Gnl::node_t>& move_info) ->
    typename Gnl::node_t {
    // const auto& [net, v, from_part, to_part] = move_info;
    assert(part[move_info.v] == move_info.from_part);

    auto weight = this->hgr.get_net_weight(move_info.net);
    // auto delta_gain_w = vector<int>(this->num_parts, 0);
    auto net_cur = this->hgr.gr[move_info.net].begin();
    auto w = (*net_cur != move_info.v) ? *net_cur : *++net_cur;
    fill(this->delta_gain_w.begin(), this->delta_gain_w.end(), 0);

    // #pragma unroll
    for (const auto& l_part : {move_info.from_part, move_info.to_part}) {
        if (part[w] == l_part) {
            // for (auto i = 0U; i != delta_gain_w.size(); ++i)
            // {
            //     delta_gain_w[i] += weight;
            //     delta_gain_v[i] += weight;
            // }
            for (auto& dgw : delta_gain_w) {
                dgw += weight;
            }
            for (auto& dgv : delta_gain_v) {
                dgv += weight;
            }
        }
        delta_gain_w[l_part] -= weight;
        weight = -weight;
    }
    return w;
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
template <typename Gnl> void FMKWayGainCalc<Gnl>::init_idx_vec(const typename Gnl::node_t& v,
                                                               const typename Gnl::node_t& net) {
    this->idx_vec.clear();
    for (const auto& w : this->hgr.gr[net]) {
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
auto FMKWayGainCalc<Gnl>::update_move_3pin_net(gsl::span<const uint8_t> part,
                                               const MoveInfo<typename Gnl::node_t>& move_info)
    -> FMKWayGainCalc<Gnl>::ret_info {
    const auto degree = this->idx_vec.size();
    auto delta_gain = vector<vector<int>>(degree, vector<int>(this->num_parts, 0));
    auto weight = this->hgr.get_net_weight(move_info.net);
    const auto part_w = part[this->idx_vec[0]];
    const auto part_u = part[this->idx_vec[1]];
    auto l = move_info.from_part;
    auto u = move_info.to_part;

    if (part_w == part_u) {
        // #pragma unroll
        for (auto i = 0; i != 2; ++i) {
            if (part_w != l) {
                delta_gain[0][l] -= weight;
                delta_gain[1][l] -= weight;
                if (part_w == u) {
                    for (auto& dgv : this->delta_gain_v) {
                        dgv -= weight;
                    }
                }
            }
            weight = -weight;
            swap(l, u);
        }
        return delta_gain;
    }

    // #pragma unroll
    for (auto i = 0; i != 2; ++i) {
        if (part_w == l) {
            for (auto& dg0 : delta_gain[0]) {
                dg0 += weight;
            }
        } else if (part_u == l) {
            for (auto& dg1 : delta_gain[1]) {
                dg1 += weight;
            }
        } else {
            delta_gain[0][l] -= weight;
            delta_gain[1][l] -= weight;
            if (part_w == u || part_u == u) {
                for (auto& dgv : this->delta_gain_v) {
                    dgv -= weight;
                }
            }
        }
        weight = -weight;
        swap(l, u);
    }
    return delta_gain;
    // return this->update_move_general_net(part, move_info);
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
template <typename Gnl>
auto FMKWayGainCalc<Gnl>::update_move_general_net(gsl::span<const uint8_t> part,
                                                  const MoveInfo<typename Gnl::node_t>& move_info)
    -> FMKWayGainCalc<Gnl>::ret_info {
    // const auto& [net, v, from_part, to_part] = move_info;
    uint8_t StackBufLocal[FM_MAX_NUM_PARTITIONS];
    FMPmr::monotonic_buffer_resource rsrcLocal(StackBufLocal, sizeof StackBufLocal);
    auto num = FMPmr::vector<uint8_t>(this->num_parts, 0, &rsrcLocal);

    // auto idx_vec = vector<typename Gnl::node_t> {};
    // for (const auto& w : this->hgr.gr[move_info.net])
    // {
    //     if (w == move_info.v)
    //     {
    //         continue;
    //     }
    //     num[part[w]] += 1;
    //     idx_vec.push_back(w);
    // }
    for (const auto& w : this->idx_vec) {
        num[part[w]] += 1;
    }
    const auto degree = idx_vec.size();
    auto delta_gain = vector<vector<int>>(degree, vector<int>(this->num_parts, 0));
    auto weight = this->hgr.get_net_weight(move_info.net);

    auto l = move_info.from_part;
    auto u = move_info.to_part;

    // #pragma unroll
    for (auto i = 0; i != 2; ++i) {
        if (num[l] == 0) {
            for (size_t index = 0U; index != degree; ++index) {
                delta_gain[index][l] -= weight;
            }
            if (num[u] > 0) {
                for (auto& dgv : this->delta_gain_v) {
                    dgv -= weight;
                }
            }
        } else if (num[l] == 1) {
            for (size_t index = 0U; index != degree; ++index) {
                if (part[this->idx_vec[index]] == l) {
                    for (auto& dg : delta_gain[index]) {
                        dg += weight;
                    }
                    break;
                }
            }
        }
        weight = -weight;
        swap(l, u);
    };
    return delta_gain;
}

// instantiation

#include <py2cpp/set.hpp>              // for set
#include <xnetwork/classes/graph.hpp>  // for Graph

#include "ckpttn/netlist.hpp"  // for Netlist

template class FMKWayGainCalc<SimpleNetlist>;