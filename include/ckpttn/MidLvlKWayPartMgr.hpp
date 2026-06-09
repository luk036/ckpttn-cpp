#pragma once

#include <cstdint>
#include <netlistx/netlist.hpp>
#include <span>

class MidLvlKWayPartMgr {
  public:
    int total_cost{};

    MidLvlKWayPartMgr(double bal_tol, std::uint8_t num_parts);

    void optimize(std::span<std::uint8_t> part, const SimpleNetlist& hyprgraph);

  private:
    double bal_tol_;
    std::uint8_t num_parts_;
    static constexpr int max_passes = 5;
    static constexpr size_t max_pair_modules = 15;
};
