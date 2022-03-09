#pragma once

// #include "dllist.hpp" // import Dllink
#include <vector>

#include "FMPmrConfig.hpp"

template <typename T> class robin {
  private:
    struct slnode {
        slnode* next;
        T key;
    };

    char StackBuf[FM_MAX_NUM_PARTITIONS * sizeof(slnode)];
    FMPmr::monotonic_buffer_resource rsrc;
    FMPmr::vector<slnode> cycle;

    struct iterator {
        slnode* cur;
        auto operator!=(const iterator& other) const -> bool { return cur != other.cur; }
        auto operator==(const iterator& other) const -> bool { return cur == other.cur; }
        auto operator++() -> iterator& {
            cur = cur->next;
            return *this;
        }
        auto operator*() const -> const T& { return cur->key; }
    };

    struct iterable_wrapper {
        robin<T>* rr;
        T fromPart;
        auto begin() { return iterator{rr->cycle[fromPart].next}; }
        auto end() { return iterator{&rr->cycle[fromPart]}; }
        // auto size() const -> size_t { return rr->cycle.size() - 1; }
    };

  public:
    explicit robin(T num_parts) : cycle(num_parts, &rsrc) {
        // num_parts -= 1;
        // for (auto k = 0U; k != num_parts; ++k)
        // {
        //     this->cycle[k].next = &this->cycle[k + 1];
        //     this->cycle[k].key = k;
        // }
        // this->cycle[num_parts].next = &this->cycle[0];
        // this->cycle[num_parts].key = num_parts;

        auto* slptr = &this->cycle[num_parts - 1];
        auto k = T(0);
        for (auto& sl : this->cycle) {
            sl.key = k;
            slptr->next = &sl;
            slptr = slptr->next;
            ++k;
        }
    }

    auto exclude(T fromPart) { return iterable_wrapper{this, fromPart}; }
};
