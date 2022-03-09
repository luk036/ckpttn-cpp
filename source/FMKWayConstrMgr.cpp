// #include <algorithm> // import std::any_of()
#include <stdint.h>  // for uint8_t

#include <algorithm>                   // for min_element
#include <ckpttn/FMKWayConstrMgr.hpp>  // for FMKWayConstrMgr, move_info_v
#include <gsl/gsl_util>                // for narrow_cast
#include <iterator>                    // for distance
#include <vector>                      // for vector

#include "ckpttn/FMConstrMgr.hpp"  // for LegalCheck, LegalCheck::allsat...
#include "ckpttn/moveinfo.hpp"     // for MoveInfoV

/**
 * @brief
 *
 * @return std::uint8_t
 */
template <typename Gnl> auto FMKWayConstrMgr<Gnl>::select_togo() const -> std::uint8_t {
    auto it = std::min_element(this->diff.cbegin(), this->diff.cend());
    return gsl::narrow_cast<std::uint8_t>(std::distance(this->diff.cbegin(), it));
}

/**
 * @brief
 *
 * @param[in] move_info_v
 * @return LegalCheck
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
#include <ckpttn/netlist.hpp>  // for Netlist, SimpleNetlist

template class FMKWayConstrMgr<SimpleNetlist>;