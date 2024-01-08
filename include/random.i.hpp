/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#if defined(CLANGD) || defined(IWYU)
#pragma once
#include "random.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_RANDOM_HPP_
#error "Must be included from random.hpp"
#endif
#endif

#include <algorithm>         // for shuffle
#include <cstdint>           // for uint32_t, uint64_t
#include <cstring>           // for strlen, size_t
#include <initializer_list>  // for initializer_list
#include <iterator>          // for distance, iterator_traits, next
#include <type_traits>       // for enable_if

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib {
inline Random::Random() : engine_() {}

inline Random::Random(std::uint32_t seed) : engine_(seed) {}

inline Random::Random(int argc, char** argv) : engine_() { reseed(argc, argv); }

inline auto Random::reseed(std::uint32_t seed) -> void { engine().seed(seed); }

inline auto Random::reseed(int argc, char** argv) -> void {
  // Magic numbers from https://docs.oracle.com/javase/8/docs/api/java/util/Random.html#next-int-
  constexpr std::uint64_t multiplier = 0x5DEECE66Dull;
  constexpr std::uint64_t addend = 0xBull;
  std::uint64_t seed = 3905348978240129619ull;

  for (int i = 1; i < argc; ++i) {
    std::size_t le = std::strlen(argv[i]);
    for (std::size_t j = 0; j < le; ++j) {
      seed = seed * multiplier + static_cast<std::uint32_t>(argv[i][j]) + addend;
    }
    seed += multiplier / addend;
  }

  reseed(static_cast<std::uint32_t>(seed));
}

inline auto Random::engine() -> Engine& { return engine_; }

template <class T>
inline auto Random::next(T from, T to) -> typename std::enable_if<std::is_integral_v<T>, T>::type {
  // Allow range from higher to lower
  if (from < to) {
    return IntegerDist<T>{from, to}(engine());
  }
  return IntegerDist<T>{to, from}(engine());
}

template <class T>
inline auto Random::next(T from, T to) ->
    typename std::enable_if<std::is_floating_point_v<T>, T>::type {
  // Allow range from higher to lower
  if (from < to) {
    return RealDist<T>{from, to}(engine());
  }
  return RealDist<T>{to, from}(engine());
}

template <class T>
inline auto Random::next() -> typename std::enable_if<std::is_same_v<T, bool>, bool>::type {
  return next<bool>(0.5);
}

template <class T>
inline auto Random::next(double true_prob) ->
    typename std::enable_if<std::is_same_v<T, bool>, bool>::type {
  if (true_prob < 0 || true_prob > 1) panic("Random::next failed: true_prob must be in [0, 1]");
  return BoolDist{true_prob}(engine());
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
