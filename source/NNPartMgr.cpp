#include <cassert>                 // for assert
#include <ckpttn/FMConstrMgr.hpp>  // for LegalCheck, LegalCheck::notsat...
#include <ckpttn/NNPartMgr.hpp>    // for NNPartMgr, part, SimpleNetlist
#include <ckpttn/moveinfo.hpp>     // for MoveInfoV
#include <cstdint>                 // for uint8_t
#include <py2cpp/range.hpp>        // for _iterator
#include <py2cpp/set.hpp>          // for set
#include <span>                    // for span
#include <vector>                  // for vector

// using node_t = typename SimpleNetlist::node_t;
// using namespace std;

/**
 * @brief Initializes the No-Nonsense partition manager with the given partition.
 *
 * Delegates initialization to both the gain manager and constraint manager.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainMgr The gain manager type
 * @tparam ConstrMgr The constraint manager type
 * @param[in,out] part The partition vector to initialize
 */
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
void NNPartMgr<Gnl, GainMgr, ConstrMgr>::init(std::span<std::uint8_t> part) {
    this->total_cost = this->gain_mgr.init(part);
    this->validator.init(part);
}

/**
 * @brief Legalizes the partition to satisfy balance constraints.
 *
 * Iteratively moves vertices from overloaded partitions to underloaded ones,
 * checking legality at each step, until all balance constraints are satisfied
 * or no further legal moves are available.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainMgr The gain manager type
 * @tparam ConstrMgr The constraint manager type
 * @param[in,out] part The partition vector to legalize
 * @return LegalCheck The result of the legality check
 */
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
auto NNPartMgr<Gnl, GainMgr, ConstrMgr>::legalize(std::span<std::uint8_t> part) -> LegalCheck {
    this->init(part);

    auto legalcheck = LegalCheck::NotSatisfied;
    while (legalcheck != LegalCheck::AllSatisfied) {
        const auto to_part = this->validator.select_togo();
        if (this->gain_mgr.is_empty_togo(to_part)) {
            break;
        }
        const auto rslt = this->gain_mgr.select_togo(to_part);
        auto&& v = rslt.first;
        auto&& gainmax = rslt.second;
        const auto from_part = part[v];
        // assert(v == v);
        assert(from_part != to_part);
        const auto move_info_v = MoveInfoV<typename Gnl::node_t>{v, from_part, to_part};
        // Check if the move of v can NotSatisfied, makebetter, or satisfied
        legalcheck = this->validator.check_legal(move_info_v);
        if (legalcheck == LegalCheck::NotSatisfied) {  // NotSatisfied
            continue;
        }
        // Update v and its neigbours (even they are in waiting_list);
        // Put neigbours to bucket
        this->gain_mgr.update_move(part, move_info_v);
        this->gain_mgr.update_move_v(move_info_v, gainmax);
        this->validator.update_move(move_info_v);
        part[v] = to_part;
        // totalgain += gainmax;
        this->total_cost -= gainmax;
        assert(this->total_cost >= 0);
    }
    return legalcheck;
}

/**
 * @brief Performs a single pass of the No-Nonsense optimization algorithm.
 *
 * Similar to FM but stops as soon as a negative gain move is encountered
 * (no look-ahead / rollback mechanism). Only selects positive gain moves.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainMgr The gain manager type
 * @tparam ConstrMgr The constraint manager type
 * @param[in,out] part The partition vector to optimize
 */
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
void NNPartMgr<Gnl, GainMgr, ConstrMgr>::_optimize_1pass(std::span<std::uint8_t> part) {
    // using SS_t = decltype(this->take_snapshot(part));
    using SS_t = std::vector<std::uint8_t>;

    auto snapshot = SS_t{};
    auto totalgain = 0;

    while (!this->gain_mgr.is_empty()) {
        // Take the gainmax with v from gain_bucket
        // auto [move_info_v, gainmax] = this->gain_mgr.select(part);
        auto result = this->gain_mgr.select(part);
        auto move_info_v = result.first;
        auto gainmax = result.second;

        if (gainmax < 0) {
            break;
        }
        // Check if the move of v can satisfied or NotSatisfied
        const auto satisfiedOK = this->validator.check_constraints(move_info_v);
        if (!satisfiedOK) {
            continue;
        }
        // Update v and its neigbours (even they are in waiting_list);
        // Put neigbours to bucket
        // const auto& [v, _, to_part] = move_info_v;
        this->gain_mgr.lock(move_info_v.to_part, move_info_v.v);
        this->gain_mgr.update_move(part, move_info_v);
        this->gain_mgr.update_move_v(move_info_v, gainmax);
        this->validator.update_move(move_info_v);
        totalgain += gainmax;
        part[move_info_v.v] = move_info_v.to_part;
    }
    this->total_cost -= totalgain;
}

/**
 * @brief Optimizes the partition using the No-Nonsense algorithm.
 *
 * Repeats NN optimization passes until no further improvement in total cost.
 * Unlike standard FM, this algorithm stops each pass at the first negative
 * gain move without snapshot/rollback.
 *
 * @tparam Gnl The hypergraph type
 * @tparam GainMgr The gain manager type
 * @tparam ConstrMgr The constraint manager type
 * @param[in,out] part The partition vector to optimize
 */
template <typename Gnl, typename GainMgr, typename ConstrMgr>  //
void NNPartMgr<Gnl, GainMgr, ConstrMgr>::optimize(std::span<std::uint8_t> part) {
    // this->init(part);
    // auto totalcostafter = this->total_cost;
    while (true) {
        this->init(part);
        auto totalcostbefore = this->total_cost;
        // assert(totalcostafter == totalcostbefore);
        this->_optimize_1pass(part);
        assert(this->total_cost <= totalcostbefore);
        if (this->total_cost == totalcostbefore) {
            break;
        }
        // totalcostafter = this->total_cost;
    }
}

#include <ckpttn/FMKWayConstrMgr.hpp>  // for FMKWayConstrMgr
#include <ckpttn/FMKWayGainMgr.hpp>    // for FMKWayGainMgr
#include <ckpttn/FMPartMgr.hpp>        // for FMPartMgr
#include <netlistx/netlist.hpp>        // for SimpleNetlist, Netlist
#include <xnetwork/classes/graph.hpp>

template class NNPartMgr<SimpleNetlist, FMKWayGainMgr<SimpleNetlist>,
                         FMKWayConstrMgr<SimpleNetlist>>;

#include <ckpttn/FMBiConstrMgr.hpp>  // for FMBiConstrMgr
#include <ckpttn/FMBiGainMgr.hpp>    // for FMBiGainMgr

template class NNPartMgr<SimpleNetlist, FMBiGainMgr<SimpleNetlist>, FMBiConstrMgr<SimpleNetlist>>;