#include <ckpttn/MidLvlPartMgr.hpp>

#include <ckpttn/FMPmrConfig.hpp>
#include <ckpttn/midlevel/hamcycle.hpp>
#include <ckpttn/midlevel/vertex.hpp>
#include <ckpttn/moveinfo.hpp>
#include <netlistx/netlist.hpp>
#include <span>
#include <vector>

template <typename Gnl>
MidLvlPartMgr<Gnl>::MidLvlPartMgr(const Gnl& hyprgraph, double bal_tol)
    : hyprgraph{hyprgraph}, gain_calc{hyprgraph, 2}, constr_mgr{hyprgraph, bal_tol} {}

template <typename Gnl>
void MidLvlPartMgr<Gnl>::optimize(std::span<std::uint8_t> part) {
    const auto num_modules = static_cast<int>(this->hyprgraph.number_of_modules());
    const auto half_bits = num_modules / 2;
    const auto total_bits = 2 * half_bits + 1;

    auto current_part = std::vector<std::uint8_t>(num_modules, 0);
    for (auto i = 0; i < half_bits && i < num_modules; ++i) {
        current_part[i] = 1;
    }

    auto current_cost = this->gain_calc.init(current_part);
    this->constr_mgr.init(current_part);

    auto current_gain = std::vector<int>(this->gain_calc.get_init_gain_list());

    auto best_part = current_part;
    auto best_cost = current_cost;

    std::vector<int> init_bits(total_bits, 0);
    for (auto i = 0; i < half_bits; ++i) {
        init_bits[i] = 1;
    }

    auto& gc = this->gain_calc;

    auto visit_fn = [&](const std::vector<int>& bits, int flipped_pos) {
        if (flipped_pos >= num_modules) {
            return;
        }
        const auto to_part = static_cast<std::uint8_t>(bits[flipped_pos]);
        const auto from_part = static_cast<std::uint8_t>(1 - to_part);
        const auto v = static_cast<node_t>(flipped_pos);
        const auto move_info_v = MoveInfoV<node_t>{v, from_part, to_part};

        const auto legal = this->constr_mgr.check_legal(move_info_v);
        const auto gain = current_gain[v];
        current_cost -= gain;

        for (const auto& net : this->hyprgraph.gr[v]) {
            const auto degree = this->hyprgraph.gr.degree(net);
            if (degree < 2 || degree > FM_MAX_DEGREE) {
                continue;
            }
            const auto move_info = MoveInfo<node_t>{net, v, from_part, to_part};
            if (degree == 2) {
                const auto w = gc.update_move_2pin_net(current_part, move_info);
                current_gain[w] += gc.delta_gain_w;
            } else {
                gc.init_idx_vec(v, net);
                if (degree == 3) {
                    const auto deltas = gc.update_move_3pin_net(current_part, move_info);
                    for (size_t i = 0; i < gc.idx_vec.size(); ++i) {
                        current_gain[gc.idx_vec[i]] += deltas[i];
                    }
                } else {
                    const auto deltas = gc.update_move_general_net(current_part, move_info);
                    for (size_t i = 0; i < gc.idx_vec.size(); ++i) {
                        current_gain[gc.idx_vec[i]] += deltas[i];
                    }
                }
            }
        }

        current_gain[v] = -gain;
        this->constr_mgr.update_move(move_info_v);
        current_part[v] = to_part;

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
