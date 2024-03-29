#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expression_lhs
// #include <__config>            // for std
#include <ckpttn/bpqueue.hpp>  // for BPQueue
#include <ckpttn/dllist.hpp>   // for Dllink
#include <cstdint>             // for int32_t, uint32_t
#include <utility>             // for pair
#include <vector>              // for vector

using namespace std;

TEST_CASE("Test BPQueue") {
    constexpr auto PMAX = 10;

    auto bpq1 = BPQueue<int, int32_t>{-PMAX, PMAX};
    auto bpq2 = BPQueue<int, int32_t>{-PMAX, PMAX};

    CHECK(bpq1.is_empty());

    auto d = Dllink<std::pair<int, uint32_t>>{std::make_pair(0, uint32_t(0))};
    auto e = Dllink<std::pair<int, uint32_t>>{std::make_pair(0, uint32_t(0))};
    auto f = Dllink<std::pair<int, uint32_t>>{std::make_pair(0, uint32_t(0))};

    CHECK(d.data.second == 0);

    bpq1.append(e, 3);
    bpq1.append(f, -PMAX);
    bpq1.append(d, 5);

    bpq2.append(bpq1.popleft(), -6);  // d
    bpq2.append(bpq1.popleft(), 3);
    bpq2.append(bpq1.popleft(), 0);

    bpq2.modify_key(d, 15);
    bpq2.modify_key(d, -3);
    CHECK(bpq1.is_empty());
    CHECK(bpq2.get_max() == 6);

    auto nodelist = vector<Dllink<pair<int, uint32_t>>>(10);

    auto i = 0U;
    for (auto &it : nodelist) {
        it.data.second = 2 * i - 10;
        i += 1;
    }
    // bpq1.appendfrom(nodelist);

    // auto count = 0U;
    // for ([[maybe_unused]] auto& _ : bpq1)
    // {
    //     count += 1;
    // }
    // CHECK(count == 10);

    // const vector<uint8_t> a {3, 4, 5, 6};
    // gsl::span<const uint8_t> s {a};
}
