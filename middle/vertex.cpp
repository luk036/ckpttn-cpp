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

#include "vertex.hpp"
#include <algorithm>
#include <cassert>
#include <vector>

Vertex::Vertex(const std::vector<int> &x) : bits_(x) {
    assert(x.size() % 2 == 1);
    assert(x.size() >= 3);
}

void Vertex::rev_inv() {
    rev_inv(0, this->bits_.size() - 2); // ignore the last bit
}

void Vertex::rev_inv(int left, int right) {
    for (int i = left; i <= right; ++i) {
        this->bits_[i] = 1 - this->bits_[i];
    }
    std::reverse(this->bits_.begin() + left, this->bits_.begin() + right + 1);
}

int Vertex::first_touchdown(int a) const {
    int height = 0;
    for (int i = a; i < this->bits_.size() - 1; ++i) { // ignore the last bit
        height += (2 * this->bits_[i] - 1);
        if (height == 0) {
            return i;
        }
    }
    return -1; // error
}

int Vertex::first_dive() const {
    int height = 0;
    for (int i = 0; i < this->bits_.size() - 1; ++i) { // ignore the last bit
        height += (2 * this->bits_[i] - 1);
        if (height == -1) {
            return i;
        }
    }
    return -1; // error
}

void Vertex::steps_height(std::vector<std::vector<int>> &usteps_neg,
                          std::vector<std::vector<int>> &usteps_pos,
                          std::vector<std::vector<int>> &dsteps_neg,
                          std::vector<std::vector<int>> &dsteps_pos) const {
    usteps_neg.resize(0);
    usteps_pos.resize(0);
    dsteps_neg.resize(0);
    dsteps_pos.resize(0);
    int height = 0;
    int min_height = 0;
    int max_height = 0;
    for (int i = 0; i < this->bits_.size() - 1; ++i) { // ignore the last bit
        if ((this->bits_[i] == 0) && (height <= 0)) {
            if (height == min_height) {
                usteps_neg.push_back(std::vector<int>());
                dsteps_neg.push_back(std::vector<int>());
            }
            dsteps_neg[-height].push_back(i);
        }
        if ((this->bits_[i] == 1) && (height >= 0)) {
            if (height == max_height) {
                usteps_pos.push_back(std::vector<int>());
                dsteps_pos.push_back(std::vector<int>());
            }
            usteps_pos[height].push_back(i);
        }
        height += (2 * this->bits_[i] - 1); // update height
        min_height = std::min(height, min_height);
        max_height = std::max(height, max_height);
        if ((this->bits_[i] == 0) && (height >= 0)) {
            dsteps_pos[height].push_back(i);
            assert(dsteps_pos[height].size() == usteps_pos[height].size());
        }
        if ((this->bits_[i] == 1) && (height <= 0)) {
            usteps_neg[-height].push_back(i);
            assert(usteps_neg[-height].size() == dsteps_neg[-height].size());
        }
    }
    assert(usteps_neg.size() == dsteps_neg.size());
}

int Vertex::count_flaws() const {
    int c = 0;
    int height = 0;
    for (int i = 0; i < this->bits_.size() - 1; ++i) { // ignore the last bit
        if ((height <= 0) && (this->bits_[i] == 0)) {
            ++c;
        }
        height += (2 * this->bits_[i] - 1);
    }
    return c;
}

int Vertex::count_ones() const {
    int c = 0;
    for (int i = 0; i < this->bits_.size() - 1; ++i) { // ignore last bit
        if (this->bits_[i] == 1) {
            ++c;
        }
    }
    return c;
}

bool Vertex::is_first_vertex() const {
    return ((count_flaws() == 0) && (count_ones() == this->bits_.size() / 2));
}

bool Vertex::is_last_vertex() const {
    return ((count_flaws() == 1) && (count_ones() == this->bits_.size() / 2));
}

