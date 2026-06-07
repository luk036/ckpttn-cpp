#pragma once

#include <cstdint>
#include <span>

enum class LegalCheck;

class MLMidLvlPartMgr {
  public:
    int total_cost{};

    explicit MLMidLvlPartMgr(double bal_tol) : MLMidLvlPartMgr(bal_tol, 2) {}

    MLMidLvlPartMgr(double bal_tol, std::uint8_t num_parts);

    void set_limitsize(size_t limit) { this->limitsize = limit; }

    template <typename Gnl>
    auto run_Partition(const Gnl& hyprgraph, std::span<std::uint8_t> part) -> LegalCheck;

  private:
    double bal_tol;
    std::uint8_t num_parts;
    size_t limitsize{50U};
    static constexpr size_t exhaustive_limit{25U};
};
