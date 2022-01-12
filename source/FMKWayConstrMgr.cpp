// #include <algorithm> // import std::any_of()
#include <algorithm>  // for min_element
#include <ckpttn/FMKWayConstrMgr.hpp>
#include <iterator>  // for distance

#include "ckpttn/FMConstrMgr.hpp"  // for LegalCheck, LegalCheck::allsatisfied
#include "ckpttn/moveinfo.hpp"     // for MoveInfo
// #include <range/v3/algorithm/any_of.hpp>

/**
 * @brief
 *
 * @return std::uint8_t
 */
auto FMKWayConstrMgr::select_togo() const -> std::uint8_t {
    auto it = std::min_element(this->diff.cbegin(), this->diff.cend());
    return std::uint8_t(std::distance(this->diff.cbegin(), it));
}

/**
 * @brief
 *
 * @param[in] move_info_v
 * @return LegalCheck
 */
auto FMKWayConstrMgr::check_legal(const MoveInfoV<node_t>& move_info_v) -> LegalCheck {
    const auto status = FMConstrMgr::check_legal(move_info_v);
    if (status != LegalCheck::allsatisfied) {
        return status;
    }
    this->illegal[move_info_v.fromPart] = 0;
    this->illegal[move_info_v.toPart] = 0;
    for (const auto& value : this->illegal) {
        if (value == 1) {
            return LegalCheck::getbetter;  // get better, but still illegal
        }
    }
    return LegalCheck::allsatisfied;  // all satisfied
}
