#include <doctest/doctest.h>

#include <cinttypes>
#include <ckpttn/dllist.hpp>  // import  dllink
#include <ckpttn/robin.hpp>   // import  robin

using namespace std;

TEST_CASE("Test dllist") {
    auto L1 = dllink{pair{0, 0}};
    auto L2 = dllink{pair{0, 0}};
    auto d = dllink{pair{0, 0}};
    auto e = dllink{pair{0, 0}};
    auto f = dllink{pair{0, 0}};
    CHECK(L1.is_empty());

    L1.appendleft(e);
    CHECK(!L1.is_empty());

    L1.appendleft(f);
    L1.append(d);
    L2.append(L1.pop());
    L2.append(L1.popleft());
    CHECK(!L1.is_empty());
    CHECK(!L2.is_empty());

    auto count = 0U;
    for ([[maybe_unused]] auto& _ : L2) {
        count += 1;
    }
    CHECK(count == 2);
}

TEST_CASE("Test robin") {
    auto RR = robin<uint8_t>(6U);
    auto count = 0U;
    for ([[maybe_unused]] auto _ : RR.exclude(2)) {
        count += 1;
    }
    CHECK(count == 5);
}
