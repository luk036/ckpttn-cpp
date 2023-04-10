#include <cassert> // for assert
#include <cstdint> // for uint8_t

// #include <__config>                                           // for std
// #include <__hash_table>                                       // for
// __hash_...
#include <algorithm>                 // for fill
#include <ckpttn/FMKWayGainCalc.hpp> // for FMKWayG...
#include <ckpttn/FMPmrConfig.hpp>    // for FM_MAX_...
#include <ckpttn/dllist.hpp>         // for Dllink
#include <ckpttn/moveinfo.hpp>       // for MoveInfo
#include <ckpttn/robin.hpp>          // for fun::Robin<>...
#include <cstddef>                   // for byte
#include <gsl/span>                  // for span
#include <initializer_list>          // for initial...
#include <transrangers.hpp>          // for all, filter, zip2
#include <transrangers_ext.hpp>      // for enumerate
#include <type_traits>               // for swap
#include <utility>                   // for pair
#include <vector>                    // for vector

using namespace std;
using namespace transrangers;

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMKWayGainCalc<Gnl>::_init_gain(const typename Gnl::node_t &net,
                                     gsl::span<const uint8_t> part) {
  const auto degree = this->hgr.gr.degree(net);
  if (degree < 2 || degree > FM_MAX_DEGREE) // [[unlikely]]
  {
    return; // does not provide any gain when moving
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
void FMKWayGainCalc<Gnl>::_init_gain_3pin_net(const typename Gnl::node_t &net,
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

  for (const auto &e : {b, c}) {
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
void FMKWayGainCalc<Gnl>::_init_gain_general_net(
    const typename Gnl::node_t &net, gsl::span<const uint8_t> part) {
  uint8_t StackBufLocal[2048];
  FMPmr::monotonic_buffer_resource rsrcLocal(StackBufLocal,
                                             sizeof StackBufLocal);
  auto num = FMPmr::vector<uint8_t>(this->num_parts, 0, &rsrcLocal);
  auto rng = all(this->hgr.gr[net]);
  rng([&](const auto &wc) {
    num[part[*wc]] += 1;
    return true;
  });

  const auto weight = this->hgr.get_net_weight(net);
  auto rng2 = all(num);
  auto rng3 = filter([](const auto &c) { return c > 0; }, rng2);

  this->totalcost = -weight;
  rng3([&](const auto & /* c */) {
    this->totalcost += weight;
    return true;
  });

  auto k = 0U;
  for (const auto &c : num) {
    if (c == 0) {
      rng([&](const auto &wc) {
        this->vertex_list[k][*wc].data.second -= weight;
        return true;
      });
    } else if (c == 1) {
      rng([&](const auto &wc) {
        if (part[*wc] == k) {
          this->_modify_gain(*wc, part[*wc], weight);
          return false;
        }
        return true;
      });
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
 * @param[in] part
 * @param[in] move_info
 * @param[out] w
 * @return ret_2pin_info
 */
template <typename Gnl>
auto FMKWayGainCalc<Gnl>::update_move_2pin_net(
    gsl::span<const uint8_t> part,
    const MoveInfo<typename Gnl::node_t> &move_info) -> typename Gnl::node_t {
  // const auto& [net, v, from_part, to_part] = move_info;
  assert(part[move_info.v] == move_info.from_part);

  auto weight = this->hgr.get_net_weight(move_info.net);
  // auto delta_gain_w = vector<int>(this->num_parts, 0);
  auto net_cur = this->hgr.gr[move_info.net].begin();
  auto w = (*net_cur != move_info.v) ? *net_cur : *++net_cur;
  fill(this->delta_gain_w.begin(), this->delta_gain_w.end(), 0);
  auto rng1 = all(this->delta_gain_w);
  auto rng2 = all(this->delta_gain_v);
  auto rng3 = zip2(rng1, rng2);

  // #pragma unroll
  for (const auto &l_part : {move_info.from_part, move_info.to_part}) {
    if (part[w] == l_part) {
      rng3([&weight](const auto &zc) {
        std::get<0>(*zc) += weight;
        std::get<1>(*zc) += weight;
        return true;
      });
    }
    this->delta_gain_w[l_part] -= weight;
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
template <typename Gnl>
void FMKWayGainCalc<Gnl>::init_idx_vec(const typename Gnl::node_t &v,
                                       const typename Gnl::node_t &net) {
  this->idx_vec.clear();
  auto rng1 = all(this->hgr.gr[net]);
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
template <typename Gnl>
auto FMKWayGainCalc<Gnl>::update_move_3pin_net(
    gsl::span<const uint8_t> part,
    const MoveInfo<typename Gnl::node_t> &move_info)
    -> FMKWayGainCalc<Gnl>::ret_info {
  const auto degree = this->idx_vec.size();
  auto delta_gain =
      vector<vector<int>>(degree, vector<int>(this->num_parts, 0));
  auto weight = this->hgr.get_net_weight(move_info.net);
  const auto part_w = part[this->idx_vec[0]];
  const auto part_u = part[this->idx_vec[1]];
  auto l = move_info.from_part;
  auto u = move_info.to_part;
  auto rngv = all(this->delta_gain_v);

  if (part_w == part_u) {
    // #pragma unroll
    for (auto i = 0; i != 2; ++i) {
      if (part_w != l) {
        delta_gain[0][l] -= weight;
        delta_gain[1][l] -= weight;
        if (part_w == u) {
          // for (auto &dgv : this->delta_gain_v) {
          //   dgv -= weight;
          // }
          rngv([&weight](const auto &dgcv) {
            *dgcv -= weight;
            return true;
          });
        }
      }
      weight = -weight;
      swap(l, u);
    }
    return delta_gain;
  }

  auto rng0 = all(delta_gain[0]);
  auto rng1 = all(delta_gain[1]);

  // #pragma unroll
  for (auto i = 0; i != 2; ++i) {
    if (part_w == l) {
      rng0([&weight](const auto &dgc0) {
        *dgc0 += weight;
        return true;
      });
    } else if (part_u == l) {
      rng1([&weight](const auto &dgc1) {
        *dgc1 += weight;
        return true;
      });
    } else {
      delta_gain[0][l] -= weight;
      delta_gain[1][l] -= weight;
      if (part_w == u || part_u == u) {
        rngv([&weight](const auto &dgcv) {
          *dgcv -= weight;
          return true;
        });
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
auto FMKWayGainCalc<Gnl>::update_move_general_net(
    gsl::span<const uint8_t> part,
    const MoveInfo<typename Gnl::node_t> &move_info)
    -> FMKWayGainCalc<Gnl>::ret_info {
  // const auto& [net, v, from_part, to_part] = move_info;
  uint8_t StackBufLocal[FM_MAX_NUM_PARTITIONS];
  FMPmr::monotonic_buffer_resource rsrcLocal(StackBufLocal,
                                             sizeof StackBufLocal);
  auto num = FMPmr::vector<uint8_t>(this->num_parts, 0, &rsrcLocal);
  auto rng1 = all(this->idx_vec);
  rng1([&](const auto &wc) {
    num[part[*wc]] += 1;
    return true;
  });

  const auto degree = idx_vec.size();
  auto delta_gain =
      vector<vector<int>>(degree, vector<int>(this->num_parts, 0));
  auto weight = this->hgr.get_net_weight(move_info.net);

  auto l = move_info.from_part;
  auto u = move_info.to_part;

  auto rng2 = all(delta_gain);
  auto rng3 = zip2(rng1, rng2);
  auto rng4 = all(this->delta_gain_v);

  // #pragma unroll
  for (auto i = 0; i != 2; ++i) {
    if (num[l] == 0) {
      rng2([&weight, &l](const auto &dgc) {
        (*dgc)[l] -= weight;
        return true;
      });

      if (num[u] > 0) {
        rng4([&weight](const auto &dgvc) {
          *dgvc -= weight;
          return true;
        });
      }
    } else if (num[l] == 1) {
      rng3([&weight, &l, &part](const auto &zc) {
        auto part_w = part[std::get<0>(*zc)];
        if (part_w == l) {
          auto rng = all(std::get<1>(*zc));
          rng([&weight](const auto &dgc) {
            *dgc += weight;
            return true;
          });
          return false;
        }
        return true;
      });
    }
    weight = -weight;
    swap(l, u);
  };
  return delta_gain;
}

// instantiation

#include "ckpttn/netlist.hpp"         // for Netlist
#include <py2cpp/set.hpp>             // for set
#include <xnetwork/classes/graph.hpp> // for Graph

template class FMKWayGainCalc<SimpleNetlist>;