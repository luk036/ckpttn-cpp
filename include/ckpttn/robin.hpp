#pragma once

#include <vector>

namespace fun {

namespace detail {
template <typename T> struct RobinSlNode {
  RobinSlNode *next;
  T key;
};

template <typename T> struct RobinIterator {
  const RobinSlNode<T> *cur;
  auto operator!=(const RobinIterator &other) const -> bool {
    return cur != other.cur;
  }
  auto operator==(const RobinIterator &other) const -> bool {
    return cur == other.cur;
  }
  auto operator++() -> RobinIterator & {
    cur = cur->next;
    return *this;
  }
  auto operator*() const -> const T & { return cur->key; }
};

template <typename T> struct RobinIterableWrapper {
  const detail::RobinSlNode<T> *node;
  // const Robin<T> *rr;
  // T from_part;
  auto begin() const -> RobinIterator<T> {
    return RobinIterator<T>{node->next};
  }
  auto end() const -> RobinIterator<T> { return RobinIterator<T>{node}; }
  // auto size() const -> size_t { return rr->cycle.size() - 1; }
};
} // namespace detail

template <typename T> struct Robin {
  std::vector<detail::RobinSlNode<T>> cycle;

  explicit Robin(T num_parts) : cycle(num_parts) {
    auto *slptr = &this->cycle[num_parts - 1];
    auto k = T(0);
    for (auto &sl : this->cycle) {
      sl.key = k;
      slptr->next = &sl;
      slptr = slptr->next;
      ++k;
    }
  }

  auto exclude(T from_part) const -> detail::RobinIterableWrapper<T> {
    return detail::RobinIterableWrapper<T>{&this->cycle[from_part]};
  }
};

} // namespace fun
