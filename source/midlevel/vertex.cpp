/*
 * Adapted from: Torsten Muetze, Jerri Nummenpalo (2018)
 * Original license: GPL v2+
 */

#include <algorithm>
#include <cassert>
#include <ckpttn/midlevel/vertex.hpp>
#include <vector>

MidVertex::MidVertex(const std::vector<int>& x) : bits_(x) {
    assert(x.size() % 2 == 1);
    assert(x.size() >= 3);
}

void MidVertex::rev_inv() { rev_inv(0, static_cast<int>(this->bits_.size()) - 2); }

void MidVertex::rev_inv(int left, int right) {
    for (int i = left; i <= right; ++i) {
        this->bits_[i] = 1 - this->bits_[i];
    }
    std::reverse(this->bits_.begin() + left, this->bits_.begin() + right + 1);
}

int MidVertex::first_touchdown(int a) const {
    int height = 0;
    for (int i = a; i < static_cast<int>(this->bits_.size()) - 1; ++i) {
        height += (2 * this->bits_[i] - 1);
        if (height == 0) {
            return i;
        }
    }
    return -1;
}

int MidVertex::first_dive() const {
    int height = 0;
    for (int i = 0; i < static_cast<int>(this->bits_.size()) - 1; ++i) {
        height += (2 * this->bits_[i] - 1);
        if (height == -1) {
            return i;
        }
    }
    return -1;
}

void MidVertex::steps_height(std::vector<std::vector<int>>& usteps_neg,
                             std::vector<std::vector<int>>& usteps_pos,
                             std::vector<std::vector<int>>& dsteps_neg,
                             std::vector<std::vector<int>>& dsteps_pos) const {
    usteps_neg.resize(0);
    usteps_pos.resize(0);
    dsteps_neg.resize(0);
    dsteps_pos.resize(0);
    int height = 0;
    int min_height = 0;
    int max_height = 0;
    for (int i = 0; i < static_cast<int>(this->bits_.size()) - 1; ++i) {
        if ((this->bits_[i] == 0) && (height <= 0)) {
            if (height == min_height) {
                usteps_neg.emplace_back();
                dsteps_neg.emplace_back();
            }
            dsteps_neg[-height].push_back(i);
        }
        if ((this->bits_[i] == 1) && (height >= 0)) {
            if (height == max_height) {
                usteps_pos.emplace_back();
                dsteps_pos.emplace_back();
            }
            usteps_pos[height].push_back(i);
        }
        height += (2 * this->bits_[i] - 1);
        min_height = std::min(height, min_height);
        max_height = std::max(height, max_height);
        if ((this->bits_[i] == 0) && (height >= 0)) {
            dsteps_pos[height].push_back(i);
        }
        if ((this->bits_[i] == 1) && (height <= 0)) {
            usteps_neg[-height].push_back(i);
        }
    }
}

int MidVertex::count_flaws() const {
    int c = 0;
    int height = 0;
    for (int i = 0; i < static_cast<int>(this->bits_.size()) - 1; ++i) {
        if ((height <= 0) && (this->bits_[i] == 0)) {
            ++c;
        }
        height += (2 * this->bits_[i] - 1);
    }
    return c;
}

int MidVertex::count_ones() const {
    int c = 0;
    for (int i = 0; i < static_cast<int>(this->bits_.size()) - 1; ++i) {
        if (this->bits_[i] == 1) {
            ++c;
        }
    }
    return c;
}

bool MidVertex::is_first_vertex() const {
    return ((count_flaws() == 0) && (count_ones() == static_cast<int>(this->bits_.size()) / 2));
}

bool MidVertex::is_last_vertex() const {
    return ((count_flaws() == 1) && (count_ones() == static_cast<int>(this->bits_.size()) / 2));
}

int MidVertex::to_first_vertex() {
    if (is_last_vertex()) {
        const int b = first_dive();
        for (int i = b - 1; i >= 0; --i) {
            this->bits_[i + 1] = this->bits_[i];
        }
        this->bits_[0] = 1;
        this->bits_[b + 1] = 0;
        return (2 * b + 2);
    }
    std::vector<std::vector<int>> usteps_neg, dsteps_neg, usteps_pos, dsteps_pos;
    steps_height(usteps_neg, usteps_pos, dsteps_neg, dsteps_pos);
    bool min_zero = (usteps_neg.size() == 0);
    bool unique_min;
    unique_min = (min_zero ? (usteps_pos.front().size() == 1) : (usteps_neg.back().size() == 1));
    bool middle_level = (2 * count_ones() + 1 == static_cast<int>(this->bits_.size()));
    int to;
    if ((!unique_min && middle_level) || (unique_min && !middle_level)) {
        to = (min_zero ? usteps_pos.front().front() : usteps_neg.back().front()) - 1;
    } else {
        to = (min_zero ? usteps_pos.front().back() : usteps_neg.back().back()) - 1;
    }
    // shift this->bits_[0,to] to the right by one
    for (int i = to; i >= 0; --i) {
        this->bits_[i + 1] = this->bits_[i];
    }
    this->bits_[0] = 1;
    for (int d = 0;
         d < static_cast<int>(dsteps_neg.size()) - ((unique_min && middle_level) ? 1 : 0); ++d) {
        this->bits_[dsteps_neg[d].front() + 1] = 1;
    }
    for (int d = 0;
         d < static_cast<int>(usteps_neg.size()) - ((unique_min && !middle_level) ? 1 : 0); ++d) {
        this->bits_[usteps_neg[d].back()] = 0;
    }
    if (!middle_level) {
        for (int d = ((min_zero && unique_min) ? 1 : 0); d <= 1; ++d) {
            this->bits_[usteps_pos[d].back()] = 0;
        }
    }
    return 2 * (to + 1) + (middle_level ? 0 : 1);
}

