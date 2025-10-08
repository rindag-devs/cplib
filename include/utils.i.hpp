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
#include "utils.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_UTILS_HPP_
#error "Must be included from utils.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef CPLIB_USE_FMT_LIB
#include "fmt/base.h"
#include "fmt/core.h"
#else
#include <format>
#endif

/* cplib_embed_ignore start */
#include "macros.hpp"
/* cplib_embed_ignore end */

namespace cplib {
namespace detail {
inline auto has_colors() -> bool {
  // https://bixense.com/clicolors/
  if (std::getenv("NO_COLOR") != nullptr) return false;
  if (std::getenv("CLICOLOR_FORCE") != nullptr) return true;
  return isatty(fileno(stderr));
}

inline auto hex_encode(int c) -> std::string {
  if (c == '\\') {
    return "\\\\";
  } else if (c == '\b') {
    return "\\b";
  } else if (c == '\f') {
    return "\\f";
  } else if (c == '\n') {
    return "\\n";
  } else if (c == '\r') {
    return "\\r";
  } else if (c == '\t') {
    return "\\t";
  } else if (!isprint(c)) {
    return cplib::format("\\x{:02x}", static_cast<int>(c));
  } else {
    return {static_cast<char>(c)};
  }
}

inline auto hex_encode(std::string_view s) -> std::string {
  std::string result;
  for (auto c : s) result += hex_encode(c);
  return result;
}
}  // namespace detail

// Impl panic {{{
namespace detail {
inline UniqueFunction<auto(std::string_view)->void> panic_impl = [](std::string_view s) {
  std::ostream stream(std::cerr.rdbuf());
  stream << "Unrecoverable error: " << s << '\n';
  exit(EXIT_FAILURE);
};
}  // namespace detail

inline auto panic(std::string_view message) -> void {
  detail::panic_impl(message);
  exit(EXIT_FAILURE);  // Usually unnecessary, but in special cases to prevent problems.
}
// /Impl panic }}}

#ifdef CPLIB_USE_FMT_LIB
template <class... Args>
[[nodiscard]] inline auto format(fmt::format_string<Args...> fmt, Args&&... args) -> std::string {
  return fmt::vformat(fmt.get(), fmt::make_format_args(args...));
}
#else
template <class... Args>
[[nodiscard]] inline auto format(std::format_string<Args...> fmt, Args&&... args) -> std::string {
  return std::vformat(fmt.get(), std::make_format_args(args...));
}
#endif

template <class T>
inline auto float_equals(T result, T expected, T max_err) -> bool {
  if (bool x_nan = std::isnan(expected), y_nan = std::isnan(result); x_nan || y_nan) {
    return x_nan && y_nan;
  }
  if (bool x_inf = std::isinf(expected), y_inf = std::isinf(result); x_inf || y_inf) {
    return x_inf && y_inf && (expected > 0) == (result > 0);
  }

  /*
    Consider:
    result = 331997342.4970105
    expected = 331997010.5000000
    max_err = 1e-6
    The verdict will be WA while the ratio result/expected is exactly 1.000001.
    We can fix this by enlarging `max_err` by 1e-15 as for absolute error.
  */
  max_err += 1e-15;

  if (std::abs(expected - result) <= max_err) return true;

  T min_v = std::min<T>(expected * (1.0 - max_err), expected * (1.0 + max_err));
  T max_v = std::max<T>(expected * (1.0 - max_err), expected * (1.0 + max_err));
  return result >= min_v && result <= max_v;
}

template <class T>
inline auto float_delta(T expected, T result) -> T {
  T absolute = std::abs(expected - result);

  if (std::abs(expected) > 1E-9) {
    double relative = std::abs(absolute / expected);
    return std::min(absolute, relative);
  }

  return absolute;
}

inline auto compress(std::string_view s) -> std::string {
  auto t = detail::hex_encode(s);
  if (t.size() <= 64) {
    return {t};
  } else {
    return static_cast<std::string>(t.substr(0, 30)) + "..." +
           static_cast<std::string>(t.substr(t.size() - 31, 31));
  }
}

inline auto trim(std::string_view s) -> std::string {
  if (s.empty()) return std::string(s);

  std::ptrdiff_t left = 0;
  while (left < static_cast<std::ptrdiff_t>(s.size()) && std::isspace(s[left])) ++left;
  if (left >= static_cast<std::ptrdiff_t>(s.size())) return "";

  std::ptrdiff_t right = static_cast<std::ptrdiff_t>(s.size()) - 1;
  while (right >= 0 && std::isspace(s[right])) --right;
  if (right < 0) return "";

  return std::string(s.substr(left, right - left + 1));
}

template <class It>
inline auto join(It first, It last) -> std::string {
  std::ostringstream ss;
  for (It i = first; i != last; ++i) {
    ss << *i;
  }
  return ss.str();
}

template <class It, class Sep>
inline auto join(It first, It last, Sep separator) -> std::string {
  std::ostringstream ss;
  bool repeated = false;
  for (It i = first; i != last; ++i) {
    if (repeated) {
      ss << separator;
    } else {
      repeated = true;
    }
    ss << *i;
  }
  return ss.str();
}

inline auto split(std::string_view s, char separator) -> std::vector<std::string> {
  std::vector<std::string> result;
  std::string item;
  for (char i : s) {
    if (i == separator) {
      result.emplace_back(item);
      item.clear();
    } else {
      item.push_back(i);
    }
  }
  result.emplace_back(item);
  return result;
}

inline auto tokenize(std::string_view s, char separator) -> std::vector<std::string> {
  std::vector<std::string> result;
  std::string item;
  for (char i : s) {
    if (i == separator) {
      if (!item.empty()) result.emplace_back(item);
      item.clear();
    } else {
      item.push_back(i);
    }
  }
  if (!item.empty()) result.emplace_back(item);
  return result;
}

template <typename Ret, typename... Args>
inline UniqueFunction<Ret(Args...)>::UniqueFunction(std::nullptr_t) : ptr(nullptr){};

template <typename Ret, typename... Args>
template <class T>
inline UniqueFunction<Ret(Args...)>::UniqueFunction(T t)
    : ptr(std::make_unique<Data<T>>(std::move(t))){};

template <typename Ret, typename... Args>
auto UniqueFunction<Ret(Args...)>::operator()(Args... args) const -> Ret {
  return (*ptr)(std::forward<Args>(args)...);
}

template <typename Ret, typename... Args>
template <class T>
UniqueFunction<Ret(Args...)>::Data<T>::Data(T&& t) : func(std::forward<T>(t)) {}

template <typename Ret, typename... Args>
template <class T>
auto UniqueFunction<Ret(Args...)>::Data<T>::operator()(Args&&... args) -> Ret {
  return func(std::forward<Args>(args)...);
}

// Impl FlatMap {{{
namespace detail {
/**
 * @brief A comparator for comparing a value_type (pair) with a key.
 * @tparam Compare The original key comparison function.
 */
template <typename Compare>
struct PairKeyCompare {
  [[no_unique_address]] Compare key_comp;

