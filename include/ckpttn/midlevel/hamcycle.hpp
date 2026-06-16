/**
 * @file hamcycle.hpp
 * @brief Hamiltonian cycle generation for middle-levels Gray code
 *
 * Adapted from: Torsten Muetze, Jerri Nummenpalo (2018)
 * Original license: GPL v2+
 */

#ifndef CKPTTN_MIDLEVEL_HAMCYCLE_HPP
#define CKPTTN_MIDLEVEL_HAMCYCLE_HPP

#include <functional>
#include <vector>

#include "vertex.hpp"

/// @brief Callback function type for visiting vertices during Hamiltonian cycle traversal
using MidVisitFunc = std::function<void(const std::vector<int>& y, int i)>;

/**
 * @brief Hamiltonian cycle generation for middle-levels Gray code
 *
 * Uses tree-based flip sequences to generate a Hamiltonian cycle
 * through the middle-levels graph. Supports an optional visit callback
 * and iteration limit for partial traversal.
 */
class MidHamCycle {
  public:
    /**
     * @brief Construct a MidHamCycle with given vertex, limit, and visit callback
     * @param[in] x The starting vertex
     * @param[in] limit Maximum number of steps
     * @param[in] visit_f Callback invoked for each visited vertex
     */
    explicit MidHamCycle(const MidVertex& x, long long limit, MidVisitFunc visit_f);
    /// @brief Get the length of the Hamiltonian cycle
    long long get_length() const { return length_; }

  private:
    MidVertex x_;
    MidVertex y_;
    long long limit_;
    MidVisitFunc visit_f_;
    long long length_;

    bool flip_seq(const std::vector<int>& seq, int& dist_to_start, bool final_path);
};

#endif
