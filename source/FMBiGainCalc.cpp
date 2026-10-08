// #include <__config>                    // for std
#include <array>                    // for array
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
 * @brief Initializes the gain values for a net in 2-way partitioning.
 *
 * Dispatches to specialized handlers based on the net degree (2-pin, 3-pin,
 * or general net). Nets with degree < 2 or > FM_MAX_DEGREE are skipped
 * as they provide no gain when moving.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] net The net to initialize gains for
 * @param[in] part The current partition assignment
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::_init_gain(const Gnl::node_t& net, std::span<const uint8_t> part) {
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
 * @brief Initializes gain values for a 2-pin net in 2-way partitioning.
 *
 * If the two pins are in different partitions, increases their gains
 * and adds to the total cost. If they are in the same partition,
 * decreases their gains.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] net The 2-pin net to initialize gains for
 * @param[in] part The current partition assignment
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::_init_gain_2pin_net(const Gnl::node_t& net, std::span<const uint8_t> part) {
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
 * @brief Initializes gain values for a 3-pin net in 2-way partitioning.
 *
 * Depending on which pins share the same partition, adjusts the gain
 * values for the three vertices and updates the total cost accordingly.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] net The 3-pin net to initialize gains for
 * @param[in] part The current partition assignment
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::_init_gain_3pin_net(const Gnl::node_t& net, std::span<const uint8_t> part) {
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
 * @brief Initializes gain values for a general net (degree > 3) in 2-way partitioning.
 *
 * Counts how many pins are in each partition, then adjusts gains:
 * - If a partition has 0 pins: all vertices lose gain (moving to that part helps)
 * - If a partition has 1 pin: that pin gains (moving it out would cut the net)
 * - Adds to total cost if pins span both partitions.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] net The general net to initialize gains for
 * @param[in] part The current partition assignment
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::_init_gain_general_net(const Gnl::node_t& net,
                                               std::span<const uint8_t> part) {
    auto num = array<size_t, 2>{0U, 0U};

    auto range = all(this->hyprgraph.gr[net]);
    range([&](const auto& weighted_cell) {
        num[part[*weighted_cell]] += 1;
        return true;
    });

    const auto cnt_base = static_cast<std::size_t>(net) * 2;
    this->net_pin_count[cnt_base] = static_cast<std::uint16_t>(num[0]);
    this->net_pin_count[cnt_base + 1] = static_cast<std::uint16_t>(num[1]);

    const uint32_t weight = this->hyprgraph.get_net_weight(net);

    // #pragma unroll
    for (const auto& part_idx : {0U, 1U}) {
        if (num[part_idx] == 0) {
            range([&](const auto& weighted_cell) {
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
 * @brief Updates gain values for a 2-pin net after a vertex move.
 *
 * Computes the delta gain for the other vertex in the 2-pin net and
 * returns that vertex for key modification.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] part The current partition assignment
 * @param[in] move_info Information about the move being performed
 * @return The other vertex in the 2-pin net (whose gain needs updating)
 */
template <typename Gnl>
auto FMBiGainCalc<Gnl>::update_move_2pin_net(std::span<const uint8_t> part,
                                             const MoveInfo<typename Gnl::node_t>& move_info)
    -> Gnl::node_t {
    auto net_cur = this->hyprgraph.gr[move_info.net].begin();
    auto node_w = (*net_cur != move_info.v) ? *net_cur : *++net_cur;
    const auto gain = static_cast<int>(this->hyprgraph.get_net_weight(move_info.net));
    const int delta = (part[node_w] == move_info.from_part) ? gain : -gain;
    this->delta_gain_w = 2 * delta;
    return node_w;
}

/**
 * @brief Initializes the index vector with all vertices in a net except the given module.
 *
 * Populates `idx_vec` with the neighbors of the given module in the specified net,
 * reserving space for degree-1 elements.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] module The module (vertex) to exclude from the index vector
 * @param[in] net The net whose other vertices are collected
 */
template <typename Gnl>
void FMBiGainCalc<Gnl>::init_idx_vec(const Gnl::node_t& module, const Gnl::node_t& net) {
    this->idx_vec.clear();
    auto degree = this->hyprgraph.gr.degree(net);
    this->idx_vec.reserve(degree - 1);
    auto range1 = all(this->hyprgraph.gr[net]);
    auto range = filter([&module](const auto& cell) { return cell != module; }, range1);
    range([&](const auto& weighted_cell) {
        this->idx_vec.emplace_back(*weighted_cell);
        return true;
    });
}

/**
 * @brief Updates gain values for a 3-pin net after a vertex move.
 *
 * Computes the delta gain contributions for the two remaining vertices
 * in the 3-pin net after the move, based on their partition assignments.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] part The current partition assignment
 * @param[in] move_info Information about the move being performed
 * @return Vector of delta gain values for the remaining vertices
 */
template <typename Gnl>
auto FMBiGainCalc<Gnl>::update_move_3pin_net(std::span<const uint8_t> part,
                                             const MoveInfo<typename Gnl::node_t>& move_info)
    -> std::span<const int> {
    // const auto& [net, v, from_part, _] = move_info;

    auto& delta_gain = this->delta_gain_buf;
    delta_gain.assign(2, 0);
    auto gain = static_cast<int>(this->hyprgraph.get_net_weight(move_info.net));
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
 * @brief Updates gain values for a general net (degree > 3) after a vertex move.
 *
 * Counts how many remaining vertices are in each partition and computes
 * delta gains for each vertex based on partition counts (0 pins vs 1 pin).
 *
 * @tparam Gnl The hypergraph type
 * @param[in] part The current partition assignment
 * @param[in] move_info Information about the move being performed
 * @return Vector of delta gain values for each remaining vertex
 */
template <typename Gnl>
auto FMBiGainCalc<Gnl>::update_move_general_net(std::span<const uint8_t> part,
                                                const MoveInfo<typename Gnl::node_t>& move_info)
    -> std::span<const SparseDelta<typename Gnl::node_t>> {
    const auto cnt_base = static_cast<std::size_t>(move_info.net) * 2;
    const auto cnt_from = this->net_pin_count[cnt_base + move_info.from_part];
    const auto cnt_to = this->net_pin_count[cnt_base + move_info.to_part];
    const auto num_from = static_cast<int>(cnt_from) - 1;
    const auto num_to = static_cast<int>(cnt_to);
    this->net_pin_count[cnt_base + move_info.from_part] = static_cast<std::uint16_t>(cnt_from - 1);
    this->net_pin_count[cnt_base + move_info.to_part] = static_cast<std::uint16_t>(cnt_to + 1);

    auto& sparse = this->sparse_buf;
    sparse.clear();
    if (num_from >= 2 && num_to >= 2) {
        return {sparse.data(), sparse.size()};
    }

    this->init_idx_vec(move_info.v, move_info.net);
    const auto degree = this->idx_vec.size();
    const auto weight = static_cast<int>(this->hyprgraph.get_net_weight(move_info.net));
    const auto gain_from = weight;
    const auto gain_to = -weight;

    auto uniq_from = degree;
    auto uniq_to = degree;
    if (num_from == 1) {
        for (std::size_t i = 0; i < degree; ++i) {
            if (part[this->idx_vec[i]] == move_info.from_part) {
                uniq_from = i;
                break;
            }
        }
    }
    if (num_to == 1) {
        for (std::size_t i = 0; i < degree; ++i) {
            if (part[this->idx_vec[i]] == move_info.to_part) {
                uniq_to = i;
                break;
            }
        }
    }

    for (std::size_t i = 0; i < degree; ++i) {
        int delta = 0;
        if (num_from == 0) {
            delta -= gain_from;
        } else if (i == uniq_from) {
            delta += gain_from;
        }
        if (num_to == 0) {
            delta -= gain_to;
        } else if (i == uniq_to) {
            delta += gain_to;
        }
        if (delta != 0) {
            const auto w = this->idx_vec[i];
            sparse.push_back(SparseDelta<typename Gnl::node_t>{
                w, static_cast<std::uint8_t>(1 - part[w]), delta});
        }
    }
    return {sparse.data(), sparse.size()};
}

// instantiation

#include <netlistx/netlist.hpp>        // for Netlist, SimpleNetlist
#include <py2cpp/set.hpp>              // for set
#include <xnetwork/classes/graph.hpp>  // for Graph

template class FMBiGainCalc<SimpleNetlist>;
