/*
 * Adapted from: Torsten Muetze, Jerri Nummenpalo (2018)
 * Original license: GPL v2+
 */

#include <cassert>
#include <ckpttn/midlevel/hamcycle.hpp>
#include <ckpttn/midlevel/tree.hpp>
#include <ckpttn/midlevel/vertex.hpp>
#include <vector>

MidHamCycle::MidHamCycle(const MidVertex& x, long long limit, MidVisitFunc visit_f)
    : x_(x), y_(x), limit_(limit), visit_f_(std::move(visit_f)) {
    assert(this->x_.size() % 2 == 1);
    const int n = this->x_.size() / 2;

    MidVertex xs(this->x_);
    int skip = 0;
    if (xs[2 * n] == 1) {
        xs.rev_inv();
        skip += xs.to_last_vertex();
        xs.rev_inv();
        xs[2 * n] = 0;
        skip++;
    }
    skip += xs.to_first_vertex();
    assert(xs.is_first_vertex());

    this->y_ = xs;
    MidTree y_tree(this->y_);

    if ((skip > 0) && (y_tree.flip_tree())) {
        if ((xs[1] == 1) && (skip <= 5)) {
            skip = 6 - skip;
        }
        std::vector<int> y_string(2 * n);
        y_tree.to_bitstring(y_string.data());
        std::vector<int> y_vec(y_string.begin(), y_string.end());
        y_vec.reserve(y_vec.size() + 1);
        y_vec.push_back(0);
        xs = MidVertex(y_vec);
        this->y_ = xs;
    }

    this->length_ = 0;

    std::vector<int> seq;
    std::vector<int> seq01;
    seq01.reserve(1);
    seq01.push_back(2 * n);
    int dist_to_start = skip;
    bool final_path = false;
    while (true) {
        bool flip = y_tree.flip_tree();
        y_tree.rotate();

        this->y_.compute_flip_seq_0(seq, flip);

        assert(this->y_.is_first_vertex());
        if (flip_seq(seq, dist_to_start, final_path)) {
            break;
        }
        assert(this->y_.is_last_vertex());

        if (flip_seq(seq01, dist_to_start, final_path)) {
            break;
        }
        assert(this->y_[2 * n] == 1);

        this->y_.compute_flip_seq_1(seq);

        assert(this->y_.is_last_vertex());
        if (flip_seq(seq, dist_to_start, final_path)) {
            break;
        }
        assert(this->y_.is_first_vertex());

        if (flip_seq(seq01, dist_to_start, final_path)) {
            break;
        }
        assert(this->y_[2 * n] == 0);

        if (this->y_ == xs) {
            final_path = true;
            dist_to_start = skip;
        }
    }
}

bool MidHamCycle::flip_seq(const std::vector<int>& seq, int& dist_to_start, bool final_path) {
    const auto seq_sz = static_cast<long long>(seq.size());
    if ((dist_to_start > 0) || final_path
        || ((this->limit_ >= 0) && (this->length_ + seq_sz >= this->limit_))) {
        for (int j = 0; j < static_cast<int>(seq.size()); ++j) {
            if ((final_path && (dist_to_start == 0))
                || ((this->limit_ >= 0) && (this->length_ == this->limit_))) {
                return true;
            }
            const int i = seq[j];
            if ((dist_to_start == 0) || final_path) {
                this->y_[i] = 1 - this->y_[i];
                visit_f_(this->y_.get_bits(), i);
                ++length_;
            } else {
                this->y_[i] = 1 - this->y_[i];
            }
            if (dist_to_start > 0) {
                dist_to_start--;
            }
        }
    } else {
        for (int j = 0; j < static_cast<int>(seq.size()); ++j) {
            const int i = seq[j];
            this->y_[i] = 1 - this->y_[i];
            visit_f_(this->y_.get_bits(), i);
        }
        this->length_ += seq_sz;
    }
    return false;
}