int MidVertex::to_last_vertex() {
    int d = 0;
    if (!is_first_vertex()) {
        d = -to_first_vertex();
    }
    assert(is_first_vertex());

    const int b = first_touchdown(0);
    // Left-shift copy: [1,b) -> [0,b-1). Use manual loop to avoid memcpy UB.
    for (int i = 0; i < b - 1; ++i) {
        this->bits_[i] = this->bits_[i + 1];
    }
    this->bits_[b - 1] = 0;
    this->bits_[b] = 1;
    d += 2 * (b - 1) + 2;
    return d;
}

void MidVertex::compute_flip_seq_0(std::vector<int>& seq, bool flip) {
    assert(is_first_vertex());

    if (!flip) {
        const int b = first_touchdown(0);
        const int length = 2 * (b - 1) + 2;
        seq.resize(length, 0);

        std::vector<int> next_step(b + 1);
        aux_pointers(0, b, next_step.data());

        int idx = 0;
        seq[idx++] = b;
        seq[idx++] = 0;
        compute_flip_seq_0_rec(seq, idx, 1, b - 1, next_step.data());
        return;
    }

    assert(flip);
    assert(this->bits_[0] == 1);
    if (this->bits_[1] == 1) {
        assert(this->bits_[2] == 0);
        seq.resize(2);
        seq[0] = 2;
        seq[1] = 0;
    } else {
        this->bits_[1] = 1;
        this->bits_[2] = 0;

        const int b = first_touchdown(0);
        const int length = 2 * (b - 1) + 2;
        seq.resize(length, 0);

        std::vector<int> next_step(b + 1);
        aux_pointers(0, b, next_step.data());

        int idx = 0;
        seq[idx++] = b;
        seq[idx++] = 0;
        compute_flip_seq_0_rec(seq, idx, 1, b - 1, next_step.data());

        this->bits_[1] = 0;
        this->bits_[2] = 1;

        assert((seq[0] == b) && (seq[1] == 0) && (seq[2] == 2) && (seq[3] == 1) && (seq[4] == 0)
               && (seq[5] == 2));
        seq[0] = b;
        seq[1] = 0;
        seq[2] = 1;
        seq[3] = 2;
        seq[4] = 0;
        seq[5] = 1;
    }
}

void MidVertex::compute_flip_seq_0_rec(std::vector<int>& seq, int& idx, int left, int right,
                                       int* next_step) const {
    const int length = right - left + 1;
    if (length <= 0) {
        return;
    }
    assert((this->bits_[left] == 1) && (this->bits_[right] == 0) && (length % 2 == 0));

    const int m = next_step[left];
    assert((m <= right) && (this->bits_[m] == 0));
    seq[idx++] = m;
    seq[idx++] = left;
    compute_flip_seq_0_rec(seq, idx, left + 1, m - 1, next_step);
    seq[idx++] = left - 1;
    seq[idx++] = m;
    compute_flip_seq_0_rec(seq, idx, m + 1, right, next_step);
}

void MidVertex::compute_flip_seq_1(std::vector<int>& seq) const {
    assert(is_last_vertex());

    const int b = first_dive();
    const auto sz = static_cast<int>(this->bits_.size());
    const int length = 2 * (sz - 2 - (b + 2) + 1) + 2;
    seq.resize(length, 0);

    std::vector<int> next_step(sz - 1);
    aux_pointers(b + 2, sz - 2, next_step.data());

    int idx = 0;
    seq[idx++] = b + 1;
    compute_flip_seq_1_rec(seq, idx, b + 2, sz - 2, next_step.data());
    seq[idx++] = b;
}

void MidVertex::compute_flip_seq_1_rec(std::vector<int>& seq, int& idx, int left, int right,
                                       int* next_step) const {
    const int length = right - left + 1;
    if (length <= 0) {
        return;
    }
    assert((this->bits_[left] == 1) && (this->bits_[right] == 0) && (length % 2 == 0));

    const int m = next_step[left];
    seq[idx++] = m;
    seq[idx++] = left;
    compute_flip_seq_1_rec(seq, idx, left + 1, m - 1, next_step);
    seq[idx++] = left - 1;
    seq[idx++] = m;
    compute_flip_seq_1_rec(seq, idx, m + 1, right, next_step);
}

void MidVertex::aux_pointers(int a, int b, int* next_step) const {
    assert((a == b + 1) || ((this->bits_[a] == 1) && (this->bits_[b] == 0)));
    std::vector<int> left_ustep_height(b - a + 1);
    int height = 0;
    for (int i = a; i <= b; ++i) {
        if (this->bits_[i] == 0) {
            assert(height >= 1);
            const int left = left_ustep_height[height - 1];
            assert((left >= 0) && (left < i));
            next_step[left] = i;
            next_step[i] = left;
        } else {
            assert(height >= 0);
            left_ustep_height[height] = i;
        }
        height += (2 * this->bits_[i] - 1);
    }
    assert(height == 0);
}

std::ostream& operator<<(std::ostream& os, const MidVertex& v) {
    for (int i = 0; i < v.size(); ++i) {
        os << v[i];
    }
    return os;
}

bool bitstrings_less_than(const int* x, const int* y, int length) {
    for (int i = 0; i < length; ++i) {
        if (x[i] < y[i]) {
            return true;
        }
        if (x[i] > y[i]) {
            return false;
        }
    }
    return false;
}

bool bitstrings_equal(const int* x, const int* y, int length) {
    for (int i = 0; i < length; ++i) {
        if (x[i] != y[i]) {
            return false;
        }
    }
    return true;
}
