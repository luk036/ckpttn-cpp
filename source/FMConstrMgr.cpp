// #include <__config>   // for std
#include <cstdint>  // for uint32_t, uint8_t

// #include <__config>                // for std
#include <algorithm>               // for fill
#include <ckpttn/FMConstrMgr.hpp>  // for FMConstrMgr, LegalCheck, move_info_v
#include <ckpttn/moveinfo.hpp>     // for MoveInfoV
#include <cmath>                   // for round
#include <span>                    // for span
// #include <transrangers.hpp>
#include <vector>  // for vector<>::iterator, vector

using namespace std;

/**
 * @brief Constructs a new FMConstrMgr object.
 *
 * Computes the total module weight and the lower bound for each partition
 * based on the balance tolerance.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] hyprgraph The hypergraph to manage constraints for
 * @param[in] bal_tol The balance tolerance
 * @param[in] num_parts The number of partitions
 */
template <typename Gnl>
FMConstrMgr<Gnl>::FMConstrMgr(const Gnl& hyprgraph, double bal_tol, uint8_t num_parts)
    : hyprgraph{hyprgraph}, bal_tol{bal_tol}, diff(num_parts, 0), num_parts{num_parts} {
    for (const auto& v : hyprgraph) {
        this->total_weight += hyprgraph.get_module_weight(v);
    }
    const auto totalweightK = this->total_weight * (2.0 / this->num_parts);
    this->lowerbound = uint32_t(round(totalweightK * this->bal_tol));
}

/**
 * @brief Initializes the constraint manager with the given partition.
 *
 * Resets the `diff` vector and accumulates module weights for each partition.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] part The partition assignment to initialize from
 */
template <typename Gnl> void FMConstrMgr<Gnl>::init(std::span<const uint8_t> part) {
    std::ranges::fill(this->diff, 0);
    for (const auto& module : this->hyprgraph) {
        // auto weight_module = this->hyprgraph.get_module_weight(module);
        this->diff[part[module]] += this->hyprgraph.get_module_weight(module);
    }
}

/**
 * @brief Checks if a proposed move is legal and whether it improves balance.
 *
 * Returns NotSatisfied if the source partition would drop below the lower bound,
 * GetBetter if the move improves balance but destination is still below bound,
 * or AllSatisfied if both partitions meet the balance constraints after the move.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] move_info_v Information about the proposed vertex move
 * @return LegalCheck The legality status of the proposed move
 */
template <typename Gnl>
auto FMConstrMgr<Gnl>::check_legal(const MoveInfoV<typename Gnl::node_t>& move_info_v)
    -> LegalCheck {
    this->weight = this->hyprgraph.get_module_weight(move_info_v.v);
    const auto diffFrom = this->diff[move_info_v.from_part];
    if (diffFrom < this->lowerbound + this->weight) {
        return LegalCheck::NotSatisfied;  // not ok, don't move
    }
    const auto diffTo = this->diff[move_info_v.to_part];
    if (diffTo + this->weight < this->lowerbound) {
        return LegalCheck::GetBetter;  // get better, but still illegal
    }
    return LegalCheck::AllSatisfied;  // all satisfied
}

/**
 * @brief Checks if a proposed move satisfies the minimum balance constraint.
 *
 * Returns true only if the source partition has enough weight to lose the
 * moved module without dropping below the lower bound.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] move_info_v Information about the proposed vertex move
 * @return true if the move satisfies balance constraints
 * @return false if the source partition would become underweight
 */
template <typename Gnl>
auto FMConstrMgr<Gnl>::check_constraints(const MoveInfoV<typename Gnl::node_t>& move_info_v)
    -> bool {
    // const auto& [v, from_part, to_part] = move_info_v;

    this->weight = this->hyprgraph.get_module_weight(move_info_v.v);
    // auto diffTo = this->diff[to_part] + this->weight;
    const auto diffFrom = this->diff[move_info_v.from_part];
    return diffFrom >= this->lowerbound + this->weight;
}

/**
 * @brief Updates the partition weight differences after a move.
 *
 * Increases the weight of the destination partition and decreases the
 * weight of the source partition by the module weight of the moved vertex.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] move_info_v Information about the performed vertex move
 */
template <typename Gnl>
void FMConstrMgr<Gnl>::update_move(const MoveInfoV<typename Gnl::node_t>& move_info_v) {
    // auto [v, from_part, to_part] = move_info_v;
    this->diff[move_info_v.to_part] += this->weight;
    this->diff[move_info_v.from_part] -= this->weight;
}

/**
 * @brief Performs a final check that all partitions satisfy balance constraints.
 *
 * Initializes the `diff` vector and verifies all partitions meet or exceed
 * the lower bound weight.
 *
 * @tparam Gnl The hypergraph type
 * @param[in] part The partition assignment to check
 * @return true if all partitions satisfy the balance constraints
 * @return false if any partition is below the lower bound
 */
template <typename Gnl> auto FMConstrMgr<Gnl>::final_check(std::span<const uint8_t> part) -> bool {
    this->init(part);
    for (const auto& localdiff : this->diff) {
        if (localdiff < this->lowerbound) {
            return false;
        }
    }
    return true;
}

// Instantiation
#include <netlistx/netlist.hpp>  // for Netlist, SimpleNetlist
#include <py2cpp/range.hpp>      // for _iterator

template class FMConstrMgr<SimpleNetlist>;
