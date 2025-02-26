// #include <__config>   // for std
#include <cstdint>  // for uint32_t, uint8_t

// #include <__config>                // for std
#include <algorithm>               // for fill
#include <ckpttn/FMConstrMgr.hpp>  // for FMConstrMgr, LegalCheck, move_info_v
#include <ckpttn/moveinfo.hpp>     // for MoveInfoV
#include <cmath>                   // for round
#include <span>                    // for span
#include <transrangers.hpp>
#include <vector>  // for vector<>::iterator, vector

using namespace std;

/** This is the constructor of the `FMConstrMgr` class template. It initializes the object with the
given parameters `hyprgraph`, `bal_tol`, and `num_parts`. */
template <typename Gnl>
FMConstrMgr<Gnl>::FMConstrMgr(const Gnl &hyprgraph, double bal_tol, uint8_t num_parts)
    : hyprgraph{hyprgraph}, bal_tol{bal_tol}, diff(num_parts, 0), num_parts{num_parts} {
    using namespace transrangers;
    this->total_weight = accumulate(
        transform([&](const auto &v) { return hyprgraph.get_module_weight(v); }, all(hyprgraph)),
        0U);
    // this->total_weight = 0U;
    // for (const auto &v : hyprgraph) {
    //   this->total_weight += hyprgraph.get_module_weight(v);
    // }
    const auto totalweightK = this->total_weight * (2.0 / this->num_parts);
    this->lowerbound = uint32_t(round(totalweightK * this->bal_tol));
}

/**
 * The `init` function in the `FMConstrMgr` class template initializes the `diff` vector based on
 * the given `part` vector.
 *
 * @param[in] part
 */
template <typename Gnl> void FMConstrMgr<Gnl>::init(std::span<const uint8_t> part) {
    fill(this->diff.begin(), this->diff.end(), 0);
    for (const auto &v : this->hyprgraph) {
        // auto weight_v = this->hyprgraph.get_module_weight(v);
        this->diff[part[v]] += this->hyprgraph.get_module_weight(v);
    }
}

/**
 * The code snippet is defining the `check_legal` function in the `FMConstrMgr` class template. This
 * function takes a `MoveInfoV` object as input and returns a `LegalCheck` enum value.
 *
 * @param[in] move_info_v
 * @return LegalCheck
 */
template <typename Gnl> auto FMConstrMgr<Gnl>::check_legal(
    const MoveInfoV<typename Gnl::node_t> &move_info_v) -> LegalCheck {
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
 * The code snippet is defining the `check_constraints` function in the `FMConstrMgr` class
 * template. This function takes a `MoveInfoV` object as input and returns a boolean value.
 *
 * @param[in] move_info_v
 * @return true
 * @return false
 */
template <typename Gnl> auto FMConstrMgr<Gnl>::check_constraints(
    const MoveInfoV<typename Gnl::node_t> &move_info_v) -> bool {
    // const auto& [v, from_part, to_part] = move_info_v;

    this->weight = this->hyprgraph.get_module_weight(move_info_v.v);
    // auto diffTo = this->diff[to_part] + this->weight;
    const auto diffFrom = this->diff[move_info_v.from_part];
    return diffFrom >= this->lowerbound + this->weight;
}

/**
 * The code snippet is defining the `update_move` function in the `FMConstrMgr` class template. This
 * function takes a `MoveInfoV` object as input and updates the `diff` vector based on the move
 * information. It increases the weight of the destination part by the weight of the moved node and
 * decreases the weight of the source part by the weight of the moved node.
 *
 * @param[in] move_info_v
 */
template <typename Gnl>
void FMConstrMgr<Gnl>::update_move(const MoveInfoV<typename Gnl::node_t> &move_info_v) {
    // auto [v, from_part, to_part] = move_info_v;
    this->diff[move_info_v.to_part] += this->weight;
    this->diff[move_info_v.from_part] -= this->weight;
}

// Instantiation
#include <ckpttn/netlist.hpp>  // for Netlist, SimpleNetlist
#include <py2cpp/range.hpp>    // for _iterator

template class FMConstrMgr<SimpleNetlist>;
