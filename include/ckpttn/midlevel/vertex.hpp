/*
 * Adapted from: Torsten Muetze, Jerri Nummenpalo (2018)
 * Middle levels Gray code algorithm, adapted for MSVC/ckpttn-cpp.
 *
 * Original license: GPL v2+
 */

#ifndef CKPTTN_MIDLEVEL_VERTEX_HPP
#define CKPTTN_MIDLEVEL_VERTEX_HPP

#include <iostream>
#include <vector>

class MidVertex {
  public:
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

inline bool operator==(const MidVertex& lhs, const MidVertex& rhs) {
    return lhs.get_bits() == rhs.get_bits();
}
inline bool operator!=(const MidVertex& lhs, const MidVertex& rhs) { return !operator==(lhs, rhs); }
std::ostream& operator<<(std::ostream& os, const MidVertex& v);

bool bitstrings_less_than(const int* x, const int* y, int length);
bool bitstrings_equal(const int* x, const int* y, int length);

#endif
