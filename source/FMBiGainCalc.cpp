// #include <__config>                    // for std
#include <array>  // for array
// #include <boost/container/pmr/vector.hpp>  // for vector
// #include <boost/container/vector.hpp>      // for operator!=, vec_iterator
#include <ckpttn/FMBiGainCalc.hpp>  // for FMBiGainCalc, part, net
#include <ckpttn/FMPmrConfig.hpp>   // for FM_MAX_DEGREE
#include <ckpttn/moveinfo.hpp>      // for MoveInfo
#include <cstddef>                  // for size_t
#include <cstdint>                  // for uint8_t
#include <span>                     // for span
#include <transrangers.hpp>         // for all, filter, zip2
#include <vector>                   // for vector

using namespace std;
using namespace transrangers;

/**
 * The code snippet is defining the `_init_gain` function template for the `FMBiGainCalc` class.
 * This function is responsible for initializing the gain values for a given net and partition.
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::_init_gain(const typename Gnl::node_t &net, std::span<const uint8_t> part) {
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
 * The `_init_gain_2pin_net` function is a member function of the `FMBiGainCalc` class. It is
 * responsible for initializing the gain values for a 2-pin net in a given partition.
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl> void FMBiGainCalc<Gnl>::_init_gain_2pin_net(const typename Gnl::node_t &net,
                                                                    std::span<const uint8_t> part) {
    auto net_cur = this->hyprgraph.gr[net].begin();
    const auto node_w = *net_cur;
    const auto node_v = *++net_cur;

    const auto weight = this->hyprgraph.get_net_weight(net);
    if (part[node_w] != part[node_v]) {
        this->total_cost += weight;
        this->_increase_gain(node_w, weight);
        this->_increase_gain(node_v, weight);
    } else {
        this->_decrease_gain(node_w, weight);
        this->_decrease_gain(node_v, weight);
    }
}

/**
 * The `_init_gain_3pin_net` function is a member function of the `FMBiGainCalc` class. It is
 * responsible for initializing the gain values for a 3-pin net in a given partition.
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl> void FMBiGainCalc<Gnl>::_init_gain_3pin_net(const typename Gnl::node_t &net,
                                                                    std::span<const uint8_t> part) {
    auto net_cur = this->hyprgraph.gr[net].begin();
    const auto node_w = *net_cur;
    const auto node_v = *++net_cur;
    const auto node_u = *++net_cur;

    const auto weight = this->hyprgraph.get_net_weight(net);
    if (part[node_u] == part[node_v]) {
        if (part[node_w] == part[node_v]) {
            // this->_modify_gain_va(-weight, node_u, node_v, node_w);
            this->_decrease_gain(node_u, weight);
            this->_decrease_gain(node_v, weight);
            this->_decrease_gain(node_w, weight);
            return;
        }
        // this->_modify_gain_va(weight, node_w);
        this->_increase_gain(node_w, weight);
    } else if (part[node_w] == part[node_v]) {
        this->_increase_gain(node_u, weight);
    } else {
        this->_increase_gain(node_v, weight);
    }
    this->total_cost += weight;
}

/**
 * The `_init_gain_general_net` function is a member function of the `FMBiGainCalc` class. It is
 * responsible for initializing the gain values for a general net (with more than 3 pins) in a given
 * partition.
 *
 * @param[in] net
 * @param[in] part
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::_init_gain_general_net(const typename Gnl::node_t &net,
                                               std::span<const uint8_t> part) {
    auto num = array<size_t, 2>{0U, 0U};

    auto range = all(this->hyprgraph.gr[net]);
    range([&](const auto &weighted_cell) {
        num[part[*weighted_cell]] += 1;
        return true;
    });

    const uint32_t weight = this->hyprgraph.get_net_weight(net);

    // #pragma unroll
    for (const auto &part_idx : {0U, 1U}) {
        if (num[part_idx] == 0) {
            range([&](const auto &weighted_cell) {
                this->_decrease_gain(*weighted_cell, weight);
                return true;
            });
        } else if (num[part_idx] == 1) {
            auto iterator = this->hyprgraph.gr[net].begin();
            for (; part[*iterator] != part_idx; ++iterator) {
            }
            this->_increase_gain(*iterator, weight);
        }
    }

    if (num[0] > 0 && num[1] > 0) {
        this->total_cost += weight;
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
auto FMBiGainCalc<Gnl>::update_move_2pin_net(std::span<const uint8_t> part,
                                             const MoveInfo<typename Gnl::node_t> &move_info) ->
    typename Gnl::node_t {
    auto net_cur = this->hyprgraph.gr[move_info.net].begin();
    auto node_w = (*net_cur != move_info.v) ? *net_cur : *++net_cur;
    const auto gain = int(this->hyprgraph.get_net_weight(move_info.net));
    const int delta = (part[node_w] == move_info.from_part) ? gain : -gain;
    this->delta_gain_w = 2 * delta;
    return node_w;
}

/**
 * @brief
 *
 * @param[in] part
 * @param[in] move_info
 * @return ret_info
 */