  // Compare pair with key
  template <typename Pair, typename Key>
  auto operator()(const Pair& p, const Key& k) const -> bool {
    return key_comp(p.first, k);
  }
};

}  // namespace detail

// Constructors & Assignment

template <typename Key, typename T, typename Compare>
inline FlatMap<Key, T, Compare>::FlatMap(const Compare& comp) : comp_(comp) {}

template <typename Key, typename T, typename Compare>
template <std::input_iterator It>
inline FlatMap<Key, T, Compare>::FlatMap(It first, It last, const Compare& comp)
    : data_(first, last), comp_(comp) {
  sort_and_unique();
}

template <typename Key, typename T, typename Compare>
inline FlatMap<Key, T, Compare>::FlatMap(std::initializer_list<value_type> ilist,
                                         const Compare& comp)
    : data_(ilist), comp_(comp) {
  sort_and_unique();
}

// Comparison Operators

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::operator==(const FlatMap& other) const -> bool {
  return data_ == other.data_;
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::operator<=>(const FlatMap& other) const {
  return data_ <=> other.data_;
}

// Element Access

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::at(const key_type& key) ->
    typename FlatMap<Key, T, Compare>::mapped_type& {
  auto it = find(key);
  if (it == end()) {
    cplib::panic(cplib::format("FlatMap::at: key not found"));
  }
  return it->second;
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::at(const key_type& key) const -> const
    typename FlatMap<Key, T, Compare>::mapped_type& {
  auto it = find(key);
  if (it == end()) {
    cplib::panic(cplib::format("FlatMap::at: key not found"));
  }
  return it->second;
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::operator[](const key_type& key) ->
    typename FlatMap<Key, T, Compare>::mapped_type& {
  auto it = lower_bound(key);
  if (it != end() && !comp_(key, it->first)) {
    // Key already exists
    return it->second;
  }
  // Key does not exist, insert it
  auto new_it = data_.emplace(it, key, mapped_type{});
  return new_it->second;
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::operator[](key_type&& key) ->
    typename FlatMap<Key, T, Compare>::mapped_type& {
  auto it = lower_bound(key);
  if (it != end() && !comp_(key, it->first)) {
    // Key already exists
    return it->second;
  }
  // Key does not exist, insert it
  auto new_it = data_.emplace(it, std::move(key), mapped_type{});
  return new_it->second;
}

// Iterators

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::begin() -> typename FlatMap<Key, T, Compare>::iterator {
  return data_.begin();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::begin() const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  return data_.begin();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::cbegin() const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  return data_.cbegin();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::end() -> typename FlatMap<Key, T, Compare>::iterator {
  return data_.end();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::end() const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  return data_.end();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::cend() const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  return data_.cend();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::rbegin() ->
    typename FlatMap<Key, T, Compare>::reverse_iterator {
  return data_.rbegin();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::rbegin() const ->
    typename FlatMap<Key, T, Compare>::const_reverse_iterator {
  return data_.rbegin();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::crbegin() const ->
    typename FlatMap<Key, T, Compare>::const_reverse_iterator {
  return data_.crbegin();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::rend() ->
    typename FlatMap<Key, T, Compare>::reverse_iterator {
  return data_.rend();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::rend() const ->
    typename FlatMap<Key, T, Compare>::const_reverse_iterator {
  return data_.rend();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::crend() const ->
    typename FlatMap<Key, T, Compare>::const_reverse_iterator {
  return data_.crend();
}

// Capacity

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::empty() const -> bool {
  return data_.empty();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::size() const -> typename FlatMap<Key, T, Compare>::size_type {
  return data_.size();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::max_size() const ->
    typename FlatMap<Key, T, Compare>::size_type {
  return data_.max_size();
}

// Modifiers

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::insert(const value_type& value)
    -> std::pair<typename FlatMap<Key, T, Compare>::iterator, bool> {
  auto it = lower_bound(value.first);
  if (it != end() && !comp_(value.first, it->first)) {
    return {it, false};  // Key exists
  }
  auto new_it = data_.insert(it, value);
  return {new_it, true};
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::insert(value_type&& value)
    -> std::pair<typename FlatMap<Key, T, Compare>::iterator, bool> {
  auto it = lower_bound(value.first);
  if (it != end() && !comp_(value.first, it->first)) {
    return {it, false};  // Key exists
  }
  auto new_it = data_.insert(it, std::move(value));
  return {new_it, true};
}

template <typename Key, typename T, typename Compare>
template <typename... Args>
inline auto FlatMap<Key, T, Compare>::emplace(Args&&... args)
    -> std::pair<typename FlatMap<Key, T, Compare>::iterator, bool> {
  value_type temp_val(std::forward<Args>(args)...);
  auto it = lower_bound(temp_val.first);
  if (it != end() && !comp_(temp_val.first, it->first)) {
    return {it, false};  // Key exists
  }
  auto new_it = data_.emplace(it, std::move(temp_val));
  return {new_it, true};
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::erase(const_iterator pos) ->
    typename FlatMap<Key, T, Compare>::iterator {
  return data_.erase(pos);
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::erase(const key_type& key) ->
    typename FlatMap<Key, T, Compare>::size_type {
  auto it = find(key);
  if (it == end()) {
    return 0;
  }
  data_.erase(it);
  return 1;
}

// Lookup

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::find(const key_type& key) ->
    typename FlatMap<Key, T, Compare>::iterator {
  auto it = lower_bound(key);
  if (it != end() && !comp_(key, it->first)) {
    return it;  // Key found
  }
  return end();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::find(const key_type& key) const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  auto it = lower_bound(key);
  if (it != end() && !comp_(key, it->first)) {
    return it;  // Key found
  }
  return end();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::contains(const key_type& key) const -> bool {
  return find(key) != end();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::lower_bound(const key_type& key) ->
    typename FlatMap<Key, T, Compare>::iterator {
  return std::lower_bound(data_.begin(), data_.end(), key, detail::PairKeyCompare<Compare>{comp_});
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::lower_bound(const key_type& key) const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  return std::lower_bound(data_.begin(), data_.end(), key, detail::PairKeyCompare<Compare>{comp_});
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::upper_bound(const key_type& key) ->
    typename FlatMap<Key, T, Compare>::iterator {
  return std::upper_bound(data_.begin(), data_.end(), key, detail::PairKeyCompare<Compare>{comp_});
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::upper_bound(const key_type& key) const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  return std::upper_bound(data_.begin(), data_.end(), key, detail::PairKeyCompare<Compare>{comp_});
}

// Private Helpers

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::sort_and_unique() -> void {
  // Sort by key
  std::sort(data_.begin(), data_.end(),
            [this](const value_type& a, const value_type& b) { return comp_(a.first, b.first); });

  // Remove duplicate keys. std::unique keeps the first element in a group of duplicates.
  auto last =
      std::unique(data_.begin(), data_.end(), [this](const value_type& a, const value_type& b) {
        // Two keys are equivalent if !(a < b) && !(b < a)
        return !comp_(a.first, b.first) && !comp_(b.first, a.first);
      });

  data_.erase(last, data_.end());
}
// /Impl FlatMap }}}

// Impl get_work_mode {{{
namespace detail {
inline WorkMode work_mode = WorkMode::NONE;
}

inline auto get_work_mode() -> WorkMode { return detail::work_mode; }
// /Impl get_work_mode }}}
}  // namespace cplib