int Vertex::to_first_vertex() {
    if (is_last_vertex()) {
        // This case is encountered during the Hamilton cycle
        // computation.
        const int b = first_dive();
        // shift this->bits_[0,...,b-1] to the right
        std::copy(this->bits_.begin(), this->bits_.begin() + b, this->bits_.begin() + 1);
        // reset boundary steps around shifted subpath
        this->bits_[0] = 1;
        this->bits_[b + 1] = 0;
        return (2 * b + 2);
    } else {
        // This case is encountered only during initialization.
        // The rule how to move from an intermediate path vertex back to
        // the first vertex is  described in detail in the paper.
        std::vector<std::vector<int>> usteps_neg, dsteps_neg, usteps_pos,
            dsteps_pos;
        steps_height(usteps_neg, usteps_pos, dsteps_neg, dsteps_pos);
        bool min_zero = (usteps_neg.size() ==
                         0); // minimum of lattice path lies on the abscissa y=0
        bool unique_min;     // minimum of lattice path is unique
        unique_min = (min_zero ? (usteps_pos.front().size() == 1)
                               : (usteps_neg.back().size() == 1));
        bool middle_level = (2 * count_ones() + 1 ==
                             this->bits_.size()); // vertex has same number of 0s and
                                            // 1s (ignoring the last bit)
        int to;
        if ((!unique_min && middle_level) || (unique_min && !middle_level)) {
            // take position of first upstep starting at minimum
            // height
            to = (min_zero ? usteps_pos.front().front()
                           : usteps_neg.back().front()) -
                 1;
        } else {
            // take position of last upstep starting at minimum
            // height
            to = (min_zero ? usteps_pos.front().back()
                           : usteps_neg.back().back()) -
                 1;
        }
        // shift this->bits_[0,to] to the right by one
        std::copy(this->bits_.begin(), this->bits_.begin() + to + 1, this->bits_.begin() + 1);
        this->bits_[0] = 1;
        // flip downsteps in the left half to upsteps
        for (int d = 0; d < ((int)dsteps_neg.size()) -
                                ((unique_min && middle_level) ? 1 : 0);
             ++d) {
            this->bits_[dsteps_neg[d].front() + 1] = 1;
        }
        // flip upsteps in the right half to downsteps
        for (int d = 0; d < ((int)usteps_neg.size()) -
                                ((unique_min && !middle_level) ? 1 : 0);
             ++d) {
            this->bits_[usteps_neg[d].back()] = 0;
        }
        if (!middle_level) {
            // flip further upsteps above the line y=0 to downsteps
            for (int d = ((min_zero && unique_min) ? 1 : 0); d <= 1; ++d) {
                this->bits_[usteps_pos[d].back()] = 0;
            }
        }
        return 2 * (to + 1) + (middle_level ? 0 : 1);
    }
}

int Vertex::to_last_vertex() {
    int d = 0;
    if (!is_first_vertex()) {
        d = -to_first_vertex();
    }
    assert(is_first_vertex());

    const int b = first_touchdown(0);
    // shift this->bits_[1,...,b-1] to the left
    std::copy(this->bits_.begin() + 1, this->bits_.begin() + b, this->bits_.begin());
    // reset two steps that form a valley to the right of the shifted
    // subpath
    this->bits_[b - 1] = 0;
    this->bits_[b] = 1;
    d += 2 * (b - 1) + 2;
    return d;
}

void Vertex::compute_flip_seq_0(std::vector<int> &seq, bool flip) {
    assert(is_first_vertex());

    if (!flip) {
        // The length of the flip sequence is 2*(b-1)+2
        // where b is the index of the first time the Dyck
        // path corresponding to the vertex returns to the
        // abscissa.
        const int b = first_touchdown(0);
        const int length = 2 * (b - 1) + 2;
        seq.resize(length, 0);

        int next_step[b + 1];
        aux_pointers(0, b, next_step);

        int idx = 0;
        seq[idx++] = b;
        seq[idx++] = 0;
        compute_flip_seq_0_rec(seq, idx, 1, b - 1, next_step);
        return;
    }

    assert(flip);
    // From the length of the bitstring representation we can recover in
    // which of the two cases tau_image/tau_preimage we are.
    assert(this->bits_[0] == 1);
    if (this->bits_[1] == 1) {
        // create path of length 2
        assert(this->bits_[2] == 0);
        seq.resize(2);
        seq[0] = 2;
        seq[1] = 0;
    } else { // tau_image
        // We artificially change the bitstring into the other starting
        // vertex of the flippable pair, as this is the path we will be
        // modifying.
        this->bits_[1] = 1;
        this->bits_[2] = 0;

        const int b = first_touchdown(0);
        const int length = 2 * (b - 1) + 2;
        seq.resize(length, 0);

        int next_step[b + 1];
        aux_pointers(0, b, next_step);

        int idx = 0;
        seq[idx++] = b;
        seq[idx++] = 0;
        compute_flip_seq_0_rec(seq, idx, 1, b - 1, next_step);

        // revert artificial bitstring modification
        this->bits_[1] = 0;
        this->bits_[2] = 1;

        // modify flip sequence
        assert((seq[0] == b) && (seq[1] == 0) && (seq[2] == 2) &&
               (seq[3] == 1) && (seq[4] == 0) && (seq[5] == 2));
        seq[0] = b;
        seq[1] = 0;
        seq[2] = 1;
        seq[3] = 2;
        seq[4] = 0;
        seq[5] = 1;
    }
}

