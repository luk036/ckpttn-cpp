#pragma once

// **Special code for two-pin nets**
// Take a snapshot when a move make **negative** gain.
// Snapshot in the form of "interface"???
#include "PartMgrBase.hpp"
#include <gsl/span>
#include <range/v3/algorithm/copy.hpp>

/*!
 * @brief FM Partition Manager
 *
 * @tparam GainMgr
 * @tparam ConstrMgr
 */
template <typename GainMgr, typename ConstrMgr> //
class FMPartMgr : public PartMgrBase<GainMgr, ConstrMgr, FMPartMgr>
{
    using Base = PartMgrBase<GainMgr, ConstrMgr, FMPartMgr>;

  public:
    /*!
     * @brief Construct a new FMPartMgr object
     *
     * @param[in] H
     * @param[in,out] gainMgr
     * @param[in,out] constrMgr
     */
    FMPartMgr(const SimpleNetlist& H, GainMgr& gainMgr, ConstrMgr& constrMgr,
        size_t K)
        : Base {H, gainMgr, constrMgr, K}
    {
    }

    /*!
     * @brief Construct a new FMPartMgr object
     *
     * @param[in] H
     * @param[in,out] gainMgr
     * @param[in,out] constrMgr
     */
    FMPartMgr(const SimpleNetlist& H, GainMgr& gainMgr, ConstrMgr& constrMgr)
        : Base {H, gainMgr, constrMgr, 2}
    {
    }

    /*!
     * @brief
     *
     * @param[in] part
     * @return Snapshot
     */
    auto take_snapshot(gsl::span<const std::uint8_t> part)
        -> std::vector<std::uint8_t>
    {
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

    /*!
     * @brief
     *
     * @param[in] snapshot
     * @param[in,out] part
     */
    auto restore_part(const std::vector<std::uint8_t>& snapshot,
        gsl::span<std::uint8_t> part) -> void
    {
        ranges::copy(snapshot, part.begin());
        // const auto N = part.size();
        // for (auto i = 0U; i != N; ++i)
        // {
        //     part[i] = snapshot[i];
        // }
    }
};
