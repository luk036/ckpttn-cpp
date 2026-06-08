#include <ckpttn/MidLvlPartMgr.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <ckpttn/midlevel/hamcycle.hpp>
#include <ckpttn/midlevel/vertex.hpp>
#include <ckpttn/moveinfo.hpp>
#include <netlistx/netlist.hpp>
#include <span>
#include <vector>

template <typename Gnl>
MidLvlPartMgr<Gnl>::MidLvlPartMgr(const Gnl& hyprgraph, double bal_tol)
    : hyprgraph{hyprgraph}, constr_mgr{hyprgraph, bal_tol} {}

template <typename Gnl>
auto MidLvlPartMgr<Gnl>::compute_total_cost(std::span<const std::uint8_t> part) const -> int {
    auto cost = 0;
    for (const auto& net : this->hyprgraph.nets) {
        auto in0 = std::size_t{0};
        auto in1 = std::size_t{0};
        for (const auto& module : this->hyprgraph.gr[net]) {
            if (part[module] == 0) {
                ++in0;
            } else {
                ++in1;
            }
        }
        if (in0 > 0 && in1 > 0) {
            cost += static_cast<int>(this->hyprgraph.get_net_weight(net));
        }
    }
    return cost;
}

template <typename Gnl>
void MidLvlPartMgr<Gnl>::optimize(std::span<std::uint8_t> part) {
    const auto num_modules = static_cast<int>(this->hyprgraph.number_of_modules());
    const auto half_bits = num_modules / 2;
    const auto total_bits = 2 * half_bits + 1;

    this->total_cost = this->compute_total_cost(part);
    auto best_part = std::vector<std::uint8_t>(part.begin(), part.end());
    auto best_cost = this->total_cost;

    // Initialize current_part from the HamCycle's starting vertex
    auto current_part = std::vector<std::uint8_t>(num_modules, 0);
    for (auto i = 0; i < half_bits && i < num_modules; ++i) {
        current_part[i] = 1;
    }
    auto current_cost = this->compute_total_cost(current_part);

    // Initialize constraint manager with the HamCycle starting state
    this->constr_mgr.init(current_part);

    const auto& hl = this->hyprgraph;
    std::vector<int> init_bits(total_bits, 0);
    for (auto i = 0; i < half_bits; ++i) {
        init_bits[i] = 1;
    }

    auto visit_fn = [&](const std::vector<int>& bits, int flipped_pos) {
        if (flipped_pos >= num_modules) {
            return;
        }
        const auto to_part = static_cast<std::uint8_t>(bits[flipped_pos]);
        const auto from_part = static_cast<std::uint8_t>(1 - to_part);
        const auto v = static_cast<node_t>(flipped_pos);
        const auto move_info_v = MoveInfoV<node_t>{v, from_part, to_part};

        // Check if the resulting partition satisfies balance constraints
        const auto legal = this->constr_mgr.check_legal(move_info_v);

        int delta = 0;
        for (const auto& net : hl.gr[v]) {
            int count_from = 0;
            int count_to = 0;
            for (const auto& other_v : hl.gr[net]) {
                if (other_v == v) {
                    continue;
                }
                if (current_part[other_v] == from_part) {
                    ++count_from;
                } else if (current_part[other_v] == to_part) {
                    ++count_to;
                }
            }
            const auto weight = static_cast<int>(hl.get_net_weight(net));
            const bool before_cut = (count_to > 0);
            const bool after_cut = (count_from > 0);
            if (!before_cut && after_cut) {
                delta += weight;
            } else if (before_cut && !after_cut) {
                delta -= weight;
            }
        }

        current_cost += delta;
        current_part[v] = to_part;
        this->constr_mgr.update_move(move_info_v);

        // Only snapshot when balance constraints are satisfied
        if (legal == LegalCheck::AllSatisfied && current_cost < best_cost) {
            best_cost = current_cost;
            best_part = current_part;
        }
    };

    MidVertex start_vertex(init_bits);
    MidHamCycle{start_vertex, -1, visit_fn};

    for (auto i = 0; i < num_modules; ++i) {
        part[i] = best_part[i];
    }
    this->total_cost = best_cost;
}

template class MidLvlPartMgr<SimpleNetlist>;
