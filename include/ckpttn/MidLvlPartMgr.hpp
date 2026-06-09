#pragma once

#include <cstdint>
#include <span>

#include "FMBiConstrMgr.hpp"
#include "FMBiGainCalc.hpp"

template <typename Gnl> class MidLvlPartMgr {
  public:
    using node_t = typename Gnl::node_t;

    int total_cost{};

    MidLvlPartMgr(const Gnl& hyprgraph, double bal_tol);

    void optimize(std::span<std::uint8_t> part);

  private:
    const Gnl& hyprgraph;
    FMBiGainCalc<Gnl> gain_calc;
    FMBiConstrMgr<Gnl> constr_mgr;
};
