// #include <algorithm> // import std::any_of()
#include <algorithm>                   // for min_element
#include <ckpttn/FMKWayConstrMgr.hpp>  // for FMKWayConstrMgr, move_info_v
#include <cstdint>                     // for uint8_t
#include <iterator>                    // for distance

#include "ckpttn/FMConstrMgr.hpp"  // for LegalCheck, LegalCheck::allsat...
#include "ckpttn/moveinfo.hpp"     // for MoveInfoV

/**
 * @brief Selects the partition with the minimum weight as the target for moves.
 *
 * @tparam Gnl The hypergraph type
 * @return The index of the partition with the minimum current weight
 */
template <typename Gnl> auto FMKWayConstrMgr<Gnl>::select_togo() const -> std::uint8_t {
    auto it = std::min_element(this->diff.cbegin(), this->diff.cend());
    return static_cast<std::uint8_t>(std::distance(this->diff.cbegin(), it));
}

/**
 * @brief Checks if a proposed move is legal for k-way partitioning.
 *
 * Delegates to the base class for balance checking, then additionally
 * verifies that all partitions have been brought to legality (no remaining
 * illegal partitions).
 *
 * @tparam Gnl The hypergraph type
 * @param[in] move_info_v Information about the proposed vertex move
 * @return LegalCheck The legality status of the proposed move
 */
template <typename Gnl>
auto FMKWayConstrMgr<Gnl>::check_legal(const MoveInfoV<typename Gnl::node_t>& move_info_v)
    -> LegalCheck {
    const auto status = FMConstrMgr<Gnl>::check_legal(move_info_v);
    if (status != LegalCheck::AllSatisfied) {
        return status;
    }
    this->illegal[move_info_v.from_part] = 0;
    this->illegal[move_info_v.to_part] = 0;
    for (const auto& value : this->illegal) {
        if (value == 1) {
            return LegalCheck::GetBetter;  // get better, but still illegal
        }
    }
    return LegalCheck::AllSatisfied;  // all satisfied
}

// Instantiation
#include <netlistx/netlist.hpp>  // for Netlist, SimpleNetlist

template class FMKWayConstrMgr<SimpleNetlist>;