void Vertex::compute_flip_seq_0_rec(std::vector<int> &seq, int &idx, int left,
                                    int right, int *next_step) const {
    const int length =
        right - left + 1; // total length of the subpath under consideration
    if (length <= 0) {    // base case of the recursion
        return;
    }
    assert((this->bits_[left] == 1) && (this->bits_[right] == 0) &&
           (length % 2 == 0));

    const int m = next_step[left];
    assert((m <= right) && (this->bits_[m] == 0));
    seq[idx++] = m;
    seq[idx++] = left;
    // descend recursively into the left subpath
    compute_flip_seq_0_rec(seq, idx, left + 1, m - 1, next_step);
    seq[idx++] = left - 1;
    seq[idx++] = m;
    // descend recursively into the right subpath
    compute_flip_seq_0_rec(seq, idx, m + 1, right, next_step);
}

void Vertex::compute_flip_seq_1(std::vector<int> &seq) const {
    assert(is_last_vertex());

    const int b = first_dive();
    const int length = 2 * ((this->bits_.size() - 2) - (b + 2) + 1) + 2;
    seq.resize(length, 0);

    int next_step[this->bits_.size() - 1];
    aux_pointers(b + 2, this->bits_.size() - 2, next_step);

    int idx = 0;
    seq[idx++] = b + 1;
    compute_flip_seq_1_rec(seq, idx, b + 2, this->bits_.size() - 2, next_step);
    seq[idx++] = b;
}

void Vertex::compute_flip_seq_1_rec(std::vector<int> &seq, int &idx, int left,
                                    int right, int *next_step) const {
    const int length =
        right - left + 1; // total length of the subpath under consideration
    if (length <= 0) {    // base case of the recursion
        return;
    }
    assert((this->bits_[left] == 1) && (this->bits_[right] == 0) &&
           (length % 2 == 0));

    // index of the first return to the same height
    const int m = next_step[left];
    // length of the two subpaths u and v in the canonical decomposition

    seq[idx++] = m;
    seq[idx++] = left;
    // descend recursively into the left subpath
    compute_flip_seq_1_rec(seq, idx, left + 1, m - 1, next_step);
    seq[idx++] = left - 1;
    seq[idx++] = m;
    // descend recursively into the right subpath
    compute_flip_seq_1_rec(seq, idx, m + 1, right, next_step);
}

void Vertex::aux_pointers(int a, int b, int *next_step) const {
    assert((a == b + 1) || ((this->bits_[a] == 1) && (this->bits_[b] == 0)));
    // The array left_ustep_height[h] contains the index of the last upstep
    // starting at height h that has been encountered when moving
    // along the Dyck path from left to right.
    // We allocate this array on the stack for speed reasons.
    int left_ustep_height[b - a + 1]; // only the first (b-a)/2+1 many
                                      // entries should be needed
    int height = 0;
    for (int i = a; i <= b; ++i) {
        if (this->bits_[i] == 0) { // downstep (0-bit)
            assert(height >= 1);
            const int left = left_ustep_height[height - 1];
            assert((left >= 0) && (left < i));
            next_step[left] = i;
            next_step[i] = left;
        } else { // upstep (1-bit)
            assert(height >= 0);
            left_ustep_height[height] = i;
        }
        height += (2 * this->bits_[i] - 1);
    }
    assert(height == 0);
}

std::ostream &operator<<(std::ostream &os, const Vertex &v) {
    // bitstrings can be printed unambiguously without separation characters
    for (int i = 0; i < v.size(); ++i) {
        os << v[i];
    }
    return os;
}

bool bitstrings_less_than(int *x, int *y, int length) {
    for (int i = 0; i < length; ++i) {
        if (x[i] < y[i]) {
            return true;
        } else if (x[i] > y[i]) {
            return false;
        }
    }
    // bitstrings are the same
    return false;
}

bool bitstrings_equal(int *x, int *y, int length) {
    for (int i = 0; i < length; ++i) {
        if (x[i] != y[i]) {
            return false;
        }
    }
    // bitstrings are the same
    return true;
}
