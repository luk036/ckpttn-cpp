/*
 * Copyright (c) 2018 Torsten Muetze, Jerri Nummenpalo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VERTEX_HPP
#define VERTEX_HPP

#include <iostream>
#include <vector>

// A class to represent and manipulate a vertex of the cube of odd dimension
// 2n+1 for use within the Hamilton cycle algorithm.

class Vertex {
  public:
    // constructor
    explicit Vertex(const std::vector<int> &x);

    const std::vector<int> &get_bits() const { return bits_; }
    // allow bit vector access directly via the vertex object
    int &operator[](int i) { return this->bits_[i]; }
    const int &operator[](int i) const { return this->bits_[i]; }
    // the size of the bit vector representing the vertex (=2n+1)
    int size() { return this->bits_.size(); }
    const int size() const { return this->bits_.size(); }

    // reverse and invert bitstring
    void rev_inv();

    // compute whether vertex is first or last vertex on a path
    bool is_first_vertex() const;
    bool is_last_vertex() const;

    // For the current vertex, compute the corresponding first/last vertex
    // on the unflipped path. Return the distance between the two vertices
    // along the path.
    int to_first_vertex();
    int to_last_vertex();

    // Given a first vertex of a path, compute the sequence of bit positions
    // to be flipped to follow the corresponding path all the way to the
    // last vertex. Flipping the bit positions one after the other, we
    // follow a path from P_n\circ 0 from its first vertex to its last
    // vertex in the graph H_n\circ 0. If flip == false, then follow the
    // unflipped path, otherwise the flipped path. This is exactly the
    // recursion rule sigma() defined in the paper. The current vertex is
    // modified temporarily, which is the only reason that this function is
    // not qualified as const.
    void compute_flip_seq_0(std::vector<int> &seq, bool flip);
    // Given a last vertex of a path, compute the sequence of bit positions
    // to be flipped to follow the corresponding path all the way to the
    // first vertex. The resulting positions are already transformed
    // according to the isomorphism \overline{\rev}
    // (complementation+reversal), so flipping the bit positions one after
    // the other, we follow a path from \overline{\rev}(P_n)\circ 1 from its
    // last vertex to its first vertex in the graph
    // \overline{\rev}(H_n)\circ 1. This recursion rule is derived from
    // sigma() defined in the paper. In the algorithm HamCycle() described
    // in the paper, we instead apply sigma() and transform the resulting
    // flip sequence afterwards. However, computing the recursion directly
    // via this modified recursion is faster in practice.
    void compute_flip_seq_1(std::vector<int> &seq) const;

  private:
    // the bitstring representation of the vertex
    std::vector<int> bits_;

    // #### auxiliary functions ####

    // reverse and invert bitstring in the range [left,right]
    void rev_inv(int left, int right);
    // auxiliary function to compute the smallest index b
    // for which the Dyck path represented by this->bits_[a,...,b]
    // returns to the abscissa
    int first_touchdown(int a) const;
    // auxiliary function to compute the smallest index b
    // for which the Dyck path represented by this->bits_[0,...,b]
    // moves below the abscissa
    int first_dive() const;
    // auxiliary function to compute the positions of all downsteps and
    // upsteps below the abscissa and above the abscissa sorted by
    // increasing depth/height of those steps
    void steps_height(std::vector<std::vector<int>> &usteps_neg,
                      std::vector<std::vector<int>> &usteps_pos,
                      std::vector<std::vector<int>> &dsteps_neg,
                      std::vector<std::vector<int>> &dsteps_pos) const;

    // interpreting the bitstring representation as a Dyck path
    // (0=downstep, 1=upstep), count the number of downsteps below the
    // abscissa
    int count_flaws() const;

    // count the number of 1-bits in this->bits_ (when ignoring the last
    // bit)
    int count_ones() const;

    // Auxiliary recursive version of the function compute_flip_seq_0().
    // Compute the flip sequence for the given first vertex, interpreted
    // as a Dyck path, in the range [left,...,right]. The value idx is the
    // index into the array seq where to write the next bit positions.
    // The auxiliary array next_step is an array of bidirectional pointers
    // below the hills of the Dyck path between pairs of an upstep
    // and downstep that 'see each other' (this allows the canonical
    // decomposition to be performed in constant time).
    // This is the recursion rule sigma() defined in the paper.
    // The array next_step is allocated on the stack for speed reasons.
    void compute_flip_seq_0_rec(std::vector<int> &seq, int &idx, int left,
                                int right, int *next_step) const;
    // Auxiliary recursive version of the function compute_flip_seq_1().
    void compute_flip_seq_1_rec(std::vector<int> &seq, int &idx, int left,
                                int right, int *next_step) const;
    // Compute auxiliary bidirectional pointers for the Dyck path
    // in the range [a,b] below the hills of the Dyck path between
    // pairs of an upstep and downstep that 'see each other'.
    // We allocate this array on the stack for speed reasons.
    void aux_pointers(int a, int b, int *next_step) const;
};

// allow comparison between vertices (by comparing the corresponding bit
// vectors)
inline bool operator==(const Vertex &lhs, const Vertex &rhs) {
    return lhs.get_bits() == rhs.get_bits();
}
inline bool operator!=(const Vertex &lhs, const Vertex &rhs) {
    return !operator==(lhs, rhs);
}
std::ostream &operator<<(std::ostream &os, const Vertex &v);

// compare two bitstrings of the same length (arrays of 0s and 1s)
// lexicographically
bool bitstrings_less_than(int *x, int *y, int length);
bool bitstrings_equal(int *x, int *y, int length);

#endif
