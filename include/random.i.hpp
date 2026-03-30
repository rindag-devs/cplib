/*
 * This file is part of CPLib.
 *
 * CPLib is free software: you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CPLib is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with CPLib. If
 * not, see <https://www.gnu.org/licenses/>.
 */

/* cplib_embed_ignore start */
#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
#include "random.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_RANDOM_HPP_
#error "Must be included from random.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib {
namespace detail {

inline constexpr auto rotl(std::uint64_t x, int k) noexcept -> std::uint64_t {
  return (x << k) | (x >> (64 - k));
}

inline constexpr auto splitmix64(std::uint64_t &x) noexcept -> std::uint64_t {
  x += 0x9e3779b97f4a7c15ull;
  std::uint64_t z = x;
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
  return z ^ (z >> 31);
}

template <std::unsigned_integral T>
inline auto rand_int_range(Random::Engine &rnd, T size) -> T {
  if (size == 0) {
    return 0;
  }

  const auto size64 = static_cast<std::uint64_t>(size);
  const auto threshold = (static_cast<std::uint64_t>(0) - size64) % size64;
  while (true) {
    const std::uint64_t x = rnd();
    if (x >= threshold) {
      return static_cast<T>(x % size64);
    }
  }
}

template <std::integral T>
inline auto rand_int_range(Random::Engine &rnd, T size) -> T {
  using UnsignedT = std::make_unsigned_t<T>;
  return static_cast<T>(rand_int_range(rnd, static_cast<UnsignedT>(size)));
}

/// Get random integer in [l,r].
template <std::integral T>
inline auto rand_int_between(Random::Engine &rnd, T l, T r) -> T {
  using UnsignedT = std::make_unsigned_t<T>;

  if (l > r) panic("rand_int_between failed: l must be <= r");

  UnsignedT size = static_cast<UnsignedT>(r) - static_cast<UnsignedT>(l);
  if (size == std::numeric_limits<UnsignedT>::max()) {
    return static_cast<T>(static_cast<UnsignedT>(rnd()));
  }

  ++size;
  return l + rand_int_range<UnsignedT>(rnd, size);
}

/// Get random float in [0,1).
template <std::floating_point T>
inline auto rand_float(Random::Engine &rnd) -> T {
  constexpr int digits = std::numeric_limits<T>::digits;
  const std::uint64_t x = rnd() >> (64 - digits);
  return std::ldexp(static_cast<T>(x), -digits);
}

/// Get random float in [l,r).
template <std::floating_point T>
inline auto rand_float_between(Random::Engine &rnd, T l, T r) -> T {
  if (l > r) panic("rand_float_between failed: l must be <= r");
  if (l == r) return l;
  T size = r - l;
  return std::fma(rand_float<T>(rnd), size, l);
}
}  // namespace detail

inline Random::Engine::Engine() noexcept { seed(0); }

inline Random::Engine::Engine(std::uint64_t seed_value) noexcept { seed(seed_value); }

inline auto Random::Engine::seed(std::uint64_t seed_value) noexcept -> void {
  std::uint64_t x = seed_value;
  for (auto &word : state_) {
    word = detail::splitmix64(x);
  }
  if ((state_[0] | state_[1] | state_[2] | state_[3]) == 0) {
    state_[0] = 1;
  }
}

inline auto Random::Engine::operator()() noexcept -> result_type {
  const std::uint64_t result = detail::rotl(state_[0] + state_[3], 23) + state_[0];
  const std::uint64_t t = state_[1] << 17;

  state_[2] ^= state_[0];
  state_[3] ^= state_[1];
  state_[1] ^= state_[2];
  state_[0] ^= state_[3];
  state_[2] ^= t;
  state_[3] = detail::rotl(state_[3], 45);

  return result;
}

inline auto Random::Engine::jump() noexcept -> void {
  constexpr std::array<std::uint64_t, 4> K_JUMP = {0x180ec6d33cfd0abaull, 0xd5a61266f0c9392cull,
                                                   0xa9582618e03fc9aaull, 0x39abdc4529b1661cull};

  std::array<std::uint64_t, 4> next_state{};
  for (std::uint64_t jump : K_JUMP) {
    for (int bit = 0; bit < 64; ++bit) {
      if ((jump & (UINT64_C(1) << bit)) != 0) {
        next_state[0] ^= state_[0];
        next_state[1] ^= state_[1];
        next_state[2] ^= state_[2];
        next_state[3] ^= state_[3];
      }
      (*this)();
    }
  }
  state_ = next_state;
}

inline auto Random::Engine::long_jump() noexcept -> void {
  constexpr std::array<std::uint64_t, 4> K_LONG_JUMP = {
      0x76e15d3efefdcbbfull, 0xc5004e441c522fb3ull, 0x77710069854ee241ull, 0x39109bb02acbe635ull};

  std::array<std::uint64_t, 4> next_state{};
  for (std::uint64_t jump : K_LONG_JUMP) {
    for (int bit = 0; bit < 64; ++bit) {
      if ((jump & (UINT64_C(1) << bit)) != 0) {
        next_state[0] ^= state_[0];
        next_state[1] ^= state_[1];
        next_state[2] ^= state_[2];
        next_state[3] ^= state_[3];
      }
      (*this)();
    }
  }
  state_ = next_state;
}

inline Random::Random() : engine_() {}

inline Random::Random(std::uint64_t seed) : engine_(seed) {}

inline Random::Random(const std::vector<std::string> &args) : engine_() { reseed(args); }

inline auto Random::reseed(std::uint64_t seed) -> void { engine().seed(seed); }

