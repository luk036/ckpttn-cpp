#if __has_include(<memory_resource>)
#include <memory_resource>
#include <vector>
#else
#include <vector>
#endif

#include <iostream>
#include <ranges>

int main()
{
    std::byte StackBuf[1000];
    std::pmr::monotonic_buffer_resource rsrc(StackBuf, sizeof StackBuf);
    std::pmr::vector<int> IdVec(100, 0, &rsrc);

    std::cout << IdVec[0] << '\n';
    return 0;
}
