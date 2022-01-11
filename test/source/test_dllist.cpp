#include <doctest/doctest.h>  // for ResultBuilder, CHECK, Expr...

#include <__config>                        // for std
#include <boost/container/pmr/vector.hpp>  // for vector
#include <boost/container/vector.hpp>      // for operator!=
#include <cinttypes>                       // for uint8_t
#include <ckpttn/dllist.hpp>               // for dllink, dll_iterator, oper...
#include <ckpttn/robin.hpp>                // for robin, robin<>::iterable_w...
#include <utility>                         // for pair

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
