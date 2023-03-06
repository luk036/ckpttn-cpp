#pragma once

// #include "dllist.hpp" // import Dllink
#include <vector>

#include "FMPmrConfig.hpp"

template <typename T> class Robin {
private:
  struct SlNode {
    SlNode *next;
    T key;
  };

  char stack_buf[FM_MAX_NUM_PARTITIONS * sizeof(SlNode)];
  FMPmr::monotonic_buffer_resource rsrc;
  FMPmr::vector<SlNode> cycle;

  struct iterator {
    SlNode *cur;
    auto operator!=(const iterator &other) const -> bool {
      return cur != other.cur;
    }
    auto operator==(const iterator &other) const -> bool {
      return cur == other.cur;
    }
    auto operator++() -> iterator & {
      cur = cur->next;
      return *this;
    }
    auto operator*() const -> const T & { return cur->key; }
  };

  struct iterable_wrapper {
    Robin<T> *rr;
    T from_part;
    auto begin() -> iterator { return iterator{rr->cycle[from_part].next}; }
    auto end() -> iterator { return iterator{&rr->cycle[from_part]}; }
    // auto size() const -> size_t { return rr->cycle.size() - 1; }
  };

public:
  explicit Robin(T num_parts) : cycle(num_parts, &rsrc) {
    // num_parts -= 1;
    // for (auto k = 0U; k != num_parts; ++k)
    // {
    //     this->cycle[k].next = &this->cycle[k + 1];
    //     this->cycle[k].key = k;
    // }
    // this->cycle[num_parts].next = &this->cycle[0];
    // this->cycle[num_parts].key = num_parts;

    auto *slptr = &this->cycle[num_parts - 1];
    auto k = T(0);
    for (auto &sl : this->cycle) {
      sl.key = k;
      slptr->next = &sl;
      slptr = slptr->next;
      ++k;
    }
  }

  auto exclude(T from_part) -> iterable_wrapper {
    return iterable_wrapper{this, from_part};
  }
};