inline auto Random::reseed(const std::vector<std::string> &args) -> void {
  std::uint64_t seed = 0;

  for (const auto &arg : args) {
    // Add a delimiter before each argument so ["ab", "c"] and ["a", "bc"]
    // do not collapse to the same byte stream.
    seed = detail::splitmix64(seed);
    for (unsigned char c : arg) {
      seed ^= static_cast<std::uint64_t>(c) + 1;
      seed = detail::splitmix64(seed);
    }
    seed ^= static_cast<std::uint64_t>(arg.size()) + 1;
    seed = detail::splitmix64(seed);
  }

  reseed(seed);
}

inline auto Random::engine() -> Engine & { return engine_; }

template <std::integral T>
inline auto Random::next(T from, T to) -> T {
  if (from > to) {
    panic("Random::next failed: from must be <= to");
  }
  return detail::rand_int_between<T>(engine(), from, to);
}

template <std::floating_point T>
inline auto Random::next(T from, T to) -> T {
  if (from > to) {
    panic("Random::next failed: from must be <= to");
  }
  return detail::rand_float_between<T>(engine(), from, to);
}

inline auto Random::next_bool() -> bool { return next_bool(0.5); }

inline auto Random::next_bool(double true_prob) -> bool {
  if (true_prob < 0 || true_prob > 1) {
    panic("Random::next_bool failed: true_prob must be in [0, 1]");
  }
  if (true_prob == 0) return false;
  if (true_prob == 1) return true;
  return detail::rand_float<double>(engine()) < true_prob;
}

template <typename T>
  requires std::integral<T> || std::floating_point<T>
inline auto Random::wnext(T from, T to, int type) -> T {
  constexpr int BRUTE_FORCE_LIMIT = 25;

  if constexpr (std::is_integral_v<T>) {
    if (from > to) {
      panic("Random::wnext failed: from must be <= to for integral types");
    }
    if (from == to) return from;

    if (std::abs(type) < BRUTE_FORCE_LIMIT) {
      T result = next(from, to);
      if (type > 0) {
        for (int i = 0; i < type; ++i) {
          result = std::max(result, next(from, to));
        }
      } else if (type < 0) {
        for (int i = 0; i < -type; ++i) {
          result = std::min(result, next(from, to));
        }
      }
      return result;
    } else {
      double p;
      if (type > 0) {
        p = std::pow(next(0.0, 1.0), 1.0 / (type + 1));
      } else {
        p = 1.0 - std::pow(next(0.0, 1.0), 1.0 / (-type + 1));
      }
      T result = static_cast<T>(from + p * static_cast<double>(to - from + 1));
      if (result > to) result = to;
      if (result < from) result = from;
      return result;
    }
  } else {  // floating_point
    if (from >= to) {
      panic("Random::wnext failed: from must be < to for floating-point types");
    }

    if (std::abs(type) < BRUTE_FORCE_LIMIT) {
      T result = next(from, to);
      if (type > 0) {
        for (int i = 0; i < type; ++i) {
          result = std::max(result, next(from, to));
        }
      } else if (type < 0) {
        for (int i = 0; i < -type; ++i) {
          result = std::min(result, next(from, to));
        }
      }
      return result;
    } else {
      double p;
      if (type > 0) {
        p = std::pow(next(0.0, 1.0), 1.0 / (type + 1));
      } else {
        p = 1.0 - std::pow(next(0.0, 1.0), 1.0 / (-type + 1));
      }
      return from + static_cast<T>(p) * (to - from);
    }
  }
}

template <class T>
inline auto Random::choice(std::initializer_list<T> init_list) -> T {
  if (init_list.size() == 0u) panic("Random::choice failed: empty init_list");
  return *choice(init_list.begin(), init_list.end());
}

template <std::forward_iterator It>
inline auto Random::choice(It first, It last) -> It {
  const auto size = std::distance(first, last);
  if (size == 0) return last;
  using diff_t = std::iter_difference_t<It>;
  std::advance(first, next<diff_t>(0, size - 1));
  return first;
}

template <std::ranges::forward_range Container>
inline auto Random::choice(Container &container) {
  return choice(std::begin(container), std::end(container));
}

template <MapLike Map>
inline auto Random::weighted_choice(const Map &map) {
  using MappedType = typename Map::mapped_type;
  using UnsignedWeight = std::make_unsigned_t<MappedType>;

  UnsignedWeight total_weight{};
  for (const auto &pair : map) {
    if (pair.second < MappedType(0)) {
      panic("Random::weighted_choice failed: weights must be non-negative");
    }
    total_weight += static_cast<UnsignedWeight>(pair.second);
  }
  if (total_weight == UnsignedWeight(0)) return std::end(map);

  UnsignedWeight random_weight = detail::rand_int_range(engine(), total_weight);
  UnsignedWeight cumulative_weight{};

  for (auto it = std::begin(map); it != std::end(map); ++it) {
    cumulative_weight += static_cast<UnsignedWeight>(it->second);
    if (cumulative_weight > random_weight) return it;
  }
  return std::end(map);  // Should not be reached if total_weight > 0
}

template <std::random_access_iterator RandomIt>
inline auto Random::shuffle(RandomIt first, RandomIt last) -> void {
  using diff_t = std::iter_difference_t<RandomIt>;

  const diff_t n = last - first;
  for (diff_t i = n; i > 1; --i) {
    const auto j = detail::rand_int_range<diff_t>(engine(), i);
    std::iter_swap(first + (i - 1), first + j);
  }
}

template <std::ranges::random_access_range Container>
inline auto Random::shuffle(Container &container) -> void {
  shuffle(std::begin(container), std::end(container));
}
}  // namespace cplib
