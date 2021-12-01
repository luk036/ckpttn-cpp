#include <ckpttn/FMBiGainCalc.hpp>
#include <ckpttn/FMBiGainMgr.hpp>

using namespace std;

/**
 * @brief
 *
 * @param[in] part
 */
auto FMBiGainMgr::init(std::span<const uint8_t> part) -> int {
    auto totalcost = Base::init(part);
    for (auto& bckt : this->gainbucket) {
        bckt.clear();
    }

    for (const auto& v : this->H) {
        auto& vlink = this->gainCalc.vertex_list[v];
        // auto toPart = 1 - part[v];
        this->gainbucket[1 - part[v]].append_direct(vlink);
    }
    for (const auto& v : this->H.module_fixed) {
        this->lock_all(part[v], v);
    }
    return totalcost;
}
