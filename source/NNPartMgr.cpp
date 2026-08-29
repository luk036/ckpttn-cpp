#include <cassert>               // for assert
#include <ckpttn/NNPartMgr.hpp>  // for NNPartMgr, part, SimpleNetlist
#include <ckpttn/moveinfo.hpp>   // for MoveInfoV
#include <cstdint>               // for uint8_t
#include <span>                  // for span
#include <vector>                // for vector

// using node_t = typename SimpleNetlist::node_t;
// using namespace std;

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
