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
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib {
namespace detail {
#ifdef __GNUC__
#define CPLIB_CLZ_CONSTEXPR constexpr
#else
#define CPLIB_CLZ_CONSTEXPR
#endif

inline CPLIB_CLZ_CONSTEXPR auto int_log2(std::uint64_t x) noexcept -> std::uint32_t {
#ifdef __GNUC__
  return 8 * sizeof(x) - __builtin_clzll(x) - 1;
#else
  constexpr static std::uint32_t tab64[64] = {
      63, 0,  58, 1,  59, 47, 53, 2,  60, 39, 48, 27, 54, 33, 42, 3,  61, 51, 37, 40, 49, 18,
      28, 20, 55, 30, 34, 11, 43, 14, 22, 4,  62, 57, 46, 52, 38, 26, 32, 41, 50, 36, 17, 19,
      29, 10, 13, 21, 56, 45, 25, 31, 35, 16, 9,  12, 44, 24, 15, 8,  23, 7,  6,  5};
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x |= x >> 32;
  return tab64[(static_cast<std::uint64_t>((x - (x >> 1)) * 0x07EDD5E59A4E28C2ULL)) >> 58];
#endif
}

template <class T>
inline CPLIB_CLZ_CONSTEXPR auto scale_int(std::uint64_t x, T size) -> T {
  auto lg = int_log2(size);
  if (x == (x & -x)) {
    return x & ((T(1) << lg) - 1);
  }
  if (lg >= 8 * sizeof(T) - 1) {
    return static_cast<T>(x);
  }
  return x & ((T(1) << (lg + 1)) - 1);
}

/// Get random integer in [0, size).
template <class T>
inline auto rand_int_range(Random::Engine& rnd, T size) -> T {
  T result;
  do {
    result = scale_int<T>(rnd(), size);
  } while (result >= size);
  return result;
}

/// Get random integer in [l,r].
template <class T>
inline auto rand_int_between(Random::Engine& rnd, T l, T r) -> T {
  using UnsignedT = std::make_unsigned_t<T>;

  if (l > r) panic("rand_int_between failed: l must be <= r");

  UnsignedT size = r - l;
  if (size == std::numeric_limits<UnsignedT>::max()) {
    return static_cast<T>(static_cast<UnsignedT>(rnd()));
  }

  ++size;
  return l + rand_int_range<UnsignedT>(rnd, size);
}

/// Get random float in [0,1).
template <class T>
inline auto rand_float(Random::Engine& rnd) -> T {
  return T(rnd()) / rnd.max();
}

/// Get random float in [l,r).
template <class T>
inline auto rand_float_between(Random::Engine& rnd, T l, T r) -> T {
  if (l > r) panic("rand_float_between failed: l must be <= r");

  T size = r - l;
  if (float_delta(l, r) <= 1E-9) return l;

  return rand_float<T>(rnd) * size + l;
}
}  // namespace detail

inline Random::Random() : engine_() {}

inline Random::Random(std::uint64_t seed) : engine_(seed) {}

inline Random::Random(const std::vector<std::string>& args) : engine_() { reseed(args); }

inline auto Random::reseed(std::uint64_t seed) -> void { engine().seed(seed); }

inline auto Random::reseed(const std::vector<std::string>& args) -> void {
  // Magic numbers from https://docs.oracle.com/javase/8/docs/api/java/util/Random.html#next-int-
  constexpr std::uint64_t multiplier = 0x5DEECE66Dull;
  constexpr std::uint64_t addend = 0xBull;
  std::uint64_t seed = 3905348978240129619ull;

  for (const auto& arg : args) {
    for (char c : arg) {
      seed = seed * multiplier + static_cast<std::uint32_t>(c) + addend;
    }
    seed += multiplier / addend;
  }

  reseed(seed & ((1ull << 48) - 1));
}

inline auto Random::engine() -> Engine& { return engine_; }

template <class T>
inline auto Random::next(T from, T to) -> std::enable_if_t<std::is_integral_v<T>, T> {
  // Allow range from higher to lower
  if (from <= to) {
    return detail::rand_int_between<T>(engine(), from, to);
  }
  return detail::rand_int_between<T>(engine(), to, from);
}

template <class T>
inline auto Random::next(T from, T to) -> std::enable_if_t<std::is_floating_point_v<T>, T> {
  // Allow range from higher to lower
  if (from <= to) {
    return detail::rand_float_between<T>(engine(), from, to);
  }
  return detail::rand_float_between<T>(engine(), to, from);
}

template <class T>
inline auto Random::next() -> std::enable_if_t<std::is_same_v<T, bool>, bool> {
  return next<bool>(0.5);
}

template <class T>
inline auto Random::next(double true_prob) -> std::enable_if_t<std::is_same_v<T, bool>, bool> {
  if (true_prob < 0 || true_prob > 1) panic("Random::next failed: true_prob must be in [0, 1]");
  return detail::rand_float<double>(engine()) < true_prob;
}

template <class T>
inline auto Random::choice(std::initializer_list<T> init_list) -> T {
  if (init_list.size() == 0u) panic("Random::choice failed: empty init_list");
  return *choice(init_list.begin(), init_list.end());
}

template <class It>
inline auto Random::choice(It first, It last) -> It {
  const auto size = std::distance(first, last);
  if (0 == size) return last;
  using diff_t = typename std::iterator_traits<It>::difference_type;
  return std::next(first, next<diff_t>(0, size - 1));
}

template <class Container>
inline auto Random::choice(Container& container) -> decltype(std::begin(container)) {
  return choice(std::begin(container), std::end(container));
}

template <class Map>
inline auto Random::weighted_choice(const Map& map) -> decltype(std::begin(map)) {
  using MappedType = typename Map::mapped_type;
  using IteratorType = decltype(std::begin(map));

  MappedType total_weight = 0;
  for (IteratorType it = std::begin(map); it != std::end(map); ++it) {
    total_weight += it->second;
  }
  if (total_weight == MappedType(0)) return std::end(map);

  MappedType random_weight = next(MappedType(0), total_weight - 1);
  MappedType sum = 0;

  for (IteratorType it = std::begin(map); it != std::end(map); ++it) {
    sum += it->second;
    if (sum > random_weight) return it;
  }
  return std::end(map);
}

template <class RandomIt>
inline auto Random::shuffle(RandomIt first, RandomIt last) -> void {
  std::shuffle(first, last, engine());
}

template <class Container>
inline auto Random::shuffle(Container& container) -> void {
  shuffle(std::begin(container), std::end(container));
}
}  // namespace cplib
