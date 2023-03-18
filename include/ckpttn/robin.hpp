#pragma once

#include <vector>

namespace fun {

// Forward declaration
template <typename T> struct Robin;

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
  const fun::Robin<T> *rr;
  T from_part;
  auto begin() const -> RobinIterator<T>;
  auto end() const -> RobinIterator<T>;
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
    return detail::RobinIterableWrapper<T>{this, from_part};
  }
};

namespace detail {

template <typename T>
inline auto RobinIterableWrapper<T>::begin() const -> RobinIterator<T> {
  return RobinIterator<T>{this->rr->cycle[from_part].next};
}

template <typename T>
inline auto RobinIterableWrapper<T>::end() const -> RobinIterator<T> {
  return RobinIterator<T>{&this->rr->cycle[from_part]};
}
} // namespace detail

} // namespace fun
