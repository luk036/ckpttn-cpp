// #include <__config>   // for std
#include <cstdint> // for uint32_t, uint8_t

// #include <__config>                // for std
#include <algorithm>              // for fill
#include <ckpttn/FMConstrMgr.hpp> // for FMConstrMgr, LegalCheck, move_info_v
#include <ckpttn/moveinfo.hpp>    // for MoveInfoV
#include <cmath>                  // for round
#include <gsl/span>               // for span
#include <transrangers.hpp>
#include <vector> // for vector<>::iterator, vector

using namespace std;

template <typename Gnl>
FMConstrMgr<Gnl>::FMConstrMgr(const Gnl &hgr, double bal_tol, uint8_t num_parts)
    : hgr{hgr}, bal_tol{bal_tol}, diff(num_parts, 0), num_parts{num_parts} {
  using namespace transrangers;
  this->totalweight = accumulate(
      transform([&](const auto &v) { return hgr.get_module_weight(v); },
                all(hgr)),
      0U);
  // this->totalweight = 0U;
  // for (const auto &v : hgr) {
  //   this->totalweight += hgr.get_module_weight(v);
  // }
  const auto totalweightK = this->totalweight * (2.0 / this->num_parts);
  this->lowerbound = uint32_t(round(totalweightK * this->bal_tol));
}

/**
 * @brief
 *
 * @param[in] part
 */
template <typename Gnl>
void FMConstrMgr<Gnl>::init(gsl::span<const uint8_t> part) {
  fill(this->diff.begin(), this->diff.end(), 0);
  for (const auto &v : this->hgr) {
    // auto weight_v = this->hgr.get_module_weight(v);
    this->diff[part[v]] += this->hgr.get_module_weight(v);
  }
}

/**
 * @brief
 *
 * @param[in] move_info_v
 * @return LegalCheck
 */
template <typename Gnl>
auto FMConstrMgr<Gnl>::check_legal(
    const MoveInfoV<typename Gnl::node_t> &move_info_v) -> LegalCheck {
  this->weight = this->hgr.get_module_weight(move_info_v.v);
  const auto diffFrom = this->diff[move_info_v.from_part];
  if (diffFrom < this->lowerbound + this->weight) {
    return LegalCheck::NotSatisfied; // not ok, don't move
  }
  const auto diffTo = this->diff[move_info_v.to_part];
  if (diffTo + this->weight < this->lowerbound) {
    return LegalCheck::GetBetter; // get better, but still illegal
  }
  return LegalCheck::AllSatisfied; // all satisfied
}

/**
 * @brief
 *
 * @param[in] move_info_v
 * @return true
 * @return false
 */
template <typename Gnl>
auto FMConstrMgr<Gnl>::check_constraints(
    const MoveInfoV<typename Gnl::node_t> &move_info_v) -> bool {
  // const auto& [v, from_part, to_part] = move_info_v;

  this->weight = this->hgr.get_module_weight(move_info_v.v);
  // auto diffTo = this->diff[to_part] + this->weight;
  const auto diffFrom = this->diff[move_info_v.from_part];
  return diffFrom >= this->lowerbound + this->weight;
}

/**
 * @brief
 *
 * @param[in] move_info_v
 */
template <typename Gnl>
void FMConstrMgr<Gnl>::update_move(
    const MoveInfoV<typename Gnl::node_t> &move_info_v) {
  // auto [v, from_part, to_part] = move_info_v;
  this->diff[move_info_v.to_part] += this->weight;
  this->diff[move_info_v.from_part] -= this->weight;
}

// Instantiation
#include <ckpttn/netlist.hpp> // for Netlist, SimpleNetlist
#include <py2cpp/range.hpp>   // for _iterator

template class FMConstrMgr<SimpleNetlist>;
