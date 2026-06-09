/*
 * Adapted from: Torsten Muetze, Jerri Nummenpalo (2018)
 * Original license: GPL v2+
 */

#ifndef CKPTTN_MIDLEVEL_HAMCYCLE_HPP
#define CKPTTN_MIDLEVEL_HAMCYCLE_HPP

#include <functional>
#include <vector>

#include "vertex.hpp"

using MidVisitFunc = std::function<void(const std::vector<int>& y, int i)>;

class MidHamCycle {
  public:
    explicit MidHamCycle(const MidVertex& x, long long limit, MidVisitFunc visit_f);
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
