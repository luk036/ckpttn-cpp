/**
 * @file vertex.hpp
 * @brief Bitstring vertex representation for middle-levels Gray code
 *
 * Adapted from: Torsten Muetze, Jerri Nummenpalo (2018)
 * Middle levels Gray code algorithm, adapted for MSVC/ckpttn-cpp.
 *
 * Original license: GPL v2+
 */

#ifndef CKPTTN_MIDLEVEL_VERTEX_HPP
#define CKPTTN_MIDLEVEL_VERTEX_HPP

#include <iostream>
#include <vector>

/**
 * @brief Bitstring vertex for middle-levels Gray code algorithm
 *
 * Represents a vertex in the middle-levels graph as a bitstring.
 * Provides methods for flip sequence computation, vertex comparison,
 * and conversion operations used in Hamiltonian cycle generation.
 */
class MidVertex {
  public:
    /**
     * @brief Construct a MidVertex from a bitstring vector
     * @param[in] x The input bitstring
     */
    explicit MidVertex(const std::vector<int>& x);

    const std::vector<int>& get_bits() const { return bits_; }
    int& operator[](int i) { return this->bits_[i]; }
    const int& operator[](int i) const { return this->bits_[i]; }
    int size() { return static_cast<int>(this->bits_.size()); }
    int size() const { return static_cast<int>(this->bits_.size()); }

    void rev_inv();
    bool is_first_vertex() const;
    bool is_last_vertex() const;
    int to_first_vertex();
    int to_last_vertex();
    void compute_flip_seq_0(std::vector<int>& seq, bool flip);
    void compute_flip_seq_1(std::vector<int>& seq) const;

  private:
    std::vector<int> bits_;
    void rev_inv(int left, int right);
    int first_touchdown(int a) const;
    int first_dive() const;
    void steps_height(std::vector<std::vector<int>>& usteps_neg,
                      std::vector<std::vector<int>>& usteps_pos,
                      std::vector<std::vector<int>>& dsteps_neg,
                      std::vector<std::vector<int>>& dsteps_pos) const;
    int count_flaws() const;
    int count_ones() const;
    void compute_flip_seq_0_rec(std::vector<int>& seq, int& idx, int left, int right,
                                int* next_step) const;
    void compute_flip_seq_1_rec(std::vector<int>& seq, int& idx, int left, int right,
                                int* next_step) const;
    void aux_pointers(int a, int b, int* next_step) const;
};

/// @brief Equality comparison for MidVertex
inline bool operator==(const MidVertex& lhs, const MidVertex& rhs) {
    return lhs.get_bits() == rhs.get_bits();
}
/// @brief Inequality comparison for MidVertex
inline bool operator!=(const MidVertex& lhs, const MidVertex& rhs) { return !operator==(lhs, rhs); }
/// @brief Stream output operator for MidVertex
std::ostream& operator<<(std::ostream& os, const MidVertex& v);

/// @brief Lexicographic comparison of bitstrings
bool bitstrings_less_than(const int* x, const int* y, int length);
/// @brief Equality comparison of bitstrings
bool bitstrings_equal(const int* x, const int* y, int length);

#endif
