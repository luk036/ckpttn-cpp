/**
 * @file ParamSweep.cpp
 * @brief Sweep the multi-level partitioner over limitsize / contraction-ratio values.
 *
 * Usage:
 *   bench_param_sweep <netfile> <arefile|-> <k> <nn:0|1> \
 *                     <limitsizes_csv> <ratios_csv> <seeds_csv>
 *
 * Example:
 *   bench_param_sweep ../../testcases/ibm03.net ../../testcases/ibm03.are 2 0 \
 *                     "10,25,50,100,200" "1.05,1.2,1.3333,1.5,2.0" "0,1,2,3,4"
 *
 * Emits one JSON object per (seed, limitsize, ratio) run so the results can be
 * post-processed (paired per-seed comparison against the default configuration).
 */

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

#include "bench_common.hpp"

namespace {

    constexpr double BAL_TOL = 0.45;

    auto split_csv(const char* text) -> std::vector<std::string> {
        auto out = std::vector<std::string>{};
        auto current = std::string{};
        for (const char* p = text; *p != '\0'; ++p) {
            if (*p == ',') {
                if (!current.empty()) out.push_back(current);
                current.clear();
            } else {
                current.push_back(*p);
            }
        }
        if (!current.empty()) out.push_back(current);
        return out;
    }

    auto basename_no_ext(std::string_view path) -> std::string {
        const auto slash = path.find_last_of("/\\");
        auto name = std::string{slash == std::string_view::npos ? path : path.substr(slash + 1)};
        const auto dot = name.find_last_of('.');
        if (dot != std::string::npos) name.resize(dot);
        return name;
    }

}  // namespace

int main(int argc, char** argv) {
    if (argc != 8) {
        std::fprintf(stderr,
                     "usage: %s <netfile> <arefile|-> <k> <nn:0|1> <limitsizes_csv> "
                     "<ratios_csv> <seeds_csv>\n",
                     argv[0]);
        return 2;
    }

    const auto testcase = basename_no_ext(argv[1]);
    const std::uint8_t k = static_cast<std::uint8_t>(std::atoi(argv[3]));
    const bool nn = std::atoi(argv[4]) != 0;
    const auto limitsizes = split_csv(argv[5]);
    const auto ratios = split_csv(argv[6]);
    const auto seeds = split_csv(argv[7]);
    const char* algo = nn ? "NN" : "FM";

    auto hyprgraph = readNetD(argv[1]);
    if (std::strcmp(argv[2], "-") != 0) readAre(hyprgraph, argv[2]);
    const auto n = hyprgraph.number_of_modules();

    for (const auto& ls : limitsizes) {
        const auto limitsize = static_cast<std::size_t>(std::stoull(ls));
        for (const auto& rs : ratios) {
            const double ratio = std::stod(rs);
            for (const auto& ss : seeds) {
                const int seed = std::atoi(ss.c_str());
                auto part = bench_make_init(n, k, seed);
                const auto t0 = std::chrono::steady_clock::now();
                const auto cost = partition_ml(hyprgraph, part, k, nn, BAL_TOL, limitsize, ratio);
                const auto t1 = std::chrono::steady_clock::now();
                const auto secs = std::chrono::duration<double>{t1 - t0}.count();
                std::printf(
                    "{\"lang\": \"cpp\", \"testcase\": \"%s\", \"k\": %u, \"algo\": \"%s\", "
                    "\"ml\": true, \"seed\": %d, \"limitsize\": %zu, \"ratio\": %.6f, "
                    "\"cost\": %d, \"time_s\": %.4f}\n",
                    testcase.c_str(), static_cast<unsigned>(k), algo, seed, limitsize, ratio, cost,
                    secs);
                std::fflush(stdout);
            }
        }
    }
    return 0;
}
