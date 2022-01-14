#pragma once

// **Special code for two-pin nets**
// Take a snapshot when a move make **negative** gain.
// Snapshot in the form of "interface"???

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint8_t

#include <algorithm>  // for copy
#include <gsl/span>   // for span
#include <vector>     // for vector
// #include <range/v3/algorithm/copy.hpp>

#include "PartMgrBase.hpp"  // for PartMgrBase, SimpleNetlist

template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
class FMPartMgr;

/**
 * @brief FM Partition Manager
 *
 * @tparam GainMgr
 * @tparam ConstrMgr
 */
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
class FMPartMgr : public PartMgrBase<Gnl, GainMgr, ConstrMgr, FMPartMgr> {
    using Base = PartMgrBase<Gnl, GainMgr, ConstrMgr, FMPartMgr>;

  public:
    /**
     * @brief Construct a new FMPartMgr object
     *
     * @param[in] H
     * @param[in,out] gainMgr
     * @param[in,out] constrMgr
     * @param[in] K
     */
    FMPartMgr(const Gnl& H, GainMgr& gainMgr, ConstrMgr& constrMgr, size_t K)
        : Base{H, gainMgr, constrMgr, K} {}

    /**
     * @brief Construct a new FMPartMgr object
     *
     * @param[in] H
     * @param[in,out] gainMgr
     * @param[in,out] constrMgr
     */
    FMPartMgr(const Gnl& H, GainMgr& gainMgr, ConstrMgr& constrMgr)
        : Base{H, gainMgr, constrMgr, 2} {}

    /**
     * @brief
     *
     * @param[in] part
     * @return std::vector<std::uint8_t>
     */
    auto take_snapshot(gsl::span<const std::uint8_t> part) -> std::vector<std::uint8_t> {
        // const auto N = part.size();
        // auto snapshot = std::vector<std::uint8_t>(N, 0U);
        // // snapshot.reserve(N);
        // for (auto i = 0U; i != N; ++i)
        // {
        //     snapshot[i] = part[i];
        // }
        auto snapshot = std::vector<std::uint8_t>(part.begin(), part.end());
        return snapshot;
    }

    /**
     * @brief
     *
     * @param[in] snapshot
     * @param[in,out] part
     */
    auto restore_part(const std::vector<std::uint8_t>& snapshot, gsl::span<std::uint8_t> part)
        -> void {
        std::copy(snapshot.begin(), snapshot.end(), part.begin());
        // const auto N = part.size();
        // for (auto i = 0U; i != N; ++i)
        // {
        //     part[i] = snapshot[i];
        // }
    }
};
