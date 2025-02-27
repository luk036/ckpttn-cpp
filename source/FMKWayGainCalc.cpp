#include <cassert>  // for assert
#include <cstdint>  // for uint8_t

// #include <__config>                                           // for std
// #include <__hash_table>                                       // for
// __hash_...
#include <algorithm>                  // for fill
#include <ckpttn/FMKWayGainCalc.hpp>  // for FMKWayG...
#include <ckpttn/FMPmrConfig.hpp>     // for FM_MAX_...
#include <ckpttn/moveinfo.hpp>        // for MoveInfo
#include <mywheel/dllist.hpp>          // for Dllink
#include <mywheel/robin.hpp>           // for fun::Robin<>...
#include <cstddef>                    // for byte
#include <initializer_list>           // for initial...
#include <span>                       // for span
#include <transrangers.hpp>           // for all, filter, zip2
#include <transrangers_ext.hpp>       // for enumerate
#include <type_traits>                // for swap
#include <utility>                    // for pair
#include <vector>                     // for vector

using namespace std;
using namespace transrangers;

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl> void FMKWayGainCalc<Gnl>::_init_gain(const typename Gnl::node_t &net,
                                                             std::span<const uint8_t> part) {
    const auto degree = this->hyprgraph.gr.degree(net);
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
void FMKWayGainCalc<Gnl>::_init_gain_2pin_net(const typename Gnl::node_t &net,
                                              std::span<const uint8_t> part) {
    auto net_cur = this->hyprgraph.gr[net].begin();
    const auto w = *net_cur;
    const auto v = *++net_cur;
    const auto part_w = part[w];
    const auto part_v = part[v];
    const auto weight = this->hyprgraph.get_net_weight(net);
    if (part_v == part_w) {
        this->_decrease_gain(w, part_v, weight);
        this->_decrease_gain(v, part_v, weight);
        // this->_modify_gain_va(-weight, part_v, w, v);
    } else {
        this->total_cost += weight;
        // this->vertex_list[part_v][w].data.second += weight;
        this->init_gain_list[part_v][w] += weight;
        // this->vertex_list[part_w][v].data.second += weight;
        this->init_gain_list[part_w][v] += weight;
    }
}

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMKWayGainCalc<Gnl>::_init_gain_3pin_net(const typename Gnl::node_t &net,
                                              std::span<const uint8_t> part) {
    auto net_cur = this->hyprgraph.gr[net].begin();
    const auto w = *net_cur;
    const auto v = *++net_cur;
    const auto u = *++net_cur;
    const auto part_w = part[w];
    const auto part_v = part[v];
    const auto part_u = part[u];
    const auto weight = this->hyprgraph.get_net_weight(net);
    auto a = w;
    auto b = v;
    auto c = u;

    if (part_u == part_v) {
        if (part_w == part_v) {
            this->_decrease_gain(u, part_v, weight);
            this->_decrease_gain(v, part_v, weight);
            this->_decrease_gain(w, part_v, weight);
            // this->_modify_gain_va(-weight, part_v, u, v, w);
            return;
        }
    } else if (part_w == part_v) {
        a = u, b = w, c = v;
    } else if (part_w == part_u) {
        a = v, b = u, c = w;
    } else {
        this->total_cost += 2 * weight;
        // this->_modify_vertex_va(weight, part_v, u, w);
        // this->_modify_vertex_va(weight, part_w, u, v);
        // this->_modify_vertex_va(weight, part_u, v, w);
        this->_increase_gain(u, part_v, weight);
        this->_increase_gain(w, part_v, weight);
        this->_increase_gain(u, part_w, weight);
        this->_increase_gain(v, part_w, weight);
        this->_increase_gain(v, part_u, weight);
        this->_increase_gain(w, part_u, weight);
        return;
    }

    for (const auto &e : {b, c}) {
        this->_decrease_gain(e, part[b], weight);
        // this->vertex_list[part[a]][e].data.second += weight;
        this->init_gain_list[part[a]][e] += weight;
    }
    // this->vertex_list[part[b]][a].data.second += weight;
    this->init_gain_list[part[b]][a] += weight;

    // this->_modify_gain_va(-weight, part[b], b, c);
    // this->_modify_vertex_va(weight, part[a], b, c);
    // this->_modify_vertex_va(weight, part[b], a);

    this->total_cost += weight;
}

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMKWayGainCalc<Gnl>::_init_gain_general_net(const typename Gnl::node_t &net,
                                                 std::span<const uint8_t> part) {
    // uint8_t StackBufLocal[2048];
    // FMPmr::monotonic_buffer_resource rsrcLocal(StackBufLocal,
    //                                            sizeof StackBufLocal);
    // auto num = FMPmr::vector<uint8_t>(this->num_parts, 0, &rsrcLocal);
    auto num = std::vector<uint8_t>(this->num_parts, 0);
    // for (const auto &w : this->hyprgraph.gr[net]) {
    //   num[part[w]] += 1;
    // }
    auto rng = all(this->hyprgraph.gr[net]);
    rng([&](const auto &wc) {
        num[part[*wc]] += 1;
        return true;
    });

    const uint32_t weight = this->hyprgraph.get_net_weight(net);
    // for (const auto &c : num) {
    //   if (c > 0) {
    //     this->total_cost += weight;
    //   }
    // }
    auto rng2 = all(num);
    auto rng3 = filter([](const auto &c) { return c > 0; }, rng2);

    rng3([&](const auto & /* c */) {
        this->total_cost += weight;
        return true;
    });
    this->total_cost -= weight;

    auto k = 0U;
    for (const auto &c : num) {
        if (c == 0) {
            // for (const auto &w : this->hyprgraph.gr[net]) {
            //   this->init_gain_list[k][w] -= int(weight);
            // }
            rng([&](const auto &wc) {
                // this->vertex_list[k][*wc].data.second -= weight;
                this->init_gain_list[k][*wc] -= int(weight);
                return true;
            });
        } else if (c == 1) {
            auto it = this->hyprgraph.gr[net].begin();
            for (; part[*it] != k; ++it);
            this->_increase_gain(*it, part[*it], weight);
            // auto rng_new = all(this->hyprgraph.gr[net]);  // reinitialize after breaking (fix
            //                                               // for Termux's clang 16)
            // rng_new([&part, k, weight, this](const auto &wc) {
            //     if (part[*wc] == k) {
            //         this->_increase_gain(*wc, part[*wc], weight);
            //         return false;
            //     }
            //     return true;
            // });
        }
        ++k;
    }

    // auto rng4 = enumerate(rng2);
    // rng4([&](const auto &cursor) {
    //   auto k = std::get<0>(*cursor);
    //   auto c = std::get<1>(*cursor);
    //   if (c == 0) {
    //     rng([&, k](const auto &wc) {
    //       this->vertex_list[k][*wc].data.second -= weight;
    //       return true;
    //     });
    //   } else if (c == 1) {
    //     rng([&, k](const auto &wc) {
    //       if (part[*wc] == k) {
    //         this->_modify_gain(*wc, part[*wc], weight);
    //         return false;
    //       }
    //       return true;
    //     });
    //   }
    //   return true;
    // });
}

/**
 * @brief
 *
 */
template <typename Gnl> auto FMKWayGainCalc<Gnl>::update_move_init() -> void {
    std::fill(this->delta_gain_v.begin(), this->delta_gain_v.end(), 0);
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
auto FMKWayGainCalc<Gnl>::update_move_2pin_net(std::span<const uint8_t> part,
                                               const MoveInfo<typename Gnl::node_t> &move_info) ->
    typename Gnl::node_t {
    // const auto& [net, v, from_part, to_part] = move_info;
    assert(part[move_info.v] == move_info.from_part);

    auto gain = int(this->hyprgraph.get_net_weight(move_info.net));
    // auto delta_gain_w = vector<int>(this->num_parts, 0);
    auto net_cur = this->hyprgraph.gr[move_info.net].begin();
    auto w = (*net_cur != move_info.v) ? *net_cur : *++net_cur;
    fill(this->delta_gain_w.begin(), this->delta_gain_w.end(), 0);
    auto rng_w = all(this->delta_gain_w);
    auto rng_v = all(this->delta_gain_v);

    // #pragma unroll
    for (const auto &l_part : {move_info.from_part, move_info.to_part}) {
        if (part[w] == l_part) {
            // for (auto &dgw : delta_gain_w) {
            //   dgw += gain;
            // }
            // for (auto &dgv : delta_gain_v) {
            //   dgv += gain;
            // }

            // luk: two ranges cannot zipped because of different lengths
            rng_w([&gain](const auto &dgwc) {
                *dgwc += gain;
                return true;
            });
            rng_v([&gain](const auto &dgvc) {
                *dgvc += gain;
                return true;
            });
        }
        this->delta_gain_w[l_part] -= gain;
        gain = -gain;
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
template <typename Gnl> void FMKWayGainCalc<Gnl>::init_idx_vec(const typename Gnl::node_t &v,
                                                               const typename Gnl::node_t &net) {
    this->idx_vec.clear();
    auto rng1 = all(this->hyprgraph.gr[net]);
    auto rng = filter([&v](const auto &w) { return w != v; }, rng1);
    rng([&](const auto &wc) {
        this->idx_vec.push_back(*wc);
        return true;
    });
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
template <typename Gnl> auto FMKWayGainCalc<Gnl>::update_move_3pin_net(
    std::span<const uint8_t> part,
    const MoveInfo<typename Gnl::node_t> &move_info) -> FMKWayGainCalc<Gnl>::ret_info {
    const auto degree = this->idx_vec.size();
    auto delta_gain = vector<vector<int>>(degree, vector<int>(this->num_parts, 0));
    auto gain = int(this->hyprgraph.get_net_weight(move_info.net));
    const auto part_w = part[this->idx_vec[0]];
    const auto part_u = part[this->idx_vec[1]];
    auto l = move_info.from_part;
    auto u = move_info.to_part;
    auto rngv = all(this->delta_gain_v);

    if (part_w == part_u) {
        // #pragma unroll
        for (auto i = 0; i != 2; ++i) {
            if (part_w != l) {
                delta_gain[0][l] -= gain;
                delta_gain[1][l] -= gain;
                if (part_w == u) {
                    // for (auto &dgv : this->delta_gain_v) {
                    //   dgv -= weight;
                    // }
                    rngv([&gain](const auto &dgcv) {
                        *dgcv -= gain;
                        return true;
                    });
                }
            }
            gain = -gain;
            swap(l, u);
        }
        return delta_gain;
    }

    auto rng0 = all(delta_gain[0]);
    auto rng1 = all(delta_gain[1]);

    // #pragma unroll
    for (auto i = 0; i != 2; ++i) {
        if (part_w == l) {
            rng0([&gain](const auto &dgc0) {
                *dgc0 += gain;
                return true;
            });
        } else if (part_u == l) {
            rng1([&gain](const auto &dgc1) {
                *dgc1 += gain;
                return true;
            });
        } else {
            delta_gain[0][l] -= gain;
            delta_gain[1][l] -= gain;
            if (part_w == u || part_u == u) {
                rngv([&gain](const auto &dgcv) {
                    *dgcv -= gain;
                    return true;
                });
            }
        }
        gain = -gain;
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
template <typename Gnl> auto FMKWayGainCalc<Gnl>::update_move_general_net(
    std::span<const uint8_t> part,
    const MoveInfo<typename Gnl::node_t> &move_info) -> FMKWayGainCalc<Gnl>::ret_info {
    // const auto& [net, v, from_part, to_part] = move_info;
    // uint8_t StackBufLocal[FM_MAX_NUM_PARTITIONS];
    // FMPmr::monotonic_buffer_resource rsrcLocal(StackBufLocal,
    //                                            sizeof StackBufLocal);
    // auto num = FMPmr::vector<uint8_t>(this->num_parts, 0, &rsrcLocal);
    auto num = std::vector<uint8_t>(this->num_parts, 0);
    auto rng1 = all(this->idx_vec);
    rng1([&](const auto &wc) {
        num[part[*wc]] += 1;
        return true;
    });

    const auto degree = idx_vec.size();
    auto delta_gain = vector<vector<int>>(degree, vector<int>(this->num_parts, 0));
    auto gain = int(this->hyprgraph.get_net_weight(move_info.net));

    auto l = move_info.from_part;
    auto u = move_info.to_part;

    auto rng2 = all(delta_gain);
    // auto rng3 = zip2(rng1, rng2);
    auto rng4 = all(this->delta_gain_v);

    // #pragma unroll
    for (auto i = 0; i != 2; ++i) {
        if (num[l] == 0) {
            rng2([&gain, &l](const auto &dgc) {
                (*dgc)[l] -= gain;
                return true;
            });

            if (num[u] > 0) {
                rng4([&gain](const auto &dgvc) {
                    *dgvc -= gain;
                    return true;
                });
            }
        } else if (num[l] == 1) {
            auto it1 = this->idx_vec.begin();
            auto it2 = delta_gain.begin();
            for (; part[*it1] != l; ++it1, ++it2);
            auto rng = all(*it2);
            rng([&gain](const auto &dgc) {
                *dgc += gain;
                return true;
            });

            // rng3([&gain, &l, &part](const auto &zc) {
            //     auto part_w = part[std::get<0>(*zc)];
            //     if (part_w == l) {
            //         auto rng = all(std::get<1>(*zc));
            //         rng([&gain](const auto &dgc) {
            //             *dgc += gain;
            //             return true;
            //         });
            //         return false;
            //     }
            //     return true;
            // });
        }
        gain = -gain;
        swap(l, u);
    };
    return delta_gain;
}

// instantiation

#include <py2cpp/set.hpp>              // for set
#include <xnetwork/classes/graph.hpp>  // for Graph

#include "ckpttn/netlist.hpp"  // for Netlist

template class FMKWayGainCalc<SimpleNetlist>;
