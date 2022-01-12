#include <cassert>                 // for assert
#include <ckpttn/FMConstrMgr.hpp>  // for LegalCheck, LegalCheck::notsat...
#include <ckpttn/PartMgrBase.hpp>  // for PartMgrBase, part, SimpleNetlist
#include <ckpttn/moveinfo.hpp>     // for MoveInfoV
#include <ckpttn/netlist.hpp>      // for Netlist
#include <cstdint>                 // for uint8_t
#include <gsl/span>                // for span
#include <py2cpp/range.hpp>        // for _iterator
#include <py2cpp/set.hpp>          // for set
#include <tuple>                   // for tuple_element<>::type
#include <tuple>                   // for get
#include <vector>                  // for vector

using node_t = typename SimpleNetlist::node_t;
// using namespace std;

/**
 * @brief
 *
 * @tparam GainMgr
 * @tparam ConstrMgr
 * @tparam Derived
 * @param[in] part
 */
template <typename GainMgr, typename ConstrMgr,
          template <typename _gainMgr, typename _constrMgr> class Derived>  //
void PartMgrBase<GainMgr, ConstrMgr, Derived>::init(gsl::span<std::uint8_t> part) {
    this->totalcost = this->gainMgr.init(part);
    this->validator.init(part);
}

/**
 * @brief
 *
 * @tparam GainMgr
 * @tparam ConstrMgr
 * @tparam Derived
 * @param[in] part
 * @return LegalCheck
 */
template <typename GainMgr, typename ConstrMgr,
          template <typename _gainMgr, typename _constrMgr> class Derived>  //
auto PartMgrBase<GainMgr, ConstrMgr, Derived>::legalize(gsl::span<std::uint8_t> part)
    -> LegalCheck {
    this->init(part);

    // Zero-weighted modules does not contribute legalization
    for (const auto& v : this->H) {
        if (this->H.get_module_weight(v) != 0U) {
            continue;
        }
        if (!this->H.module_fixed.contains(v)) {
            continue;
        }
        this->gainMgr.lock_all(part[v], v);
    }

    auto legalcheck = LegalCheck::notsatisfied;
    while (legalcheck != LegalCheck::allsatisfied) {
        const auto toPart = this->validator.select_togo();
        if (this->gainMgr.is_empty_togo(toPart)) {
            break;
        }
        const auto rslt = this->gainMgr.select_togo(toPart);
        auto&& v = get<0>(rslt);
        auto&& gainmax = get<1>(rslt);
        const auto fromPart = part[v];
        // assert(v == v);
        assert(fromPart != toPart);
        const auto move_info_v = MoveInfoV<node_t>{v, fromPart, toPart};
        // Check if the move of v can notsatisfied, makebetter, or satisfied
        legalcheck = this->validator.check_legal(move_info_v);
        if (legalcheck == LegalCheck::notsatisfied) {  // notsatisfied
            continue;
        }
        // Update v and its neigbours (even they are in waitinglist);
        // Put neigbours to bucket
        this->gainMgr.update_move(part, move_info_v);
        this->gainMgr.update_move_v(move_info_v, gainmax);
        this->validator.update_move(move_info_v);
        part[v] = toPart;
        // totalgain += gainmax;
        this->totalcost -= gainmax;
        assert(this->totalcost >= 0);
    }
    return legalcheck;
}

/**
 * @brief
 *
 * @tparam GainMgr
 * @tparam ConstrMgr
 * @tparam Derived
 * @param[in] part
 */
template <typename GainMgr, typename ConstrMgr,
          template <typename _gainMgr, typename _constrMgr> class Derived>  //
void PartMgrBase<GainMgr, ConstrMgr, Derived>::_optimize_1pass(gsl::span<std::uint8_t> part) {
    // using SS_t = decltype(self.take_snapshot(part));
    using SS_t = std::vector<std::uint8_t>;

    auto snapshot = SS_t{};
    auto totalgain = 0;
    auto deferredsnapshot = false;
    auto besttotalgain = 0;

    while (!this->gainMgr.is_empty()) {
        // Take the gainmax with v from gainbucket
        auto [move_info_v, gainmax] = this->gainMgr.select(part);

        // Check if the move of v can satisfied or notsatisfied
        const auto satisfiedOK = this->validator.check_constraints(move_info_v);
        if (!satisfiedOK) {
            continue;
        }
        if (gainmax < 0) {
            // become down turn
            if (!deferredsnapshot || totalgain > besttotalgain) {
                // Take a snapshot before move
                // snapshot = part;
                snapshot = self.take_snapshot(part);
                besttotalgain = totalgain;
            }
            deferredsnapshot = true;
        } else if (totalgain + gainmax >= besttotalgain) {
            besttotalgain = totalgain + gainmax;
            deferredsnapshot = false;
        }
        // Update v and its neigbours (even they are in waitinglist);
        // Put neigbours to bucket
        // const auto& [v, _, toPart] = move_info_v;
        this->gainMgr.lock(move_info_v.toPart, move_info_v.v);
        this->gainMgr.update_move(part, move_info_v);
        this->gainMgr.update_move_v(move_info_v, gainmax);
        this->validator.update_move(move_info_v);
        totalgain += gainmax;
        part[move_info_v.v] = move_info_v.toPart;
    }
    if (deferredsnapshot) {
        // restore the previous best solution
        // part = snapshot;
        self.restore_part(snapshot, part);
        totalgain = besttotalgain;
    }
    this->totalcost -= totalgain;
}

/**
 * @brief
 *
 * @tparam GainMgr
 * @tparam ConstrMgr
 * @tparam Derived
 * @param[in] part
 */
template <typename GainMgr, typename ConstrMgr,
          template <typename _gainMgr, typename _constrMgr> class Derived>  //
void PartMgrBase<GainMgr, ConstrMgr, Derived>::optimize(gsl::span<std::uint8_t> part) {
    // this->init(part);
    // auto totalcostafter = this->totalcost;
    while (true) {
        this->init(part);
        auto totalcostbefore = this->totalcost;
        // assert(totalcostafter == totalcostbefore);
        this->_optimize_1pass(part);
        assert(this->totalcost <= totalcostbefore);
        if (this->totalcost == totalcostbefore) {
            break;
        }
        // totalcostafter = this->totalcost;
    }
}

#include <ckpttn/FMKWayConstrMgr.hpp>  // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>    // for FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>        // for FMPartMgr

template class PartMgrBase<FMKWayGainMgr, FMKWayConstrMgr, FMPartMgr>;

#include <ckpttn/FMBiConstrMgr.hpp>  // for FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>    // for FMBiGainMgr

template class PartMgrBase<FMBiGainMgr, FMBiConstrMgr, FMPartMgr>;