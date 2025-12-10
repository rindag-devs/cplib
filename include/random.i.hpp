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
#include <bit>
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

inline constexpr auto int_log2(std::uint64_t x) noexcept -> std::uint32_t {
  if (x == 0) return 0;
  return std::numeric_limits<std::uint64_t>::digits - 1 - std::countl_zero(x);
}

template <class T>
inline constexpr auto scale_int(std::uint64_t x, T size) -> T {
  auto lg = int_log2(size);
  if (std::has_single_bit(static_cast<std::make_unsigned_t<T>>(size))) {
    return x & (size - 1);
  }
  if (lg >= 8 * sizeof(T) - 1) {
    return static_cast<T>(x);
  }
  return x & ((T(1) << (lg + 1)) - 1);
}

/// Get random integer in [0, size).
template <std::integral T>
inline auto rand_int_range(Random::Engine& rnd, T size) -> T {
  T result;
  do {
    result = scale_int<T>(rnd(), size);
  } while (result >= size);
  return result;
}

/// Get random integer in [l,r].
template <std::integral T>
inline auto rand_int_between(Random::Engine& rnd, T l, T r) -> T {
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
inline auto rand_float(Random::Engine& rnd) -> T {
  return static_cast<T>(rnd()) / rnd.max();
}

/// Get random float in [l,r).
template <std::floating_point T>
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

template <std::integral T>
inline auto Random::next(T from, T to) -> T {
  if (from <= to) {
    return detail::rand_int_between<T>(engine(), from, to);
  }
  panic("Random::next failed: from must be <= to");
}

template <std::floating_point T>
inline auto Random::next(T from, T to) -> T {
  // Allow range from higher to lower
  if (from <= to) {
    return detail::rand_float_between<T>(engine(), from, to);
  }
  panic("Random::next failed: from must be <= to");
}

inline auto Random::next_bool() -> bool { return next_bool(0.5); }

inline auto Random::next_bool(double true_prob) -> bool {
  if (true_prob < 0 || true_prob > 1) {
    panic("Random::next_bool failed: true_prob must be in [0, 1]");
  }
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
inline auto Random::choice(Container& container) {
  return choice(std::begin(container), std::end(container));
}

template <MapLike Map>
inline auto Random::weighted_choice(const Map& map) {
  using MappedType = typename Map::mapped_type;

  MappedType total_weight{};
  for (const auto& pair : map) {
    total_weight += pair.second;
  }
  if (total_weight == MappedType(0)) return std::end(map);

  MappedType random_weight = next(MappedType(0), total_weight - 1);
  MappedType cumulative_weight{};

  for (auto it = std::begin(map); it != std::end(map); ++it) {
    cumulative_weight += it->second;
    if (cumulative_weight > random_weight) return it;
  }
  return std::end(map);  // Should not be reached if total_weight > 0
}

template <std::random_access_iterator RandomIt>
inline auto Random::shuffle(RandomIt first, RandomIt last) -> void {
  std::shuffle(first, last, engine());
}

template <std::ranges::random_access_range Container>
inline auto Random::shuffle(Container& container) -> void {
  shuffle(std::begin(container), std::end(container));
}
}  // namespace cplib
