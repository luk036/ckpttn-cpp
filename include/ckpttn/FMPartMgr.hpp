#pragma once

// **Special code for two-pin nets**
// Take a snapshot when a move make **negative** gain.
// Snapshot in the form of "interface"???

#include <cstddef>  // for size_t

#include "PartMgrBase.hpp"  // for PartMgrBase, SimpleNetlist

// forward declaration
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
class FMPartMgr;

/**
 * @brief FM Partition Manager
 *
 * @tparam Gnl
 * @tparam GainMgr
 * @tparam ConstrMgr
 */
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
class FMPartMgr : public PartMgrBase<Gnl, GainMgr, ConstrMgr> {
    using Base = PartMgrBase<Gnl, GainMgr, ConstrMgr>;

  public:
    /**
     * @brief Construct a new FMPartMgr object
     *
     * @param[in] hgr
     * @param[in,out] gain_mgr
     * @param[in,out] constr_mgr
     * @param[in] num_parts
     */
    FMPartMgr(const Gnl& hgr, GainMgr& gain_mgr, ConstrMgr& constr_mgr, size_t num_parts)
        : Base{hgr, gain_mgr, constr_mgr, num_parts} {}

    /**
     * @brief Construct a new FMPartMgr object
     *
     * @param[in] hgr
     * @param[in,out] gain_mgr
     * @param[in,out] constr_mgr
     */
    FMPartMgr(const Gnl& hgr, GainMgr& gain_mgr, ConstrMgr& constr_mgr)
        : Base{hgr, gain_mgr, constr_mgr, 2} {}

    // /**
    //  * @brief
    //  *
    //  * @param[in] part
    //  * @return std::vector<std::uint8_t>
    //  */
    // auto take_snapshot(gsl::span<const std::uint8_t> part) -> std::vector<std::uint8_t> {
    //     // const auto N = part.size();
    //     // auto snapshot = std::vector<std::uint8_t>(N, 0U);
    //     // // snapshot.reserve(N);
    //     // for (auto i = 0U; i != N; ++i)
    //     // {
    //     //     snapshot[i] = part[i];
    //     // }
    //     auto snapshot = std::vector<std::uint8_t>(part.begin(), part.end());
    //     return snapshot;
    // }

    // /**
    //  * @brief
    //  *
    //  * @param[in] snapshot
    //  * @param[in,out] part
    //  */
    // auto restore_part(const std::vector<std::uint8_t>& snapshot, gsl::span<std::uint8_t> part)
    //     -> void {
    //     std::copy(snapshot.begin(), snapshot.end(), part.begin());
    //     // const auto N = part.size();
    //     // for (auto i = 0U; i != N; ++i)
    //     // {
    //     //     part[i] = snapshot[i];
    //     // }
    // }
};
