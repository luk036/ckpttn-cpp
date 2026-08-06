#include <ckpttn/version.h>

#include <iostream>

auto main() -> int {
    const auto ok = (CKPTTN_VERSION_MAJOR >= 1);
    std::cout << "ckpttn installed test: version " << CKPTTN_VERSION << "\n";
    return ok ? 0 : 1;
}
