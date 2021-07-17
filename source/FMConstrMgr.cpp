#include <ckpttn/FMConstrMgr.hpp>
#include <ckpttn/netlist.hpp> // import Netlist
#include <transrangers.hpp>

FMConstrMgr::FMConstrMgr(const SimpleNetlist& H, double BalTol, std::uint8_t K)
    : H {H}
    , BalTol {BalTol}
    , diff(K, 0)
    , K {K}
{
    using namespace transrangers;
    this->totalweight = accumulate(transform([&](const auto& v){
        return H.get_module_weight(v); }, all(H)), 0U);
    const auto totalweightK = this->totalweight * (2.0 / this->K);
    this->lowerbound = uint32_t(std::round(totalweightK * this->BalTol));
}

/**
 * @brief
 *
 * @param[in] part
 */
void FMConstrMgr::init(gsl::span<const std::uint8_t> part)
{
    std::fill(this->diff.begin(), this->diff.end(), 0);
    for (const auto& v : this->H)
    {
        // auto weight_v = this->H.get_module_weight(v);
        this->diff[part[v]] += this->H.get_module_weight(v);
    }
}

/**
 * @brief
 *
 * @param[in] move_info_v
 * @return LegalCheck
 */
auto FMConstrMgr::check_legal(const MoveInfoV<node_t>& move_info_v)
    -> LegalCheck
{
    this->weight = this->H.get_module_weight(move_info_v.v);
    const auto diffFrom = this->diff[move_info_v.fromPart];
    if (diffFrom < this->lowerbound + this->weight)
    {
        return LegalCheck::notsatisfied; // not ok, don't move
    }
    const auto diffTo = this->diff[move_info_v.toPart];
    if (diffTo + this->weight < this->lowerbound)
    {
        return LegalCheck::getbetter; // get better, but still illegal
    }
    return LegalCheck::allsatisfied; // all satisfied
}

/**
 * @brief
 *
 * @param[in] move_info_v
 * @return true
 * @return false
 */
auto FMConstrMgr::check_constraints(const MoveInfoV<node_t>& move_info_v)
    -> bool
{
    // const auto& [v, fromPart, toPart] = move_info_v;

    this->weight = this->H.get_module_weight(move_info_v.v);
    // auto diffTo = this->diff[toPart] + this->weight;
    const auto diffFrom = this->diff[move_info_v.fromPart];
    return diffFrom >= this->lowerbound + this->weight;
}

/**
 * @brief
 *
 * @param[in] move_info_v
 */
void FMConstrMgr::update_move(const MoveInfoV<node_t>& move_info_v)
{
    // auto [v, fromPart, toPart] = move_info_v;
    this->diff[move_info_v.toPart] += this->weight;
    this->diff[move_info_v.fromPart] -= this->weight;
}
