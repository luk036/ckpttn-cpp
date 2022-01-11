#include <__config>                    // for std
#include <__hash_table>                // for __hash_const_iterator, operator!=
#include <array>                       // for array
#include <boost/container/vector.hpp>  // for operator!=, vec_iterator, vector
#include <ckpttn/FMBiGainCalc.hpp>
#include <ckpttn/FMPmrConfig.hpp>      // for FM_MAX_DEGREE
#include <initializer_list>            // for initializer_list
#include <py2cpp/set.hpp>              // for set
#include <vector>                      // for vector
#include <xnetwork/classes/graph.hpp>  // for Graph

#include "ckpttn/netlist.hpp"  // for SimpleNetlist, MoveInfo
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
void FMBiGainCalc::_init_gain(  //
    const node_t& net, gsl::span<const uint8_t> part) {
    const auto degree = this->H.G.degree(net);
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
void FMBiGainCalc::_init_gain_2pin_net(  //
    const node_t& net, gsl::span<const uint8_t> part) {
    auto netCur = this->H.G[net].begin();
    const auto w = *netCur;
    const auto v = *++netCur;

    const auto weight = this->H.get_net_weight(net);
    if (part[w] != part[v]) {
        this->totalcost += weight;
        this->_modify_gain_va(weight, w, v);
    } else {
        this->_modify_gain_va(-weight, w, v);
    }
}

/**
 * @brief
 *
 * @param[in] net
 * @param[in] part
 */
void FMBiGainCalc::_init_gain_3pin_net(const node_t& net, gsl::span<const uint8_t> part) {
    auto netCur = this->H.G[net].begin();
    const auto w = *netCur;
    const auto v = *++netCur;
    const auto u = *++netCur;

    const auto weight = this->H.get_net_weight(net);
    if (part[u] == part[v]) {
        if (part[w] == part[v]) {
            this->_modify_gain_va(-weight, u, v, w);
            return;
        }
        this->_modify_gain_va(weight, w);
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
void FMBiGainCalc::_init_gain_general_net(const node_t& net, gsl::span<const uint8_t> part) {
    auto num = array<size_t, 2>{0U, 0U};
    for (const auto& w : this->H.G[net]) {
        num[part[w]] += 1;
    }
    const auto weight = this->H.get_net_weight(net);

    // #pragma unroll
    for (const auto& k : {0U, 1U}) {
        if (num[k] == 0) {
            for (const auto& w : this->H.G[net]) {
                this->_modify_gain(w, -weight);
            }
        } else if (num[k] == 1) {
            for (const auto& w : this->H.G[net]) {
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
auto FMBiGainCalc::update_move_2pin_net(gsl::span<const uint8_t> part,
                                        const MoveInfo<node_t>& move_info) -> FMBiGainCalc::node_t {
    auto netCur = this->H.G[move_info.net].begin();
    auto w = (*netCur != move_info.v) ? *netCur : *++netCur;
    const auto weight = this->H.get_net_weight(move_info.net);
    const int delta = (part[w] == move_info.fromPart) ? weight : -weight;
    this->deltaGainW = 2 * delta;
    return w;
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
void FMBiGainCalc::init_IdVec(const node_t& v, const node_t& net) {
    // auto rng = this->H.G[net] |
    //         ranges::views::remove_if([&](auto w) { return w == v; });
    // using namespace transrangers;
    // auto rng = filter([&](const auto& w) { return w != v; }, all(this->H.G[net]));
    // this->IdVec = FMPmr::vector<node_t>(rng.begin(), rng.end(), &this->rsrc);

    this->IdVec.clear();
    auto&& rng = this->H.G[net];
    this->IdVec.reserve(rng.size() - 1);
    for (const auto& w : rng) {
        if (w == v) {
            continue;
        }
        this->IdVec.push_back(w);
    }
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
auto FMBiGainCalc::update_move_3pin_net(gsl::span<const uint8_t> part,
                                        const MoveInfo<node_t>& move_info) -> vector<int> {
    // const auto& [net, v, fromPart, _] = move_info;
    auto num = array<size_t, 2>{0U, 0U};
    for (const auto& w : this->IdVec) {
        num[part[w]] += 1;
    }
    // for (const auto& w : this->H.G[move_info.net])
    // {
    //     if (w == move_info.v)
    //     {
    //         continue;
    //     }
    //     num[part[w]] += 1;
    //     IdVec.push_back(w);
    // }
    auto deltaGain = vector<int>{0, 0};
    auto weight = this->H.get_net_weight(move_info.net);
    const auto part_w = part[this->IdVec[0]];

    if (part_w != move_info.fromPart) {
        weight = -weight;
    }
    if (part_w == part[this->IdVec[1]]) {
        deltaGain[0] += weight;
        deltaGain[1] += weight;
    } else {
        deltaGain[0] += weight;
        deltaGain[1] -= weight;
    }
    return deltaGain;
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
auto FMBiGainCalc::update_move_general_net(gsl::span<const uint8_t> part,
                                           const MoveInfo<node_t>& move_info) -> vector<int> {
    // const auto& [net, v, fromPart, toPart] = move_info;
    auto num = array<uint8_t, 2>{0, 0};
    // auto IdVec = vector<node_t> {};
    for (const auto& w : this->IdVec) {
        num[part[w]] += 1;
    }

    // for (const auto& w : this->H.G[move_info.net])
    // {
    //     if (w == move_info.v)
    //     {
    //         continue;
    //     }
    //     num[part[w]] += 1;
    //     IdVec.push_back(w);
    // }
    const auto degree = this->IdVec.size();
    auto deltaGain = vector<int>(degree, 0);
    auto weight = this->H.get_net_weight(move_info.net);

    // #pragma unroll
    for (const auto& l : {move_info.fromPart, move_info.toPart}) {
        if (num[l] == 0) {
            for (size_t index = 0U; index != degree; ++index) {
                deltaGain[index] -= weight;
            }
        } else if (num[l] == 1) {
            for (size_t index = 0U; index != degree; ++index) {
                auto part_w = part[this->IdVec[index]];
                if (part_w == l) {
                    deltaGain[index] += weight;
                    break;
                }
            }
        }
        weight = -weight;
    }
    return deltaGain;
}
