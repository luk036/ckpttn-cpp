#pragma once

#include <cstdint>
#include <netlistx/netlist.hpp>
#include <span>

class MLMidLvlKWayPartMgr {
  public:
    int total_cost{};

    MLMidLvlKWayPartMgr(double bal_tol, std::uint8_t num_parts);

    void set_limitsize(size_t limit) { this->limitsize_ = limit; }

    void optimize(std::span<std::uint8_t> part, const SimpleNetlist& hyprgraph);

  private:
    double bal_tol_;
    std::uint8_t num_parts_;
    size_t limitsize_{50U};
    static constexpr size_t base_exhaustive{25U};
};
