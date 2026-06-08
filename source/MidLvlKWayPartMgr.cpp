#include <ckpttn/MidLvlKWayPartMgr.hpp>

#include <ckpttn/FMKWayConstrMgr.hpp>
#include <ckpttn/MidLvlPartMgr.hpp>
#include <ckpttn/midlevel/hamcycle.hpp>
#include <ckpttn/midlevel/vertex.hpp>
#include <ckpttn/moveinfo.hpp>
#include <cstdint>
#include <netlistx/netlist.hpp>
#include <span>
#include <vector>

MidLvlKWayPartMgr::MidLvlKWayPartMgr(double bal_tol, std::uint8_t num_parts)
    : bal_tol_{bal_tol}, num_parts_{num_parts} {}

static auto compute_kway_cost(const SimpleNetlist& hl, std::span<const std::uint8_t> part) -> int {
    auto cost = 0;
    for (const auto& net : hl.nets) {
        auto seen = std::uint8_t{0};
        for (const auto& v : hl.gr[net]) {
            seen |= static_cast<std::uint8_t>(1U << part[v]);
        }
        auto num_parts = 0;
        for (auto p = 0U; p < 8U; ++p) {
            if (seen & (1U << p)) {
                ++num_parts;
            }
        }
        if (num_parts > 1) {
            cost += static_cast<int>(hl.get_net_weight(net));
        }
    }
    return cost;
}

void MidLvlKWayPartMgr::optimize(std::span<std::uint8_t> part,
                                 const SimpleNetlist& hyprgraph) {
    const auto total_modules = hyprgraph.number_of_modules();
    auto current_part = std::vector<std::uint8_t>(part.begin(), part.end());

    // Create constraint manager for balance tracking
    auto constr_mgr = FMKWayConstrMgr<SimpleNetlist>(hyprgraph, this->bal_tol_, this->num_parts_);

    auto improved = true;
    auto pass = 0;

    while (improved && pass < max_passes) {
        improved = false;
        ++pass;

        for (auto i = 0U; i < num_parts_ - 1U; ++i) {
            for (auto j = i + 1U; j < num_parts_; ++j) {
                auto selected = std::vector<std::uint32_t>{};
                for (auto v = 0U; v < total_modules; ++v) {
                    if (current_part[v] == i || current_part[v] == j) {
                        selected.push_back(static_cast<std::uint32_t>(v));
                    }
                }
                if (selected.size() <= 1 || selected.size() > max_pair_modules) {
                    continue;
                }

                const auto num_modules = static_cast<int>(selected.size());
                const auto half_bits = num_modules / 2;
                const auto total_bits = 2 * half_bits + 1;

                auto init_bits = std::vector<int>(total_bits, 0);
                auto init_part = std::vector<std::uint8_t>(num_modules, 0);
                for (auto pos = 0; pos < num_modules; ++pos) {
                    auto v = selected[pos];
                    init_part[pos] = (current_part[v] == j) ? 1U : 0U;
                    if (pos < half_bits) {
                        init_bits[pos] = 1;
                    }
                }

                // Initialize constraint manager to match current partition state
                constr_mgr.init(current_part);

                auto best_part = init_part;
                auto best_cost = compute_kway_cost(hyprgraph, current_part);

                auto local_part = init_part;
                auto local_cost = best_cost;

                const auto& hl = hyprgraph;
                auto visit_fn = [&](const std::vector<int>& bits, int flipped_pos) {
                    if (flipped_pos >= num_modules) {
                        return;
                    }
                    const auto v = selected[flipped_pos];
                    const auto to_part = static_cast<std::uint8_t>(
                        bits[flipped_pos] == 0 ? i : j);
                    const auto from_part = static_cast<std::uint8_t>(
                        bits[flipped_pos] == 0 ? j : i);

                    const auto move_info_v = MoveInfoV<SimpleNetlist::node_t>{
                        v, from_part, to_part};

                    // Check balance constraints for this move
                    const auto legal = constr_mgr.check_legal(move_info_v);

                    int delta = 0;
                    for (const auto& net : hl.gr[v]) {
                        int cnt_from = 0;
                        int cnt_to = 0;
                        int cnt_other = 0;
                        for (const auto& w : hl.gr[net]) {
                            if (w == v) {
                                continue;
                            }
                            if (current_part[w] == from_part) {
                                ++cnt_from;
                            } else if (current_part[w] == to_part) {
                                ++cnt_to;
                            } else {
                                ++cnt_other;
                            }
                        }
                        const auto wt = static_cast<int>(hl.get_net_weight(net));
                        const bool before = (cnt_to > 0 || cnt_other > 0);
                        const bool after = (cnt_from > 0 || cnt_other > 0);
                        if (!before && after) {
                            delta += wt;
                        } else if (before && !after) {
                            delta -= wt;
                        }
                    }

                    local_cost += delta;
                    current_part[v] = to_part;
                    local_part[flipped_pos] = static_cast<std::uint8_t>(
                        bits[flipped_pos] == 0 ? 0U : 1U);
                    constr_mgr.update_move(move_info_v);

                    // Only snapshot when balance constraints are satisfied
                    if (legal == LegalCheck::AllSatisfied && local_cost < best_cost) {
                        best_cost = local_cost;
                        best_part = local_part;
                    }
                };

                MidVertex start_vertex(init_bits);
                MidHamCycle{start_vertex, -1, visit_fn};

                for (auto pos = 0; pos < num_modules; ++pos) {
                    auto v = selected[pos];
                    current_part[v] = (best_part[pos] == 1U)
                                          ? static_cast<std::uint8_t>(j)
                                          : static_cast<std::uint8_t>(i);
                }

                if (best_cost < static_cast<int>(this->total_cost) || pass == 1) {
                    this->total_cost = best_cost;
                    improved = true;
                }
            }
        }
    }

    for (auto v = 0U; v < total_modules; ++v) {
        part[v] = current_part[v];
    }
}