template <typename Gnl> void FMBiGainCalc<Gnl>::init_idx_vec(const typename Gnl::node_t &module,
                                                             const typename Gnl::node_t &net) {
    this->idx_vec.clear();
    auto range1 = all(this->hyprgraph.gr[net]);
    auto range = filter([&module](const auto &cell) { return cell != module; }, range1);
    range([&](const auto &weighted_cell) {
        this->idx_vec.push_back(*weighted_cell);
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
template <typename Gnl> auto FMBiGainCalc<Gnl>::update_move_3pin_net(
    std::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t> &move_info) -> vector<int> {
    // const auto& [net, v, from_part, _] = move_info;

    auto delta_gain = vector<int>{0, 0};
    auto gain = int(this->hyprgraph.get_net_weight(move_info.net));
    const auto part_w = part[this->idx_vec[0]];

    if (part_w != move_info.from_part) {
        gain = -gain;
    }
    if (part_w == part[this->idx_vec[1]]) {
        delta_gain[0] += gain;
        delta_gain[1] += gain;
    } else {
        delta_gain[0] += gain;
        delta_gain[1] -= gain;
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
template <typename Gnl> auto FMBiGainCalc<Gnl>::update_move_general_net(
    std::span<const uint8_t> part, const MoveInfo<typename Gnl::node_t> &move_info) -> vector<int> {
    // const auto& [net, v, from_part, to_part] = move_info;
    auto num = array<size_t, 2>{0, 0};
    auto range1 = all(this->idx_vec);
    range1([&](const auto &weighted_cell) {
        num[part[*weighted_cell]] += 1;
        return true;
    });

    const auto degree = this->idx_vec.size();
    auto delta_gain = vector<int>(degree, 0);
    auto gain = int(this->hyprgraph.get_net_weight(move_info.net));
    auto range2 = all(delta_gain);
    // auto range3 = zip2(range1, range2);

    // #pragma unroll
    for (const auto &target_part : {move_info.from_part, move_info.to_part}) {
        if (num[target_part] == 0) {
            range2([&](const auto &delta_gain_cell) {
                *delta_gain_cell -= gain;
                return true;
            });
        } else if (num[target_part] == 1) {
            auto iter1 = this->idx_vec.begin();
            auto iter2 = delta_gain.begin();
            for (; part[*iter1] != target_part; ++iter1, ++iter2) {
            }
            *iter2 += gain;

            // range3([&](const auto &zipped_cell) {
            //     auto part_w = part[std::get<0>(*zipped_cell)];
            //     if (part_w == target_part) {
            //         std::get<1>(*zipped_cell) += gain;
            //         return false;
            //     }
            //     return true;
            // });
        }
        gain = -gain;
    }
    return delta_gain;
}

// instantiation

#include <netlistx/netlist.hpp>        // for Netlist, SimpleNetlist
#include <py2cpp/set.hpp>              // for set
#include <xnetwork/classes/graph.hpp>  // for Graph

template class FMBiGainCalc<SimpleNetlist>;
