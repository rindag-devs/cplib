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

#ifndef CPLIB_HPP_
#define CPLIB_HPP_

// clang-format off

// --- Start embedded: macros.hpp ---
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

#ifndef CPLIB_MACROS_HPP_
#define CPLIB_MACROS_HPP_

#define CPLIB_VERSION "0.0.1-SNAPSHOT"

#define CPLIB_STARTUP_TEXT       \
  "cplib (CPLib) " CPLIB_VERSION \
  "\n"                           \
  "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023-2026\n"

#if (_WIN32 || __WIN32__ || __WIN32 || _WIN64 || __WIN64__ || __WIN64 || WINNT || __WINNT || \
     __WINNT__ || __CYGWIN__)
#if !defined(_MSC_VER) || _MSC_VER > 1400
#define NOMINMAX 1
#include <windows.h>  
#else
#include <unistd.h>  
#endif
#include <fcntl.h>     
#include <io.h>        
#include <sys/stat.h>  
#define ON_WINDOWS
#if defined(_MSC_VER) && _MSC_VER > 1400
#pragma warning(disable : 4127)
#pragma warning(disable : 4146)
#pragma warning(disable : 4458)
#endif
#else
#include <fcntl.h>      
#include <sys/ioctl.h>  
#include <sys/stat.h>   
#include <unistd.h>     
#endif

#endif
// --- End embedded: macros.hpp ---
// --- Start embedded: utils.hpp ---
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

#ifndef CPLIB_UTILS_HPP_
#define CPLIB_UTILS_HPP_

#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace cplib {
/**
 * Panic the program with a message and exit.
 *
 * If the program has been registered as a CPLib program, `panic` will call the `quit` method of
 * State with `INTERNAL_ERROR` status.
 *
 * @param message The message to print.
 */
[[noreturn]] auto panic(std::string_view message) -> void;

/**
 * Format string using std::format syntax.
template <class... Args>
 *
 * @param fmt The format string.
 * @param args The variadic arguments to be formatted.
 * @return The formatted string.
 */
template <class... Args>
[[nodiscard]] auto format(std::format_string<Args...> fmt, Args &&...args) -> std::string;

template <typename T>
concept formattable = requires(T &v, std::format_context ctx) {
  std::formatter<std::remove_cvref_t<T>>().format(v, ctx);
};

/**
 * Determine whether the two floating-point values are equals within the accuracy range.
 *
 * @tparam T The type of the values.
 * @param result The actual floating-point value.
 * @param expected The expected floating-point value.
 * @param max_err The maximum allowable error.
 * @return True if the values are equal within the accuracy range, false otherwise.
 */
template <class T>
auto float_equals(T result, T expected, T max_err) -> bool;

/**
 * Calculate the minimum relative and absolute error between two floating-point values.
 *
 * @tparam T The type of the values.
 * @param expected The expected floating-point value.
 * @param result The actual floating-point value.
 * @return The minimum error between the values.
 */
template <class T>
auto float_delta(T expected, T result) -> T;

/**
 * Compress string to at most 64 bytes.
 *
 * @param s The input string.
 * @return The compressed string.
 */
auto compress(std::string_view s) -> std::string;

/**
 * Trim spaces at beginning and end of string.
 *
 * @param s The input string.
 * @return The trimmed string.
 */
auto trim(std::string_view s) -> std::string;

/**
 * Concatenate the values between [first,last) into a string without separator.
 *
 * @tparam It The type of the iterator.
 * @param first Iterator to the first value.
 * @param last Iterator to the last value (exclusive).
 * @return The concatenated string.
 */
template <class It>
auto join(It first, It last) -> std::string;

/**
 * Concatenate the values between [first,last) into a string through separator.
 *
 * @tparam It The type of the iterator.
 * @param first Iterator to the first value.
 * @param last Iterator to the last value (exclusive).
 * @param separator The separator.
 * @return The concatenated string.
 */
template <class It, class Sep>
auto join(It first, It last, Sep separator) -> std::string;

/**
 * Splits string s by character separators returning exactly k+1 items, where k is the number of
 * separator occurrences. Split the string into a list of strings using separator.
 *
 * @param s The input string.
 * @param separator The separator character.
 * @return The vector of split strings.
 */
auto split(std::string_view s, char separator) -> std::vector<std::string>;

/**
 * Similar to `split`, but ignores the empty string.
 *
 * @param s The input string.
 * @param separator The separator character.
 * @return The vector of tokenized strings.
 */
auto tokenize(std::string_view s, char separator) -> std::vector<std::string>;

/**
 * `UniqueFunction` is similar to `std::function`, it is a wrapper for functions.
 *
 * The difference between it and `std::function` is that `std::function` requires the wrapped object
 * to be copy-assignable, but `UniqueFunction` only requires it to be move-assignable.
 *
 * In the same way, `UniqueFunction` itself cannot be copy-assignable, but can only be
 * move-assignable.
 *
 * @tparam T The type of the stored function.
 */
template <typename T>
struct UniqueFunction;

/**
 * Template specialization for Ret(Args...) type.
 *
 * @tparam Ret The return type of the stored function.
 * @tparam Args The argument types of the stored function.
 */
template <typename Ret, typename... Args>
struct UniqueFunction<Ret(Args...)> {
 public:
  /**
   * Creates an empty UniqueFunction.
   */
  UniqueFunction() = default;

  /**
   * Creates an empty UniqueFunction.
   */
  explicit UniqueFunction(std::nullptr_t);

  /**
   * Constructor.
   *
   * @param t The function to be stored.
   * @tparam T The type of the function to be stored.
   */
  template <class T>
  UniqueFunction(T t);  // NOLINT(google-explicit-constructor)

  [[nodiscard]] explicit operator bool() const noexcept;

  /**
   * Function call operator.
   *
   * @param args The arguments to be passed to the stored function.
   * @return The return value of the stored function.
   */
  auto operator()(Args... args) const -> Ret;

 private:
  /**
   * Base class for polymorphism.
   */
  struct Base {
    virtual ~Base() = default;

    /**
     * Function call operator for derived classes.
     *
     * @param args The arguments to be passed to the derived function.
     * @return The return value of the derived function.
     */
    virtual auto operator()(Args &&...args) -> Ret = 0;
  };

  /**
   * Data class storing the actual function.
   *
   * @tparam T The type of the stored function.
   */
  template <typename T>
  struct Data : Base {
    T func;

    explicit Data(T &&t);

    /**
     * Function call operator implementation.
     *
     * @param args The arguments to be passed to the stored function.
     * @return The return value of the stored function.
     */
    auto operator()(Args &&...args) -> Ret override;
  };

  std::unique_ptr<Base> ptr{nullptr};
};

/**
 * @brief A map-like container implemented as a sorted std::vector of pairs.
 *
 * @details This container provides an interface similar to std::map but with
 * different performance characteristics. Lookups (find, lower_bound, etc.) are
 * O(log n), while insertions and deletions are O(n) due to the underlying
 * vector. It is cache-friendly and can be faster than std::map for small
 * datasets or when lookups dominate the workload.
 *
 * @tparam Key The key type.
 * @tparam T The mapped type.
 * @tparam Compare The comparison function object type.
 */
template <typename Key, typename T, typename Compare = std::less<Key>>
struct FlatMap {
 public:
  // Member Types
  using key_type = Key;
  using mapped_type = T;
  using value_type = std::pair<Key, T>;
  using key_compare = Compare;
  using reference = value_type &;
  using const_reference = const value_type &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = typename std::vector<value_type>::iterator;
  using const_iterator = typename std::vector<value_type>::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  // Data Members
  std::vector<value_type> data_;
  [[no_unique_address]] key_compare comp_;

 public:
  // Constructors & Assignment

  /** @brief Default constructor. */
  FlatMap() = default;

  /** @brief Constructs with a specific comparator. */
  explicit FlatMap(const Compare &comp);

  /** @brief Constructs from a range of elements. */
  template <std::input_iterator It>
  explicit FlatMap(It first, It last, const Compare &comp = Compare{});

  /** @brief Constructs from an initializer list. */
  FlatMap(std::initializer_list<value_type> ilist, const Compare &comp = Compare{});

  // Defaulted copy/move constructors and assignment operators are sufficient.
  FlatMap(const FlatMap &) = default;
  FlatMap(FlatMap &&) = default;
  auto operator=(const FlatMap &) -> FlatMap & = default;
  auto operator=(FlatMap &&) -> FlatMap & = default;

  // Comparison Operators
  auto operator==(const FlatMap &other) const -> bool;
  auto operator<=>(const FlatMap &other) const;

  // Element Access
  /** @brief Access specified element with bounds checking. */
  auto at(const key_type &key) -> mapped_type &;
  auto at(const key_type &key) const -> const mapped_type &;

  /** @brief Access or insert specified element. */
  auto operator[](const key_type &key) -> mapped_type &;
  auto operator[](key_type &&key) -> mapped_type &;

  // Iterators

  auto begin() -> iterator;
  [[nodiscard]] auto begin() const -> const_iterator;
  [[nodiscard]] auto cbegin() const -> const_iterator;

  auto end() -> iterator;
  [[nodiscard]] auto end() const -> const_iterator;
  [[nodiscard]] auto cend() const -> const_iterator;

  auto rbegin() -> reverse_iterator;
  [[nodiscard]] auto rbegin() const -> const_reverse_iterator;
  [[nodiscard]] auto crbegin() const -> const_reverse_iterator;

  auto rend() -> reverse_iterator;
  [[nodiscard]] auto rend() const -> const_reverse_iterator;
  [[nodiscard]] auto crend() const -> const_reverse_iterator;

  // Capacity

  /** @brief Checks if the container is empty. */
  [[nodiscard]] auto empty() const -> bool;

  /** @brief Returns the number of elements. */
  [[nodiscard]] auto size() const -> size_type;

  /** @brief Returns the maximum possible number of elements. */
  [[nodiscard]] auto max_size() const -> size_type;

  /** @brief Reserves storage for at least the specified number of elements. */
  auto reserve(size_type new_cap) -> void;

  /** @brief Returns the current storage capacity. */
  [[nodiscard]] auto capacity() const -> size_type;

  // Modifiers

  /** @brief Inserts a new element into the container. */
  auto insert(const value_type &value) -> std::pair<iterator, bool>;
  auto insert(value_type &&value) -> std::pair<iterator, bool>;

  /** @brief Constructs an element in-place. */
  template <typename... Args>
  auto emplace(Args &&...args) -> std::pair<iterator, bool>;

  /** @brief Erases an element at a specific position. */
  auto erase(const_iterator pos) -> iterator;

  /** @brief Erases an element with a specific key. */
  auto erase(const key_type &key) -> size_type;

  // Lookup

  /** @brief Finds an element with a specific key. */
  auto find(const key_type &key) -> iterator;
  auto find(const key_type &key) const -> const_iterator;

  /** @brief Checks if the container contains an element with a specific key. */
  auto contains(const key_type &key) const -> bool;

  /** @brief Returns an iterator to the first element not less than the given key. */
  auto lower_bound(const key_type &key) -> iterator;
  auto lower_bound(const key_type &key) const -> const_iterator;

  /** @brief Returns an iterator to the first element greater than the given key. */
  auto upper_bound(const key_type &key) -> iterator;
  auto upper_bound(const key_type &key) const -> const_iterator;

 private:
  /**
   * @brief A helper to sort and unique the underlying vector.
   * @details This is used by constructors that take a range of elements.
   * It sorts the data and removes duplicate keys, keeping the first
   * occurrence, which matches the behavior of std::map's range constructor.
   */
  auto sort_and_unique() -> void;
};

/**
 * `WorkMode` indicates the current mode of cplib.
 */
enum struct WorkMode : std::uint8_t {
  NONE,
  CHECKER,
  INTERACTOR,
  VALIDATOR,
  GENERATOR,
};

/**
 * Get current work mode of cplib.
 *
 * @return The current work mode.
 */
auto get_work_mode() -> WorkMode;
}  // namespace cplib

// --- Start embedded: utils.i.hpp ---
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


#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


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
  } else if (std::isprint(static_cast<unsigned char>(c)) == 0) {
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
// `panic_impl` is process-global mutable state. Any replacement installed here must remain valid
// for the rest of the program lifetime, because `panic()` may still be reached from static-lifetime
// objects after the installing scope has otherwise exited.
inline UniqueFunction<auto(std::string_view)->void> panic_impl = [](std::string_view s) -> void {
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

template <class... Args>
[[nodiscard]] inline auto format(std::format_string<Args...> fmt, Args &&...args) -> std::string {
  return std::vformat(fmt.get(), std::make_format_args(args...));
}

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
  while (left < static_cast<std::ptrdiff_t>(s.size()) &&
         std::isspace(static_cast<unsigned char>(s[left])) != 0) {
    ++left;
  }
  if (left >= static_cast<std::ptrdiff_t>(s.size())) return "";

  std::ptrdiff_t right = static_cast<std::ptrdiff_t>(s.size()) - 1;
  while (right >= 0 && std::isspace(static_cast<unsigned char>(s[right])) != 0) --right;
  if (right < 0) return "";

  return std::string(s.substr(left, right - left + 1));
}

template <class It>
inline auto join(It first, It last) -> std::string {
  std::string result;
  for (It i = first; i != last; ++i) {
    std::format_to(std::back_inserter(result), "{}", *i);
  }
  return result;
}

template <class It, class Sep>
inline auto join(It first, It last, Sep separator) -> std::string {
  std::string result;
  bool repeated = false;
  for (It i = first; i != last; ++i) {
    if (repeated) {
      std::format_to(std::back_inserter(result), "{}", separator);
    } else {
      repeated = true;
    }
    std::format_to(std::back_inserter(result), "{}", *i);
  }
  return result;
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
inline UniqueFunction<Ret(Args...)>::operator bool() const noexcept {
  return ptr != nullptr;
}

template <typename Ret, typename... Args>
template <class T>
inline UniqueFunction<Ret(Args...)>::UniqueFunction(T t)
    : ptr(std::make_unique<Data<T>>(std::move(t))){};

template <typename Ret, typename... Args>
auto UniqueFunction<Ret(Args...)>::operator()(Args... args) const -> Ret {
  if (!ptr) {
    cplib::panic("Attempted to call an empty UniqueFunction");
  }
  return (*ptr)(std::forward<Args>(args)...);
}

template <typename Ret, typename... Args>
template <class T>
UniqueFunction<Ret(Args...)>::Data<T>::Data(T &&t) : func(std::forward<T>(t)) {}

template <typename Ret, typename... Args>
template <class T>
auto UniqueFunction<Ret(Args...)>::Data<T>::operator()(Args &&...args) -> Ret {
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
  auto operator()(const Pair &p, const Key &k) const -> bool {
    return key_comp(p.first, k);
  }
};

}  // namespace detail

// Constructors & Assignment

template <typename Key, typename T, typename Compare>
inline FlatMap<Key, T, Compare>::FlatMap(const Compare &comp) : comp_(comp) {}

template <typename Key, typename T, typename Compare>
template <std::input_iterator It>
inline FlatMap<Key, T, Compare>::FlatMap(It first, It last, const Compare &comp)
    : data_(first, last), comp_(comp) {
  sort_and_unique();
}

template <typename Key, typename T, typename Compare>
inline FlatMap<Key, T, Compare>::FlatMap(std::initializer_list<value_type> ilist,
                                         const Compare &comp)
    : data_(ilist), comp_(comp) {
  sort_and_unique();
}

// Comparison Operators

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::operator==(const FlatMap &other) const -> bool {
  return data_ == other.data_;
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::operator<=>(const FlatMap &other) const {
  return data_ <=> other.data_;
}

// Element Access

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::at(const key_type &key) ->
    typename FlatMap<Key, T, Compare>::mapped_type & {
  auto it = find(key);
  if (it == end()) {
    cplib::panic(cplib::format("FlatMap::at: key not found"));
  }
  return it->second;
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::at(const key_type &key) const -> const
    typename FlatMap<Key, T, Compare>::mapped_type & {
  auto it = find(key);
  if (it == end()) {
    cplib::panic(cplib::format("FlatMap::at: key not found"));
  }
  return it->second;
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::operator[](const key_type &key) ->
    typename FlatMap<Key, T, Compare>::mapped_type & {
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
inline auto FlatMap<Key, T, Compare>::operator[](key_type &&key) ->
    typename FlatMap<Key, T, Compare>::mapped_type & {
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

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::reserve(typename FlatMap<Key, T, Compare>::size_type new_cap)
    -> void {
  data_.reserve(new_cap);
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::capacity() const ->
    typename FlatMap<Key, T, Compare>::size_type {
  return data_.capacity();
}

// Modifiers

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::insert(const value_type &value)
    -> std::pair<typename FlatMap<Key, T, Compare>::iterator, bool> {
  auto it = lower_bound(value.first);
  if (it != end() && !comp_(value.first, it->first)) {
    return {it, false};  // Key exists
  }
  auto new_it = data_.insert(it, value);
  return {new_it, true};
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::insert(value_type &&value)
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
inline auto FlatMap<Key, T, Compare>::emplace(Args &&...args)
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
inline auto FlatMap<Key, T, Compare>::erase(const key_type &key) ->
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
inline auto FlatMap<Key, T, Compare>::find(const key_type &key) ->
    typename FlatMap<Key, T, Compare>::iterator {
  auto it = lower_bound(key);
  if (it != end() && !comp_(key, it->first)) {
    return it;  // Key found
  }
  return end();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::find(const key_type &key) const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  auto it = lower_bound(key);
  if (it != end() && !comp_(key, it->first)) {
    return it;  // Key found
  }
  return end();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::contains(const key_type &key) const -> bool {
  return find(key) != end();
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::lower_bound(const key_type &key) ->
    typename FlatMap<Key, T, Compare>::iterator {
  return std::lower_bound(data_.begin(), data_.end(), key, detail::PairKeyCompare<Compare>{comp_});
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::lower_bound(const key_type &key) const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  return std::lower_bound(data_.begin(), data_.end(), key, detail::PairKeyCompare<Compare>{comp_});
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::upper_bound(const key_type &key) ->
    typename FlatMap<Key, T, Compare>::iterator {
  return std::upper_bound(
      data_.begin(), data_.end(), key,
      [this](const key_type &lhs, const value_type &rhs) -> bool { return comp_(lhs, rhs.first); });
}

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::upper_bound(const key_type &key) const ->
    typename FlatMap<Key, T, Compare>::const_iterator {
  return std::upper_bound(
      data_.begin(), data_.end(), key,
      [this](const key_type &lhs, const value_type &rhs) -> bool { return comp_(lhs, rhs.first); });
}

// Private Helpers

template <typename Key, typename T, typename Compare>
inline auto FlatMap<Key, T, Compare>::sort_and_unique() -> void {
  // Sort by key
  std::sort(data_.begin(), data_.end(),
            [this](const value_type &a, const value_type &b) { return comp_(a.first, b.first); });

  // Remove duplicate keys. std::unique keeps the first element in a group of duplicates.
  auto last =
      std::unique(data_.begin(), data_.end(), [this](const value_type &a, const value_type &b) {
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
// --- End embedded: utils.i.hpp ---

#endif
// --- End embedded: utils.hpp ---
// --- Start embedded: no_rand.hpp ---
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

#ifndef CPLIB_NO_RAND_HPP_
#define CPLIB_NO_RAND_HPP_



#include <cstdlib>


/*
 * CPLib generators must be exactly reproducible from their declared seed and arguments. Blocking
 * C library RNG entry points prevents accidental use of hidden global state or implementation-
 * defined sequences that would make generated data depend on the host environment.
 */

#ifdef __GLIBC__
#define CPLIB_RAND_THROW_STATEMENT noexcept(true)
#else
#define CPLIB_RAND_THROW_STATEMENT
#endif

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((error("Do not use `rand`, use `rnd.next` instead")))
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4273)
#endif
inline auto rand() CPLIB_RAND_THROW_STATEMENT -> int {
  cplib::panic("Do not use `rand`, use `rnd.next` instead");
}

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((error("Do not use `srand`, use `cplib::Random` for random generator")))
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4273)
#endif
inline auto srand(unsigned int) CPLIB_RAND_THROW_STATEMENT -> void {
  cplib::panic("Do not use `srand`, use `cplib::Random` for random generator");
}

#endif
// --- End embedded: no_rand.hpp ---
// --- Start embedded: json.hpp ---
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

#ifndef CPLIB_JSON_HPP_
#define CPLIB_JSON_HPP_

#include <cstdint>
#include <optional>
#include <streambuf>
#include <string>
#include <string_view>
#include <variant>
#include <vector>


namespace cplib::json {

struct Value;

using Null = std::nullopt_t;
using String = std::string;
using Int = std::int64_t;
using Real = double;
using Bool = bool;
using List = std::vector<Value>;
using Map = FlatMap<std::string, Value>;

struct Raw {
  std::string inner;
  explicit Raw(std::string inner);
};

struct Value {
  std::variant<Null, String, Int, Real, Bool, List, Map, Raw> inner;

  auto write_string(std::streambuf &buf) const -> void;
  [[nodiscard]] auto to_string() const -> std::string;

  [[nodiscard]] auto is_null() const -> bool;
  [[nodiscard]] auto is_string() const -> bool;
  [[nodiscard]] auto is_int() const -> bool;
  [[nodiscard]] auto is_real() const -> bool;
  [[nodiscard]] auto is_bool() const -> bool;
  [[nodiscard]] auto is_list() const -> bool;
  [[nodiscard]] auto is_map() const -> bool;

  [[nodiscard]] auto as_string() -> String &;
  [[nodiscard]] auto as_string() const -> const String &;

  [[nodiscard]] auto as_int() -> Int &;
  [[nodiscard]] auto as_int() const -> const Int &;

  [[nodiscard]] auto as_real() -> Real &;
  [[nodiscard]] auto as_real() const -> const Real &;

  [[nodiscard]] auto as_bool() -> Bool &;
  [[nodiscard]] auto as_bool() const -> const Bool &;

  [[nodiscard]] auto as_list() -> List &;
  [[nodiscard]] auto as_list() const -> const List &;

  [[nodiscard]] auto as_map() -> Map &;
  [[nodiscard]] auto as_map() const -> const Map &;

  static auto encode_list(std::streambuf &buf, const List &inner) -> void;
  static auto encode_map(std::streambuf &buf, const Map &inner) -> void;
  static auto encode_string(std::streambuf &buf, std::string_view inner) -> void;
};

}  // namespace cplib::json

// --- Start embedded: json.i.hpp ---
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


#include <array>
#include <charconv>
#include <ios>
#include <sstream>
#include <streambuf>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>


namespace cplib::json {

namespace detail {
inline auto write_json_int(std::streambuf &buf, Int value) -> void {
  std::array<char, 32> digits;
  auto [ptr, ec] = std::to_chars(digits.data(), digits.data() + digits.size(), value);
  if (ec != std::errc()) {
    panic("JSON integer to string failed");
  }
  buf.sputn(digits.data(), ptr - digits.data());
}

inline auto write_json_real(std::streambuf &buf, Real value) -> void {
  std::array<char, 64> digits;
  auto [ptr, ec] = std::to_chars(digits.data(), digits.data() + digits.size(), value,
                                 std::chars_format::general, 10);
  if (ec != std::errc()) {
    panic("JSON real to string failed");
  }
  buf.sputn(digits.data(), ptr - digits.data());
}
}  // namespace detail

inline Raw::Raw(std::string inner) : inner(std::move(inner)) {}

inline auto Value::encode_string(std::streambuf &buf, std::string_view inner) -> void {
  buf.sputc('\"');
  for (char c : inner) {
    switch (c) {
      case '"':
        buf.sputc('\\');
        buf.sputc('\"');
        break;
      case '\\':
        buf.sputc('\\');
        buf.sputc('\\');
        break;
      case '\b':
        buf.sputc('\\');
        buf.sputc('b');
        break;
      case '\f':
        buf.sputc('\\');
        buf.sputc('f');
        break;
      case '\n':
        buf.sputc('\\');
        buf.sputc('n');
        break;
      case '\r':
        buf.sputc('\\');
        buf.sputc('r');
        break;
      case '\t':
        buf.sputc('\\');
        buf.sputc('t');
        break;
      default:
        if (('\x00' <= c && c <= '\x1f') || c == '\x7f') {
          buf.sputc('\\');
          buf.sputc('u');
          for (int i = 3; i >= 0; --i) {
            unsigned int digit = (static_cast<unsigned int>(c) >> (i * 4)) & 0x0F;
            buf.sputc(static_cast<char>(digit < 10 ? '0' + digit : 'a' + (digit - 10)));
          }
        } else {
          buf.sputc(c);
        }
    }
  }
  buf.sputc('\"');
}

inline auto Value::encode_list(std::streambuf &buf, const List &inner) -> void {
  buf.sputc('[');
  if (!inner.empty()) {
    auto it = inner.begin();
    it->write_string(buf);
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      it->write_string(buf);
    }
  }
  buf.sputc(']');
}

inline auto Value::encode_map(std::streambuf &buf, const Map &inner) -> void {
  buf.sputc('{');
  if (!inner.empty()) {
    auto it = inner.begin();
    encode_string(buf, it->first);
    buf.sputc(':');
    it->second.write_string(buf);
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      encode_string(buf, it->first);
      buf.sputc(':');
      it->second.write_string(buf);
    }
  }
  buf.sputc('}');
}

inline auto Value::write_string(std::streambuf &buf) const -> void {
  std::visit(
      [&buf](const auto &arg) -> void {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, Null>) {
          constexpr std::string_view NULL_STR = "null";
          buf.sputn(NULL_STR.data(), NULL_STR.size());
        } else if constexpr (std::is_same_v<T, String>) {
          encode_string(buf, arg);
        } else if constexpr (std::is_same_v<T, Int>) {
          detail::write_json_int(buf, arg);
        } else if constexpr (std::is_same_v<T, Real>) {
          detail::write_json_real(buf, arg);
        } else if constexpr (std::is_same_v<T, Bool>) {
          if (arg) {
            constexpr std::string_view TRUE_STR = "true";
            buf.sputn(TRUE_STR.data(), TRUE_STR.size());
          } else {
            constexpr std::string_view FALSE_STR = "false";
            buf.sputn(FALSE_STR.data(), FALSE_STR.size());
          }
        } else if constexpr (std::is_same_v<T, List>) {
          encode_list(buf, arg);
        } else if constexpr (std::is_same_v<T, Map>) {
          encode_map(buf, arg);
        } else if constexpr (std::is_same_v<T, Raw>) {
          buf.sputn(arg.inner.data(), arg.inner.size());
        } else {
          panic("JSON value to string failed: unknown type");
        }
      },
      inner);
}

[[nodiscard]] inline auto Value::to_string() const -> std::string {
  std::stringbuf buf(std::ios_base::out);
  write_string(buf);
  return buf.str();
}

[[nodiscard]] inline auto Value::is_null() const -> bool {
  return std::holds_alternative<Null>(inner);
}
[[nodiscard]] inline auto Value::is_string() const -> bool {
  return std::holds_alternative<String>(inner);
}
[[nodiscard]] inline auto Value::is_int() const -> bool {
  return std::holds_alternative<Int>(inner);
}
[[nodiscard]] inline auto Value::is_real() const -> bool {
  return std::holds_alternative<Real>(inner);
}
[[nodiscard]] inline auto Value::is_bool() const -> bool {
  return std::holds_alternative<Bool>(inner);
}
[[nodiscard]] inline auto Value::is_list() const -> bool {
  return std::holds_alternative<List>(inner);
}
[[nodiscard]] inline auto Value::is_map() const -> bool {
  return std::holds_alternative<Map>(inner);
}

[[nodiscard]] inline auto Value::as_string() -> String & { return std::get<String>(inner); }
[[nodiscard]] inline auto Value::as_string() const -> const String & {
  return std::get<String>(inner);
}

[[nodiscard]] inline auto Value::as_int() -> Int & { return std::get<Int>(inner); }
[[nodiscard]] inline auto Value::as_int() const -> const Int & { return std::get<Int>(inner); }

[[nodiscard]] inline auto Value::as_real() -> Real & { return std::get<Real>(inner); }
[[nodiscard]] inline auto Value::as_real() const -> const Real & { return std::get<Real>(inner); }

[[nodiscard]] inline auto Value::as_bool() -> Bool & { return std::get<Bool>(inner); }
[[nodiscard]] inline auto Value::as_bool() const -> const Bool & { return std::get<Bool>(inner); }

[[nodiscard]] inline auto Value::as_list() -> List & { return std::get<List>(inner); }
[[nodiscard]] inline auto Value::as_list() const -> const List & { return std::get<List>(inner); }

[[nodiscard]] inline auto Value::as_map() -> Map & { return std::get<Map>(inner); }
[[nodiscard]] inline auto Value::as_map() const -> const Map & { return std::get<Map>(inner); }

}  // namespace cplib::json
// --- End embedded: json.i.hpp ---

#endif
// --- End embedded: json.hpp ---
// --- Start embedded: pattern.hpp ---
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

#ifndef CPLIB_PATTERN_HPP_
#define CPLIB_PATTERN_HPP_

#include <string>
#include <string_view>

#include "regex.h"

namespace cplib {

/**
 * Regex pattern in POSIX-Extended style. Used for matching strings.
 *
 * Format see
 * `https://en.wikibooks.org/wiki/Regular_Expressions/POSIX-Extended_Regular_Expressions`.
 */
struct Pattern {
 public:
  /**
   * Create pattern instance by string.
   *
   * @param src The source string representing the regex pattern.
   */
  explicit Pattern(std::string src);

  Pattern(const Pattern &other);
  auto operator=(const Pattern &other) -> Pattern &;
  Pattern(Pattern &&) noexcept = default;
  auto operator=(Pattern &&) noexcept -> Pattern & = default;

  /**
   * Checks if given string matches the pattern.
   *
   * @param s The input string to be matched against the pattern.
   * @return True if the given string matches the pattern, False otherwise.
   */
  [[nodiscard]] auto match(std::string_view s) const -> bool;

  /**
   * Returns the source string of the pattern.
   *
   * @return The source string representing the regex pattern.
   */
  [[nodiscard]] auto src() const -> std::string_view;

 private:
  struct RegexHandle {
    regex_t regex{};
    bool compiled = false;

    RegexHandle() = default;
    RegexHandle(const RegexHandle &) = delete;
    auto operator=(const RegexHandle &) -> RegexHandle & = delete;
    RegexHandle(RegexHandle &&other) noexcept;
    auto operator=(RegexHandle &&other) noexcept -> RegexHandle &;
    ~RegexHandle();
  };

  static auto compile_regex(std::string_view src) -> RegexHandle;

  std::string src_;
  RegexHandle re_;
};
}  // namespace cplib

// --- Start embedded: pattern.i.hpp ---
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


#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "regex.h"


namespace cplib {
inline Pattern::RegexHandle::RegexHandle(RegexHandle &&other) noexcept
    : regex(other.regex), compiled(std::exchange(other.compiled, false)) {}

inline auto Pattern::RegexHandle::operator=(RegexHandle &&other) noexcept -> RegexHandle & {
  if (this == &other) {
    return *this;
  }
  if (compiled) {
    regfree(&regex);
  }
  regex = other.regex;
  compiled = std::exchange(other.compiled, false);
  return *this;
}

inline Pattern::RegexHandle::~RegexHandle() {
  if (compiled) {
    regfree(&regex);
  }
}

namespace detail {

inline auto get_regex_err_msg(int err_code, const regex_t *re) -> std::string {
  std::size_t len = regerror(err_code, re, nullptr, 0);
  std::string buf(len, 0);
  regerror(err_code, re, buf.data(), len);
  return buf;
}
}  // namespace detail

inline auto Pattern::compile_regex(std::string_view src) -> RegexHandle {
  Pattern::RegexHandle re;
  if (int err =
          regcomp(&re.regex, ("^" + std::string(src) + "$").c_str(), REG_EXTENDED | REG_NOSUB);
      err) {
    auto err_msg = detail::get_regex_err_msg(err, &re.regex);
    panic("Pattern constructor failed: " + err_msg);
  }
  re.compiled = true;
  return re;
}

inline Pattern::Pattern(std::string src) : src_(std::move(src)), re_(compile_regex(src_)) {}

inline Pattern::Pattern(const Pattern &other) : src_(other.src_), re_(compile_regex(src_)) {}

inline auto Pattern::operator=(const Pattern &other) -> Pattern & {
  if (this == &other) {
    return *this;
  }
  src_ = other.src_;
  re_ = compile_regex(src_);
  return *this;
}

inline auto Pattern::match(std::string_view s) const -> bool {
#ifdef REG_STARTEND
  regmatch_t match_range{};
  match_range.rm_so = 0;
  match_range.rm_eo = static_cast<regoff_t>(s.size());
  const char *input = s.empty() ? "" : std::addressof(s.front());
  int result = regexec(&re_.regex, input, 1, &match_range, REG_STARTEND);
#else
  const std::string buffer(s);
  int result = regexec(&re_.regex, buffer.c_str(), 0, nullptr, 0);
#endif

  if (!result) return true;

  if (result == REG_NOMATCH) return false;

  auto err_msg = detail::get_regex_err_msg(result, &re_.regex);
  panic("Pattern match failed: " + err_msg);
  return false;
}

inline auto Pattern::src() const -> std::string_view { return src_; }
}  // namespace cplib
// --- End embedded: pattern.i.hpp ---

#endif
// --- End embedded: pattern.hpp ---
// --- Start embedded: random.hpp ---
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

#ifndef CPLIB_RANDOM_HPP_
#define CPLIB_RANDOM_HPP_

#include <array>
#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

namespace cplib {

// Concept to check if a type behaves like a map for weighted choice.
// It must be a range, and its elements must have a 'second' member we can use as a weight.
template <typename T>
concept MapLike = std::ranges::range<T> && requires(const std::ranges::range_value_t<T> &value) {
  typename T::mapped_type;
  { value.second } -> std::convertible_to<const typename T::mapped_type &>;
};

/**
 * Random number generator that provides various methods to generate random numbers and perform
 * random operations.
 */
struct Random {
 public:
  struct Engine {
   public:
    using result_type = std::uint64_t;

    Engine() noexcept;
    explicit Engine(std::uint64_t seed) noexcept;

    static constexpr auto min() noexcept -> result_type { return 0; }
    static constexpr auto max() noexcept -> result_type { return ~static_cast<result_type>(0); }

    auto seed(std::uint64_t seed) noexcept -> void;
    auto operator()() noexcept -> result_type;
    auto jump() noexcept -> void;
    auto long_jump() noexcept -> void;

   private:
    std::array<result_type, 4> state_{};
  };

  /**
   * Construct a random generator with default seed.
   */
  explicit Random();

  /**
   * Construct a random generator with given seed.
   *
   * @param seed The seed of the random generator.
   */
  explicit Random(std::uint64_t seed);

  /**
   * Construct a random generator with seed which generated command-line arguments.
   *
   * @param args The command-line arguments.
   */
  explicit Random(const std::vector<std::string> &args);

  /**
   * Reseed the generator with a new seed.
   *
   * @param seed The new seed value.
   */
  auto reseed(std::uint64_t seed) -> void;

  /**
   * Reseed the generator with a seed generated by `argc` and `argv`.
   *
   * @param args The command-line arguments.
   */
  auto reseed(const std::vector<std::string> &args) -> void;

  /**
   * Get the engine used by the random generator.
   *
   * @return A reference to the engine.
   */
  auto engine() -> Engine &;

  /**
   * Generate a random integer in the range [from, to].
   *
   * @tparam T The type of the integer.
   * @param from The lower bound of the range.
   * @param to The upper bound of the range.
   * @return The randomly generated integer.
   */
  template <std::integral T>
  auto next(T from, T to) -> T;

  /**
   * Generate a random floating-point number in the range [from, to].
   *
   * @tparam T The type of the floating-point number.
   * @param from The lower bound of the range.
   * @param to The upper bound of the range.
   * @return The randomly generated floating-point number.
   */
  template <std::floating_point T>
  auto next(T from, T to) -> T;

  /**
   * Generate a random boolean value with 50% probability of being true.
   *
   * @return The randomly generated boolean value.
   */
  auto next_bool() -> bool;

  /**
   * Generate a random boolean value with a given probability of being true.
   *
   * @param true_prob The probability of the boolean being true.
   * @return The randomly generated boolean value.
   */
  auto next_bool(double true_prob) -> bool;

  /**
   * Generate a weighted random number in the range [from, to] with specified weighting type.
   *
   * @tparam T The type of the number (must be integral or floating-point).
   * @param from The lower bound of the range.
   * @param to The upper bound of the range.
   * @param type The weighting type. Positive values prefer higher numbers, negative values prefer
   * lower numbers.
   * @return The weighted random number.
   */
  template <typename T>
    requires std::integral<T> || std::floating_point<T>
  auto wnext(T from, T to, int type) -> T;

  /**
   * Return a random value from the given initializer_list.
   *
   * @tparam T The type of the value.
   * @param init_list The initializer_list to choose from.
   * @return A random value from the initializer_list.
   */
  template <class T>
  auto choice(std::initializer_list<T> init_list) -> T;

  /**
   * Return a random iterator from the given range.
   *
   * @tparam It The type of the iterator.
   * @param first The beginning of the range.
   * @param last The end of the range.
   * @return A random iterator from the range.
   */
  template <std::forward_iterator It>
  auto choice(It first, It last) -> It;

  /**
   * Return a random iterator from the given container.
   *
   * @tparam Container The type of the container.
   * @param container The container to choose from.
   * @return A random iterator from the container.
   */
  template <std::ranges::forward_range Container>
  auto choice(Container &container);

  /**
   * Return a random iterator from the given map container by utilizing the values of the map
   * container as weights for weighted random number generation.
   *
   * @tparam Map The type of the map container.
   * @param map The map container to choose from.
   * @return A random iterator from the map container.
   */
  template <MapLike Map>
  auto weighted_choice(const Map &map);

  /**
   * Shuffle the elements in the given range.
   *
   * @tparam RandomIt The type of the random access iterator.
   * @param first The beginning of the range.
   * @param last The end of the range.
   */
  template <std::random_access_iterator RandomIt>
  auto shuffle(RandomIt first, RandomIt last) -> void;

  /**
   * Shuffle the elements in the given container.
   *
   * @tparam Container The type of the container.
   * @param container The container to shuffle.
   */
  template <std::ranges::random_access_range Container>
  auto shuffle(Container &container) -> void;

 private:
  Engine engine_;
};
}  // namespace cplib

// --- Start embedded: random.i.hpp ---
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


namespace cplib {
namespace detail {

template <std::integral T>
using wnext_uint_t =
    std::conditional_t<(sizeof(T) < sizeof(std::uint64_t)), std::uint64_t, std::make_unsigned_t<T>>;

template <std::integral T>
inline auto wnext_integral_width(T from, T to) -> wnext_uint_t<T> {
  using UnsignedT = wnext_uint_t<T>;
  return static_cast<UnsignedT>(to) - static_cast<UnsignedT>(from) + UnsignedT(1);
}

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
      using UnsignedT = detail::wnext_uint_t<T>;
      const auto width = detail::wnext_integral_width(from, to);
      UnsignedT offset;
      if (width == 0) {
        constexpr auto bits = std::numeric_limits<UnsignedT>::digits;
        const auto scaled = std::ldexp(static_cast<long double>(p), bits);
        const auto max_value = static_cast<long double>(std::numeric_limits<UnsignedT>::max());
        offset = scaled >= max_value ? std::numeric_limits<UnsignedT>::max()
                                     : static_cast<UnsignedT>(scaled);
      } else {
        offset = static_cast<UnsignedT>(p * static_cast<double>(width));
      }
      T result = static_cast<T>(static_cast<UnsignedT>(from) + offset);
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
// --- End embedded: random.i.hpp ---

#endif
// --- End embedded: random.hpp ---
// --- Start embedded: io.hpp ---
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

#ifndef CPLIB_IO_HPP_
#define CPLIB_IO_HPP_

#include <array>
#include <cstddef>
#include <cstdio>
#include <ios>
#include <memory>
#include <optional>
#include <streambuf>
#include <string>
#include <string_view>


namespace cplib::io {

/// Represents a position in a file.
struct Position {
  /// Line number, starting from 0.
  std::size_t line;

  /// Column number, starting from 0.
  std::size_t col;

  /// Byte number, starting from 0.
  std::size_t byte;

  explicit Position();
  explicit Position(std::size_t line, std::size_t col, std::size_t byte);

  [[nodiscard]] auto to_json() const -> json::Map;
};

/// Buffer for input stream.
struct InBuf : std::streambuf {
 public:
  static constexpr int PB_SIZE = 1024;    // Size of putback area
  static constexpr int BUF_SIZE = 65536;  // Size of the data buffer

  /**
   * Constructor
   * - Initialize file descriptor
   * - Initialize empty data buffer
   * - No putback area
   * => Force underflow()
   */
  explicit InBuf(int fd);

  explicit InBuf(std::string_view path);

  ~InBuf() override;

 protected:
  // Insert new characters into the buffer
  auto underflow() -> int_type override;

  int fd_;
  bool need_close_;
  std::array<char, BUF_SIZE + PB_SIZE> buf_;  // Data buffer
};

/**
 * An input stream struct that provides various functionalities for reading and manipulating
 * streams.
 */
struct InStream {
 public:
  /**
   * Constructs an InStream object.
   *
   * @param buf A unique pointer to a stream buffer.
   * @param name The name of the stream.
   * @param strict Indicates if the stream is in strict mode.
   * @param fail_func A function that will be called when a failure occurs.
   */
  explicit InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict);

  /**
   * Returns the name of the stream.
   *
   * @return The name of the stream as a string view.
   */
  [[nodiscard]] auto name() const -> std::string_view;

  /**
   * Moves the stream pointer to the first non-blank character or EOF.
   */
  auto skip_blanks() -> void;

  /**
   * Returns the current character in the stream, or EOF if reached, without removing it from the
   * stream.
   *
   * @return The current character in the stream as an integer, or EOF if reached.
   */
  auto seek() -> int;

  /**
   * Returns the current character and moves the pointer one character forward, or EOF if reached.
   *
   * @return The current character in the stream as an integer, or EOF if reached.
   */
  auto read() -> int;

  /**
   * Reads at most n characters from the stream.
   *
   * @param n The maximum number of characters to read.
   * @return The read characters as a string.
   */
  auto read_n(std::size_t n) -> std::string;

  /**
   * Checks if the stream is in strict mode.
   *
   * @return True if the stream is in strict mode, false otherwise.
   */
  [[nodiscard]] auto is_strict() const -> bool;

  /**
   * Sets the strict mode of the stream.
   *
   * @param b The value to set strict mode to.
   */
  auto set_strict(bool b) -> void;

  /**
   * Returns the current position.
   *
   * @return The current line position.
   */
  [[nodiscard]] auto pos() const -> Position;

  /**
   * Checks if the current position is EOF.
   *
   * @return True if the current position is EOF, false otherwise.
   */
  auto eof() -> bool;

  /**
   * Moves the pointer to the first non-whitespace character and calls [`eof()`].
   *
   * @return True if the pointer is at EOF, false otherwise.
   */
  auto seek_eof() -> bool;

  /**
   * Checks if the current position contains '\n'.
   *
   * @return True if the current position contains '\n', false otherwise.
   */
  auto eoln() -> bool;

  /**
   * Moves the pointer to the first non-whitespace character and calls [`eoln()`].
   *
   * @return True if the pointer is at '\n', false otherwise.
   */
  auto seek_eoln() -> bool;

  /**
   * Moves the stream pointer to the first character of the next line (if it exists).
   */
  auto next_line() -> void;

  /**
   * Reads a new token from the stream.
   * Skip the previous blanks in non-strict mode.
   *
   * @return The read token as a string.
   */
  auto read_token() -> std::string;

  /**
   * Reads a new "word" from the stream.
   * Skip the previous blanks in non-strict mode.
   *
   * A "word" is defined as a string consisting only of characters from [A-Za-z0-9+\-_\.].
   *
   * @return The read word as a string.
   */
  auto read_word() -> std::string;

  /**
   * If the current position contains EOF, do nothing and return `std::nullopt`.
   *
   * Otherwise, reads a line from the current position to EOLN or EOF. Moves the stream pointer to
   * the first character of the new line (if possible).
   *
   * @return An optional string containing the line read, or `std::nullopt` if the current position
   *         contains EOF.
   */
  auto read_line() -> std::optional<std::string>;

 private:
  std::unique_ptr<std::streambuf> buf_;
  std::string name_;
  bool strict_;  // In strict mode, whitespace characters are not ignored
  Position pos_{};
};

/// Output stream buffer.
struct OutBuf : std::streambuf {
 public:
  explicit OutBuf(int fd);

  explicit OutBuf(std::string_view path);

  ~OutBuf() override;

 protected:
  /// Write one character
  auto overflow(int_type c) -> int_type override;
  /// Write multiple characters
  auto xsputn(const char *s, std::streamsize num) -> std::streamsize override;

  int fd_;  // File descriptor
  bool need_close_;
};
}  // namespace cplib::io

// --- Start embedded: io.i.hpp ---
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


#include <sys/types.h>

#include <array>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <memory>
#include <optional>
#include <ostream>
#include <streambuf>
#include <string>
#include <string_view>
#include <utility>


namespace cplib::io {

namespace detail {
using ReadFunc = ssize_t (*)(int, void *, std::size_t);
using WriteFunc = ssize_t (*)(int, const void *, std::size_t);

[[noreturn]] inline auto panic_io_error(std::string_view operation) -> void {
  panic(cplib::format("{} failed: {}", operation, std::strerror(errno)));
}

inline auto read_available(int fd, char *data, std::size_t len, ReadFunc read_func)
    -> std::streamsize {
  while (true) {
    const auto num_read = read_func(fd, data, len);
    if (num_read >= 0) {
      return static_cast<std::streamsize>(num_read);
    }
    if (errno == EINTR) {
      continue;
    }
    panic_io_error("read");
  }
}

inline auto read_available(int fd, char *data, std::size_t len) -> std::streamsize {
  return read_available(fd, data, len, read);
}

inline auto write_all(int fd, const char *data, std::size_t len, WriteFunc write_func)
    -> std::streamsize {
  std::size_t total = 0;
  while (total < len) {
    const auto written = write_func(fd, data + total, len - total);
    if (written > 0) {
      total += static_cast<std::size_t>(written);
      continue;
    }
    if (written == 0) {
      break;
    }
    if (errno == EINTR) {
      continue;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      break;
    }
    panic_io_error("write");
  }

  return static_cast<std::streamsize>(total);
}

inline auto write_all(int fd, const char *data, std::size_t len) -> std::streamsize {
  return write_all(fd, data, len, write);
}
}  // namespace detail

inline Position::Position() : line(0), col(0), byte(0) {}

inline Position::Position(std::size_t line, std::size_t col, std::size_t byte)
    : line(line), col(col), byte(byte) {}

inline auto Position::to_json() const -> json::Map {
  return {
      {"line", json::Value(static_cast<json::Int>(line))},
      {"col", json::Value(static_cast<json::Int>(col))},
      {"byte", json::Value(static_cast<json::Int>(byte))},
  };
}

inline InBuf::InBuf(int fd) : fd_(fd), need_close_(false) {
  /*
    We recommend using binary mode on Windows. However, Codeforces Polygon doesn’t think so.
    Since the only Online Judge that uses Windows seems to be Codeforces, make it happy.
  */
#if defined(ON_WINDOWS) && !defined(ONLINE_JUDGE)
  _setmode(fd_, _O_BINARY);
#endif
  setg(buf_.data() + PB_SIZE,   // Beginning of putback area
       buf_.data() + PB_SIZE,   // Read position
       buf_.data() + PB_SIZE);  // End position
}

inline InBuf::InBuf(std::string_view path) : need_close_(true) {
  int flags = 0;
#ifdef ON_WINDOWS
  flags |= _O_RDONLY;
#ifndef ONLINE_JUDGE
  flags |= _O_BINARY;
#endif
#else
  flags |= O_RDONLY;
#endif
  fd_ = open(std::string(path).c_str(), flags);
  if (fd_ < 0) {
    panic("Failed to open file: " + std::string(path));
  }
  setg(buf_.data() + PB_SIZE,   // Beginning of putback area
       buf_.data() + PB_SIZE,   // Read position
       buf_.data() + PB_SIZE);  // End position
}

inline InBuf::~InBuf() {
  if (need_close_) {
    close(fd_);
  }
}

inline auto InBuf::underflow() -> int_type {
  // Is read position before end of buffer?
  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }

  /*
   * Process size of putback area
   * - Use number of characters read
   * - But at most size of putback area
   */
  std::ptrdiff_t num_putback = gptr() - eback();
  if (num_putback > PB_SIZE) {
    num_putback = PB_SIZE;
  }

  // Copy up to PB_SIZE characters previously read into the putback area
  std::memmove(buf_.data() + (PB_SIZE - num_putback), gptr() - num_putback, num_putback);

  // Read at most bufSize new characters
  const auto num = detail::read_available(fd_, buf_.data() + PB_SIZE, BUF_SIZE, read);
  if (num == 0) {
    return EOF;
  }

  // Reset buffer pointers
  setg(buf_.data() + (PB_SIZE - num_putback),  // Beginning of putback area
       buf_.data() + PB_SIZE,                  // Read position
       buf_.data() + PB_SIZE + num);           // End of buffer

  // Return next character
  return traits_type::to_int_type(*gptr());
}

inline InStream::InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict)
    : buf_(std::move(buf)), name_(std::move(name)), strict_(strict) {}

inline auto InStream::name() const -> std::string_view { return name_; }

inline auto InStream::skip_blanks() -> void {
  while (true) {
    if (int c = seek(); c == EOF || std::isspace(static_cast<unsigned char>(c)) == 0) break;
    read();
  }
}

inline auto InStream::seek() -> int { return buf_->sgetc(); }

inline auto InStream::read() -> int {
  int c = buf_->sbumpc();
  if (c == EOF) return EOF;
  ++pos_.byte;
  if (c == '\n') {
    ++pos_.line, pos_.col = 0;
  } else {
    ++pos_.col;
  }
  return c;
}

inline auto InStream::read_n(std::size_t n) -> std::string {
  std::string buf;
  buf.reserve(n);
  for (std::size_t i = 0; i < n; ++i) {
    if (eof()) break;
    buf.push_back(static_cast<char>(read()));
  }
  return buf;
}

inline auto InStream::is_strict() const -> bool { return strict_; }

inline auto InStream::set_strict(bool b) -> void {
  if (pos().byte > 0) {
    panic(cplib::format(
        "Can't set strict mode of input stream `{}` when not at the beginning of the file",
        name()));
  }
  strict_ = b;
}

inline auto InStream::pos() const -> Position { return pos_; }

inline auto InStream::eof() -> bool { return seek() == EOF; }

inline auto InStream::seek_eof() -> bool {
  if (!strict_) skip_blanks();
  return eof();
}

inline auto InStream::eoln() -> bool { return seek() == '\n'; }

inline auto InStream::seek_eoln() -> bool {
  if (!strict_) skip_blanks();
  return eoln();
}

inline auto InStream::next_line() -> void {
  int c;
  do {
    c = read();
  } while (c != EOF && c != '\n');
}

inline auto InStream::read_token() -> std::string {
  if (!strict_) skip_blanks();

  std::string token;
  while (true) {
    if (int c = seek(); c == EOF || std::isspace(static_cast<unsigned char>(c)) != 0) break;
    token.push_back(static_cast<char>(read()));
  }
  return token;
}

inline auto InStream::read_word() -> std::string {
  if (!strict_) skip_blanks();

  std::string word;
  while (true) {
    if (int c = seek(); c == EOF || (!std::isalnum(static_cast<unsigned char>(c)) && c != '+' &&
                                     c != '-' && c != '_' && c != '.')) {
      break;
    }
    word.push_back(static_cast<char>(read()));
  }
  return word;
}

inline auto InStream::read_line() -> std::optional<std::string> {
  std::string line;
  if (eof()) return std::nullopt;
  while (true) {
    int c = read();
    if (c == EOF) {
      if (is_strict()) {
        // In strict mode, read_line must end with '\n' and cannot end with EOF.
        return std::nullopt;
      }
      break;
    }
    if (c == '\n') break;
    line.push_back(static_cast<char>(c));
  }
  return line;
}

// https://www.josuttis.com/cppcode/fdstream.html
inline OutBuf::OutBuf(int fd) : fd_(fd), need_close_(false) {
  /*
    We recommend using binary mode on Windows. However, Codeforces Polygon
    doesn’t think so. Since the only Online Judge that uses Windows seems to
    be Codeforces, make it happy.
  */
#if defined(ON_WINDOWS) && !defined(ONLINE_JUDGE)
  _setmode(fd_, _O_BINARY);
#endif
}

inline OutBuf::OutBuf(std::string_view path) : need_close_(true) {
  int flags = 0;
#ifdef ON_WINDOWS
  flags |= _O_WRONLY | _O_CREAT | _O_TRUNC;
#ifndef ONLINE_JUDGE
  flags |= _O_BINARY;
#endif
#else
  flags |= O_WRONLY | O_CREAT | O_TRUNC;
#endif
  fd_ = open(std::string(path).c_str(), flags, 0666);
  if (fd_ < 0) {
    panic("Failed to open file: " + std::string(path));
  }
}

inline OutBuf::~OutBuf() {
  if (need_close_) {
    close(fd_);
  }
}

inline auto OutBuf::overflow(int_type c) -> int_type {
  if (c != EOF) {
    auto z = static_cast<char>(c);
    if (detail::write_all(fd_, &z, 1, write) != 1) {
      return EOF;
    }
  }
  return c;
}

inline auto OutBuf::xsputn(const char *s, std::streamsize num) -> std::streamsize {
  if (num <= 0) return 0;
  return detail::write_all(fd_, s, static_cast<std::size_t>(num), write);
}

namespace detail {
// Open the givin file and create a `std::::ostream`
inline auto make_ostream_by_path(std::string_view path, std::unique_ptr<std::streambuf> &buf,
                                 std::ostream &stream) -> void {
  buf = std::make_unique<io::OutBuf>(path);
  stream.rdbuf(buf.get());
}

// Use file with givin fileno as output stream and create a `std::::ostream`
inline auto make_ostream_by_fileno(int fileno, std::unique_ptr<std::streambuf> &buf,
                                   std::ostream &stream) -> void {
  buf = std::make_unique<io::OutBuf>(fileno);
  stream.rdbuf(buf.get());
}
}  // namespace detail
}  // namespace cplib::io
// --- End embedded: io.i.hpp ---

#endif
// --- End embedded: io.hpp ---
// --- Start embedded: trace.hpp ---
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

#ifndef CPLIB_TRACE_HPP_
#define CPLIB_TRACE_HPP_

#include <concepts>
#include <cstdint>
#include <memory>
#include <optional>
#include <streambuf>
#include <string>
#include <string_view>
#include <vector>


namespace cplib::trace {

/**
 * Trace level.
 */
enum struct Level : std::uint8_t {
  NONE,        /// Do not trace.
  STACK_ONLY,  /// Enable trace stack only.
  FULL,        /// Trace the whole input stream. Enable trace stack and trace tree.
};

template <typename T>
concept Trace = requires(const T &t) {
  { t.node_name() } -> std::same_as<std::string>;
  { t.to_plain_text() } -> std::same_as<std::string>;
  { t.to_colored_text() } -> std::same_as<std::string>;
  { t.to_stack_json() } -> std::same_as<json::Value>;
  { t.to_tree_json() } -> std::same_as<json::Value>;
} && std::copyable<T>;

/**
 * `TraceStack` represents a stack of trace.
 */
template <Trace T>
struct TraceStack {
  std::vector<T> stack;
  bool fatal;

  /// Convert to JSON map.
  [[nodiscard]] auto to_json() const -> json::Value;

  /// Convert to human-friendly plain text.
  /// Each line does not contain the trailing `\n` character.
  [[nodiscard]] auto to_plain_text_lines() const -> std::vector<std::string>;

  /// Convert to human-friendly colored text (ANSI escape color).
  /// Each line does not contain the trailing `\n` character.
  [[nodiscard]] auto to_colored_text_lines() const -> std::vector<std::string>;

  /// Convert to human-friendly plain text, using compact (one line) style.
  [[nodiscard]] auto to_plain_text_compact() const -> std::string;

  /// Convert to human-friendly colored text (ANSI escape color), using compact (one line) style.
  [[nodiscard]] auto to_colored_text_compact() const -> std::string;
};

/**
 * `TraceTreeNode` represents a node of trace tree.
 */
template <Trace T>
struct TraceTreeNode {
 public:
  T trace;
  json::Map tags{};

  /**
   * Create a TraceTreeNode with trace.
   *
   * @param trace The trace of the node.
   */
  explicit TraceTreeNode(T trace);

  /// Copy constructor (deleted to prevent copying).
  TraceTreeNode(const TraceTreeNode &) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const TraceTreeNode &) -> TraceTreeNode & = delete;

  /// Move constructor.
  TraceTreeNode(TraceTreeNode &&) noexcept = default;

  /// Move assignment operator.
  auto operator=(TraceTreeNode &&) noexcept -> TraceTreeNode & = default;

  /**
   * Get the children of the node.
   *
   * @return The children of the node.
   */
  [[nodiscard]] auto get_children() const -> const std::vector<std::unique_ptr<TraceTreeNode>> &;

  /**
   * Get the parent of the node.
   *
   * @return The parent of the node.
   */
  [[nodiscard]] auto get_parent() -> TraceTreeNode *;

  /**
   * Convert to JSON value.
   *
   * If node has tag `#hidden`, return `std::nullopt`.
   *
   * For performance reasons, returning a modifiable map will create a lot of objects, so we choose
   * to return the `json::Raw` type.
   *
   * @return The JSON value or `std::nullopt`.
   */
  [[nodiscard]] auto to_json() const -> std::optional<json::Raw>;

  /**
   * Convert a node to its child and return it again (as reference).
   *
   * @param child The child node.
   * @return The child node.
   */
  auto add_child(std::unique_ptr<TraceTreeNode> child) -> std::unique_ptr<TraceTreeNode> &;

 private:
  std::vector<std::unique_ptr<TraceTreeNode>> children_{};
  TraceTreeNode *parent_{nullptr};

  auto write_json(std::streambuf &buf) const -> void;
};

/**
 * `Traced` represents a base class for objects that manage trace information.
 */
template <Trace T>
struct Traced {
 public:
  explicit Traced(Level trace_level, T root);
  virtual ~Traced() = 0;
  Traced(const Traced &) = delete;
  auto operator=(const Traced &) -> Traced & = delete;
  Traced(Traced &&) noexcept = default;
  auto operator=(Traced &&) noexcept -> Traced & = default;

  /**
   * Get the trace level.
   */
  [[nodiscard]] auto get_trace_level() const -> Level;

  /**
   * Make a trace stack from the current trace.
   *
   * Only available when `TraceLevel::STACK_ONLY` or higher is set.
   * Otherwise, an error will be panicked.
   */
  [[nodiscard]] auto make_trace_stack(bool fatal) const -> TraceStack<T>;

  /**
   * Get the trace tree.
   *
   * Only available when `TraceLevel::FULL` is set.
   * Otherwise, an error will be panicked.
   */
  [[nodiscard]] auto get_trace_tree() const -> const TraceTreeNode<T> *;

  /**
   * Attach a tag to the current trace.
   *
   * @param key The tag key.
   * @param value The tag value.
   */
  auto attach_tag(std::string_view key, json::Value value) -> void;

  /**
   * Get current trace.
   *
   * Only available when `TraceLevel::STACK_ONLY` or higher is set.
   * Otherwise, an error will be panicked.
   */
  auto get_current_trace() const -> const T &;

 protected:
  /**
   * Set current trace.
   *
   * Only available when `TraceLevel::STACK_ONLY` or higher is set.
   * Otherwise, an error will be panicked.
   */
  auto set_current_trace(T trace) -> void;

  /**
   * Push a new trace onto the stack/tree.
   *
   * @param t The trace to push.
   */
  auto push_trace(T trace) -> void;

  /**
   * Pop the current trace from the stack/tree.
   * This should be called when the scope of the trace ends.
   */
  auto pop_trace() -> void;

 private:
  trace::Level trace_level_;
  std::vector<T> active_traces_;
  std::unique_ptr<TraceTreeNode<T>> trace_tree_root_;
  TraceTreeNode<T> *trace_tree_current_;
};

}  // namespace cplib::trace

// --- Start embedded: trace.i.hpp ---
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


#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <memory>
#include <optional>
#include <sstream>
#include <streambuf>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace cplib::trace {

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_json() const -> json::Value {
  json::Map map;
  json::List stack_list;

  stack_list.reserve(stack.size());
  for (const auto &trace : stack) {
    stack_list.emplace_back(trace.to_stack_json());
  }

  map.emplace("stack", std::move(stack_list));
  map.emplace("fatal", fatal);
  return json::Value(map);
}

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_plain_text_lines() const -> std::vector<std::string> {
  std::vector<std::string> lines;
  lines.reserve(stack.size());

  if (fatal) {
    lines.emplace_back("[fatal]");
  }

  std::size_t id = 0;
  for (const auto &trace : stack) {
    auto line = cplib::format("#{}: {}", id, trace.to_plain_text());
    ++id;
    lines.emplace_back(std::move(line));
  }

  return lines;
}

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_colored_text_lines() const -> std::vector<std::string> {
  std::vector<std::string> lines;
  lines.reserve(stack.size());

  if (fatal) {
    lines.emplace_back("\x1b[0;31m[fatal]\x1b[0m");
  }

  std::size_t id = 0;
  for (const auto &trace : stack) {
    auto line = cplib::format("#{}: {}", id, trace.to_colored_text());
    ++id;
    lines.emplace_back(std::move(line));
  }

  return lines;
}

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_plain_text_compact() const -> std::string {
  std::stringbuf buf(std::ios_base::out);

  if (fatal) {
    constexpr std::string_view FATAL_TEXT = "[fatal] ";
    buf.sputn(FATAL_TEXT.data(), FATAL_TEXT.size());
  }

  for (auto it = stack.begin(); it != stack.end(); ++it) {
    if (std::next(it) == stack.end()) {
      auto node_text = it->to_plain_text();
      buf.sputn(node_text.c_str(), node_text.size());
      continue;
    }
    auto node_text = it->node_name();
    buf.sputn(node_text.c_str(), node_text.size());
    buf.sputc('/');
  }

  return buf.str();
}

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_colored_text_compact() const -> std::string {
  std::stringbuf buf(std::ios_base::out);

  if (fatal) {
    constexpr std::string_view FATAL_TEXT = "\x1b[0;31m[fatal]\x1b[0m ";
    buf.sputn(FATAL_TEXT.data(), FATAL_TEXT.size());
  }

  for (auto it = stack.begin(); it != stack.end(); ++it) {
    if (std::next(it) == stack.end()) {
      auto node_text = it->to_colored_text();
      buf.sputn(node_text.c_str(), node_text.size());
      continue;
    }
    auto node_text = it->node_name();
    buf.sputn(node_text.c_str(), node_text.size());
    constexpr std::string_view SLASH = "\x1b[0;90m/\x1b[0m";
    buf.sputn(SLASH.data(), SLASH.size());
  }

  return buf.str();
}

template <Trace T>
inline TraceTreeNode<T>::TraceTreeNode(T trace) : trace(std::move(trace)) {}

template <Trace T>
[[nodiscard]] inline auto TraceTreeNode<T>::get_children() const
    -> const std::vector<std::unique_ptr<TraceTreeNode>> & {
  return children_;
}

template <Trace T>
[[nodiscard]] inline auto TraceTreeNode<T>::get_parent() -> TraceTreeNode * {
  return parent_;
}

template <Trace T>
[[nodiscard]] inline auto TraceTreeNode<T>::to_json() const -> std::optional<json::Raw> {
  if (tags.contains("#hidden")) {
    return std::nullopt;
  }
  std::stringbuf buf(std::ios::out);
  write_json(buf);
  return json::Raw(buf.str());
}

template <Trace T>
inline auto TraceTreeNode<T>::add_child(std::unique_ptr<TraceTreeNode> child)
    -> std::unique_ptr<TraceTreeNode> & {
  child->parent_ = this;
  return children_.emplace_back(std::move(child));
}

template <Trace T>
inline auto TraceTreeNode<T>::write_json(std::streambuf &buf) const -> void {
  assert(!tags.contains("#hidden"));

  constexpr std::string_view TRACE_HEADER = "{\"trace\":";
  buf.sputn(TRACE_HEADER.data(), TRACE_HEADER.size());
  trace.to_tree_json().write_string(buf);

  if (!tags.empty()) {
    constexpr std::string_view TAGS_HEADER = ",\"tags\":";
    buf.sputn(TAGS_HEADER.data(), TAGS_HEADER.size());
    json::Value::encode_map(buf, tags);
  }

  if (const auto &children = get_children();
      std::ranges::any_of(children, [](const auto &c) { return !c->tags.contains("#hidden"); })) {
    constexpr std::string_view CHILDREN_HEADER = ",\"children\":[";
    buf.sputn(CHILDREN_HEADER.data(), CHILDREN_HEADER.size());
    bool first = true;
    for (const auto &child : children) {
      if (child->tags.contains("#hidden")) continue;
      if (first) {
        first = false;
      } else {
        buf.sputc(',');
      }
      child->write_json(buf);
    }
    buf.sputc(']');
  }

  buf.sputc('}');
}

template <Trace T>
inline Traced<T>::Traced(Level trace_level, T root)
    : trace_level_(trace_level),
      active_traces_({root}),
      trace_tree_root_(trace_level >= Level::FULL
                           ? std::make_unique<TraceTreeNode<T>>(std::move(root))
                           : nullptr),
      trace_tree_current_(trace_tree_root_.get()) {}

template <Trace T>
inline Traced<T>::~Traced() = default;

template <Trace T>
[[nodiscard]] inline auto Traced<T>::get_trace_level() const -> Level {
  return trace_level_;
}

template <Trace T>
[[nodiscard]] inline auto Traced<T>::make_trace_stack(bool fatal) const -> TraceStack<T> {
  return {.stack = active_traces_, .fatal = fatal};
}

template <Trace T>
[[nodiscard]] inline auto Traced<T>::get_trace_tree() const -> const TraceTreeNode<T> * {
  if (get_trace_level() < Level::FULL) {
    panic("Traced::get_trace_tree requires `Level::FULL`");
  }

  return trace_tree_root_.get();
}

template <Trace T>
inline auto Traced<T>::attach_tag(std::string_view key, json::Value value) -> void {
  if (get_trace_level() < Level::FULL) {
    panic("Traced::attach_tag requires `Level::FULL`");
  }

  trace_tree_current_->tags.emplace(key, std::move(value));
}

template <Trace T>
inline auto Traced<T>::get_current_trace() const -> const T & {
  if (get_trace_level() < Level::STACK_ONLY) {
    panic("Traced::get_current_trace requires `Level::STACK_ONLY`");
  }

  return active_traces_.back();
}

template <Trace T>
inline auto Traced<T>::set_current_trace(T trace) -> void {
  if (get_trace_level() < Level::STACK_ONLY) {
    panic("Traced::set_current_trace requires `Level::STACK_ONLY`");
  }

  active_traces_.back() = trace;

  if (get_trace_level() < Level::FULL) {
    return;
  }
  trace_tree_current_->trace = std::move(trace);
}

template <Trace T>
inline auto Traced<T>::push_trace(T trace) -> void {
  if (trace_level_ < Level::STACK_ONLY) {
    return;
  }

  active_traces_.emplace_back(trace);

  if (trace_level_ < Level::FULL) {
    return;
  }

  auto &child =
      trace_tree_current_->add_child(std::make_unique<trace::TraceTreeNode<T>>(std::move(trace)));
  trace_tree_current_ = child.get();
}

template <Trace T>
inline auto Traced<T>::pop_trace() -> void {
  if (trace_level_ < Level::STACK_ONLY) {
    return;
  }

  if (active_traces_.size() <= 1) {
    panic("Pop trace failed: cannot pop the root element");
  }
  active_traces_.pop_back();

  if (trace_level_ < Level::FULL) {
    return;
  }

  trace_tree_current_ = trace_tree_current_->get_parent();
}

}  // namespace cplib::trace
// --- End embedded: trace.i.hpp ---

#endif
// --- End embedded: trace.hpp ---
// --- Start embedded: evaluate.hpp ---
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

#ifndef CPLIB_EVALUATE_HPP_
#define CPLIB_EVALUATE_HPP_

#include <compare>
#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>


namespace cplib::evaluate {
/// Represents the outcome of a test case or a set of test cases.
struct Result {
  /// Defines the status of a solution.
  struct Status {
   public:
    /// Enum values for different solution statuses.
    /// The order of these values is important for comparison:
    /// WRONG_ANSWER < PARTIALLY_CORRECT < ACCEPTED.
    enum Value : std::uint8_t {
      /// Indicates the solution is incorrect.
      WRONG_ANSWER,
      /// Indicates the solution is partially correct.
      PARTIALLY_CORRECT,
      /// Indicates the solution is accepted.
      ACCEPTED,
    };

    /**
     * Default constructor for Status.
     */
    Status() = default;

    /**
     * Constructor for Status with a given value.
     *
     * @param value The value of the status.
     */
    constexpr Status(Value value);  // NOLINT(google-explicit-constructor)

    /**
     * Implicit conversion operator to Value.
     *
     * @return The value of the status.
     */
    constexpr operator Value() const;  // NOLINT(google-explicit-constructor)

    /**
     * Deleted conversion operator to bool.
     *
     * @return Deleted conversion operator to bool.
     */
    explicit operator bool() const = delete;

    /**
     * Get the string representation of the status.
     *
     * @return The string representation of the status.
     */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view;

   private:
    Value value_;
  };

  Status status;

  // Between [0, 1].
  double score;

  std::string message;

  /**
   * Controls how messages are aggregated in hierarchical evaluations.
   *
   * In a complex evaluation, multiple sub-results (e.g., from `ev.eq`, `ev.approx`) are
   * combined. This flag ensures that the messages from these sub-results are correctly
   * merged into the final result of the parent evaluation, and not propagated further up
   * the evaluation stack.
   *
   * - `false` (default, "unnamed"): The result is from a sub-evaluation. Its message can be
   *   merged into a parent result when combined using operators like `+=` or `&=`.
   * - `true` ("named"): The result is the final outcome of a top-level `Evaluator::operator()`
   *   call. Its message is considered final for its corresponding node in the trace
   *   stack/tree and will not be merged into parent results.
   */
  bool named{false};

  /// Creates a Result representing zero score and ACCEPTED status, for additive accumulation.
  /// @return A Result object with ACCEPTED status and 0.0 score.
  static auto zero() -> Result;

  /// Creates an ACCEPTED Result with a full score (1.0).
  /// @return A Result object with ACCEPTED status and 1.0 score.
  static auto ac() -> Result;

  /// Creates an ACCEPTED Result with a full score (1.0) and a message.
  /// @param message An optional message for the result.
  /// @return A Result object with ACCEPTED status, 1.0 score, and the given message.
  static auto ac(std::string message) -> Result;

  /// Creates a WRONG_ANSWER Result with zero score (0.0) and a message.
  /// @param message A message explaining the wrong answer.
  /// @return A Result object with WRONG_ANSWER status, 0.0 score, and the given message.
  static auto wa(std::string message) -> Result;

  /// Creates a PARTIALLY_CORRECT Result with a specified score and message.
  /// @param score The partial score (will be clamped between 0.0 and 1.0).
  /// @param message A message explaining the partial correctness.
  /// @return A Result object with PARTIALLY_CORRECT status, clamped score, and the given message.
  static auto pc(double score, std::string message) -> Result;

  /// Creates a Result with a specified status, score and message.
  Result(Status status, double score, std::string message);

  /// C++ 20 three-way comparison operator.
  /// Compares Results primarily by status (lower enum value is "worse"),
  /// then by score (lower score is "worse"). Message is not part of comparison logic.
  /// @param other The Result to compare with.
  /// @return A std::strong_ordering indicating the relationship.
  constexpr auto operator<=>(const Result &other) const -> std::strong_ordering;

  /// Scales the score of the Result by a given factor.
  /// @param scale The scaling factor.
  /// @return A new Result object with the scaled score. Status and messages remain unchanged.
  [[nodiscard]] auto operator*(double scale) const -> Result;

  /// Scales the score of this Result by a given factor in-place. The message remains unchanged.
  /// @param scale The scaling factor.
  /// @return A reference to this Result object.
  auto operator*=(double scale) -> Result &;

  /// Combines two Results by adding their scores. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// @param other The Result to add.
  /// @return A new Result object representing the sum.
  [[nodiscard]] auto operator+(const Result &other) const -> Result;

  /// Combines this Result with another by adding scores in-place. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// @param other The Result to add.
  /// @return A reference to this Result object.
  auto operator+=(const Result &other) -> Result &;

  /// Combines two Results by taking the minimum of their scores. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// This is typically used for "all or nothing" scenarios or combining results where
  /// the lowest score dictates the overall outcome.
  /// @param other The Result to combine with (min operation).
  /// @return A new Result object representing the minimum.
  [[nodiscard]] auto operator&(const Result &other) const -> Result;

  /// Combines this Result with another by taking the minimum of their scores in-place. The message
  /// remains unchanged. The resulting status is the "worst" (minimum enum value) of the two
  /// statuses.
  /// @param other The Result to combine with (min operation).
  /// @return A reference to this Result object.
  auto operator&=(const Result &other) -> Result &;

  /**
   * Convert to json value.
   */
  [[nodiscard]] auto to_json() const -> json::Map;

 private:
  Result() = default;
};

#ifndef CPLIB_EVALUATOR_TRACE_LEVEL_MAX
#define CPLIB_EVALUATOR_TRACE_LEVEL_MAX static_cast<int>(::cplib::trace::Level::FULL)
#endif

struct Evaluator;

/**
 * Concept for types that can be evaluated for problem solutions.
 *
 * A type `T` satisfies `Evaluatable` if it provides a static member function named `evaluate` that
 * takes two arguments of type `const T&` (representing the participant's answer and the jury's
 * answer/correct output) and additional arguments and returns a `cplib::Result`.
 */
template <typename T, typename... Args>
concept Evaluatable = requires(Evaluator &ev, const T &pans, const T &jans, Args &&...args) {
  { T::evaluate(ev, pans, jans, std::forward<Args>(args)...) } -> std::same_as<Result>;
};

struct EvaluatorTrace {
  std::string var_name;
  std::optional<Result> result;

  explicit EvaluatorTrace(std::string var_name);

  /// Get name of node.
  [[nodiscard]] auto node_name() const -> std::string;

  /// Convert trace to plain text.
  [[nodiscard]] auto to_plain_text() const -> std::string;

  /// Convert trace to colored text.
  [[nodiscard]] auto to_colored_text() const -> std::string;

  /// Convert incomplete trace to JSON map.
  [[nodiscard]] auto to_stack_json() const -> json::Value;

  /// Convert complete trace to JSON map.
  [[nodiscard]] auto to_tree_json() const -> json::Value;
};

struct Evaluator : trace::Traced<EvaluatorTrace> {
  using FailFunc = UniqueFunction<auto(const Evaluator &, std::string_view)->void>;
  using EvaluationHook = UniqueFunction<auto(const Evaluator &, const Result &)->void>;

  /**
   * Create an evaluator.
   */
  explicit Evaluator(trace::Level trace_level, FailFunc fail_func, EvaluationHook evaluation_hook);

  /// Copy constructor (deleted to prevent copying).
  Evaluator(const Evaluator &) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const Evaluator &) -> Evaluator & = delete;

  /// Move constructor.
  Evaluator(Evaluator &&) noexcept = default;

  /// Move assignment operator.
  auto operator=(Evaluator &&) noexcept -> Evaluator & = default;

  /**
   * Call fail func with a message.
   *
   * @param message The error message.
   */
  [[noreturn]] auto fail(std::string_view message) -> void;

  /**
   * Evaluate answers from participant and jury with additional arguments.
   *
   * @param var_name Variable name.
   * @param pans Participant's answer.
   * @param jans Jury's answer.
   * @param args Additional arguments.
   * @return Result of the evaluation.
   */
  template <typename T, class... Args>
  auto operator()(std::string_view var_name, const T &pans, const T &jans, Args &&...args) -> Result
    requires Evaluatable<T, Args...>;

  template <std::equality_comparable T>
  auto eq(std::string_view var_name, const T &pans, const T &jans) -> Result;

  template <std::floating_point T>
  auto approx(std::string_view var_name, const T &pans, const T &jans, const T &max_err) -> Result;

  template <class T>
  auto approx_abs(std::string_view var_name, const T &pans, const T &jans, const T &abs_err)
      -> Result
    requires std::is_arithmetic_v<T>;

 private:
  FailFunc fail_func_;
  EvaluationHook evaluation_hook_;

  auto pre_evaluate(std::string_view var_name) -> void;
  auto post_evaluate(Result &result) -> void;
};

}  // namespace cplib::evaluate

// --- Start embedded: evaluate.i.hpp ---
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


#include <algorithm>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>
#include <utility>


namespace cplib::evaluate {

inline constexpr Result::Status::Status(Value value) : value_(value) {}

inline constexpr Result::Status::operator Value() const { return value_; }

inline constexpr auto Result::Status::to_string() const -> std::string_view {
  switch (value_) {
    case ACCEPTED:
      return "accepted";
    case WRONG_ANSWER:
      return "wrong_answer";
    case PARTIALLY_CORRECT:
      return "partially_correct";
    default:
      panic(cplib::format("Unknown result status: {}", static_cast<int>(value_)));
      return "unknown";
  }
}

// Impl Result {{{
namespace detail {
inline auto merge_message(const std::string &a, const std::string &b) -> std::string {
  if (a.empty()) {
    return b;
  }
  if (b.empty()) {
    return a;
  }
  return a + '\n' + b;
}

inline auto convert_to_strong_ordering(std::partial_ordering po) -> std::strong_ordering {
  if (po == std::partial_ordering::equivalent) {
    return std::strong_ordering::equal;
  } else if (po == std::partial_ordering::less) {
    return std::strong_ordering::less;
  } else if (po == std::partial_ordering::greater) {
    return std::strong_ordering::greater;
  } else {
    panic("Cannot convert partial_ordering to strong_ordering: comparison is not total");
  }
}
}  // namespace detail

inline Result::Result(Status status, double score, std::string message)
    : status(status), message({std::move(message)}) {
  if (!std::isfinite(score)) {
    panic("Score must be an finite number");
  }
  this->score = score;
}

inline auto Result::zero() -> Result { return {Status::ACCEPTED, 0.0, ""}; };

inline auto Result::ac() -> Result { return {Status::ACCEPTED, 1.0, ""}; }

inline auto Result::ac(std::string message) -> Result {
  return {Status::ACCEPTED, 1.0, std::move(message)};
}

inline auto Result::wa(std::string message) -> Result {
  return {Status::WRONG_ANSWER, 0.0, std::move(message)};
}
inline auto Result::pc(double score, std::string message) -> Result {
  return {Status::PARTIALLY_CORRECT, score, std::move(message)};
}

inline constexpr auto Result::operator<=>(const Result &other) const -> std::strong_ordering {
  if (status != other.status) {
    return status <=> other.status;
  }
  if (score != other.score) {
    return detail::convert_to_strong_ordering(score <=> other.score);
  }
  return std::strong_ordering::equal;
}

inline auto Result::operator*(double scale) const -> Result {
  Result res = *this;
  res.score *= scale;
  return res;
}

inline auto Result::operator*=(double scale) -> Result & {
  score *= scale;
  return *this;
}

inline auto Result::operator+(const Result &other) const -> Result {
  Result res;
  res.status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  res.score = score + other.score;
  if (other.named) {
    res.message = message;
  } else {
    res.message = detail::merge_message(message, other.message);
  }
  return res;
}

inline auto Result::operator+=(const Result &other) -> Result & {
  status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  score += other.score;
  if (!other.named) {
    message = detail::merge_message(message, other.message);
  }
  return *this;
}

inline auto Result::operator&(const Result &other) const -> Result {
  Result res;
  res.status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  res.score = std::min(score, other.score);
  if (other.named) {
    res.message = message;
  } else {
    res.message = detail::merge_message(message, other.message);
  }
  return res;
}

inline auto Result::operator&=(const Result &other) -> Result & {
  status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  score = std::min(score, other.score);
  if (!other.named) {
    message = detail::merge_message(message, other.message);
  }
  return *this;
}

[[nodiscard]] inline auto Result::to_json() const -> json::Map {
  return {
      {"status", json::Value(json::String(status.to_string()))},
      {"score", json::Value(score)},
      {"message", json::Value(message)},
  };
}
// /Impl Result }}}

namespace detail {
inline auto status_to_title_string(Result::Status status) -> std::string {
  switch (status) {
    case Result::Status::ACCEPTED:
      return "Accepted";
    case Result::Status::WRONG_ANSWER:
      return "Wrong Answer";
    case Result::Status::PARTIALLY_CORRECT:
      return "Partially Correct";
    default:
      panic(cplib::format("Unknown result status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto status_to_colored_title_string(Result::Status status) -> std::string {
  switch (status) {
    case Result::Status::ACCEPTED:
      return "\x1b[0;32mAccepted\x1b[0m";
    case Result::Status::WRONG_ANSWER:
      return "\x1b[0;31mWrong Answer\x1b[0m";
    case Result::Status::PARTIALLY_CORRECT:
      return "\x1b[0;36mPartially Correct\x1b[0m";
    default:
      panic(cplib::format("Unknown result status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline EvaluatorTrace::EvaluatorTrace(std::string var_name)
    : var_name(std::move(var_name)), result(std::nullopt) {}

[[nodiscard]] inline auto EvaluatorTrace::node_name() const -> std::string { return var_name; }

[[nodiscard]] inline auto EvaluatorTrace::to_plain_text() const -> std::string {
  if (result.has_value()) {
    return cplib::format("{}: {} {:.2f}%, {}", var_name, result->status.to_string(),
                         result->score * 100.0, result->message);
  } else {
    return cplib::format("{}: Unfinished", var_name);
  }
}

[[nodiscard]] inline auto EvaluatorTrace::to_colored_text() const -> std::string {
  if (result.has_value()) {
    return cplib::format("\x1b[0;33m{}\x1b[0m: {} \x1b[0;33m{:.2f}%\x1b[0m, {}", var_name,
                         detail::status_to_colored_title_string(result->status),
                         result->score * 100.0, result->message);
  } else {
    return cplib::format("\x1b[0;33m{}\x1b[0m: \x1b[0;33mUnfinished\x1b[0m", var_name);
  }
}

[[nodiscard]] inline auto EvaluatorTrace::to_stack_json() const -> json::Value {
  json::Map map{
      {"var_name", json::Value(var_name)},
  };
  if (result.has_value()) {
    map.emplace("result", json::Value(result->to_json()));
  }
  return {map};
}

[[nodiscard]] inline auto EvaluatorTrace::to_tree_json() const -> json::Value {
  return to_stack_json();
}

inline Evaluator::Evaluator(trace::Level trace_level, FailFunc fail_func,
                            EvaluationHook evaluation_hook)
    : trace::Traced<EvaluatorTrace>(
          static_cast<trace::Level>(
              std::min(static_cast<int>(trace_level), CPLIB_EVALUATOR_TRACE_LEVEL_MAX)),
          EvaluatorTrace("<eval>")),
      fail_func_(std::move(fail_func)),
      evaluation_hook_(std::move(evaluation_hook)) {}

[[noreturn]] inline auto Evaluator::fail(std::string_view message) -> void {
  fail_func_(*this, message);
  std::exit(EXIT_FAILURE);
}

inline auto Evaluator::pre_evaluate(std::string_view var_name) -> void {
  auto trace_level = get_trace_level();
  if (trace_level >= trace::Level::STACK_ONLY) {
    EvaluatorTrace trace{std::string(var_name)};
    push_trace(std::move(trace));
  }
}

inline auto Evaluator::post_evaluate(Result &result) -> void {
  result.named = true;

  auto trace_level = get_trace_level();
  if (trace_level >= trace::Level::STACK_ONLY) {
    auto trace = get_current_trace();
    trace.result = result;
    set_current_trace(trace);
  }

  if (result.status != Result::Status::ACCEPTED) {
    evaluation_hook_(*this, result);
  }

  if (trace_level >= trace::Level::STACK_ONLY) {
    pop_trace();
  }
}

template <typename T, class... Args>
inline auto Evaluator::operator()(std::string_view var_name, const T &pans, const T &jans,
                                  Args &&...args) -> Result
  requires Evaluatable<T, Args...>
{
  pre_evaluate(var_name);
  Result result = T::evaluate(*this, pans, jans, std::forward<Args>(args)...);
  post_evaluate(result);
  return result;
}

template <std::equality_comparable T>
inline auto Evaluator::eq(std::string_view var_name, const T &pans, const T &jans) -> Result {
  pre_evaluate(var_name);
  Result result = Result::ac();
  if (pans != jans) {
    if constexpr (std::convertible_to<T, std::string_view>) {
      result = Result::wa(cplib::format("`{}` is not equal: expected `{}`, got `{}`", var_name,
                                        compress(cplib::detail::hex_encode(jans)),
                                        compress(cplib::detail::hex_encode(pans))));
    } else if constexpr (cplib::formattable<T>) {
      auto jans_str = cplib::format("{}", jans);
      auto pans_str = cplib::format("{}", pans);
      result = Result::wa(cplib::format("`{}` is not equal: expected {}, got {}", var_name,
                                        compress(jans_str), compress(pans_str)));
    } else {
      result = Result::wa(cplib::format("`{}` is not equal", var_name));
    }
  }
  post_evaluate(result);
  return result;
}

template <std::floating_point T>
inline auto Evaluator::approx(std::string_view var_name, const T &pans, const T &jans,
                              const T &max_err) -> Result {
  pre_evaluate(var_name);
  Result result = Result::ac();
  if (!float_equals(pans, jans, max_err)) {
    T delta = float_delta(jans, pans);
    result = Result::wa(cplib::format(
        "`{}` is not approximately equal: expected {:.10g}, got {:.10g}, delta {:.10g}", var_name,
        jans, pans, delta));
  }
  post_evaluate(result);
  return result;
}

template <class T>
inline auto Evaluator::approx_abs(std::string_view var_name, const T &pans, const T &jans,
                                  const T &abs_err) -> Result
  requires std::is_arithmetic_v<T>
{
  pre_evaluate(var_name);
  Result result = Result::ac();
  if (pans < jans - abs_err || pans > jans + abs_err) {
    T delta = pans - jans;
    if (delta < 0) {
      delta = -delta;
    }
    result = Result::wa(cplib::format(
        "`{}` is not approximately equal in absolute error: expected {}, got {}, delta {}",
        var_name, jans, pans, delta));
  }
  post_evaluate(result);
  return result;
}

}  // namespace cplib::evaluate
// --- End embedded: evaluate.i.hpp ---

#endif
// --- End embedded: evaluate.hpp ---
// --- Start embedded: var.hpp ---
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

#ifndef CPLIB_VAR_HPP_
#define CPLIB_VAR_HPP_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>


#ifndef CPLIB_READER_TRACE_LEVEL_MAX
#define CPLIB_READER_TRACE_LEVEL_MAX static_cast<int>(::cplib::trace::Level::FULL)
#endif

namespace cplib::var {
template <class T, class D>
struct Var;

/**
 * `ReaderTrace` represents trace information for a variable.
 */
struct ReaderTrace {
  std::string var_name;
  io::Position pos;

  /// The length of the variable in the raw stream, units of bytes.
  /// Incomplete variables will have zero length.
  std::size_t byte_length{0};

  explicit ReaderTrace(std::string var_name, io::Position pos);

  /// Get name of node;
  [[nodiscard]] auto node_name() const -> std::string;

  /// Convert trace to plain text.
  [[nodiscard]] auto to_plain_text() const -> std::string;

  /// Convert trace to colored text.
  [[nodiscard]] auto to_colored_text() const -> std::string;

  /// Convert incomplete trace to JSON map.
  [[nodiscard]] auto to_stack_json() const -> json::Value;

  /// Convert complete trace to JSON map.
  [[nodiscard]] auto to_tree_json() const -> json::Value;
};

/**
 * `Reader` represents a traced input stream with line and column information.
 */
struct Reader : trace::Traced<ReaderTrace> {
 public:
  using FailFunc = UniqueFunction<auto(const Reader &, std::string_view)->void>;

  /// Create a reader of input stream.
  explicit Reader(std::unique_ptr<io::InStream> inner, trace::Level trace_level,
                  FailFunc fail_func);

  /// Copy constructor (deleted to prevent copying).
  Reader(const Reader &) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const Reader &) -> Reader & = delete;

  /// Move constructor.
  Reader(Reader &&) noexcept = default;

  /// Move assignment operator.
  auto operator=(Reader &&) noexcept -> Reader & = default;

  /**
   * Get the inner wrapped input stream.
   *
   * @return Reference to the inner input stream.
   */
  [[nodiscard]] auto inner() -> io::InStream &;

  /**
   * Get the inner wrapped input stream.
   *
   * @return Reference to the inner input stream.
   */
  [[nodiscard]] auto inner() const -> const io::InStream &;

  /**
   * Call fail func with a message.
   *
   * @param message The error message.
   */
  [[noreturn]] auto fail(std::string_view message) -> void;

  /**
   * Read a variable based on a variable reading template.
   *
   * @tparam T The target type of the variable reading template.
   * @tparam D The derived class of the variable reading template.
   * @param v The variable reading template.
   * @return The value read from the input stream.
   */
  template <class T, class D>
  auto read(const Var<T, D> &v) -> T;

  /**
   * Read multiple variables and put them into a tuple.
   *
   * @tparam T The types of the variable reading templates.
   * @param vars The variable reading templates.
   * @return A tuple containing the values read from the input stream.
   */
  template <class... T>
  auto operator()(T... vars) -> std::tuple<typename T::Var::Target...>;

 private:
  std::unique_ptr<io::InStream> inner_;
  FailFunc fail_func_;
};

template <class T>
struct Vec;

/**
 * `Var` describes a "variable reading template".
 *
 * @tparam T The target type of the variable reading template.
 * @tparam D The derived class of the variable reading template. When other classes inherit from
 * `Var`, this template parameter should be the class itself.
 */
template <class T, class D>
struct Var {
 public:
  using Target = T;
  using Derived = D;

  /**
   * Destructor.
   */
  virtual ~Var() = 0;

  /**
   * Get the name of the variable reading template.
   *
   * @return The name of the variable as a string_view.
   */
  [[nodiscard]] auto name() const -> std::string_view;

  /**
   * Clone itself, the derived class (template class D) needs to implement the copy constructor.
   *
   * @return A copy of the variable.
   */
  [[nodiscard]] auto clone() const -> D;

  /**
   * Creates a copy with a new name.
   *
   * @param name The new name for the variable.
   * @return A copy of the variable with the new name.
   */
  [[nodiscard]] auto renamed(std::string_view name) const -> D;

  /**
   * Creates a copy with an index-based name.
   *
   * @param index The index to use as the new name.
   * @return A copy of the variable with the new name.
   */
  [[nodiscard]] auto renamed(std::size_t index) const -> D;

  /**
   * Creates a copy with a matrix-style index-based name.
   *
   * @param index0 The first index.
   * @param index1 The second index.
   * @return A copy of the variable with the new name.
   */
  [[nodiscard]] auto renamed(std::size_t index0, std::size_t index1) const -> D;

  /**
   * Parse a variable from a string.
   *
   * @param s The string representation of the variable.
   * @return The parsed value of the variable.
   */
  [[nodiscard]] auto parse(std::string_view s) const -> T;

  /**
   * Creates a `var::Vec<D>` containing self of size `len`.
   *
   * @param len The size of the `var::Vec`.
   * @return A `var::Vec<D>` containing copies of the variable.
   */
  auto operator*(std::size_t len) const -> Vec<D>;

  /**
   * Read the value of the variable from a `Reader` object.
   *
   * @param in The `Reader` object to read from.
   * @return The value of the variable.
   */
  virtual auto read_from(Reader &in) const -> T = 0;

 private:
  /**
   * Default constructor.
   */
  explicit Var();

  /**
   * Constructor with a specified name.
   *
   * @param name The name of the variable.
   */
  explicit Var(std::string name);

  friend D;

  std::string name_;
};

/**
 * `Int` is a variable reading template, indicating to read an integer in a given range in decimal
 * form.
 *
 * @tparam T The target type of the variable reading template.
 */
template <std::integral T>
struct Int : Var<T, Int<T>> {
 public:
  std::optional<T> min, max;

  /**
   * Default constructor.
   */
  explicit Int();

  /**
   * Constructor with name parameter.
   *
   * @param name The name of the Int variable.
   */
  explicit Int(std::string name);

  /**
   * Constructor with min and max parameters.
   *
   * @param min The minimum value of the Int variable.
   * @param max The maximum value of the Int variable.
   */
  explicit Int(std::optional<T> min, std::optional<T> max);

  /**
   * Constructor with min, max, and name parameters.
   *
   * @param name The name of the Int variable.
   * @param min The minimum value of the Int variable.
   * @param max The maximum value of the Int variable.
   */
  explicit Int(std::string name, std::optional<T> min, std::optional<T> max);

  /**
   * Read the value of the Int variable from a reader.
   *
   * @param in The reader to read from.
   * @return The read value.
   */
  auto read_from(Reader &in) const -> T override;
};

/**
 * `Float` is a variable reading template, indicating to read a floating-point number in a given
 * range in fixed form or scientific form.
 */
template <std::floating_point T>
struct Float : Var<T, Float<T>> {
 public:
  std::optional<T> min, max;

  /**
   * Default constructor.
   */
  explicit Float();

  /**
   * Constructor with name parameter.
   *
   * @param name The name of the Float variable.
   */
  explicit Float(std::string name);

  /**
   * Constructor with min and max range parameters.
   *
   * @param min The minimum value of the Float variable.
   * @param max The maximum value of the Float variable.
   */
  explicit Float(std::optional<T> min, std::optional<T> max);

  /**
   * Constructor with min and max range parameters and name parameter.
   *
   * @param name The name of the Float variable.
   * @param min The minimum value of the Float variable.
   * @param max The maximum value of the Float variable.
   */
  explicit Float(std::string name, std::optional<T> min, std::optional<T> max);

  /**
   * Read the value from the input reader.
   *
   * @param in The input reader.
   * @return The value read from the input reader.
   */
  auto read_from(Reader &in) const -> T override;
};

/**
 * `StrictFloat` is a variable reading template, indicating to read a floating-point number in a
 * given range in fixed for with digit count restrictions.
 */
template <std::floating_point T>
struct StrictFloat : Var<T, StrictFloat<T>> {
 public:
  T min, max;
  std::size_t min_n_digit, max_n_digit;

  /**
   * Constructor with min, max range, and digit count restrictions parameters.
   *
   * @param min The minimum value of the StrictFloat variable.
   * @param max The maximum value of the StrictFloat variable.
   * @param min_n_digit The minimum number of digits of the StrictFloat variable.
   * @param max_n_digit The maximum number of digits of the StrictFloat variable.
   */
  explicit StrictFloat(T min, T max, std::size_t min_n_digit, std::size_t max_n_digit);

  /**
   * Constructor with min, max range, digit count restrictions parameters, and name parameter.
   *
   * @param name The name of the StrictFloat variable.
   * @param min The minimum value of the StrictFloat variable.
   * @param max The maximum value of the StrictFloat variable.
   * @param min_n_digit The minimum number of digits of the StrictFloat variable.
   * @param max_n_digit The maximum number of digits of the StrictFloat variable.
   */
  explicit StrictFloat(std::string name, T min, T max, std::size_t min_n_digit,
                       std::size_t max_n_digit);

  /**
   * Read the value from the input reader.
   *
   * @param in The input reader.
   * @return The value read from the input reader.
   */
  auto read_from(Reader &in) const -> T override;
};

/**
 * `YesNo` is a variable reading template, indicating to read an boolean value.
 *
 * "Yes" mean true, "No" mean false. Case insensitive.
 *
 * @tparam T The target type of the variable reading template.
 */
struct YesNo : Var<bool, YesNo> {
 public:
  /**
   * Default constructor.
   */
  explicit YesNo();

  /**
   * Constructor with name parameter.
   *
   * @param name The name of the YesNo variable.
   */
  explicit YesNo(std::string name);

  /**
   * Read the value of the YesNo variable from a reader.
   *
   * @param in The reader to read from.
   * @return The read value.
   */
  auto read_from(Reader &in) const -> bool override;
};

/**
 * `String` is a variable reading template, indicating to read a string.
 */
struct String : Var<std::string, String> {
 public:
  /**
   * `Mode` represents the reading mode of the string.
   */
  struct Mode {
   public:
    enum Value : std::uint8_t {
      /// Reads a blank-separated token.
      TOKEN,
      /// Reads a sequence of alphanumeric characters.
      WORD,
      /// Reads until the end of the line.
      LINE,
    };

    /**
     * Default constructor for Mode.
     */
    Mode() = default;

    /**
     * Constructor for Mode with a given value.
     *
     * @param value The value of the mode.
     */
    constexpr Mode(Value value);  // NOLINT(google-explicit-constructor)

    /**
     * Implicit conversion operator to Value.
     *
     * @return The value of the mode.
     */
    constexpr operator Value() const;  // NOLINT(google-explicit-constructor)

    /**
     * Deleted conversion operator to bool.
     *
     * @return Deleted conversion operator to bool.
     */
    explicit operator bool() const = delete;

   private:
    Value value_;
  };

  std::optional<Pattern> pat;
  Mode mode;

  explicit String();
  explicit String(Mode mode);
  explicit String(Pattern pat);
  explicit String(Mode mode, Pattern pat);
  explicit String(std::string name);
  explicit String(std::string name, Mode mode);
  explicit String(std::string name, Pattern pat);
  explicit String(std::string name, Mode mode, Pattern pat);

  /**
   * Read the value from the input reader.
   *
   * @param in The input reader.
   * @return The value read from the input reader.
   */
  auto read_from(Reader &in) const -> std::string override;
};

/**
 * `Separator` is a variable reading template, indicating to read a character as a separator.
 *
 *- If `sep` is `std::nullopt`, it does noting.
 * - Otherwise, if it is in strict mode, read exact one character determine whether it is same as
 *`sep`.
 * - Otherwise, if `std::isspace(sep)`, read the next consecutive whitespaces.
 * - Otherwise, try skipping blanks and read exact one character `sep`.
 */
struct Separator : Var<std::nullopt_t, Separator> {
 public:
  std::optional<unsigned char> sep;

  /**
   * Constructs a `Separator` object with the specified separator character.
   *
   * @param sep The separator character.
   */
  explicit Separator(std::optional<unsigned char> sep);

  /**
   * Constructs a `Separator` object with the specified separator character and name.
   *
   * @param name The name of the `Separator`.
   * @param sep The separator character.
   */
  explicit Separator(std::string name, std::optional<unsigned char> sep);

  /**
   * Reads the separator character from the input reader.
   *
   * @param in The input reader.
   * @return `std::nullopt` to indicate that no value is read.
   */
  auto read_from(Reader &in) const -> std::nullopt_t override;
};

/**
 * `Vec` is a variable reading template, used to read a fixed length of variables of the same type
 * and return them as `std::vector`.
 *
 * @tparam T The type of the inner variable reading template.
 */
template <class T>
struct Vec : Var<std::vector<typename T::Var::Target>, Vec<T>> {
 public:
  /// The type of the element in the vector.
  T element;
  /// The length of the vector.
  std::size_t len;
  /// The separator between elements.
  Separator sep;

  /**
   * Constructor.
   *
   * @param element The type of the element in the vector.
   * @param len The length of the vector.
   */
  explicit Vec(T element, std::size_t len);

  /**
   * Constructor with separator.
   *
   * @param element The type of the element in the vector.
   * @param len The length of the vector.
   * @param sep The separator between elements.
   */
  explicit Vec(T element, std::size_t len, Separator sep);

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The vector of elements.
   */
  auto read_from(Reader &in) const -> std::vector<typename T::Var::Target> override;
};

/**
 * `Mat` is a variable reading template used to read a fixed-size variable matrix. The elements in
 * the matrix have the same type.
 *
 * @tparam T The type of the inner variable reading template.
 */
template <class T>
struct Mat : Var<std::vector<std::vector<typename T::Var::Target>>, Mat<T>> {
 public:
  /// The type of the element in the matrix.
  T element;
  /// The length of the first dimension of the matrix.
  std::size_t len0;
  /// The length of the second dimension of the matrix.
  std::size_t len1;
  /// The separator used for the first dimension.
  Separator sep0;
  /// The separator used for the second dimension.
  Separator sep1;

  /**
   * Constructor.
   *
   * @param element The type of the element in the matrix.
   * @param len0 The length of the first dimension of the matrix.
   * @param len1 The length of the second dimension of the matrix.
   */
  explicit Mat(T element, std::size_t len0, std::size_t len1);

  /**
   * Constructor with separators.
   *
   * @param element The type of the element in the matrix.
   * @param len0 The length of the first dimension of the matrix.
   * @param len1 The length of the second dimension of the matrix.
   * @param sep0 The separator used for the first dimension.
   * @param sep1 The separator used for the second dimension.
   */
  explicit Mat(T element, std::size_t len0, std::size_t len1, Separator sep0, Separator sep1);

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The matrix of elements.
   */
  auto read_from(Reader &in) const -> std::vector<std::vector<typename T::Var::Target>> override;
};

/**
 * `Pair` is a variable reading template that reads two variables separated by a given separator
 * and returns them as `std::pair`.
 *
 * @tparam F The type of the first variable reading template.
 * @tparam S The type of the second variable reading template.
 */
template <class F, class S>
struct Pair : Var<std::pair<typename F::Var::Target, typename S::Var::Target>, Pair<F, S>> {
 public:
  /// The first element of the pair.
  F first;
  /// The second element of the pair.
  S second;
  /// The separator used when converting to string.
  Separator sep;

  /**
   * Constructor from std::pair.
   *
   * @param pr The std::pair to initialize the Pair with.
   */
  explicit Pair(std::pair<F, S> pr);

  /**
   * Constructor from std::pair with separator.
   *
   * @param pr The std::pair to initialize the Pair with.
   * @param sep The separator used when converting to string.
   */
  explicit Pair(std::pair<F, S> pr, Separator sep);

  /**
   * Constructor from std::pair with name.
   *
   * @param name The name of the pair.
   * @param pr The std::pair to initialize the Pair with.
   */
  explicit Pair(std::string name, std::pair<F, S> pr);

  /**
   * Constructor from std::pair with separator and name.
   *
   * @param name The name of the pair.
   * @param pr The std::pair to initialize the Pair with.
   * @param sep The separator used when converting to string.
   */
  explicit Pair(std::string name, std::pair<F, S> pr, Separator sep);

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The pair of elements.
   */
  auto read_from(Reader &in) const
      -> std::pair<typename F::Var::Target, typename S::Var::Target> override;
};

/**
 * Tuple of variables.
 *
 * @tparam T The type of the variable reading templates.
 */
template <class... T>
struct Tuple : Var<std::tuple<typename T::Var::Target...>, Tuple<T...>> {
 public:
  /// The elements of the tuple.
  std::tuple<T...> elements;
  /// The separator used when converting to string.
  Separator sep;

  /**
   * Constructor.
   *
   * @param elements The elements of the tuple.
   */
  explicit Tuple(std::tuple<T...> elements);

  /**
   * Constructor with separator.
   *
   * @param elements The elements of the tuple.
   * @param sep The separator used when converting to string.
   */
  explicit Tuple(std::tuple<T...> elements, Separator sep);

  /**
   * Constructor with name.
   *
   * @param name The name of the tuple.
   * @param elements The elements of the tuple.
   */
  explicit Tuple(std::string name, std::tuple<T...> elements);

  /**
   * Constructor with separator and name.
   *
   * @param name The name of the tuple.
   * @param elements The elements of the tuple.
   * @param sep The separator used when converting to string.
   */
  explicit Tuple(std::string name, std::tuple<T...> elements, Separator sep);

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The tuple of elements.
   */
  auto read_from(Reader &in) const -> std::tuple<typename T::Var::Target...> override;

 private:
  /**
   * @brief Implementation detail for reading a tuple with separators.
   *
   * This helper function uses a fold expression over an index sequence to read each
   * element of the tuple. It correctly inserts a separator read operation
   * before each element except the first one.
   *
   * @tparam Is The compile-time sequence of indices for the tuple elements.
   * @param in The reader object.
   * @param seq An empty `std::index_sequence` object to deduce the indices `Is`.
   * @return The tuple of elements read from the stream.
   */
  template <std::size_t... Is>
  auto read_from_impl(Reader &in, std::index_sequence<Is...>) const
      -> std::tuple<typename T::Var::Target...>;
};

/**
 * `FnVar` is used to wrap a function into a variable reading template.
 *
 * The type of the first parameter of the wrapped function must be `var::Reader`, and subsequent
 * parameters (which may not exist) are arbitrary.
 *
 * @tparam F The type of the function.
 */
template <class F>
struct FnVar : Var<typename std::function<F>::result_type, FnVar<F>> {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the variable to be read.
   * @param f The function object.
   * @param args The second to last arguments to the function, the count and type must match the
   * parameters of `f`.
   */
  template <class... Args>
  FnVar(std::string name, std::function<F> f, Args &&...args);

  /**
   * Read from reader.
   *
   * Use `in` as the first argument, together with subsequent arguments passed in the constructor,
   * to call the wrapped function.
   *
   * @param in The reader object.
   * @return The result of the function.
   */
  auto read_from(Reader &in) const -> typename std::function<F>::result_type override;

 private:
  /// The inner function.
  std::function<typename std::function<F>::result_type(Reader &in)> inner_function_;
};

// Defines the requirements for a type T to be "readable"
// with a static 'read' method that takes a Reader and additional arguments.
template <typename T, typename... Args>
concept Readable = requires(Reader &reader, Args &&...args) {
  // T must have a static member function named 'read'.
  // It must be callable with a Reader& and the given Args.
  // Its return type must be convertible to T (or exactly T).
  { T::read(reader, std::forward<Args>(args)...) } -> std::same_as<T>;
};

/**
 * For a type `T` which has implemented `static auto T::read(var::Reader&, ...) -> T`,
 * `ExtVar<T>` provides a variable template for creating `T` by calling `T::read`.
 *
 * @tparam T The type of the variable, which must have a static `auto T::read(var::Reader&, ...)
 * -> T`
 */
template <class T>
struct ExtVar : Var<T, ExtVar<T>> {
 public:
  /**
   * Constructor.
   *
   * @param name The name of the variable to be read.
   * @param args The second to last arguments to the function `T::read`.
   */
  template <class... Args>
  explicit ExtVar(std::string name, Args &&...args)
    requires Readable<T, Args...>;

  template <class... Args>
  explicit ExtVar(std::size_t index, Args &&...args)
    requires Readable<T, Args...>;

  /**
   * Read from reader.
   *
   * Use `in` as the first argument, together with subsequent arguments passed in the constructor,
   * to call `T::read`.
   *
   * @param in The reader object.
   * @return The result of `T::read`.
   */
  auto read_from(Reader &in) const -> T override;

 private:
  /// The inner function that encapsulates the call to T::read.
  std::function<T(Reader &)> inner_function_;
};

/**
 * `ExtVec` is a variable reading template that reads a sequence of variables of a custom
 * readable type `T` and returns them as a `std::vector<T>`. It iterates over a provided
 * range, and for each element in the range, it calls `T::read`, passing the range element
 * as an additional argument. This is useful for reading collections where each element's
 * parsing depends on its index or corresponding data.
 *
 * @tparam T The readable type of the elements in the vector. Must satisfy the `Readable` concept.
 */
template <class T>
struct ExtVec : Var<std::vector<T>, ExtVec<T>> {
 public:
  /**
   * Constructor.
   *
   * @tparam Range The type of the range-like object to iterate over.
   * @tparam Args The types of the fixed arguments to pass to `T::read`.
   * @param name The name of the variable.
   * @param range The range-like object (e.g., `std::vector`, `std::views::iota`) whose elements
   *              will be passed as the final argument to `T::read`.
   * @param sep The separator between elements.
   * @param args The fixed arguments to be passed to `T::read` before the range element.
   */
  template <std::ranges::range Range, class... Args>
  explicit ExtVec(std::string name, Range &&range, Separator sep, Args &&...args)
    requires Readable<T, Args..., std::ranges::range_value_t<Range>>;

  /**
   * Read from reader.
   *
   * @param in The reader object.
   * @return The vector of elements.
   */
  auto read_from(Reader &in) const -> std::vector<T> override;

 private:
  /// The inner function that encapsulates the call to T::read for each element.
  std::function<std::vector<T>(Reader &)> inner_function_;
};

using i8 = Int<std::int8_t>;
using u8 = Int<std::uint8_t>;
using i16 = Int<std::int16_t>;
using u16 = Int<std::uint16_t>;
using i32 = Int<std::int32_t>;
using u32 = Int<std::uint32_t>;
using i64 = Int<std::int64_t>;
using u64 = Int<std::uint64_t>;
using f32 = Float<float>;
using f64 = Float<double>;
using fext = Float<long double>;
using f32s = StrictFloat<float>;
using f64s = StrictFloat<double>;
using fexts = StrictFloat<long double>;

const auto space = Separator("space", ' ');
const auto tab = Separator("tab", '\t');
const auto eoln = Separator("eoln", '\n');
};  // namespace cplib::var

// --- Start embedded: var.i.hpp ---
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


#include <algorithm>
#include <cctype>
#include <charconv>
#include <concepts>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <ios>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

// --- Start embedded: float_parse.i.hpp ---
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

#ifndef CPLIB_FLOAT_PARSE_I_HPP_
#define CPLIB_FLOAT_PARSE_I_HPP_

#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

namespace cplib::detail::float_parse::core {

struct Value128 {
  std::uint64_t low;
  std::uint64_t high;
};
struct AdjustedMantissa {
  std::uint64_t mantissa;
  std::int32_t power_2;
};

inline auto leading_zeroes(std::uint64_t x) noexcept -> int {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_clzll(x);
#else
  int n = 0;
  while ((x & (std::uint64_t{1} << 63)) == 0) {
    ++n;
    x <<= 1;
  }
  return n;
#endif
}

inline auto full_multiplication(std::uint64_t a, std::uint64_t b) noexcept -> Value128 {
#if defined(__SIZEOF_INT128__)
  __uint128_t r = static_cast<__uint128_t>(a) * b;
  return {.low = static_cast<std::uint64_t>(r), .high = static_cast<std::uint64_t>(r >> 64)};
#else
  std::uint64_t a0 = static_cast<std::uint32_t>(a), a1 = a >> 32;
  std::uint64_t b0 = static_cast<std::uint32_t>(b), b1 = b >> 32;
  std::uint64_t p00 = a0 * b0;
  std::uint64_t p01 = a0 * b1;
  std::uint64_t p10 = a1 * b0;
  std::uint64_t p11 = a1 * b1;
  std::uint64_t middle =
      (p00 >> 32) + static_cast<std::uint32_t>(p01) + static_cast<std::uint32_t>(p10);
  std::uint64_t low = (middle << 32) | static_cast<std::uint32_t>(p00);
  std::uint64_t high = p11 + (p01 >> 32) + (p10 >> 32) + (middle >> 32);
  return {.low = low, .high = high};
#endif
}

namespace powers {
inline constexpr int smallest_power_of_five = -342;
inline constexpr int largest_power_of_five = 308;
inline constexpr auto power_of_five_128 = std::to_array<std::uint64_t>({
    0xeef453d6923bd65a, 0x113faa2906a13b3f,
    0x9558b4661b6565f8, 0x4ac7ca59a424c507,
    0xbaaee17fa23ebf76, 0x5d79bcf00d2df649,
    0xe95a99df8ace6f53, 0xf4d82c2c107973dc,
    0x91d8a02bb6c10594, 0x79071b9b8a4be869,
    0xb64ec836a47146f9, 0x9748e2826cdee284,
    0xe3e27a444d8d98b7, 0xfd1b1b2308169b25,
    0x8e6d8c6ab0787f72, 0xfe30f0f5e50e20f7,
    0xb208ef855c969f4f, 0xbdbd2d335e51a935,
    0xde8b2b66b3bc4723, 0xad2c788035e61382,
    0x8b16fb203055ac76, 0x4c3bcb5021afcc31,
    0xaddcb9e83c6b1793, 0xdf4abe242a1bbf3d,
    0xd953e8624b85dd78, 0xd71d6dad34a2af0d,
    0x87d4713d6f33aa6b, 0x8672648c40e5ad68,
    0xa9c98d8ccb009506, 0x680efdaf511f18c2,
    0xd43bf0effdc0ba48, 0x212bd1b2566def2,
    0x84a57695fe98746d, 0x14bb630f7604b57,
    0xa5ced43b7e3e9188, 0x419ea3bd35385e2d,
    0xcf42894a5dce35ea, 0x52064cac828675b9,
    0x818995ce7aa0e1b2, 0x7343efebd1940993,
    0xa1ebfb4219491a1f, 0x1014ebe6c5f90bf8,
    0xca66fa129f9b60a6, 0xd41a26e077774ef6,
    0xfd00b897478238d0, 0x8920b098955522b4,
    0x9e20735e8cb16382, 0x55b46e5f5d5535b0,
    0xc5a890362fddbc62, 0xeb2189f734aa831d,
    0xf712b443bbd52b7b, 0xa5e9ec7501d523e4,
    0x9a6bb0aa55653b2d, 0x47b233c92125366e,
    0xc1069cd4eabe89f8, 0x999ec0bb696e840a,
    0xf148440a256e2c76, 0xc00670ea43ca250d,
    0x96cd2a865764dbca, 0x380406926a5e5728,
    0xbc807527ed3e12bc, 0xc605083704f5ecf2,
    0xeba09271e88d976b, 0xf7864a44c633682e,
    0x93445b8731587ea3, 0x7ab3ee6afbe0211d,
    0xb8157268fdae9e4c, 0x5960ea05bad82964,
    0xe61acf033d1a45df, 0x6fb92487298e33bd,
    0x8fd0c16206306bab, 0xa5d3b6d479f8e056,
    0xb3c4f1ba87bc8696, 0x8f48a4899877186c,
    0xe0b62e2929aba83c, 0x331acdabfe94de87,
    0x8c71dcd9ba0b4925, 0x9ff0c08b7f1d0b14,
    0xaf8e5410288e1b6f, 0x7ecf0ae5ee44dd9,
    0xdb71e91432b1a24a, 0xc9e82cd9f69d6150,
    0x892731ac9faf056e, 0xbe311c083a225cd2,
    0xab70fe17c79ac6ca, 0x6dbd630a48aaf406,
    0xd64d3d9db981787d, 0x92cbbccdad5b108,
    0x85f0468293f0eb4e, 0x25bbf56008c58ea5,
    0xa76c582338ed2621, 0xaf2af2b80af6f24e,
    0xd1476e2c07286faa, 0x1af5af660db4aee1,
    0x82cca4db847945ca, 0x50d98d9fc890ed4d,
    0xa37fce126597973c, 0xe50ff107bab528a0,
    0xcc5fc196fefd7d0c, 0x1e53ed49a96272c8,
    0xff77b1fcbebcdc4f, 0x25e8e89c13bb0f7a,
    0x9faacf3df73609b1, 0x77b191618c54e9ac,
    0xc795830d75038c1d, 0xd59df5b9ef6a2417,
    0xf97ae3d0d2446f25, 0x4b0573286b44ad1d,
    0x9becce62836ac577, 0x4ee367f9430aec32,
    0xc2e801fb244576d5, 0x229c41f793cda73f,
    0xf3a20279ed56d48a, 0x6b43527578c1110f,
    0x9845418c345644d6, 0x830a13896b78aaa9,
    0xbe5691ef416bd60c, 0x23cc986bc656d553,
    0xedec366b11c6cb8f, 0x2cbfbe86b7ec8aa8,
    0x94b3a202eb1c3f39, 0x7bf7d71432f3d6a9,
    0xb9e08a83a5e34f07, 0xdaf5ccd93fb0cc53,
    0xe858ad248f5c22c9, 0xd1b3400f8f9cff68,
    0x91376c36d99995be, 0x23100809b9c21fa1,
    0xb58547448ffffb2d, 0xabd40a0c2832a78a,
    0xe2e69915b3fff9f9, 0x16c90c8f323f516c,
    0x8dd01fad907ffc3b, 0xae3da7d97f6792e3,
    0xb1442798f49ffb4a, 0x99cd11cfdf41779c,
    0xdd95317f31c7fa1d, 0x40405643d711d583,
    0x8a7d3eef7f1cfc52, 0x482835ea666b2572,
    0xad1c8eab5ee43b66, 0xda3243650005eecf,
    0xd863b256369d4a40, 0x90bed43e40076a82,
    0x873e4f75e2224e68, 0x5a7744a6e804a291,
    0xa90de3535aaae202, 0x711515d0a205cb36,
    0xd3515c2831559a83, 0xd5a5b44ca873e03,
    0x8412d9991ed58091, 0xe858790afe9486c2,
    0xa5178fff668ae0b6, 0x626e974dbe39a872,
    0xce5d73ff402d98e3, 0xfb0a3d212dc8128f,
    0x80fa687f881c7f8e, 0x7ce66634bc9d0b99,
    0xa139029f6a239f72, 0x1c1fffc1ebc44e80,
    0xc987434744ac874e, 0xa327ffb266b56220,
    0xfbe9141915d7a922, 0x4bf1ff9f0062baa8,
    0x9d71ac8fada6c9b5, 0x6f773fc3603db4a9,
    0xc4ce17b399107c22, 0xcb550fb4384d21d3,
    0xf6019da07f549b2b, 0x7e2a53a146606a48,
    0x99c102844f94e0fb, 0x2eda7444cbfc426d,
    0xc0314325637a1939, 0xfa911155fefb5308,
    0xf03d93eebc589f88, 0x793555ab7eba27ca,
    0x96267c7535b763b5, 0x4bc1558b2f3458de,
    0xbbb01b9283253ca2, 0x9eb1aaedfb016f16,
    0xea9c227723ee8bcb, 0x465e15a979c1cadc,
    0x92a1958a7675175f, 0xbfacd89ec191ec9,
    0xb749faed14125d36, 0xcef980ec671f667b,
    0xe51c79a85916f484, 0x82b7e12780e7401a,
    0x8f31cc0937ae58d2, 0xd1b2ecb8b0908810,
    0xb2fe3f0b8599ef07, 0x861fa7e6dcb4aa15,
    0xdfbdcece67006ac9, 0x67a791e093e1d49a,
    0x8bd6a141006042bd, 0xe0c8bb2c5c6d24e0,
    0xaecc49914078536d, 0x58fae9f773886e18,
    0xda7f5bf590966848, 0xaf39a475506a899e,
    0x888f99797a5e012d, 0x6d8406c952429603,
    0xaab37fd7d8f58178, 0xc8e5087ba6d33b83,
    0xd5605fcdcf32e1d6, 0xfb1e4a9a90880a64,
    0x855c3be0a17fcd26, 0x5cf2eea09a55067f,
    0xa6b34ad8c9dfc06f, 0xf42faa48c0ea481e,
    0xd0601d8efc57b08b, 0xf13b94daf124da26,
    0x823c12795db6ce57, 0x76c53d08d6b70858,
    0xa2cb1717b52481ed, 0x54768c4b0c64ca6e,
    0xcb7ddcdda26da268, 0xa9942f5dcf7dfd09,
    0xfe5d54150b090b02, 0xd3f93b35435d7c4c,
    0x9efa548d26e5a6e1, 0xc47bc5014a1a6daf,
    0xc6b8e9b0709f109a, 0x359ab6419ca1091b,
    0xf867241c8cc6d4c0, 0xc30163d203c94b62,
    0x9b407691d7fc44f8, 0x79e0de63425dcf1d,
    0xc21094364dfb5636, 0x985915fc12f542e4,
    0xf294b943e17a2bc4, 0x3e6f5b7b17b2939d,
    0x979cf3ca6cec5b5a, 0xa705992ceecf9c42,
    0xbd8430bd08277231, 0x50c6ff782a838353,
    0xece53cec4a314ebd, 0xa4f8bf5635246428,
    0x940f4613ae5ed136, 0x871b7795e136be99,
    0xb913179899f68584, 0x28e2557b59846e3f,
    0xe757dd7ec07426e5, 0x331aeada2fe589cf,
    0x9096ea6f3848984f, 0x3ff0d2c85def7621,
    0xb4bca50b065abe63, 0xfed077a756b53a9,
    0xe1ebce4dc7f16dfb, 0xd3e8495912c62894,
    0x8d3360f09cf6e4bd, 0x64712dd7abbbd95c,
    0xb080392cc4349dec, 0xbd8d794d96aacfb3,
    0xdca04777f541c567, 0xecf0d7a0fc5583a0,
    0x89e42caaf9491b60, 0xf41686c49db57244,
    0xac5d37d5b79b6239, 0x311c2875c522ced5,
    0xd77485cb25823ac7, 0x7d633293366b828b,
    0x86a8d39ef77164bc, 0xae5dff9c02033197,
    0xa8530886b54dbdeb, 0xd9f57f830283fdfc,
    0xd267caa862a12d66, 0xd072df63c324fd7b,
    0x8380dea93da4bc60, 0x4247cb9e59f71e6d,
    0xa46116538d0deb78, 0x52d9be85f074e608,
    0xcd795be870516656, 0x67902e276c921f8b,
    0x806bd9714632dff6, 0xba1cd8a3db53b6,
    0xa086cfcd97bf97f3, 0x80e8a40eccd228a4,
    0xc8a883c0fdaf7df0, 0x6122cd128006b2cd,
    0xfad2a4b13d1b5d6c, 0x796b805720085f81,
    0x9cc3a6eec6311a63, 0xcbe3303674053bb0,
    0xc3f490aa77bd60fc, 0xbedbfc4411068a9c,
    0xf4f1b4d515acb93b, 0xee92fb5515482d44,
    0x991711052d8bf3c5, 0x751bdd152d4d1c4a,
    0xbf5cd54678eef0b6, 0xd262d45a78a0635d,
    0xef340a98172aace4, 0x86fb897116c87c34,
    0x9580869f0e7aac0e, 0xd45d35e6ae3d4da0,
    0xbae0a846d2195712, 0x8974836059cca109,
    0xe998d258869facd7, 0x2bd1a438703fc94b,
    0x91ff83775423cc06, 0x7b6306a34627ddcf,
    0xb67f6455292cbf08, 0x1a3bc84c17b1d542,
    0xe41f3d6a7377eeca, 0x20caba5f1d9e4a93,
    0x8e938662882af53e, 0x547eb47b7282ee9c,
    0xb23867fb2a35b28d, 0xe99e619a4f23aa43,
    0xdec681f9f4c31f31, 0x6405fa00e2ec94d4,
    0x8b3c113c38f9f37e, 0xde83bc408dd3dd04,
    0xae0b158b4738705e, 0x9624ab50b148d445,
    0xd98ddaee19068c76, 0x3badd624dd9b0957,
    0x87f8a8d4cfa417c9, 0xe54ca5d70a80e5d6,
    0xa9f6d30a038d1dbc, 0x5e9fcf4ccd211f4c,
    0xd47487cc8470652b, 0x7647c3200069671f,
    0x84c8d4dfd2c63f3b, 0x29ecd9f40041e073,
    0xa5fb0a17c777cf09, 0xf468107100525890,
    0xcf79cc9db955c2cc, 0x7182148d4066eeb4,
    0x81ac1fe293d599bf, 0xc6f14cd848405530,
    0xa21727db38cb002f, 0xb8ada00e5a506a7c,
    0xca9cf1d206fdc03b, 0xa6d90811f0e4851c,
    0xfd442e4688bd304a, 0x908f4a166d1da663,
    0x9e4a9cec15763e2e, 0x9a598e4e043287fe,
    0xc5dd44271ad3cdba, 0x40eff1e1853f29fd,
    0xf7549530e188c128, 0xd12bee59e68ef47c,
    0x9a94dd3e8cf578b9, 0x82bb74f8301958ce,
    0xc13a148e3032d6e7, 0xe36a52363c1faf01,
    0xf18899b1bc3f8ca1, 0xdc44e6c3cb279ac1,
    0x96f5600f15a7b7e5, 0x29ab103a5ef8c0b9,
    0xbcb2b812db11a5de, 0x7415d448f6b6f0e7,
    0xebdf661791d60f56, 0x111b495b3464ad21,
    0x936b9fcebb25c995, 0xcab10dd900beec34,
    0xb84687c269ef3bfb, 0x3d5d514f40eea742,
    0xe65829b3046b0afa, 0xcb4a5a3112a5112,
    0x8ff71a0fe2c2e6dc, 0x47f0e785eaba72ab,
    0xb3f4e093db73a093, 0x59ed216765690f56,
    0xe0f218b8d25088b8, 0x306869c13ec3532c,
    0x8c974f7383725573, 0x1e414218c73a13fb,
    0xafbd2350644eeacf, 0xe5d1929ef90898fa,
    0xdbac6c247d62a583, 0xdf45f746b74abf39,
    0x894bc396ce5da772, 0x6b8bba8c328eb783,
    0xab9eb47c81f5114f, 0x66ea92f3f326564,
    0xd686619ba27255a2, 0xc80a537b0efefebd,
    0x8613fd0145877585, 0xbd06742ce95f5f36,
    0xa798fc4196e952e7, 0x2c48113823b73704,
    0xd17f3b51fca3a7a0, 0xf75a15862ca504c5,
    0x82ef85133de648c4, 0x9a984d73dbe722fb,
    0xa3ab66580d5fdaf5, 0xc13e60d0d2e0ebba,
    0xcc963fee10b7d1b3, 0x318df905079926a8,
    0xffbbcfe994e5c61f, 0xfdf17746497f7052,
    0x9fd561f1fd0f9bd3, 0xfeb6ea8bedefa633,
    0xc7caba6e7c5382c8, 0xfe64a52ee96b8fc0,
    0xf9bd690a1b68637b, 0x3dfdce7aa3c673b0,
    0x9c1661a651213e2d, 0x6bea10ca65c084e,
    0xc31bfa0fe5698db8, 0x486e494fcff30a62,
    0xf3e2f893dec3f126, 0x5a89dba3c3efccfa,
    0x986ddb5c6b3a76b7, 0xf89629465a75e01c,
    0xbe89523386091465, 0xf6bbb397f1135823,
    0xee2ba6c0678b597f, 0x746aa07ded582e2c,
    0x94db483840b717ef, 0xa8c2a44eb4571cdc,
    0xba121a4650e4ddeb, 0x92f34d62616ce413,
    0xe896a0d7e51e1566, 0x77b020baf9c81d17,
    0x915e2486ef32cd60, 0xace1474dc1d122e,
    0xb5b5ada8aaff80b8, 0xd819992132456ba,
    0xe3231912d5bf60e6, 0x10e1fff697ed6c69,
    0x8df5efabc5979c8f, 0xca8d3ffa1ef463c1,
    0xb1736b96b6fd83b3, 0xbd308ff8a6b17cb2,
    0xddd0467c64bce4a0, 0xac7cb3f6d05ddbde,
    0x8aa22c0dbef60ee4, 0x6bcdf07a423aa96b,
    0xad4ab7112eb3929d, 0x86c16c98d2c953c6,
    0xd89d64d57a607744, 0xe871c7bf077ba8b7,
    0x87625f056c7c4a8b, 0x11471cd764ad4972,
    0xa93af6c6c79b5d2d, 0xd598e40d3dd89bcf,
    0xd389b47879823479, 0x4aff1d108d4ec2c3,
    0x843610cb4bf160cb, 0xcedf722a585139ba,
    0xa54394fe1eedb8fe, 0xc2974eb4ee658828,
    0xce947a3da6a9273e, 0x733d226229feea32,
    0x811ccc668829b887, 0x806357d5a3f525f,
    0xa163ff802a3426a8, 0xca07c2dcb0cf26f7,
    0xc9bcff6034c13052, 0xfc89b393dd02f0b5,
    0xfc2c3f3841f17c67, 0xbbac2078d443ace2,
    0x9d9ba7832936edc0, 0xd54b944b84aa4c0d,
    0xc5029163f384a931, 0xa9e795e65d4df11,
    0xf64335bcf065d37d, 0x4d4617b5ff4a16d5,
    0x99ea0196163fa42e, 0x504bced1bf8e4e45,
    0xc06481fb9bcf8d39, 0xe45ec2862f71e1d6,
    0xf07da27a82c37088, 0x5d767327bb4e5a4c,
    0x964e858c91ba2655, 0x3a6a07f8d510f86f,
    0xbbe226efb628afea, 0x890489f70a55368b,
    0xeadab0aba3b2dbe5, 0x2b45ac74ccea842e,
    0x92c8ae6b464fc96f, 0x3b0b8bc90012929d,
    0xb77ada0617e3bbcb, 0x9ce6ebb40173744,
    0xe55990879ddcaabd, 0xcc420a6a101d0515,
    0x8f57fa54c2a9eab6, 0x9fa946824a12232d,
    0xb32df8e9f3546564, 0x47939822dc96abf9,
    0xdff9772470297ebd, 0x59787e2b93bc56f7,
    0x8bfbea76c619ef36, 0x57eb4edb3c55b65a,
    0xaefae51477a06b03, 0xede622920b6b23f1,
    0xdab99e59958885c4, 0xe95fab368e45eced,
    0x88b402f7fd75539b, 0x11dbcb0218ebb414,
    0xaae103b5fcd2a881, 0xd652bdc29f26a119,
    0xd59944a37c0752a2, 0x4be76d3346f0495f,
    0x857fcae62d8493a5, 0x6f70a4400c562ddb,
    0xa6dfbd9fb8e5b88e, 0xcb4ccd500f6bb952,
    0xd097ad07a71f26b2, 0x7e2000a41346a7a7,
    0x825ecc24c873782f, 0x8ed400668c0c28c8,
    0xa2f67f2dfa90563b, 0x728900802f0f32fa,
    0xcbb41ef979346bca, 0x4f2b40a03ad2ffb9,
    0xfea126b7d78186bc, 0xe2f610c84987bfa8,
    0x9f24b832e6b0f436, 0xdd9ca7d2df4d7c9,
    0xc6ede63fa05d3143, 0x91503d1c79720dbb,
    0xf8a95fcf88747d94, 0x75a44c6397ce912a,
    0x9b69dbe1b548ce7c, 0xc986afbe3ee11aba,
    0xc24452da229b021b, 0xfbe85badce996168,
    0xf2d56790ab41c2a2, 0xfae27299423fb9c3,
    0x97c560ba6b0919a5, 0xdccd879fc967d41a,
    0xbdb6b8e905cb600f, 0x5400e987bbc1c920,
    0xed246723473e3813, 0x290123e9aab23b68,
    0x9436c0760c86e30b, 0xf9a0b6720aaf6521,
    0xb94470938fa89bce, 0xf808e40e8d5b3e69,
    0xe7958cb87392c2c2, 0xb60b1d1230b20e04,
    0x90bd77f3483bb9b9, 0xb1c6f22b5e6f48c2,
    0xb4ecd5f01a4aa828, 0x1e38aeb6360b1af3,
    0xe2280b6c20dd5232, 0x25c6da63c38de1b0,
    0x8d590723948a535f, 0x579c487e5a38ad0e,
    0xb0af48ec79ace837, 0x2d835a9df0c6d851,
    0xdcdb1b2798182244, 0xf8e431456cf88e65,
    0x8a08f0f8bf0f156b, 0x1b8e9ecb641b58ff,
    0xac8b2d36eed2dac5, 0xe272467e3d222f3f,
    0xd7adf884aa879177, 0x5b0ed81dcc6abb0f,
    0x86ccbb52ea94baea, 0x98e947129fc2b4e9,
    0xa87fea27a539e9a5, 0x3f2398d747b36224,
    0xd29fe4b18e88640e, 0x8eec7f0d19a03aad,
    0x83a3eeeef9153e89, 0x1953cf68300424ac,
    0xa48ceaaab75a8e2b, 0x5fa8c3423c052dd7,
    0xcdb02555653131b6, 0x3792f412cb06794d,
    0x808e17555f3ebf11, 0xe2bbd88bbee40bd0,
    0xa0b19d2ab70e6ed6, 0x5b6aceaeae9d0ec4,
    0xc8de047564d20a8b, 0xf245825a5a445275,
    0xfb158592be068d2e, 0xeed6e2f0f0d56712,
    0x9ced737bb6c4183d, 0x55464dd69685606b,
    0xc428d05aa4751e4c, 0xaa97e14c3c26b886,
    0xf53304714d9265df, 0xd53dd99f4b3066a8,
    0x993fe2c6d07b7fab, 0xe546a8038efe4029,
    0xbf8fdb78849a5f96, 0xde98520472bdd033,
    0xef73d256a5c0f77c, 0x963e66858f6d4440,
    0x95a8637627989aad, 0xdde7001379a44aa8,
    0xbb127c53b17ec159, 0x5560c018580d5d52,
    0xe9d71b689dde71af, 0xaab8f01e6e10b4a6,
    0x9226712162ab070d, 0xcab3961304ca70e8,
    0xb6b00d69bb55c8d1, 0x3d607b97c5fd0d22,
    0xe45c10c42a2b3b05, 0x8cb89a7db77c506a,
    0x8eb98a7a9a5b04e3, 0x77f3608e92adb242,
    0xb267ed1940f1c61c, 0x55f038b237591ed3,
    0xdf01e85f912e37a3, 0x6b6c46dec52f6688,
    0x8b61313bbabce2c6, 0x2323ac4b3b3da015,
    0xae397d8aa96c1b77, 0xabec975e0a0d081a,
    0xd9c7dced53c72255, 0x96e7bd358c904a21,
    0x881cea14545c7575, 0x7e50d64177da2e54,
    0xaa242499697392d2, 0xdde50bd1d5d0b9e9,
    0xd4ad2dbfc3d07787, 0x955e4ec64b44e864,
    0x84ec3c97da624ab4, 0xbd5af13bef0b113e,
    0xa6274bbdd0fadd61, 0xecb1ad8aeacdd58e,
    0xcfb11ead453994ba, 0x67de18eda5814af2,
    0x81ceb32c4b43fcf4, 0x80eacf948770ced7,
    0xa2425ff75e14fc31, 0xa1258379a94d028d,
    0xcad2f7f5359a3b3e, 0x96ee45813a04330,
    0xfd87b5f28300ca0d, 0x8bca9d6e188853fc,
    0x9e74d1b791e07e48, 0x775ea264cf55347e,
    0xc612062576589dda, 0x95364afe032a819e,
    0xf79687aed3eec551, 0x3a83ddbd83f52205,
    0x9abe14cd44753b52, 0xc4926a9672793543,
    0xc16d9a0095928a27, 0x75b7053c0f178294,
    0xf1c90080baf72cb1, 0x5324c68b12dd6339,
    0x971da05074da7bee, 0xd3f6fc16ebca5e04,
    0xbce5086492111aea, 0x88f4bb1ca6bcf585,
    0xec1e4a7db69561a5, 0x2b31e9e3d06c32e6,
    0x9392ee8e921d5d07, 0x3aff322e62439fd0,
    0xb877aa3236a4b449, 0x9befeb9fad487c3,
    0xe69594bec44de15b, 0x4c2ebe687989a9b4,
    0x901d7cf73ab0acd9, 0xf9d37014bf60a11,
    0xb424dc35095cd80f, 0x538484c19ef38c95,
    0xe12e13424bb40e13, 0x2865a5f206b06fba,
    0x8cbccc096f5088cb, 0xf93f87b7442e45d4,
    0xafebff0bcb24aafe, 0xf78f69a51539d749,
    0xdbe6fecebdedd5be, 0xb573440e5a884d1c,
    0x89705f4136b4a597, 0x31680a88f8953031,
    0xabcc77118461cefc, 0xfdc20d2b36ba7c3e,
    0xd6bf94d5e57a42bc, 0x3d32907604691b4d,
    0x8637bd05af6c69b5, 0xa63f9a49c2c1b110,
    0xa7c5ac471b478423, 0xfcf80dc33721d54,
    0xd1b71758e219652b, 0xd3c36113404ea4a9,
    0x83126e978d4fdf3b, 0x645a1cac083126ea,
    0xa3d70a3d70a3d70a, 0x3d70a3d70a3d70a4,
    0xcccccccccccccccc, 0xcccccccccccccccd,
    0x8000000000000000, 0x0,
    0xa000000000000000, 0x0,
    0xc800000000000000, 0x0,
    0xfa00000000000000, 0x0,
    0x9c40000000000000, 0x0,
    0xc350000000000000, 0x0,
    0xf424000000000000, 0x0,
    0x9896800000000000, 0x0,
    0xbebc200000000000, 0x0,
    0xee6b280000000000, 0x0,
    0x9502f90000000000, 0x0,
    0xba43b74000000000, 0x0,
    0xe8d4a51000000000, 0x0,
    0x9184e72a00000000, 0x0,
    0xb5e620f480000000, 0x0,
    0xe35fa931a0000000, 0x0,
    0x8e1bc9bf04000000, 0x0,
    0xb1a2bc2ec5000000, 0x0,
    0xde0b6b3a76400000, 0x0,
    0x8ac7230489e80000, 0x0,
    0xad78ebc5ac620000, 0x0,
    0xd8d726b7177a8000, 0x0,
    0x878678326eac9000, 0x0,
    0xa968163f0a57b400, 0x0,
    0xd3c21bcecceda100, 0x0,
    0x84595161401484a0, 0x0,
    0xa56fa5b99019a5c8, 0x0,
    0xcecb8f27f4200f3a, 0x0,
    0x813f3978f8940984, 0x4000000000000000,
    0xa18f07d736b90be5, 0x5000000000000000,
    0xc9f2c9cd04674ede, 0xa400000000000000,
    0xfc6f7c4045812296, 0x4d00000000000000,
    0x9dc5ada82b70b59d, 0xf020000000000000,
    0xc5371912364ce305, 0x6c28000000000000,
    0xf684df56c3e01bc6, 0xc732000000000000,
    0x9a130b963a6c115c, 0x3c7f400000000000,
    0xc097ce7bc90715b3, 0x4b9f100000000000,
    0xf0bdc21abb48db20, 0x1e86d40000000000,
    0x96769950b50d88f4, 0x1314448000000000,
    0xbc143fa4e250eb31, 0x17d955a000000000,
    0xeb194f8e1ae525fd, 0x5dcfab0800000000,
    0x92efd1b8d0cf37be, 0x5aa1cae500000000,
    0xb7abc627050305ad, 0xf14a3d9e40000000,
    0xe596b7b0c643c719, 0x6d9ccd05d0000000,
    0x8f7e32ce7bea5c6f, 0xe4820023a2000000,
    0xb35dbf821ae4f38b, 0xdda2802c8a800000,
    0xe0352f62a19e306e, 0xd50b2037ad200000,
    0x8c213d9da502de45, 0x4526f422cc340000,
    0xaf298d050e4395d6, 0x9670b12b7f410000,
    0xdaf3f04651d47b4c, 0x3c0cdd765f114000,
    0x88d8762bf324cd0f, 0xa5880a69fb6ac800,
    0xab0e93b6efee0053, 0x8eea0d047a457a00,
    0xd5d238a4abe98068, 0x72a4904598d6d880,
    0x85a36366eb71f041, 0x47a6da2b7f864750,
    0xa70c3c40a64e6c51, 0x999090b65f67d924,
    0xd0cf4b50cfe20765, 0xfff4b4e3f741cf6d,
    0x82818f1281ed449f, 0xbff8f10e7a8921a4,
    0xa321f2d7226895c7, 0xaff72d52192b6a0d,
    0xcbea6f8ceb02bb39, 0x9bf4f8a69f764490,
    0xfee50b7025c36a08, 0x2f236d04753d5b4,
    0x9f4f2726179a2245, 0x1d762422c946590,
    0xc722f0ef9d80aad6, 0x424d3ad2b7b97ef5,
    0xf8ebad2b84e0d58b, 0xd2e0898765a7deb2,
    0x9b934c3b330c8577, 0x63cc55f49f88eb2f,
    0xc2781f49ffcfa6d5, 0x3cbf6b71c76b25fb,
    0xf316271c7fc3908a, 0x8bef464e3945ef7a,
    0x97edd871cfda3a56, 0x97758bf0e3cbb5ac,
    0xbde94e8e43d0c8ec, 0x3d52eeed1cbea317,
    0xed63a231d4c4fb27, 0x4ca7aaa863ee4bdd,
    0x945e455f24fb1cf8, 0x8fe8caa93e74ef6a,
    0xb975d6b6ee39e436, 0xb3e2fd538e122b44,
    0xe7d34c64a9c85d44, 0x60dbbca87196b616,
    0x90e40fbeea1d3a4a, 0xbc8955e946fe31cd,
    0xb51d13aea4a488dd, 0x6babab6398bdbe41,
    0xe264589a4dcdab14, 0xc696963c7eed2dd1,
    0x8d7eb76070a08aec, 0xfc1e1de5cf543ca2,
    0xb0de65388cc8ada8, 0x3b25a55f43294bcb,
    0xdd15fe86affad912, 0x49ef0eb713f39ebe,
    0x8a2dbf142dfcc7ab, 0x6e3569326c784337,
    0xacb92ed9397bf996, 0x49c2c37f07965404,
    0xd7e77a8f87daf7fb, 0xdc33745ec97be906,
    0x86f0ac99b4e8dafd, 0x69a028bb3ded71a3,
    0xa8acd7c0222311bc, 0xc40832ea0d68ce0c,
    0xd2d80db02aabd62b, 0xf50a3fa490c30190,
    0x83c7088e1aab65db, 0x792667c6da79e0fa,
    0xa4b8cab1a1563f52, 0x577001b891185938,
    0xcde6fd5e09abcf26, 0xed4c0226b55e6f86,
    0x80b05e5ac60b6178, 0x544f8158315b05b4,
    0xa0dc75f1778e39d6, 0x696361ae3db1c721,
    0xc913936dd571c84c, 0x3bc3a19cd1e38e9,
    0xfb5878494ace3a5f, 0x4ab48a04065c723,
    0x9d174b2dcec0e47b, 0x62eb0d64283f9c76,
    0xc45d1df942711d9a, 0x3ba5d0bd324f8394,
    0xf5746577930d6500, 0xca8f44ec7ee36479,
    0x9968bf6abbe85f20, 0x7e998b13cf4e1ecb,
    0xbfc2ef456ae276e8, 0x9e3fedd8c321a67e,
    0xefb3ab16c59b14a2, 0xc5cfe94ef3ea101e,
    0x95d04aee3b80ece5, 0xbba1f1d158724a12,
    0xbb445da9ca61281f, 0x2a8a6e45ae8edc97,
    0xea1575143cf97226, 0xf52d09d71a3293bd,
    0x924d692ca61be758, 0x593c2626705f9c56,
    0xb6e0c377cfa2e12e, 0x6f8b2fb00c77836c,
    0xe498f455c38b997a, 0xb6dfb9c0f956447,
    0x8edf98b59a373fec, 0x4724bd4189bd5eac,
    0xb2977ee300c50fe7, 0x58edec91ec2cb657,
    0xdf3d5e9bc0f653e1, 0x2f2967b66737e3ed,
    0x8b865b215899f46c, 0xbd79e0d20082ee74,
    0xae67f1e9aec07187, 0xecd8590680a3aa11,
    0xda01ee641a708de9, 0xe80e6f4820cc9495,
    0x884134fe908658b2, 0x3109058d147fdcdd,
    0xaa51823e34a7eede, 0xbd4b46f0599fd415,
    0xd4e5e2cdc1d1ea96, 0x6c9e18ac7007c91a,
    0x850fadc09923329e, 0x3e2cf6bc604ddb0,
    0xa6539930bf6bff45, 0x84db8346b786151c,
    0xcfe87f7cef46ff16, 0xe612641865679a63,
    0x81f14fae158c5f6e, 0x4fcb7e8f3f60c07e,
    0xa26da3999aef7749, 0xe3be5e330f38f09d,
    0xcb090c8001ab551c, 0x5cadf5bfd3072cc5,
    0xfdcb4fa002162a63, 0x73d9732fc7c8f7f6,
    0x9e9f11c4014dda7e, 0x2867e7fddcdd9afa,
    0xc646d63501a1511d, 0xb281e1fd541501b8,
    0xf7d88bc24209a565, 0x1f225a7ca91a4226,
    0x9ae757596946075f, 0x3375788de9b06958,
    0xc1a12d2fc3978937, 0x52d6b1641c83ae,
    0xf209787bb47d6b84, 0xc0678c5dbd23a49a,
    0x9745eb4d50ce6332, 0xf840b7ba963646e0,
    0xbd176620a501fbff, 0xb650e5a93bc3d898,
    0xec5d3fa8ce427aff, 0xa3e51f138ab4cebe,
    0x93ba47c980e98cdf, 0xc66f336c36b10137,
    0xb8a8d9bbe123f017, 0xb80b0047445d4184,
    0xe6d3102ad96cec1d, 0xa60dc059157491e5,
    0x9043ea1ac7e41392, 0x87c89837ad68db2f,
    0xb454e4a179dd1877, 0x29babe4598c311fb,
    0xe16a1dc9d8545e94, 0xf4296dd6fef3d67a,
    0x8ce2529e2734bb1d, 0x1899e4a65f58660c,
    0xb01ae745b101e9e4, 0x5ec05dcff72e7f8f,
    0xdc21a1171d42645d, 0x76707543f4fa1f73,
    0x899504ae72497eba, 0x6a06494a791c53a8,
    0xabfa45da0edbde69, 0x487db9d17636892,
    0xd6f8d7509292d603, 0x45a9d2845d3c42b6,
    0x865b86925b9bc5c2, 0xb8a2392ba45a9b2,
    0xa7f26836f282b732, 0x8e6cac7768d7141e,
    0xd1ef0244af2364ff, 0x3207d795430cd926,
    0x8335616aed761f1f, 0x7f44e6bd49e807b8,
    0xa402b9c5a8d3a6e7, 0x5f16206c9c6209a6,
    0xcd036837130890a1, 0x36dba887c37a8c0f,
    0x802221226be55a64, 0xc2494954da2c9789,
    0xa02aa96b06deb0fd, 0xf2db9baa10b7bd6c,
    0xc83553c5c8965d3d, 0x6f92829494e5acc7,
    0xfa42a8b73abbf48c, 0xcb772339ba1f17f9,
    0x9c69a97284b578d7, 0xff2a760414536efb,
    0xc38413cf25e2d70d, 0xfef5138519684aba,
    0xf46518c2ef5b8cd1, 0x7eb258665fc25d69,
    0x98bf2f79d5993802, 0xef2f773ffbd97a61,
    0xbeeefb584aff8603, 0xaafb550ffacfd8fa,
    0xeeaaba2e5dbf6784, 0x95ba2a53f983cf38,
    0x952ab45cfa97a0b2, 0xdd945a747bf26183,
    0xba756174393d88df, 0x94f971119aeef9e4,
    0xe912b9d1478ceb17, 0x7a37cd5601aab85d,
    0x91abb422ccb812ee, 0xac62e055c10ab33a,
    0xb616a12b7fe617aa, 0x577b986b314d6009,
    0xe39c49765fdf9d94, 0xed5a7e85fda0b80b,
    0x8e41ade9fbebc27d, 0x14588f13be847307,
    0xb1d219647ae6b31c, 0x596eb2d8ae258fc8,
    0xde469fbd99a05fe3, 0x6fca5f8ed9aef3bb,
    0x8aec23d680043bee, 0x25de7bb9480d5854,
    0xada72ccc20054ae9, 0xaf561aa79a10ae6a,
    0xd910f7ff28069da4, 0x1b2ba1518094da04,
    0x87aa9aff79042286, 0x90fb44d2f05d0842,
    0xa99541bf57452b28, 0x353a1607ac744a53,
    0xd3fa922f2d1675f2, 0x42889b8997915ce8,
    0x847c9b5d7c2e09b7, 0x69956135febada11,
    0xa59bc234db398c25, 0x43fab9837e699095,
    0xcf02b2c21207ef2e, 0x94f967e45e03f4bb,
    0x8161afb94b44f57d, 0x1d1be0eebac278f5,
    0xa1ba1ba79e1632dc, 0x6462d92a69731732,
    0xca28a291859bbf93, 0x7d7b8f7503cfdcfe,
    0xfcb2cb35e702af78, 0x5cda735244c3d43e,
    0x9defbf01b061adab, 0x3a0888136afa64a7,
    0xc56baec21c7a1916, 0x88aaa1845b8fdd0,
    0xf6c69a72a3989f5b, 0x8aad549e57273d45,
    0x9a3c2087a63f6399, 0x36ac54e2f678864b,
    0xc0cb28a98fcf3c7f, 0x84576a1bb416a7dd,
    0xf0fdf2d3f3c30b9f, 0x656d44a2a11c51d5,
    0x969eb7c47859e743, 0x9f644ae5a4b1b325,
    0xbc4665b596706114, 0x873d5d9f0dde1fee,
    0xeb57ff22fc0c7959, 0xa90cb506d155a7ea,
    0x9316ff75dd87cbd8, 0x9a7f12442d588f2,
    0xb7dcbf5354e9bece, 0xc11ed6d538aeb2f,
    0xe5d3ef282a242e81, 0x8f1668c8a86da5fa,
    0x8fa475791a569d10, 0xf96e017d694487bc,
    0xb38d92d760ec4455, 0x37c981dcc395a9ac,
    0xe070f78d3927556a, 0x85bbe253f47b1417,
    0x8c469ab843b89562, 0x93956d7478ccec8e,
    0xaf58416654a6babb, 0x387ac8d1970027b2,
    0xdb2e51bfe9d0696a, 0x6997b05fcc0319e,
    0x88fcf317f22241e2, 0x441fece3bdf81f03,
    0xab3c2fddeeaad25a, 0xd527e81cad7626c3,
    0xd60b3bd56a5586f1, 0x8a71e223d8d3b074,
    0x85c7056562757456, 0xf6872d5667844e49,
    0xa738c6bebb12d16c, 0xb428f8ac016561db,
    0xd106f86e69d785c7, 0xe13336d701beba52,
    0x82a45b450226b39c, 0xecc0024661173473,
    0xa34d721642b06084, 0x27f002d7f95d0190,
    0xcc20ce9bd35c78a5, 0x31ec038df7b441f4,
    0xff290242c83396ce, 0x7e67047175a15271,
    0x9f79a169bd203e41, 0xf0062c6e984d386,
    0xc75809c42c684dd1, 0x52c07b78a3e60868,
    0xf92e0c3537826145, 0xa7709a56ccdf8a82,
    0x9bbcc7a142b17ccb, 0x88a66076400bb691,
    0xc2abf989935ddbfe, 0x6acff893d00ea435,
    0xf356f7ebf83552fe, 0x583f6b8c4124d43,
    0x98165af37b2153de, 0xc3727a337a8b704a,
    0xbe1bf1b059e9a8d6, 0x744f18c0592e4c5c,
    0xeda2ee1c7064130c, 0x1162def06f79df73,
    0x9485d4d1c63e8be7, 0x8addcb5645ac2ba8,
    0xb9a74a0637ce2ee1, 0x6d953e2bd7173692,
    0xe8111c87c5c1ba99, 0xc8fa8db6ccdd0437,
    0x910ab1d4db9914a0, 0x1d9c9892400a22a2,
    0xb54d5e4a127f59c8, 0x2503beb6d00cab4b,
    0xe2a0b5dc971f303a, 0x2e44ae64840fd61d,
    0x8da471a9de737e24, 0x5ceaecfed289e5d2,
    0xb10d8e1456105dad, 0x7425a83e872c5f47,
    0xdd50f1996b947518, 0xd12f124e28f77719,
    0x8a5296ffe33cc92f, 0x82bd6b70d99aaa6f,
    0xace73cbfdc0bfb7b, 0x636cc64d1001550b,
    0xd8210befd30efa5a, 0x3c47f7e05401aa4e,
    0x8714a775e3e95c78, 0x65acfaec34810a71,
    0xa8d9d1535ce3b396, 0x7f1839a741a14d0d,
    0xd31045a8341ca07c, 0x1ede48111209a050,
    0x83ea2b892091e44d, 0x934aed0aab460432,
    0xa4e4b66b68b65d60, 0xf81da84d5617853f,
    0xce1de40642e3f4b9, 0x36251260ab9d668e,
    0x80d2ae83e9ce78f3, 0xc1d72b7c6b426019,
    0xa1075a24e4421730, 0xb24cf65b8612f81f,
    0xc94930ae1d529cfc, 0xdee033f26797b627,
    0xfb9b7cd9a4a7443c, 0x169840ef017da3b1,
    0x9d412e0806e88aa5, 0x8e1f289560ee864e,
    0xc491798a08a2ad4e, 0xf1a6f2bab92a27e2,
    0xf5b5d7ec8acb58a2, 0xae10af696774b1db,
    0x9991a6f3d6bf1765, 0xacca6da1e0a8ef29,
    0xbff610b0cc6edd3f, 0x17fd090a58d32af3,
    0xeff394dcff8a948e, 0xddfc4b4cef07f5b0,
    0x95f83d0a1fb69cd9, 0x4abdaf101564f98e,
    0xbb764c4ca7a4440f, 0x9d6d1ad41abe37f1,
    0xea53df5fd18d5513, 0x84c86189216dc5ed,
    0x92746b9be2f8552c, 0x32fd3cf5b4e49bb4,
    0xb7118682dbb66a77, 0x3fbc8c33221dc2a1,
    0xe4d5e82392a40515, 0xfabaf3feaa5334a,
    0x8f05b1163ba6832d, 0x29cb4d87f2a7400e,
    0xb2c71d5bca9023f8, 0x743e20e9ef511012,
    0xdf78e4b2bd342cf6, 0x914da9246b255416,
    0x8bab8eefb6409c1a, 0x1ad089b6c2f7548e,
    0xae9672aba3d0c320, 0xa184ac2473b529b1,
    0xda3c0f568cc4f3e8, 0xc9e5d72d90a2741e,
    0x8865899617fb1871, 0x7e2fa67c7a658892,
    0xaa7eebfb9df9de8d, 0xddbb901b98feeab7,
    0xd51ea6fa85785631, 0x552a74227f3ea565,
    0x8533285c936b35de, 0xd53a88958f87275f,
    0xa67ff273b8460356, 0x8a892abaf368f137,
    0xd01fef10a657842c, 0x2d2b7569b0432d85,
    0x8213f56a67f6b29b, 0x9c3b29620e29fc73,
    0xa298f2c501f45f42, 0x8349f3ba91b47b8f,
    0xcb3f2f7642717713, 0x241c70a936219a73,
    0xfe0efb53d30dd4d7, 0xed238cd383aa0110,
    0x9ec95d1463e8a506, 0xf4363804324a40aa,
    0xc67bb4597ce2ce48, 0xb143c6053edcd0d5,
    0xf81aa16fdc1b81da, 0xdd94b7868e94050a,
    0x9b10a4e5e9913128, 0xca7cf2b4191c8326,
    0xc1d4ce1f63f57d72, 0xfd1c2f611f63a3f0,
    0xf24a01a73cf2dccf, 0xbc633b39673c8cec,
    0x976e41088617ca01, 0xd5be0503e085d813,
    0xbd49d14aa79dbc82, 0x4b2d8644d8a74e18,
    0xec9c459d51852ba2, 0xddf8e7d60ed1219e,
    0x93e1ab8252f33b45, 0xcabb90e5c942b503,
    0xb8da1662e7b00a17, 0x3d6a751f3b936243,
    0xe7109bfba19c0c9d, 0xcc512670a783ad4,
    0x906a617d450187e2, 0x27fb2b80668b24c5,
    0xb484f9dc9641e9da, 0xb1f9f660802dedf6,
    0xe1a63853bbd26451, 0x5e7873f8a0396973,
    0x8d07e33455637eb2, 0xdb0b487b6423e1e8,
    0xb049dc016abc5e5f, 0x91ce1a9a3d2cda62,
    0xdc5c5301c56b75f7, 0x7641a140cc7810fb,
    0x89b9b3e11b6329ba, 0xa9e904c87fcb0a9d,
    0xac2820d9623bf429, 0x546345fa9fbdcd44,
    0xd732290fbacaf133, 0xa97c177947ad4095,
    0x867f59a9d4bed6c0, 0x49ed8eabcccc485d,
    0xa81f301449ee8c70, 0x5c68f256bfff5a74,
    0xd226fc195c6a2f8c, 0x73832eec6fff3111,
    0x83585d8fd9c25db7, 0xc831fd53c5ff7eab,
    0xa42e74f3d032f525, 0xba3e7ca8b77f5e55,
    0xcd3a1230c43fb26f, 0x28ce1bd2e55f35eb,
    0x80444b5e7aa7cf85, 0x7980d163cf5b81b3,
    0xa0555e361951c366, 0xd7e105bcc332621f,
    0xc86ab5c39fa63440, 0x8dd9472bf3fefaa7,
    0xfa856334878fc150, 0xb14f98f6f0feb951,
    0x9c935e00d4b9d8d2, 0x6ed1bf9a569f33d3,
    0xc3b8358109e84f07, 0xa862f80ec4700c8,
    0xf4a642e14c6262c8, 0xcd27bb612758c0fa,
    0x98e7e9cccfbd7dbd, 0x8038d51cb897789c,
    0xbf21e44003acdd2c, 0xe0470a63e6bd56c3,
    0xeeea5d5004981478, 0x1858ccfce06cac74,
    0x95527a5202df0ccb, 0xf37801e0c43ebc8,
    0xbaa718e68396cffd, 0xd30560258f54e6ba,
    0xe950df20247c83fd, 0x47c6b82ef32a2069,
    0x91d28b7416cdd27e, 0x4cdc331d57fa5441,
    0xb6472e511c81471d, 0xe0133fe4adf8e952,
    0xe3d8f9e563a198e5, 0x58180fddd97723a6,
    0x8e679c2f5e44ff8f, 0x570f09eaa7ea7648,
});
}  // namespace powers

template <typename T>
struct BinaryFormat;

template <>
struct BinaryFormat<double> {
  using EquivUint = std::uint64_t;
  static constexpr auto mantissa_explicit_bits() noexcept -> int { return 52; }
  static constexpr auto minimum_exponent() noexcept -> int { return -1023; }
  static constexpr auto infinite_power() noexcept -> int { return 0x7ff; }
  static constexpr auto sign_index() noexcept -> int { return 63; }
  static constexpr auto smallest_power_of_ten() noexcept -> int { return -342; }
  static constexpr auto largest_power_of_ten() noexcept -> int { return 308; }
  static constexpr auto min_exponent_round_to_even() noexcept -> int { return -4; }
  static constexpr auto max_exponent_round_to_even() noexcept -> int { return 23; }
};

template <>
struct BinaryFormat<float> {
  using EquivUint = std::uint32_t;
  static constexpr auto mantissa_explicit_bits() noexcept -> int { return 23; }
  static constexpr auto minimum_exponent() noexcept -> int { return -127; }
  static constexpr auto infinite_power() noexcept -> int { return 0xff; }
  static constexpr auto sign_index() noexcept -> int { return 31; }
  static constexpr auto smallest_power_of_ten() noexcept -> int { return -64; }
  static constexpr auto largest_power_of_ten() noexcept -> int { return 38; }
  static constexpr auto min_exponent_round_to_even() noexcept -> int { return -17; }
  static constexpr auto max_exponent_round_to_even() noexcept -> int { return 10; }
};

inline auto power(std::int32_t q) noexcept -> std::int32_t {
  return (((152170 + 65536) * q) >> 16) + 63;
}

template <int bit_precision>
inline auto compute_product_approximation(std::int64_t q, std::uint64_t w) noexcept -> Value128 {
  int const index = 2 * static_cast<int>(q - powers::smallest_power_of_five);
  Value128 product = full_multiplication(w, powers::power_of_five_128[index]);
  constexpr std::uint64_t precision_mask = (bit_precision < 64)
                                               ? (UINT64_C(0xffffffffffffffff) >> bit_precision)
                                               : UINT64_C(0xffffffffffffffff);
  if ((product.high & precision_mask) == precision_mask) {
    Value128 second = full_multiplication(w, powers::power_of_five_128[index + 1]);
    product.low += second.high;
    if (second.high > product.low) ++product.high;
  }
  return product;
}

template <typename Binary>
inline auto compute_float(std::int64_t q, std::uint64_t w) noexcept -> AdjustedMantissa {
  AdjustedMantissa answer{.mantissa = 0, .power_2 = 0};
  if ((w == 0) || (q < Binary::smallest_power_of_ten())) return answer;
  if (q > Binary::largest_power_of_ten()) {
    return {.mantissa = 0, .power_2 = Binary::infinite_power()};
  }
  int lz = leading_zeroes(w);
  w <<= lz;
  Value128 product = compute_product_approximation<Binary::mantissa_explicit_bits() + 3>(q, w);
  int upperbit = static_cast<int>(product.high >> 63);
  int shift = upperbit + 64 - Binary::mantissa_explicit_bits() - 3;
  answer.mantissa = product.high >> shift;
  answer.power_2 = static_cast<std::int32_t>(power(static_cast<std::int32_t>(q)) + upperbit - lz -
                                             Binary::minimum_exponent());
  if (answer.power_2 <= 0) {
    if (-answer.power_2 + 1 >= 64) return {.mantissa = 0, .power_2 = 0};
    answer.mantissa >>= -answer.power_2 + 1;
    answer.mantissa += (answer.mantissa & 1);
    answer.mantissa >>= 1;
    answer.power_2 = (answer.mantissa < (UINT64_C(1) << Binary::mantissa_explicit_bits())) ? 0 : 1;
    return answer;
  }
  if ((product.low <= 1) && (q >= Binary::min_exponent_round_to_even()) &&
      (q <= Binary::max_exponent_round_to_even()) && ((answer.mantissa & 3) == 1)) {
    if ((answer.mantissa << shift) == product.high) answer.mantissa &= ~UINT64_C(1);
  }
  answer.mantissa += (answer.mantissa & 1);
  answer.mantissa >>= 1;
  if (answer.mantissa >= (UINT64_C(2) << Binary::mantissa_explicit_bits())) {
    answer.mantissa = (UINT64_C(1) << Binary::mantissa_explicit_bits());
    ++answer.power_2;
  }
  answer.mantissa &= ~(UINT64_C(1) << Binary::mantissa_explicit_bits());
  if (answer.power_2 >= Binary::infinite_power()) {
    return {.mantissa = 0, .power_2 = Binary::infinite_power()};
  }
  return answer;
}

template <typename T>
inline auto to_float(bool negative, AdjustedMantissa am, T &value) noexcept -> void {
  using Binary = BinaryFormat<T>;
  using EquivUint = typename Binary::EquivUint;
  auto word = static_cast<EquivUint>(am.mantissa);
  word = static_cast<EquivUint>(
      word | (static_cast<EquivUint>(am.power_2) << Binary::mantissa_explicit_bits()));
  word = static_cast<EquivUint>(word | (static_cast<EquivUint>(negative) << Binary::sign_index()));
  std::memcpy(&value, &word, sizeof(T));
}

}  // namespace cplib::detail::float_parse::core

namespace cplib::detail::float_parse {

enum class Mode : std::uint8_t { General, Fixed };

namespace impl {

inline auto is_digit(char c) noexcept -> bool { return static_cast<unsigned>(c - '0') < 10; }
inline auto lower(char c) noexcept -> char { return static_cast<char>(c | 0x20); }

inline auto read_u64(const char *p) noexcept -> std::uint64_t {
  std::uint64_t v;
  std::memcpy(&v, p, sizeof(v));
  return v;
}

inline auto eight_digits(std::uint64_t v) noexcept -> bool {
  return !((((v + 0x4646464646464646ull) | (v - 0x3030303030303030ull)) & 0x8080808080808080ull));
}

inline auto parse_eight(std::uint64_t v) noexcept -> std::uint32_t {
  v -= 0x3030303030303030ull;
  v = (v * 10) + (v >> 8);
  v = (((v & 0x000000ff000000ffull) * 0x000f424000000064ull) +
       (((v >> 16) & 0x000000ff000000ffull) * 0x0000271000000001ull)) >>
      32;
  return static_cast<std::uint32_t>(v);
}

inline auto validate_decimal(const char *first, const char *last, Mode mode) noexcept -> bool {
  if (first == last) return false;
  const char *p = first;
  if (*p == '-') {
    if (++p == last) return false;
  } else if (*p == '+') {
    return false;
  }
  if (!is_digit(*p) && *p != '.') return false;
  bool has_digit = false;
  while (p != last && is_digit(*p)) {
    has_digit = true;
    ++p;
  }
  if (p != last && *p == '.') {
    ++p;
    while (p != last && is_digit(*p)) {
      has_digit = true;
      ++p;
    }
  }
  if (!has_digit) return false;
  if (p != last && (*p == 'e' || *p == 'E')) {
    if (mode != Mode::General) return false;
    ++p;
    if (p != last && (*p == '-' || *p == '+')) {
      ++p;
    }
    if (p == last || !is_digit(*p)) return false;
    while (p != last && is_digit(*p)) {
      ++p;
    }
  }
  return p == last;
}

inline auto ieq(const char *p, const char *s, int n) noexcept -> bool {
  for (int i = 0; i < n; ++i) {
    if (lower(p[i]) != s[i]) return false;
  }
  return true;
}

template <typename T>
inline auto parse_infnan(const char *first, const char *last, bool neg, T &value) noexcept -> bool {
  const char *p = first + (neg ? 1 : 0);
  const std::ptrdiff_t n = last - p;
  if (n == 3 && ieq(p, "inf", 3)) {
    value = neg ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity();
    return true;
  }
  if (n == 8 && ieq(p, "infinity", 8)) {
    value = neg ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::infinity();
    return true;
  }
  if (n >= 3 && ieq(p, "nan", 3)) {
    if (n == 3) {
      value = neg ? -std::numeric_limits<T>::quiet_NaN() : std::numeric_limits<T>::quiet_NaN();
      return true;
    }
    if (p[3] != '(' || last[-1] != ')') return false;
    for (const char *q = p + 4; q + 1 < last; ++q) {
      const char c = *q;
      const bool ok = is_digit(c) || (lower(c) >= 'a' && lower(c) <= 'z') || c == '_';
      if (!ok) return false;
    }
    value = neg ? -std::numeric_limits<T>::quiet_NaN() : std::numeric_limits<T>::quiet_NaN();
    return true;
  }
  return false;
}

inline auto parse_long_double_decimal(const char *first, const char *last, Mode mode,
                                      long double &value) -> bool {
  bool neg = first != last && *first == '-';
  const char *body = first + (neg ? 1 : 0);
  if (body != last && (!is_digit(*body) && *body != '.')) {
    return parse_infnan(first, last, neg, value);
  }
  if (!validate_decimal(first, last, mode)) return false;
  std::string token(first, last);
  char *end = nullptr;
  errno = 0;
  value = std::strtold(token.c_str(), &end);
  return end == token.c_str() + token.size() && errno != ERANGE;
}

template <typename T>
struct Format;
template <>
struct Format<float> {
  static constexpr int min_fast_exp = -10;
  static constexpr int max_fast_exp = 10;
  static constexpr std::uint64_t max_fast_mantissa = UINT64_C(1) << 24;
};
template <>
struct Format<double> {
  static constexpr int min_fast_exp = -22;
  static constexpr int max_fast_exp = 22;
  static constexpr std::uint64_t max_fast_mantissa = UINT64_C(1) << 53;
};

template <typename T>
inline auto clinger(std::uint64_t mantissa, int exp10, bool neg, T &value) noexcept -> bool {
  if (exp10 < Format<T>::min_fast_exp || exp10 > Format<T>::max_fast_exp ||
      mantissa > Format<T>::max_fast_mantissa) {
    return false;
  }
  static constexpr auto pow10 =
      std::to_array<T>({T(1e0),  T(1e1),  T(1e2),  T(1e3),  T(1e4),  T(1e5),  T(1e6),  T(1e7),
                        T(1e8),  T(1e9),  T(1e10), T(1e11), T(1e12), T(1e13), T(1e14), T(1e15),
                        T(1e16), T(1e17), T(1e18), T(1e19), T(1e20), T(1e21), T(1e22)});
  T x = static_cast<T>(mantissa);
  if (exp10 < 0) {
    x /= pow10[-exp10];
  } else {
    x *= pow10[exp10];
  }
  value = neg ? -x : x;
  return true;
}

template <typename T>
inline auto extended_clinger(std::uint64_t mantissa, int exp10, bool neg, T &value) noexcept
    -> bool {
  if (exp10 < -30 || exp10 > 30) return false;
  static constexpr auto pow10 = std::to_array<double>({
      1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10,
      1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21,
      1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30,
  });
  auto x = static_cast<double>(mantissa);
  if (exp10 < 0) {
    x /= pow10[-exp10];
  } else {
    x *= pow10[exp10];
  }
  value = neg ? -static_cast<T>(x) : static_cast<T>(x);
  return value == value && value != std::numeric_limits<T>::infinity() &&
         value != -std::numeric_limits<T>::infinity();
}

template <typename T>
inline auto core_scale(std::uint64_t mantissa, int exp10, bool neg, T &value) noexcept -> bool {
  auto am = core::compute_float<core::BinaryFormat<T>>(exp10, mantissa);
  if (am.power_2 < 0) return false;
  core::to_float(neg, am, value);
  if ((mantissa != 0 && am.mantissa == 0 && am.power_2 == 0) ||
      am.power_2 == core::BinaryFormat<T>::infinite_power()) {
    return false;
  }
  return true;
}

inline auto scan_digits(const char *&q, const char *last, int &ndigits, int &first_sig,
                        std::uint64_t &mantissa, int &kept) noexcept -> void {
  while (last - q >= 8) {
    const std::uint64_t raw = read_u64(q);
    if (!eight_digits(raw)) {
      break;
    }
    const std::uint32_t chunk = parse_eight(raw);
    if (first_sig < 0) {
      if (chunk == 0) {
        q += 8;
        ndigits += 8;
        continue;
      }
      const char *r = q;
      while (*r == '0') {
        ++r;
      }
      while (q != r) {
        ++q;
        ++ndigits;
      }
      first_sig = ndigits;
      continue;
    }
    if (kept <= 11) {
      mantissa = mantissa * 100000000ull + chunk;
      kept += 8;
    }
    q += 8;
    ndigits += 8;
  }
  while (q != last && is_digit(*q)) {
    if (*q != '0' && first_sig < 0) {
      first_sig = ndigits;
    }
    if (first_sig >= 0 && kept < 19) {
      mantissa = mantissa * 10 + static_cast<unsigned>(*q - '0');
      ++kept;
    }
    ++ndigits;
    ++q;
  }
}

}  // namespace impl

template <typename T>
inline auto parse(const char *first, const char *last, Mode mode, T &value) noexcept -> bool {
  static_assert(
      std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, long double>,
      "float/double/long double only");
  if constexpr (std::is_same_v<T, long double>) {
    if constexpr (std::numeric_limits<long double>::digits == std::numeric_limits<double>::digits &&
                  std::numeric_limits<long double>::max_exponent ==
                      std::numeric_limits<double>::max_exponent) {
      double tmp{};
      if (!parse(first, last, mode, tmp)) {
        return false;
      }
      value = static_cast<long double>(tmp);
      return true;
    }
    return impl::parse_long_double_decimal(first, last, mode, value);
  } else {
    if (first == last) {
      return false;
    }
    bool neg = false;
    const char *p = first;
    if (*p == '-') {
      neg = true;
      if (++p == last) {
        return false;
      }
    } else if (*p == '+') {
      return false;
    }
    if (!impl::is_digit(*p) && *p != '.') {
      return impl::parse_infnan(first, last, neg, value);
    }

    int ndigits = 0;
    int first_sig = -1;
    int decimal_pos = 0;
    std::uint64_t mantissa = 0;
    int kept = 0;
    const char *q = p;
    impl::scan_digits(q, last, ndigits, first_sig, mantissa, kept);
    decimal_pos = ndigits;
    if (q != last && *q == '.') {
      ++q;
      impl::scan_digits(q, last, ndigits, first_sig, mantissa, kept);
    }
    if (ndigits == 0) {
      return false;
    }
    if (mode == Mode::Fixed && q != last) {
      return false;
    }

    int explicit_exp = 0;
    if (q != last && (*q == 'e' || *q == 'E')) {
      if (mode != Mode::General) return false;
      ++q;
      bool exp_neg = false;
      if (q != last && (*q == '-' || *q == '+')) {
        exp_neg = *q++ == '-';
      }
      if (q == last || !impl::is_digit(*q)) {
        return false;
      }
      while (q != last && impl::is_digit(*q)) {
        if (explicit_exp < 100000) {
          explicit_exp = explicit_exp * 10 + (*q - '0');
        }
        ++q;
      }
      if (exp_neg) {
        explicit_exp = -explicit_exp;
      }
    }
    if (q != last) {
      return false;
    }
    if (first_sig < 0) {
      value = neg ? T(-0.0) : T(0.0);
      return true;
    }
    if (first_sig + kept < ndigits) {
      return false;
    }
    const int exp10 = explicit_exp + decimal_pos - (first_sig + kept);
    return impl::clinger(mantissa, exp10, neg, value) ||
           impl::extended_clinger(mantissa, exp10, neg, value) ||
           impl::core_scale(mantissa, exp10, neg, value);
  }
}

}  // namespace cplib::detail::float_parse

#endif  // CPLIB_FLOAT_PARSE_I_HPP_
// --- End embedded: float_parse.i.hpp ---


namespace cplib::var {

inline ReaderTrace::ReaderTrace(std::string var_name, io::Position pos)
    : var_name(std::move(var_name)), pos(pos) {}

[[nodiscard]] inline auto ReaderTrace::node_name() const -> std::string { return var_name; }

[[nodiscard]] inline auto ReaderTrace::to_plain_text() const -> std::string {
  return cplib::format("{} @ line {}, col {}, byte {}", var_name, pos.line + 1, pos.col + 1,
                       pos.byte + 1);
}

[[nodiscard]] inline auto ReaderTrace::to_colored_text() const -> std::string {
  return cplib::format(
      "\x1b[0;33m{}\x1b[0m @ line \x1b[0;33m{}\x1b[0m, col \x1b[0;33m{}\x1b[0m, byte "
      "\x1b[0;33m{}\x1b[0m",
      var_name, pos.line + 1, pos.col + 1, pos.byte + 1);
}

[[nodiscard]] inline auto ReaderTrace::to_stack_json() const -> json::Value {
  return {json::Map{
      {"var_name", json::Value(var_name)},
      {"pos", json::Value(pos.to_json())},
  }};
}

[[nodiscard]] inline auto ReaderTrace::to_tree_json() const -> json::Value {
  return {json::Map{
      {"n", json::Value(var_name)},
      {"b", json::Value(static_cast<json::Int>(pos.byte))},
      {"l", json::Value(static_cast<json::Int>(byte_length))},
  }};
}

inline Reader::Reader(std::unique_ptr<io::InStream> inner, trace::Level trace_level,
                      FailFunc fail_func)
    : trace::Traced<ReaderTrace>(
          static_cast<trace::Level>(
              std::min(static_cast<int>(trace_level), CPLIB_READER_TRACE_LEVEL_MAX)),
          ReaderTrace(cplib::format("<{}>", inner ? inner->name() : "dummy"), io::Position())),
      inner_(std::move(inner)),
      fail_func_(std::move(fail_func)) {}

inline auto Reader::inner() -> io::InStream & { return *inner_; }

inline auto Reader::inner() const -> const io::InStream & { return *inner_; }

[[noreturn]] inline auto Reader::fail(std::string_view message) -> void {
  fail_func_(*this, message);
  std::exit(EXIT_FAILURE);
}

template <class T, class D>
inline auto Reader::read(const Var<T, D> &v) -> T {
  auto trace_level = get_trace_level();

  if (trace_level >= trace::Level::STACK_ONLY) {
    ReaderTrace trace{std::string(v.name()), inner().pos()};
    push_trace(std::move(trace));
  }

  auto result = v.read_from(*this);

  if (trace_level >= trace::Level::STACK_ONLY) {
    auto trace = get_current_trace();
    trace.byte_length = inner().pos().byte - trace.pos.byte;
    set_current_trace(std::move(trace));
    pop_trace();
  }
  return result;
}

template <class... T>
inline auto Reader::operator()(T... vars) -> std::tuple<typename T::Var::Target...> {
  return {read(vars)...};
}

namespace detail {
// Open the given file and create a `var::Reader`
inline auto make_reader_by_path(std::string_view path, std::string name, bool strict,
                                trace::Level trace_level, Reader::FailFunc fail_func)
    -> var::Reader {
  auto buf = std::make_unique<io::InBuf>(path);
  return var::Reader(std::make_unique<io::InStream>(std::move(buf), std::move(name), strict),
                     trace_level, std::move(fail_func));
}

// Use file with givin fileno as input stream and create a `var::Reader`
inline auto make_reader_by_fileno(int fileno, std::string name, bool strict,
                                  trace::Level trace_level, Reader::FailFunc fail_func)
    -> var::Reader {
  auto buf = std::make_unique<io::InBuf>(fileno);
  var::Reader reader(std::make_unique<io::InStream>(std::move(buf), std::move(name), strict),
                     trace_level, std::move(fail_func));
  return reader;
}
}  // namespace detail

namespace detail {
inline constexpr std::string_view VAR_DEFAULT_NAME("<unnamed>");
}

template <class T, class D>
inline Var<T, D>::~Var() = default;

template <class T, class D>
inline auto Var<T, D>::name() const -> std::string_view {
  return name_;
}

template <class T, class D>
inline auto Var<T, D>::clone() const -> D {
  D clone = *static_cast<const D *>(this);
  return clone;
}

template <class T, class D>
inline auto Var<T, D>::renamed(std::string_view name) const -> D {
  D clone = *static_cast<const D *>(this);
  clone.name_ = name;
  return clone;
}

template <class T, class D>
inline auto Var<T, D>::renamed(std::size_t index) const -> D {
  D clone = *static_cast<const D *>(this);
  clone.name_.resize(20);
  auto [ptr, ec] =
      std::to_chars(clone.name_.data(), clone.name_.data() + clone.name_.size(), index);
  if (ec != std::errc()) {
    panic("Var index rename failed");
  }
  clone.name_.resize(static_cast<std::size_t>(ptr - clone.name_.data()));
  return clone;
}

template <class T, class D>
inline auto Var<T, D>::renamed(std::size_t index0, std::size_t index1) const -> D {
  D clone = *static_cast<const D *>(this);
  clone.name_.resize(41);
  auto begin = clone.name_.data();
  auto [mid, ec0] = std::to_chars(begin, begin + 20, index0);
  if (ec0 != std::errc()) {
    panic("Var matrix index rename failed");
  }
  *mid++ = '_';
  auto [end, ec1] = std::to_chars(mid, begin + clone.name_.size(), index1);
  if (ec1 != std::errc()) {
    panic("Var matrix index rename failed");
  }
  clone.name_.resize(static_cast<std::size_t>(end - begin));
  return clone;
}

template <class T, class D>
inline auto Var<T, D>::parse(std::string_view s) const -> T {
  auto buf = std::make_unique<std::stringbuf>(std::string(s), std::ios_base::in);
  auto reader = Reader(std::make_unique<io::InStream>(std::move(buf), "str", true),
                       trace::Level::NONE, [](const Reader &, std::string_view msg) -> void {
                         panic(std::string("Var::parse failed: ") + std::string(msg));
                       });
  T result = reader.read(*this);
  if (!reader.inner().eof()) {
    panic("Var::parse failed, extra characters in string");
  }
  return result;
}

template <class T, class D>
inline auto Var<T, D>::operator*(std::size_t len) const -> Vec<D> {
  return Vec<D>(*static_cast<const D *>(this), len);
}

template <class T, class D>
inline Var<T, D>::Var() : name_(std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T, class D>
inline Var<T, D>::Var(std::string name) : name_(std::move(name)) {}

template <std::integral T>
inline Int<T>::Int() : Int<T>(std::string(detail::VAR_DEFAULT_NAME), std::nullopt, std::nullopt) {}

template <std::integral T>
inline Int<T>::Int(std::string name) : Int<T>(std::move(name), std::nullopt, std::nullopt) {}

template <std::integral T>
inline Int<T>::Int(std::optional<T> min, std::optional<T> max)
    : Int<T>(std::string(detail::VAR_DEFAULT_NAME), std::move(min), std::move(max)) {}

template <std::integral T>
inline Int<T>::Int(std::string name, std::optional<T> min, std::optional<T> max)
    : Var<T, Int<T>>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <std::integral T>
inline auto Int<T>::read_from(Reader &in) const -> T {
  auto token = in.inner().read_word();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected an integer, got EOF");
    } else {
      in.fail(cplib::format("Expected an integer, got whitespace `{}`",
                            cplib::detail::hex_encode(in.inner().seek())));
    }
  }

  T result{};
  const char *first = token.data();
  const char *last = token.data() + token.length();
  auto [ptr, ec] = std::from_chars(first, last, result);

  if (ec == std::errc::invalid_argument || ptr != last) {
    // * ec == std::errc::invalid_argument: String is not a valid integer format (e.g. "abc",
    //   "NaN", "Inf")
    // * ptr != last: The string is not fully parsed (for example "123abc")
    in.fail(cplib::format("Expected an integer, got `{}`", compress(token)));
  } else if (ec == std::errc::result_out_of_range) {
    // The parsing is successful, but the value exceeds the range of T
    in.fail(cplib::format("Integer value `{}` is out of range for type `{}`", compress(token),
                          typeid(T).name()));
  }

  if (min.has_value() && result < *min) {
    in.fail(cplib::format("Expected an integer >= {}, got `{}`", std::to_string(*min),
                          compress(token)));
  }

  if (max.has_value() && result > *max) {
    in.fail(cplib::format("Expected an integer <= {}, got `{}`", std::to_string(*max),
                          compress(token)));
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#v", json::Value(static_cast<json::Int>(result)));
  }

  return result;
}

template <std::floating_point T>
inline Float<T>::Float()
    : Float<T>(std::string(detail::VAR_DEFAULT_NAME), std::nullopt, std::nullopt) {}

template <std::floating_point T>
inline Float<T>::Float(std::string name) : Float<T>(std::move(name), std::nullopt, std::nullopt) {}

template <std::floating_point T>
inline Float<T>::Float(std::optional<T> min, std::optional<T> max)
    : Float<T>(std::string(detail::VAR_DEFAULT_NAME), std::move(min), std::move(max)) {}

template <std::floating_point T>
inline Float<T>::Float(std::string name, std::optional<T> min, std::optional<T> max)
    : Var<T, Float<T>>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <std::floating_point T>
inline auto Float<T>::read_from(Reader &in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected a float, got EOF");
    } else {
      in.fail(cplib::format("Expected a float, got whitespace `{}`",
                            cplib::detail::hex_encode(in.inner().seek())));
    }
  }

  T result{};
  const char *first = token.data();
  const char *last = token.data() + token.length();

  // `Float<T>` usually uses with non-strict streams, so it should support both fixed format and
  // scientific.
  bool parsed = cplib::detail::float_parse::parse(
      first, last, cplib::detail::float_parse::Mode::General, result);

  if (!parsed) {
    in.fail(cplib::format("Expected a float, got `{}`", compress(token)));
  }

  if (min.has_value() && result < *min) {
    in.fail(
        cplib::format("Expected a float >= {}, got `{}`", std::to_string(*min), compress(token)));
  }

  if (max.has_value() && result > *max) {
    in.fail(
        cplib::format("Expected a float <= {}, got `{}`", std::to_string(*max), compress(token)));
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#v", json::Value(static_cast<json::Real>(result)));
  }

  return result;
}

namespace detail {
inline auto get_decimal_places(std::string_view token_sv) -> std::size_t {
  std::size_t dot_pos = token_sv.find('.');
  if (dot_pos == std::string_view::npos) {
    // No decimal point found, so 0 digits after point
    return 0;
  } else {
    // Calculate digits after the decimal point
    // token_sv.length() - (dot_pos + 1)
    // Example: "123.45"
    // dot_pos = 3
    // length = 6
    // 6 - (3 + 1) = 6 - 4 = 2
    return token_sv.length() - (dot_pos + 1);
  }
}
}  // namespace detail

template <std::floating_point T>
inline StrictFloat<T>::StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit)
    : StrictFloat<T>(std::string(detail::VAR_DEFAULT_NAME), min, max, min_n_digit, max_n_digit) {}

template <std::floating_point T>
inline StrictFloat<T>::StrictFloat(std::string name, T min, T max, size_t min_n_digit,
                                   size_t max_n_digit)
    : Var<T, StrictFloat<T>>(std::move(name)),
      min(std::move(min)),
      max(std::move(max)),
      min_n_digit(min_n_digit),
      max_n_digit(max_n_digit) {
  if (min > max) panic("StrictFloat constructor failed: min must be <= max");
  if (min_n_digit > max_n_digit) {
    panic("StrictFloat constructor failed: min_n_digit must be <= max_n_digit");
  }
}

template <std::floating_point T>
inline auto StrictFloat<T>::read_from(Reader &in) const -> T {
  auto token = in.inner().read_word();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected a strict float, got EOF");
    } else {
      in.fail(cplib::format("Expected a strict float, got whitespace `{}`",
                            cplib::detail::hex_encode(in.inner().seek())));
    }
  }

  T result;
  const char *first = token.data();
  const char *last = token.data() + token.length();

  // Strict float accepts fixed decimal syntax only, without scientific notation.
  bool parsed = cplib::detail::float_parse::parse(first, last,
                                                  cplib::detail::float_parse::Mode::Fixed, result);

  if (!parsed) {
    in.fail(cplib::format("Expected a strict float, got `{}`", compress(token)));
  }

  std::size_t n_after_point = detail::get_decimal_places(token);

  if (n_after_point < min_n_digit) {
    in.fail(cplib::format(
        "Expected a strict float with >= {} digits after point, got `{}` with {} digits "
        "after point",
        min_n_digit, compress(token), n_after_point));
  }

  if (n_after_point > max_n_digit) {
    in.fail(cplib::format(
        "Expected a strict float with <= {} digits after point, got `{}` with {} digits "
        "after point",
        max_n_digit, compress(token), n_after_point));
  }

  if (result < min) {
    in.fail(cplib::format("Expected a strict float >= {}, got `{}`", std::to_string(min),
                          compress(token)));
  }

  if (result > max) {
    in.fail(cplib::format("Expected a strict float <= {}, got `{}`", std::to_string(max),
                          compress(token)));
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#v", json::Value(static_cast<json::Real>(result)));
  }

  return result;
}

inline YesNo::YesNo() : YesNo(std::string(detail::VAR_DEFAULT_NAME)) {}

inline YesNo::YesNo(std::string name) : Var<bool, YesNo>(std::move(name)) {}

inline auto YesNo::read_from(Reader &in) const -> bool {
  auto word = in.inner().read_word();
  auto lower_token = word;
  std::ranges::transform(lower_token, lower_token.begin(), [](unsigned char c) -> char {
    return static_cast<char>(std::tolower(c));
  });

  bool result;
  if (lower_token == "yes") {
    result = true;
  } else if (lower_token == "no") {
    result = false;
  } else {
    in.fail("Expected `Yes` or `No`, got " + word);
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#v", json::Value(result));
  }

  return result;
}

inline constexpr String::Mode::Mode(Value value) : value_(value) {}

inline constexpr String::Mode::operator Value() const { return value_; }

inline String::String()
    : Var<std::string, String>(std::string(detail::VAR_DEFAULT_NAME)),
      pat(std::nullopt),
      mode(Mode::TOKEN) {}
inline String::String(Mode mode)
    : Var<std::string, String>(std::string(detail::VAR_DEFAULT_NAME)),
      pat(std::nullopt),
      mode(mode) {}
inline String::String(Pattern pat)
    : Var<std::string, String>(std::string(detail::VAR_DEFAULT_NAME)),
      pat(std::move(pat)),
      mode(Mode::TOKEN) {}
inline String::String(Mode mode, Pattern pat)
    : Var<std::string, String>(std::string(detail::VAR_DEFAULT_NAME)),
      pat(std::move(pat)),
      mode(mode) {}
inline String::String(std::string name)
    : Var<std::string, String>(std::move(name)), pat(std::nullopt), mode(Mode::TOKEN) {}
inline String::String(std::string name, Mode mode)
    : Var<std::string, String>(std::move(name)), pat(std::nullopt), mode(mode) {}
inline String::String(std::string name, Pattern pat)
    : Var<std::string, String>(std::move(name)), pat(std::move(pat)), mode(Mode::TOKEN) {}
inline String::String(std::string name, Mode mode, Pattern pat)
    : Var<std::string, String>(std::move(name)), pat(std::move(pat)), mode(mode) {}

inline auto String::read_from(Reader &in) const -> std::string {
  std::string result;
  std::string kind;

  switch (mode) {
    case Mode::TOKEN: {
      kind = "token";
      result = in.inner().read_token();
      if (result.empty()) {
        if (in.inner().eof()) {
          in.fail("Expected a " + kind + ", got EOF");
        } else {
          in.fail(cplib::format("Expected a {}, got whitespace `{}`", kind,
                                cplib::detail::hex_encode(in.inner().seek())));
        }
      }
      break;
    }
    case Mode::WORD: {
      kind = "word";
      result = in.inner().read_word();
      if (result.empty()) {
        if (in.inner().eof()) {
          in.fail("Expected a " + kind + ", got EOF");
        } else {
          in.fail(cplib::format("Expected a {}, got whitespace `{}`", kind,
                                cplib::detail::hex_encode(in.inner().seek())));
        }
      }
      break;
    }
    case Mode::LINE: {
      kind = "line";
      auto line = in.inner().read_line();
      if (!line.has_value()) {
        in.fail("Expected a " + kind + ", got EOF");
      }
      result = *line;
      break;
    }
    default: {
      in.fail("Unexpected var::String:Mode");
    }
  }

  if (pat.has_value() && !pat->match(result)) {
    in.fail(cplib::format("Expected a {} matching `{}`, got `{}`", kind, compress(pat->src()),
                          compress(result)));
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#v", json::Value(result));
  }

  return result;
}

// Impl Separator {{{
inline Separator::Separator(std::optional<unsigned char> sep)
    : Separator(std::string(detail::VAR_DEFAULT_NAME), sep) {}

inline Separator::Separator(std::string name, std::optional<unsigned char> sep)
    : Var<std::nullopt_t, Separator>(std::move(name)), sep(sep) {}

inline auto Separator::read_from(Reader &in) const -> std::nullopt_t {
  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#hidden", json::Value(true));
  }

  if (!sep.has_value()) {
    return std::nullopt;
  }

  unsigned char s = *sep;

  if (in.inner().eof()) {
    in.fail(cplib::format("Expected a separator `{}`, got EOF", cplib::detail::hex_encode(s)));
  }

  if (in.inner().is_strict()) {
    auto got = in.inner().read();
    if (got != s) {
      in.fail(cplib::format("Expected a separator `{}`, got `{}`", cplib::detail::hex_encode(s),
                            cplib::detail::hex_encode(got)));
    }
  } else if (std::isspace(static_cast<unsigned char>(s)) != 0) {
    auto got = in.inner().read();
    if (std::isspace(static_cast<unsigned char>(got)) == 0) {
      in.fail(cplib::format("Expected a separator `{}`, got `{}`", cplib::detail::hex_encode(s),
                            cplib::detail::hex_encode(got)));
    }
  } else {
    in.inner().skip_blanks();
    auto got = in.inner().read();
    if (got != s) {
      in.fail(cplib::format("Expected a separator `{}`, got `{}`", cplib::detail::hex_encode(s),
                            cplib::detail::hex_encode(got)));
    }
  }

  return std::nullopt;
}
// /Impl Separator }}}

template <class T>
inline Vec<T>::Vec(T element, size_t len) : Vec<T>(element, len, var::space) {}

template <class T>
inline Vec<T>::Vec(T element, size_t len, Separator sep)
    : Var<std::vector<typename T::Var::Target>, Vec<T>>(std::string(element.name())),
      element(std::move(element)),
      len(len),
      sep(std::move(sep)) {}

template <class T>
inline auto Vec<T>::read_from(Reader &in) const -> std::vector<typename T::Var::Target> {
  std::vector<typename T::Var::Target> result(len);
  for (std::size_t i = 0; i < len; ++i) {
    if (i > 0) in.read(sep);
    result[i] = in.read(element.renamed(i));
  }
  return result;
}

template <class T>
inline Mat<T>::Mat(T element, size_t len0, size_t len1)
    : Mat<T>(element, len0, len1, var::eoln, var::space) {}

template <class T>
inline Mat<T>::Mat(T element, size_t len0, size_t len1, Separator sep0, Separator sep1)
    : Var<std::vector<std::vector<typename T::Var::Target>>, Mat<T>>(std::string(element.name())),
      element(std::move(element)),
      len0(len0),
      len1(len1),
      sep0(std::move(sep0)),
      sep1(std::move(sep1)) {}

template <class T>
inline auto Mat<T>::read_from(Reader &in) const
    -> std::vector<std::vector<typename T::Var::Target>> {
  std::vector<std::vector<typename T::Var::Target>> result(
      len0, std::vector<typename T::Var::Target>(len1));
  for (std::size_t i = 0; i < len0; ++i) {
    if (i > 0) in.read(sep0);
    for (std::size_t j = 0; j < len1; ++j) {
      if (j > 0) in.read(sep1);
      result[i][j] = in.read(element.renamed(i, j));
    }
  }
  return result;
}

template <class F, class S>
inline Pair<F, S>::Pair(std::pair<F, S> pr)
    : Pair<F, S>(std::string(detail::VAR_DEFAULT_NAME), std::move(pr), var::space) {}

template <class F, class S>
inline Pair<F, S>::Pair(std::pair<F, S> pr, Separator sep)
    : Pair<F, S>(std::string(detail::VAR_DEFAULT_NAME), std::move(pr), std::move(sep)) {}

template <class F, class S>
inline Pair<F, S>::Pair(std::string name, std::pair<F, S> pr, Separator sep)
    : Var<std::pair<typename F::Var::Target, typename S::Var::Target>, Pair<F, S>>(std::move(name)),
      first(std::move(pr.first)),
      second(std::move(pr.second)),
      sep(std::move(sep)) {}

template <class F, class S>
inline auto Pair<F, S>::read_from(Reader &in) const
    -> std::pair<typename F::Var::Target, typename S::Var::Target> {
  auto result_first = in.read(first.renamed("first"));
  in.read(sep);
  auto result_second = in.read(second.renamed("second"));
  return {result_first, result_second};
}

template <class... T>
inline Tuple<T...>::Tuple(std::tuple<T...> elements)
    : Tuple<T...>(std::string(detail::VAR_DEFAULT_NAME), std::move(elements), var::space) {}

template <class... T>
inline Tuple<T...>::Tuple(std::tuple<T...> elements, Separator sep)
    : Tuple<T...>(std::string(detail::VAR_DEFAULT_NAME), std::move(elements), std::move(sep)) {}

template <class... T>
inline Tuple<T...>::Tuple(std::string name, std::tuple<T...> elements, Separator sep)
    : Var<std::tuple<typename T::Var::Target...>, Tuple<T...>>(std::move(name)),
      elements(std::move(elements)),
      sep(std::move(sep)) {}

template <class... T>
inline auto Tuple<T...>::read_from(Reader &in) const -> std::tuple<typename T::Var::Target...> {
  // Delegate to the implementation helper with a generated index sequence.
  return read_from_impl(in, std::index_sequence_for<T...>{});
}

template <class... T>
template <std::size_t... Is>
inline auto Tuple<T...>::read_from_impl(Reader &in, std::index_sequence<Is...>) const
    -> std::tuple<typename T::Var::Target...> {
  // Create the result tuple that will be populated.
  std::tuple<typename T::Var::Target...> result;
  std::size_t cnt = 0;
  auto renamed_inc = [&cnt](auto var) -> decltype(var) { return var.renamed(cnt++); };

  // Use a C++17 fold expression over the comma operator to process each element sequentially.
  // This is a concise way to apply an operation to each element of a parameter pack.
  ((void)([&] {
     // For every element after the first one (where index Is > 0), read the separator.
     // `if constexpr` ensures this check is done at compile-time, so there is no
     // runtime overhead for the first element.
     if constexpr (Is > 0) {
       in.read(sep);
     }
     // Read the current element using its corresponding Var template from the `elements`
     // tuple and assign it to the correct position in the `result` tuple.
     std::get<Is>(result) = in.read(renamed_inc(std::get<Is>(elements)));
   }()),
   ...);

  return result;
}

template <class F>
template <class... Args>
inline FnVar<F>::FnVar(std::string name, std::function<F> f, Args &&...args)
    : Var<typename std::function<F>::result_type, FnVar<F>>(std::move(name)),
      inner_function_([captured_args = std::make_tuple(std::forward<Args>(args)...),
                       f](Reader &in) -> typename std::function<F>::result_type {
        return std::apply(
            [&in, f](auto &&...unpacked_args) -> typename std::function<F>::result_type {
              return f(in, std::forward<decltype(unpacked_args)>(unpacked_args)...);
            },
            captured_args);
      }) {}

template <class F>
inline auto FnVar<F>::read_from(Reader &in) const -> typename std::function<F>::result_type {
  return inner_function_(in);
}

template <class T>
template <class... Args>
inline ExtVar<T>::ExtVar(std::string name, Args &&...args)
  requires Readable<T, Args...>
    : Var<T, ExtVar<T>>(std::move(name)),
      // Capture arguments into a tuple, then use std::apply to call T::read.
      // This is a robust way to handle variadic arguments within std::function.
      inner_function_(
          [captured_args = std::make_tuple(std::forward<Args>(args)...)](Reader &in) -> T {
            return std::apply(
                [&in](auto &&...unpacked_args) -> T {
                  // Call T::read with the Reader and the unpacked arguments
                  return T::read(in, std::forward<decltype(unpacked_args)>(unpacked_args)...);
                },
                captured_args);
          }) {}

template <class T>
template <class... Args>
inline ExtVar<T>::ExtVar(std::size_t index, Args &&...args)
  requires Readable<T, Args...>
    : ExtVar<T>(std::string(), std::forward<Args>(args)...) {
  this->name_.resize(20);
  auto [ptr, ec] =
      std::to_chars(this->name_.data(), this->name_.data() + this->name_.size(), index);
  if (ec != std::errc()) {
    panic("ExtVar index rename failed");
  }
  this->name_.resize(static_cast<std::size_t>(ptr - this->name_.data()));
}

template <class T>
inline auto ExtVar<T>::read_from(Reader &in) const -> T {
  return inner_function_(in);
}

template <class T>
template <std::ranges::range Range, class... Args>
inline ExtVec<T>::ExtVec(std::string name, Range &&range, Separator sep, Args &&...args)
  requires Readable<T, Args..., std::ranges::range_value_t<Range>>
    : Var<std::vector<T>, ExtVec<T>>(std::move(name)),
      inner_function_([range = std::forward<Range>(range), sep,
                       captured_args = std::make_tuple(std::forward<Args>(args)...)](
                          Reader &in) -> std::vector<T> {
        std::vector<T> result;
        if constexpr (std::ranges::sized_range<Range>) {
          result.reserve(std::ranges::size(range));
        }

        std::size_t i = 0;
        for (const auto &range_element : range) {
          if (i > 0) {
            in.read(sep);
          }

          // Use std::apply to construct an ExtVar for each element.
          // This unpacks the tuple of fixed arguments and passes them, along with the
          // current range_element, to the ExtVar constructor. This correctly sets up
          // the call to T::read with all necessary arguments and handles tracing.
          auto element = std::apply(
              [&](auto &&...fixed_args) {
                return in.read(
                    ExtVar<T>(i, std::forward<decltype(fixed_args)>(fixed_args)..., range_element));
              },
              captured_args);

          result.push_back(std::move(element));
          ++i;
        }
        return result;
      }) {}

template <class T>
inline auto ExtVec<T>::read_from(Reader &in) const -> std::vector<T> {
  return inner_function_(in);
}
}  // namespace cplib::var
// --- End embedded: var.i.hpp ---

#endif
// --- End embedded: var.hpp ---
// --- Start embedded: cmd_args.hpp ---
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

#ifndef CPLIB_CMD_ARGS_HPP_
#define CPLIB_CMD_ARGS_HPP_

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace cplib::cmd_args {

/// Parsed command-line args.
struct ParsedArgs {
  /// Command-line parameters that do not start with "--"will be stored in `ordered` in their
  /// original relative order.
  std::vector<std::string> ordered;

  // `--flag`
  std::vector<std::string> flags;

  // `--var=value` or `--var value`
  std::map<std::string, std::string> vars;

  ParsedArgs() = default;

  /// Parse from raw command-line args.
  explicit ParsedArgs(const std::vector<std::string> &args);

  [[nodiscard]] auto has_flag(std::string_view name) const -> bool;
};
}  // namespace cplib::cmd_args

// --- Start embedded: cmd_args.i.hpp ---
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


#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cplib::cmd_args {

namespace detail {
inline auto split_var(std::string_view var) -> std::pair<std::string_view, std::string_view> {
  auto eq_pos = var.find('=');
  return {var.substr(2, eq_pos - 2), var.substr(eq_pos + 1)};
}
}  // namespace detail

inline ParsedArgs::ParsedArgs(const std::vector<std::string> &args) {
  std::optional<std::string> last_flag;

  for (const auto &arg : args) {
    if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-') {
      if (arg.find('=') != std::string::npos) {
        // `--var=value`
        auto [key, value] = detail::split_var(arg);
        vars.emplace(std::string(key), std::string(value));
        if (last_flag.has_value()) {
          flags.emplace_back(*last_flag);
        }
        last_flag = std::nullopt;
      } else {
        // `--flag` or `--var value`
        if (last_flag.has_value()) {
          flags.emplace_back(*last_flag);
        }
        last_flag = arg.substr(2);
      }
    } else if (last_flag.has_value()) {
      // `value` of `--var value`
      vars.emplace(std::string(*last_flag), std::string(arg));
      last_flag = std::nullopt;
    } else {
      // Parameter without "--"
      ordered.emplace_back(arg);
      last_flag = std::nullopt;
    }
  }

  if (last_flag.has_value()) {
    flags.emplace_back(*last_flag);
  }

  // Sort for binary search
  std::ranges::sort(flags);
}

inline auto ParsedArgs::has_flag(std::string_view name) const -> bool {
  return std::ranges::binary_search(flags, name);
}

}  // namespace cplib::cmd_args
// --- End embedded: cmd_args.i.hpp ---

#endif
// --- End embedded: cmd_args.hpp ---
// --- Start embedded: checker.hpp ---
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

#ifndef CPLIB_CHECKER_HPP_
#define CPLIB_CHECKER_HPP_

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace cplib::checker {
/**
 * `Report` represents a report showing when checker exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  struct Status {
   public:
    enum Value : std::uint8_t {
      /// Indicates an internal error occurred.
      INTERNAL_ERROR,
      /// Indicates the solution is accepted
      ACCEPTED,
      /// Indicates the solution is incorrect.
      WRONG_ANSWER,
      /// Indicates the solution is partially correct.
      PARTIALLY_CORRECT,
    };

    /**
     * Default constructor for Status.
     */
    Status() = default;

    /**
     * Constructor for Status with a given value.
     *
     * @param value The value of the status.
     */
    constexpr Status(Value value);  // NOLINT(google-explicit-constructor)

    /**
     * Constructor for Status from cplib::evaluate::Result::Status.
     *
     * @param status The evaluate result status.
     */
    constexpr explicit Status(evaluate::Result::Status status);

    /**
     * Implicit conversion operator to Value.
     *
     * @return The value of the status.
     */
    constexpr operator Value() const;  // NOLINT(google-explicit-constructor)

    /**
     * Deleted conversion operator to bool.
     *
     * @return Deleted conversion operator to bool.
     */
    explicit operator bool() const = delete;

    /**
     * Get the string representation of the status.
     *
     * @return The string representation of the status.
     */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view;

   private:
    Value value_;
  };

  /// The status of the report.
  Status status;

  /// The score of the report (full score is 1.0).
  double score;

  /// The message associated with the report.
  std::string message;

  /**
   * Constructor for Report.
   * @param status The status of the report.
   * @param score The score of the report.
   * @param message The message associated with the report.
   */
  Report(Status status, double score, std::string message);
};

struct State;

/**
 * `Initializer` used to initialize the state.
 */
struct Initializer {
 public:
  virtual ~Initializer() = 0;

  auto set_state(State &state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string> &args) -> void = 0;

 protected:
  auto state() -> State &;
  auto set_inf_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_ouf_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_ans_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_inf_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_ouf_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_ans_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_evaluator(trace::Level trace_level) -> void;

 private:
  State *state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report &report) -> int = 0;

  auto attach_reader_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void;
  auto attach_evaluator_trace_stack(trace::TraceStack<evaluate::EvaluatorTrace> trace_stack)
      -> void;
  [[nodiscard]] auto get_evaluator_trace_stacks() const
      -> const std::vector<trace::TraceStack<evaluate::EvaluatorTrace>> &;

 protected:
  std::vector<trace::TraceStack<var::ReaderTrace>> reader_trace_stacks_{};
  std::vector<trace::TraceStack<evaluate::EvaluatorTrace>> evaluator_trace_stacks_{};
};

/**
 * Represents the state of the checker.
 */
struct State {
 public:
  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Output file reader.
  var::Reader ouf;

  /// Answer file reader.
  var::Reader ans;

  /// Evaluator
  evaluate::Evaluator evaluator;

  /// Initializer parses command-line arguments and initializes `checker::State`
  std::unique_ptr<Initializer> initializer;

  /// Reporter reports the given `checker::Report` and exits the program.
  std::unique_ptr<Reporter> reporter;

  /**
   * Constructs a new `State` object with the given initializer.
   *
   * @param initializer The initializer.
   */
  explicit State(std::unique_ptr<Initializer> initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Disables the check for redundant content in the output file.
   */
  auto disable_check_dirt() -> void;

  /**
   * Quits the checker with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(Report report) -> void;

  /**
   * Quits the checker with the `report::Status::ACCEPTED` status.
   */
  [[noreturn]] auto quit_ac() -> void;

  /**
   * Quits the checker with the `report::Status::WRONG_ANSWER` status and the given message.
   *
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_wa(std::string_view message) -> void;

  /**
   * Quits the checker with the `report::Status::PARTIALLY_CORRECT` status, the given points, and
   * message.
   *
   * @param points The points to be awarded.
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_pc(double points, std::string_view message) -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};

  /// Whether to check for redundant content in the output file.
  bool check_dirt_{true};
};

/**
 * Default initializer of checker.
 */
struct DefaultInitializer : Initializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param arg0 The name of the program.
   * @param args The command-line arguments.
   */
  auto init(std::string_view arg0, const std::vector<std::string> &args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

template <class Input, class Output>
auto run_checker(int argc, char **argv, std::unique_ptr<Initializer> initializer) -> int;

/**
 * Macro to register checker with custom initializer.
 *
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_CHECKER_OPT(input_struct_, output_struct_, initializer_)          \
  auto main(int argc, char **argv) -> int {                                              \
    return ::cplib::checker::run_checker<input_struct_, output_struct_>(                 \
        argc, argv, ::std::unique_ptr<::cplib::checker::Initializer>(new initializer_)); \
  }

#ifndef CPLIB_CHECKER_DEFAULT_INITIALIZER
#define CPLIB_CHECKER_DEFAULT_INITIALIZER ::cplib::checker::DefaultInitializer()
#endif

/**
 * Macro to register checker with default initializer.
 */
#define CPLIB_REGISTER_CHECKER(input_struct_, output_struct_) \
  CPLIB_REGISTER_CHECKER_OPT(input_struct_, output_struct_, CPLIB_CHECKER_DEFAULT_INITIALIZER)
}  // namespace cplib::checker

// --- Start embedded: checker.i.hpp ---
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


#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace cplib::checker {
inline constexpr Report::Status::Status(Value value) : value_(value) {}

inline constexpr Report::Status::Status(evaluate::Result::Status status) {
  switch (status) {
    case evaluate::Result::Status::WRONG_ANSWER:
      value_ = WRONG_ANSWER;
      break;
    case evaluate::Result::Status::PARTIALLY_CORRECT:
      value_ = PARTIALLY_CORRECT;
      break;
    case evaluate::Result::Status::ACCEPTED:
      value_ = ACCEPTED;
      break;
    default:
      panic(cplib::format("Construct checker report status failed: unknown evaluate status {}",
                          static_cast<int>(status)));
  }
}

inline constexpr Report::Status::operator Value() const { return value_; }

inline constexpr auto Report::Status::to_string() const -> std::string_view {
  switch (value_) {
    case INTERNAL_ERROR:
      return "internal_error";
    case ACCEPTED:
      return "accepted";
    case WRONG_ANSWER:
      return "wrong_answer";
    case PARTIALLY_CORRECT:
      return "partially_correct";
    default:
      panic(cplib::format("Unknown checker report status: {}", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, double score, std::string message)
    : status(status), score(score), message(std::move(message)) {}

inline Initializer::~Initializer() = default;

inline auto Initializer::set_state(State &state) -> void { state_ = &state; };

inline auto Initializer::state() -> State & { return *state_; };

inline auto Initializer::set_inf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_ouf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->ouf = var::detail::make_reader_by_fileno(
      fileno, "ouf", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_ans_fileno(int fileno, trace::Level trace_level) -> void {
  state_->ans = var::detail::make_reader_by_fileno(
      fileno, "ans", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_inf_path(std::string_view path, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_ouf_path(std::string_view path, trace::Level trace_level) -> void {
  state_->ouf = var::detail::make_reader_by_path(
      path, "ouf", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_ans_path(std::string_view path, trace::Level trace_level) -> void {
  state_->ans = var::detail::make_reader_by_path(
      path, "ans", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_evaluator(trace::Level trace_level) -> void {
  state_->evaluator = evaluate::Evaluator(
      trace_level,
      [this, trace_level](const evaluate::Evaluator &evaluator, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_evaluator_trace_stack(evaluator.make_trace_stack(true));
        }
        panic(msg);
      },
      [this, trace_level](const evaluate::Evaluator &evaluator,
                          const evaluate::Result &result) -> void {
        if (trace_level >= trace::Level::STACK_ONLY && !result.message.empty()) {
          state_->reporter->attach_evaluator_trace_stack(evaluator.make_trace_stack(false));
        }
      });
}

inline Reporter::~Reporter() = default;

inline auto Reporter::attach_reader_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack)
    -> void {
  reader_trace_stacks_.emplace_back(std::move(trace_stack));
}

inline auto Reporter::attach_evaluator_trace_stack(
    trace::TraceStack<evaluate::EvaluatorTrace> trace_stack) -> void {
  evaluator_trace_stacks_.emplace_back(std::move(trace_stack));
}

inline auto Reporter::get_evaluator_trace_stacks() const
    -> const std::vector<trace::TraceStack<evaluate::EvaluatorTrace>> & {
  return evaluator_trace_stacks_;
}

// Impl State {{{

inline State::State(std::unique_ptr<Initializer> initializer)
    : rnd(),
      inf(var::Reader(nullptr, trace::Level::NONE, {})),
      ouf(var::Reader(nullptr, trace::Level::NONE, {})),
      ans(var::Reader(nullptr, trace::Level::NONE, {})),
      evaluator(evaluate::Evaluator(trace::Level::NONE, {}, {})),
      initializer(std::move(initializer)),
      reporter(std::make_unique<JsonReporter>()) {
  this->initializer->set_state(*this);
  cplib::detail::panic_impl = [this](std::string_view msg) -> void {
    quit(Report(Report::Status::INTERNAL_ERROR, 0.0, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::CHECKER;
}

inline State::~State() {
  if (!exited_) panic("Checker must exit by calling method `State::quit*`");
}

inline auto State::disable_check_dirt() -> void { check_dirt_ = false; }

inline auto State::quit(Report report) -> void {
  exited_ = true;

  if (check_dirt_ &&
      (report.status == Report::Status::ACCEPTED ||
       report.status == Report::Status::PARTIALLY_CORRECT) &&
      !ouf.inner().seek_eof()) {
    report = Report(Report::Status::WRONG_ANSWER, 0.0, "Extra content in the output file");
  }

  auto exit_code = reporter->report(report);
  std::exit(exit_code);
}

inline auto State::quit_ac() -> void { quit(Report(Report::Status::ACCEPTED, 1.0, "")); }

inline auto State::quit_wa(std::string_view message) -> void {
  quit(Report(Report::Status::WRONG_ANSWER, 0.0, std::string(message)));
}

inline auto State::quit_pc(double points, std::string_view message) -> void {
  quit(Report(Report::Status::PARTIALLY_CORRECT, points, std::string(message)));
}
// /Impl State }}}

namespace detail {
inline auto collect_args(int argc, char **argv) -> std::vector<std::string> {
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }
  return args;
}

inline auto first_evaluator_trace_message(
    const std::vector<trace::TraceStack<evaluate::EvaluatorTrace>> &trace_stacks) -> std::string {
  for (const auto &trace_stack : trace_stacks) {
    if (trace_stack.stack.empty()) {
      continue;
    }
    const auto &trace_result = trace_stack.stack.back().result;
    if (!trace_result.has_value()) {
      continue;
    }
    const auto &trace_message = trace_result->message;
    if (trace_message.empty()) {
      continue;
    }
    return trace_message;
  }
  return "";
}
}  // namespace detail

template <class Input, class Output>
auto run_checker(int argc, char **argv, std::unique_ptr<Initializer> initializer) -> int {
  static_assert(var::Readable<Input>, "Input should be Readable");
  static_assert(var::Readable<Output, const Input &>, "Output should be Readable");
  static_assert(evaluate::Evaluatable<Output, const Input &>, "Output should be Evaluatable");

  auto args = detail::collect_args(argc, argv);

  /* std::exit only destroys static objects */
  static auto state = State(std::move(initializer));
  state.initializer->init(argv[0], args);

  Input input{state.inf.read(var::ExtVar<Input>("input"))};
  Output output{state.ouf.read(var::ExtVar<Output>("output", input))};
  Output answer{state.ans.read(var::ExtVar<Output>("answer", input))};
  evaluate::Result result = state.evaluator("output", output, answer, input);
  std::string report_message =
      detail::first_evaluator_trace_message(state.reporter->get_evaluator_trace_stacks());
  Report report{Report::Status(result.status), result.score, report_message};
  state.quit(report);
  return 0;
}

// Impl DefaultInitializer {{{
namespace detail {
constexpr std::string_view ARGS_USAGE =
    "<input_file> <output_file> <answer_file> [--report-format={auto|json|text}]";

inline auto print_help_message(std::string_view program_name) -> void {
  std::string msg =
      cplib::format(CPLIB_STARTUP_TEXT
                    "\n"
                    "Usage:\n"
                    "  {} {}\n"
                    "\n"
                    "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
                    "enable colors",
                    program_name, ARGS_USAGE);
  panic(msg);
}

inline auto detect_reporter(State &state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = std::make_unique<JsonReporter>();
  } else if (cplib::detail::has_colors()) {
    state.reporter = std::make_unique<ColoredTextReporter>();
  } else {
    state.reporter = std::make_unique<PlainTextReporter>();
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State &state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = std::make_unique<JsonReporter>();
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = std::make_unique<ColoredTextReporter>();
    } else {
      state.reporter = std::make_unique<PlainTextReporter>();
    }
  } else {
    return false;
  }
  return true;
}
}  // namespace detail

inline auto DefaultInitializer::init(std::string_view arg0, const std::vector<std::string> &args)
    -> void {
  auto &state = this->state();

  detail::detect_reporter(state);

  auto parsed_args = cmd_args::ParsedArgs(args);

  for (const auto &[key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(state, value)) {
        panic(cplib::format("Unknown {} option: {}", key, value));
      }
    } else {
      panic("Unknown command-line argument variable: " + key);
    }
  }

  for (const auto &flag : parsed_args.flags) {
    if (flag == "help") {
      detail::print_help_message(arg0);
    } else {
      panic("Unknown command-line argument flag: " + flag);
    }
  }

  if (parsed_args.ordered.size() != 3) {
    panic("Program must be run with the following arguments:\n  " +
          std::string(detail::ARGS_USAGE));
  }
  auto inf_path = parsed_args.ordered[0];
  auto ouf_path = parsed_args.ordered[1];
  auto ans_path = parsed_args.ordered[2];
  set_inf_path(inf_path, trace::Level::STACK_ONLY);
  set_ouf_path(ouf_path, trace::Level::STACK_ONLY);
  set_ans_path(ans_path, trace::Level::STACK_ONLY);
  set_evaluator(trace::Level::STACK_ONLY);
}
// /Impl DefaultInitializer }}}

// Impl reporters {{{
namespace detail {
inline auto status_to_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "Internal Error";
    case Report::Status::ACCEPTED:
      return "Accepted";
    case Report::Status::WRONG_ANSWER:
      return "Wrong Answer";
    case Report::Status::PARTIALLY_CORRECT:
      return "Partially Correct";
    default:
      panic(cplib::format("Unknown checker report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto status_to_colored_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "\x1b[0;35mInternal Error\x1b[0m";
    case Report::Status::ACCEPTED:
      return "\x1b[0;32mAccepted\x1b[0m";
    case Report::Status::WRONG_ANSWER:
      return "\x1b[0;31mWrong Answer\x1b[0m";
    case Report::Status::PARTIALLY_CORRECT:
      return "\x1b[0;36mPartially Correct\x1b[0m";
    default:
      panic(cplib::format("Unknown checker report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto JsonReporter::report(const Report &report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"score", json::Value(report.score)},
      {"message", json::Value(report.message)},
  };

  if (!reader_trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(reader_trace_stacks_.size());
    std::ranges::transform(reader_trace_stacks_, std::back_inserter(trace_stacks),
                           [](const auto &s) -> json::Value { return json::Value(s.to_json()); });
    map.emplace("reader_trace_stacks", trace_stacks);
  }

  if (!evaluator_trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(evaluator_trace_stacks_.size());
    std::ranges::transform(evaluator_trace_stacks_, std::back_inserter(trace_stacks),
                           [](const auto &s) -> json::Value { return json::Value(s.to_json()); });
    map.emplace("evaluator_trace_stacks", trace_stacks);
  }

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report &report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2) << detail::status_to_title_string(report.status)
         << ", scores " << report.score * 100.0 << " of 100.\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!reader_trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto &stack : reader_trace_stacks_) {
      for (const auto &line : stack.to_plain_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (!evaluator_trace_stacks_.empty()) {
    stream << "\nEvaluator trace stacks:\n";
    for (const auto &stack : evaluator_trace_stacks_) {
      stream << "  " << stack.to_plain_text_compact() << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report &report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2)
         << detail::status_to_colored_title_string(report.status) << ", scores \x1b[0;33m"
         << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!reader_trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto &stack : reader_trace_stacks_) {
      for (const auto &line : stack.to_colored_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (!evaluator_trace_stacks_.empty()) {
    stream << "\nEvaluator trace stacks:\n";
    for (const auto &stack : evaluator_trace_stacks_) {
      stream << "  " << stack.to_colored_text_compact() << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}
// /Impl reporters }}}
}  // namespace cplib::checker
// --- End embedded: checker.i.hpp ---

#endif
// --- End embedded: checker.hpp ---
// --- Start embedded: interactor.hpp ---
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

#ifndef CPLIB_INTERACTOR_HPP_
#define CPLIB_INTERACTOR_HPP_

#include <cstdint>
#include <memory>
#include <ostream>
#include <streambuf>
#include <string>
#include <string_view>
#include <vector>


namespace cplib::interactor {
/**
 * `Report` represents a report showing when interactor exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  struct Status {
   public:
    enum Value : std::uint8_t {
      /// Indicates an internal error occurred.
      INTERNAL_ERROR,
      /// Indicates the solution is accepted
      ACCEPTED,
      /// Indicates the solution is incorrect.
      WRONG_ANSWER,
      /// Indicates the solution is partially correct.
      PARTIALLY_CORRECT,
    };

    /**
     * Default constructor for Status.
     */
    Status() = default;

    /**
     * Constructor for Status with a given value.
     *
     * @param value The value of the status.
     */
    constexpr Status(Value value);  // NOLINT(google-explicit-constructor)

    /**
     * Implicit conversion operator to Value.
     *
     * @return The value of the status.
     */
    constexpr operator Value() const;  // NOLINT(google-explicit-constructor)

    /**
     * Deleted conversion operator to bool.
     *
     * @return Deleted conversion operator to bool.
     */
    explicit operator bool() const = delete;

    /**
     * Get the string representation of the status.
     *
     * @return The string representation of the status.
     */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view;

   private:
    Value value_;
  };

  /// The status of the report.
  Status status;

  /// The score of the report (full score is 1.0).
  double score;

  /// The message associated with the report.
  std::string message;

  /**
   * Constructor for Report.
   * @param status The status of the report.
   * @param score The score of the report.
   * @param message The message associated with the report.
   */
  Report(Status status, double score, std::string message);
};

struct State;

/**
 * `Initializer` used to initialize the state.
 */
struct Initializer {
 public:
  virtual ~Initializer() = 0;

  auto set_state(State &state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string> &args) -> void = 0;

 protected:
  auto state() -> State &;

  auto set_inf_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_from_user_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_to_user_fileno(int fileno) -> void;
  auto set_inf_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_from_user_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_to_user_path(std::string_view path) -> void;

 private:
  State *state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report &report) -> int = 0;

  auto attach_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void;

 protected:
  std::vector<trace::TraceStack<var::ReaderTrace>> trace_stacks_{};
};

/**
 * Represents the state of the validator.
 */
struct State {
 public:
  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Reader used to read user program stdout.
  var::Reader from_user;

  /// Output stream used to send information to user program stdin.
  std::ostream to_user;

  /// Stream buffer of `to_user`.
  std::unique_ptr<std::streambuf> to_user_buf;

  /// Initializer parses command-line arguments and initializes `interactor::State`
  std::unique_ptr<Initializer> initializer;

  /// Reporter reports the given `interactor::Report` and exits the program.
  std::unique_ptr<Reporter> reporter;

  /**
   * Constructs a new `State` object with the given initializer.
   *
   * @param initializer The initializer.
   */
  explicit State(std::unique_ptr<Initializer> initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Quits the interactor with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(const Report &report) -> void;

  /**
   * Quits the interactor with the `report::Status::ACCEPTED` status.
   */
  [[noreturn]] auto quit_ac() -> void;

  /**
   * Quits the interactor with the `report::Status::WRONG_ANSWER` status and the given message.
   *
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_wa(std::string_view message) -> void;

  /**
   * Quits the interactor with the `report::Status::PARTIALLY_CORRECT` status, the given points, and
   * message.
   *
   * @param points The points to be awarded.
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_pc(double points, std::string_view message) -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};
};

/**
 * Default initializer of interactor.
 */
struct DefaultInitializer : Initializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param arg0 The name of the program.
   * @param args The command-line arguments.
   */
  auto init(std::string_view arg0, const std::vector<std::string> &args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

using MainFunc = auto (*)() -> void;

auto run_interactor(State &state, int argc, char **argv, MainFunc main_func) -> int;

/**
 * Macro to register interactor with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_INTERACTOR_OPT(var_, initializer_)                          \
  auto interactor_main() -> void;                                                  \
  auto var_ = ::cplib::interactor::State(                                          \
      ::std::unique_ptr<::cplib::interactor::Initializer>(new initializer_));      \
  auto main(int argc, char **argv) -> int {                                        \
    return ::cplib::interactor::run_interactor(var_, argc, argv, interactor_main); \
  }

#ifndef CPLIB_INTERACTOR_DEFAULT_INITIALIZER
#define CPLIB_INTERACTOR_DEFAULT_INITIALIZER ::cplib::interactor::DefaultInitializer()
#endif

/**
 * Macro to register interactor with default initializer.
 *
 * @param var The variable name of state object to be initialized.
 */
#define CPLIB_REGISTER_INTERACTOR(var) \
  CPLIB_REGISTER_INTERACTOR_OPT(var, CPLIB_INTERACTOR_DEFAULT_INITIALIZER)
}  // namespace cplib::interactor

// --- Start embedded: interactor.i.hpp ---
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


#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace cplib::interactor {
inline constexpr Report::Status::Status(Value value) : value_(value) {}

inline constexpr Report::Status::operator Value() const { return value_; }

inline constexpr auto Report::Status::to_string() const -> std::string_view {
  switch (value_) {
    case INTERNAL_ERROR:
      return "internal_error";
    case ACCEPTED:
      return "accepted";
    case WRONG_ANSWER:
      return "wrong_answer";
    case PARTIALLY_CORRECT:
      return "partially_correct";
    default:
      panic(cplib::format("Unknown interactor report status: {}", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, double score, std::string message)
    : status(status), score(score), message(std::move(message)) {}

inline Initializer::~Initializer() = default;

inline auto Initializer::set_state(State &state) -> void { state_ = &state; };

inline auto Initializer::state() -> State & { return *state_; };

inline auto Initializer::set_inf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_from_user_fileno(int fileno, trace::Level trace_level) -> void {
  state_->from_user = var::detail::make_reader_by_fileno(
      fileno, "from_user", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_to_user_fileno(int fileno) -> void {
  io::detail::make_ostream_by_fileno(fileno, state_->to_user_buf, state_->to_user);
}

inline auto Initializer::set_inf_path(std::string_view path, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_from_user_path(std::string_view path, trace::Level trace_level)
    -> void {
  state_->from_user = var::detail::make_reader_by_path(
      path, "from_user", false, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_to_user_path(std::string_view path) -> void {
  io::detail::make_ostream_by_path(path, state_->to_user_buf, state_->to_user);
}

inline Reporter::~Reporter() = default;

inline auto Reporter::attach_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void {
  trace_stacks_.emplace_back(std::move(trace_stack));
}

// Impl State {{{
inline State::State(std::unique_ptr<Initializer> initializer)
    : rnd(),
      inf(var::Reader(nullptr, trace::Level::NONE, {})),
      from_user(var::Reader(nullptr, trace::Level::NONE, {})),
      to_user(std::ostream(nullptr)),
      to_user_buf(nullptr),
      initializer(std::move(initializer)),
      reporter(std::make_unique<JsonReporter>()) {
  this->initializer->set_state(*this);
  cplib::detail::panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, 0.0, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::INTERACTOR;
}

inline State::~State() {
  if (!exited_) panic("Interactor must exit by calling method `State::quit*`");
}

inline auto State::quit(const Report &report) -> void {
  exited_ = true;

  auto exit_code = reporter->report(report);
  std::exit(exit_code);
}

inline auto State::quit_ac() -> void { quit(Report(Report::Status::ACCEPTED, 1.0, "")); }

inline auto State::quit_wa(std::string_view message) -> void {
  quit(Report(Report::Status::WRONG_ANSWER, 0.0, std::string(message)));
}

inline auto State::quit_pc(double points, std::string_view message) -> void {
  quit(Report(Report::Status::PARTIALLY_CORRECT, points, std::string(message)));
}
// /Impl State }}}

namespace detail {
inline auto collect_args(int argc, char **argv) -> std::vector<std::string> {
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }
  return args;
}
}  // namespace detail

inline auto run_interactor(State &state, int argc, char **argv, MainFunc main_func) -> int {
  auto args = detail::collect_args(argc, argv);
  state.initializer->init(argv[0], args);
  main_func();
  return 0;
}

// Impl DefaultInitializer {{{
namespace detail {
constexpr std::string_view ARGS_USAGE = "<input_file> [--report-format={auto|json|text}]";

inline auto print_help_message(std::string_view program_name) -> void {
  std::string msg =
      cplib::format(CPLIB_STARTUP_TEXT
                    "\n"
                    "Usage:\n"
                    "  {} {}\n"
                    "\n"
                    "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
                    "enable colors",
                    program_name, ARGS_USAGE);
  panic(msg);
}

inline auto detect_reporter(State &state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = std::make_unique<JsonReporter>();
  } else if (cplib::detail::has_colors()) {
    state.reporter = std::make_unique<ColoredTextReporter>();
  } else {
    state.reporter = std::make_unique<PlainTextReporter>();
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State &state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = std::make_unique<JsonReporter>();
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = std::make_unique<ColoredTextReporter>();
    } else {
      state.reporter = std::make_unique<PlainTextReporter>();
    }
  } else {
    return false;
  }
  return true;
}

// Disable stdin & stdout
inline auto disable_stdio() -> void {
  std::ios_base::sync_with_stdio(false);
  /* FIXME: Under msvc stdin/stdout is an rvalue, cannot prevent users from using stdio. */
  // stdin = nullptr;
  // stdout = nullptr;
  std::cin.rdbuf(nullptr);
  std::cout.rdbuf(nullptr);
  std::cin.tie(nullptr);
  std::cerr.tie(nullptr);
  std::wcin.rdbuf(nullptr);
  std::wcout.rdbuf(nullptr);
  std::wcin.tie(nullptr);
  std::wcerr.tie(nullptr);
}
}  // namespace detail

inline auto DefaultInitializer::init(std::string_view arg0, const std::vector<std::string> &args)
    -> void {
  auto &state = this->state();

  detail::detect_reporter(state);

  auto parsed_args = cmd_args::ParsedArgs(args);

  for (const auto &[key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(state, value)) {
        panic(cplib::format("Unknown {} option: {}", key, value));
      }
    } else {
      panic("Unknown command-line argument variable: " + key);
    }
  }

  for (const auto &flag : parsed_args.flags) {
    if (flag == "help") {
      detail::print_help_message(arg0);
    } else {
      panic("Unknown command-line argument flag: " + flag);
    }
  }

  if (parsed_args.ordered.size() != 1) {
    panic("Program must be run with the following arguments:\n  " +
          std::string(detail::ARGS_USAGE));
  }
  auto inf_path = parsed_args.ordered[0];

  set_inf_path(inf_path, trace::Level::STACK_ONLY);
  set_from_user_fileno(fileno(stdin), trace::Level::STACK_ONLY);
  set_to_user_fileno(fileno(stdout));

  detail::disable_stdio();
}
// /Impl DefaultInitializer }}}

// Impl reporters {{{
namespace detail {
inline auto status_to_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "Internal Error";
    case Report::Status::ACCEPTED:
      return "Accepted";
    case Report::Status::WRONG_ANSWER:
      return "Wrong Answer";
    case Report::Status::PARTIALLY_CORRECT:
      return "Partially Correct";
    default:
      panic(cplib::format("Unknown interactor report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto status_to_colored_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "\x1b[0;35mInternal Error\x1b[0m";
    case Report::Status::ACCEPTED:
      return "\x1b[0;32mAccepted\x1b[0m";
    case Report::Status::WRONG_ANSWER:
      return "\x1b[0;31mWrong Answer\x1b[0m";
    case Report::Status::PARTIALLY_CORRECT:
      return "\x1b[0;36mPartially Correct\x1b[0m";
    default:
      panic(cplib::format("Unknown interactor report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto JsonReporter::report(const Report &report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"score", json::Value(report.score)},
      {"message", json::Value(report.message)},
  };

  if (!trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(trace_stacks_.size());
    std::ranges::transform(trace_stacks_, std::back_inserter(trace_stacks),
                           [](auto &s) { return json::Value(s.to_json()); });
    map.emplace("reader_trace_stacks", trace_stacks);
  }

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report &report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2) << detail::status_to_title_string(report.status)
         << ", scores " << report.score * 100.0 << " of 100.\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto &stack : trace_stacks_) {
      for (const auto &line : stack.to_plain_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report &report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2)
         << detail::status_to_colored_title_string(report.status) << ", scores \x1b[0;33m"
         << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto &stack : trace_stacks_) {
      for (const auto &line : stack.to_colored_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}
// /Impl reporters }}}
}  // namespace cplib::interactor
// --- End embedded: interactor.i.hpp ---

#endif
// --- End embedded: interactor.hpp ---
// --- Start embedded: validator.hpp ---
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

#ifndef CPLIB_VALIDATOR_HPP_
#define CPLIB_VALIDATOR_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace cplib::validator {
/**
 * `Report` represents a report showing when validator exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  struct Status {
   public:
    enum Value : std::uint8_t {
      /// Indicates an internal error occurred.
      INTERNAL_ERROR,
      /// Indicates the input file is valid.
      VALID,
      /// Indicates the input file is invalid.
      INVALID,
    };

    /**
     * Default constructor for Status.
     */
    Status() = default;

    /**
     * Constructor for Status with a given value.
     *
     * @param value The value of the status.
     */
    constexpr Status(Value value);  // NOLINT(google-explicit-constructor)

    /**
     * Implicit conversion operator to Value.
     *
     * @return The value of the status.
     */
    constexpr operator Value() const;  // NOLINT(google-explicit-constructor)

    /**
     * Deleted conversion operator to bool.
     *
     * @return Deleted conversion operator to bool.
     */
    explicit operator bool() const = delete;

    /**
     * Get the string representation of the status.
     *
     * @return The string representation of the status.
     */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view;

   private:
    Value value_;
  };

  /// The status of the report.
  Status status;

  /// The message associated with the report.
  std::string message;

  /**
   * Constructor for Report.
   * @param status The status of the report.
   * @param message The message associated with the report.
   */
  Report(Status status, std::string message);
};

struct Trait {
  /// Type of function which checks the trait is satisfied.
  using CheckFunc = std::function<auto()->bool>;

  /// The name of the trait.
  std::string name;

  /// The function used to check if the trait is satisfied.
  CheckFunc check_func;

  /// The map of traits that this trait depends on.
  ///
  /// `dependencies[name] = value` means that the current trait will only be evaluated when `name`
  /// is evaluated and `name == value`.
  std::map<std::string, bool> dependencies;

  /**
   * Constructor that takes the name and check function.
   *
   * @param name The name of the trait.
   * @param check_func The function used to check if the trait is satisfied.
   */
  Trait(std::string name, CheckFunc check_func);

  /**
   * Constructor that takes the name, check function, and dependencies.
   *
   * @param name The name of the trait.
   * @param check_func The function used to check if the trait is satisfied.
   * @param dependencies The map of traits that this trait depends on.
   */
  Trait(std::string name, CheckFunc check_func, std::map<std::string, bool> dependencies);
};

struct State;

/**
 * `Initializer` used to initialize the state.
 */
struct Initializer {
 public:
  virtual ~Initializer() = 0;

  auto set_state(State &state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string> &args) -> void = 0;

 protected:
  auto state() -> State &;

  auto set_inf_fileno(int fileno, trace::Level level) -> void;
  auto set_inf_path(std::string_view path, trace::Level level) -> void;

 private:
  State *state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report &report) -> int = 0;

  auto attach_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void;

  auto attach_trace_tree(const trace::TraceTreeNode<var::ReaderTrace> *root) -> void;

  auto attach_trait_status(const std::map<std::string, bool> &trait_status) -> void;

 protected:
  std::vector<trace::TraceStack<var::ReaderTrace>> trace_stacks_{};
  const trace::TraceTreeNode<var::ReaderTrace> *trace_tree_{};
  std::map<std::string, bool> trait_status_{};
};

struct State {
 public:
  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Initializer parses command-line arguments and initializes `validator::State`
  std::unique_ptr<Initializer> initializer;

  /// Reporter reports the given `validator::Report` and exits the program.
  std::unique_ptr<Reporter> reporter;

  /**
   * Constructs a new `State` object with the given initializer.
   *
   * @param initializer The initializer.
   */
  explicit State(std::unique_ptr<Initializer> initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Sets the traits of the validator.
   *
   * @warning Calling this function multiple times will overwrite the last trait list.
   *
   * @param traits The list of traits.
   */
  auto traits(std::vector<Trait> traits) -> void;

  /**
   * Quits the validator with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(Report report) -> void;

  /**
   * Quits the validator with the `report::Status::VALID` status.
   */
  [[noreturn]] auto quit_valid() -> void;

  /**
   * Quits the validator with the `report::Status::INVALID` status and given message.
   *
   * @param message The message to be reported.
   */
  [[noreturn]] auto quit_invalid(std::string_view message) -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};

  /// The list of traits of the validator.
  std::vector<Trait> traits_;

  /// The edge set of a directed graph established based on the dependencies between traits.
  std::vector<std::vector<std::pair<std::size_t, bool>>> trait_edges_;
};

/**
 * Default initializer of validator.
 */
struct DefaultInitializer : Initializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param arg0 The name of the program.
   * @param args The command-line arguments.
   */
  auto init(std::string_view arg0, const std::vector<std::string> &args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

template <class Input, class TraitsFunc>
auto run_validator(int argc, char **argv, std::unique_ptr<Initializer> initializer,
                   TraitsFunc traits_func) -> int;

/**
 * Macro to register validator with custom initializer.
 *
 * @param initializer_ The initializer function.
 * @param traits_func_ The function returns a list of traits.
 */
#define CPLIB_REGISTER_VALIDATOR_OPT(input_struct_, traits_func_, initializer_)           \
  auto main(int argc, char **argv) -> int {                                               \
    return ::cplib::validator::run_validator<input_struct_>(                              \
        argc, argv, ::std::unique_ptr<::cplib::validator::Initializer>(new initializer_), \
        traits_func_);                                                                    \
  }

#ifndef CPLIB_VALIDATOR_DEFAULT_INITIALIZER
#define CPLIB_VALIDATOR_DEFAULT_INITIALIZER ::cplib::validator::DefaultInitializer()
#endif

/**
 * Macro to register validator with default initializer.
 */
#define CPLIB_REGISTER_VALIDATOR(input_struct_, traits_func_) \
  CPLIB_REGISTER_VALIDATOR_OPT(input_struct_, traits_func_, CPLIB_VALIDATOR_DEFAULT_INITIALIZER)
}  // namespace cplib::validator

// --- Start embedded: validator.i.hpp ---
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


#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <queue>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace cplib::validator {
inline constexpr Report::Status::Status(Value value) : value_(value) {}

inline constexpr Report::Status::operator Value() const { return value_; }

inline constexpr auto Report::Status::to_string() const -> std::string_view {
  switch (value_) {
    case INTERNAL_ERROR:
      return "internal_error";
    case VALID:
      return "valid";
    case INVALID:
      return "invalid";
    default:
      panic(cplib::format("Unknown validator report status: {}", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, std::string message)
    : status(status), message(std::move(message)) {}

inline Trait::Trait(std::string name, CheckFunc check_func)
    : Trait(std::move(name), std::move(check_func), {}) {}

inline Trait::Trait(std::string name, CheckFunc check_func,
                    std::map<std::string, bool> dependencies)
    : name(std::move(name)),
      check_func(std::move(check_func)),
      dependencies(std::move(dependencies)) {}

inline Initializer::~Initializer() = default;

inline auto Initializer::set_state(State &state) -> void { state_ = &state; };

inline auto Initializer::state() -> State & { return *state_; };

inline auto Initializer::set_inf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", true, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_invalid(msg);
      });
}

inline auto Initializer::set_inf_path(std::string_view path, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", true, trace_level,
      [this, trace_level](const var::Reader &reader, std::string_view msg) -> void {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_invalid(msg);
      });
}

inline Reporter::~Reporter() = default;

inline auto Reporter::attach_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void {
  trace_stacks_.emplace_back(std::move(trace_stack));
}

inline auto Reporter::attach_trace_tree(const trace::TraceTreeNode<var::ReaderTrace> *root)
    -> void {
  if (!root) {
    panic("Reporter::attach_trace_tree failed: Trace tree root pointer is nullptr");
  }

  trace_tree_ = root;
}

inline auto Reporter::attach_trait_status(const std::map<std::string, bool> &trait_status) -> void {
  trait_status_ = trait_status;
}

// Impl State {{{
namespace detail {
/**
 * In topological sorting, `callback` is called every time a new node is reached.
 *
 * If `follow_unmatched_edge` is false, outgoing edges with a value different from the value
 * returned by the callback for the current node will not be visited.
 */
inline auto topo_sort(const std::vector<std::vector<std::pair<std::size_t, bool>>> &edges,
                      const bool follow_unmatched_edge,
                      const std::function<auto(std::size_t)->bool> &callback) -> void {
  std::vector<std::size_t> degree(edges.size(), 0);

  for (const auto &edge : edges) {
    for (auto [to, v] : edge) ++degree[to];
  }

  std::queue<std::size_t> queue;

  for (std::size_t i = 0; i < edges.size(); ++i) {
    if (degree[i] == 0) queue.push(i);
  }

  while (!queue.empty()) {
    auto front = queue.front();
    queue.pop();
    auto result = callback(front);
    for (auto [to, v] : edges[front]) {
      if (!follow_unmatched_edge && v != result) continue;
      --degree[to];
      if (!degree[to]) queue.push(to);
    }
  }
}

// Returns std::nullopt if failed
inline auto build_edges(std::vector<Trait> &traits)
    -> std::optional<std::vector<std::vector<std::pair<std::size_t, bool>>>> {
  // Check duplicate name
  std::ranges::sort(traits, [](const Trait &x, const Trait &y) -> bool { return x.name < y.name; });
  if (std::ranges::unique(traits, [](const Trait &x, const Trait &y) -> bool {
        return x.name == y.name;
      }).end() != traits.end()) {
    // Found duplicate name
    return std::nullopt;
  }

  std::vector<std::vector<std::pair<std::size_t, bool>>> edges(traits.size());

  for (std::size_t i = 0; i < traits.size(); ++i) {
    auto &trait = traits[i];
    for (const auto &[name, value] : trait.dependencies) {
      auto it = std::ranges::lower_bound(traits, name, std::less{}, &Trait::name);
      // IMPORTANT: Check if the dependency was actually found and is an exact match.
      if (it == traits.end() || it->name != name) {
        return std::nullopt;
      }
      auto dep_id = it - traits.begin();
      edges[dep_id].emplace_back(i, value);
    }
  }

  return edges;
}

inline auto have_loop(const std::vector<std::vector<std::pair<std::size_t, bool>>> &edges) -> bool {
  std::vector<std::uint8_t> visited(edges.size(), 0);  // Never use std::vector<bool>

  topo_sort(edges, true, [&](std::size_t node) -> bool {
    visited[node] = 1;
    return true;
  });

  for (auto v : visited) {
    if (!v) return true;
  }
  return false;
}

inline auto validate_traits(const std::vector<Trait> &traits,
                            const std::vector<std::vector<std::pair<std::size_t, bool>>> &edges)
    -> std::map<std::string, bool> {
  std::map<std::string, bool> results;

  topo_sort(edges, true, [&](std::size_t id) -> bool {
    auto &node = traits[id];
    auto result = node.check_func();
    results.emplace(node.name, result);
    return result;
  });

  return results;
}
}  // namespace detail

inline State::State(std::unique_ptr<Initializer> initializer)
    : rnd(),
      inf(var::Reader(nullptr, trace::Level::NONE, {})),
      initializer(std::move(initializer)),
      reporter(std::make_unique<JsonReporter>()),
      traits_(),
      trait_edges_() {
  this->initializer->set_state(*this);
  cplib::detail::panic_impl = [this](std::string_view msg) -> void {
    quit(Report(Report::Status::INTERNAL_ERROR, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::VALIDATOR;
}

inline State::~State() {
  if (!exited_) panic("Validator must exit by calling method `State::quit*`");
}

inline auto State::traits(std::vector<Trait> traits) -> void {
  traits_ = std::move(traits);

  auto edges = detail::build_edges(traits_);
  if (!edges.has_value()) panic("Traits do not form a simple graph");

  if (detail::have_loop(*edges)) panic("Traits do not form a DAG");

  trait_edges_ = *edges;
}

inline auto State::quit(Report report) -> void {
  exited_ = true;

  if (report.status == Report::Status::VALID && !inf.inner().eof()) {
    report = Report(Report::Status::INVALID, "Extra content in the input file");
  }

  if (report.status == Report::Status::VALID) {
    reporter->attach_trait_status(detail::validate_traits(traits_, trait_edges_));

    if (inf.get_trace_level() >= trace::Level::FULL) {
      reporter->attach_trace_tree(inf.get_trace_tree());
    }
  }

  auto exit_code = reporter->report(report);
  std::exit(exit_code);
}

inline auto State::quit_valid() -> void { quit(Report(Report::Status::VALID, "")); }

inline auto State::quit_invalid(std::string_view message) -> void {
  quit(Report(Report::Status::INVALID, std::string(message)));
}
// /Impl State }}}

namespace detail {
inline auto collect_args(int argc, char **argv) -> std::vector<std::string> {
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }
  return args;
}
}  // namespace detail

template <class Input, class TraitsFunc>
auto run_validator(int argc, char **argv, std::unique_ptr<Initializer> initializer,
                   TraitsFunc traits_func) -> int {
  static_assert(var::Readable<Input>, "Input should be Readable");

  auto args = detail::collect_args(argc, argv);

  /* std::exit only destroys static objects */
  static auto state = State(std::move(initializer));
  state.initializer->init(argv[0], args);

  Input input{state.inf.read(var::ExtVar<Input>("input"))};
  state.traits(traits_func(input));
  state.quit_valid();
  return 0;
}

// Impl DefaultInitializer {{{
namespace detail {
constexpr std::string_view ARGS_USAGE =
    "[<input_file>] [--report-format={auto|json|text}] [--reader-trace-level={0|1|2}]";

inline auto print_help_message(std::string_view program_name) -> void {
  std::string msg =
      cplib::format(CPLIB_STARTUP_TEXT
                    "Usage:\n"
                    "  {} {}\n"
                    "\n"
                    "If <input_file> does not exist, stdin will be used as input\n"
                    "\n"
                    "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
                    "enable colors",
                    program_name, ARGS_USAGE);
  panic(msg);
}

inline auto detect_reporter(State &state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = std::make_unique<JsonReporter>();
  } else if (cplib::detail::has_colors()) {
    state.reporter = std::make_unique<ColoredTextReporter>();
  } else {
    state.reporter = std::make_unique<PlainTextReporter>();
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State &state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = std::make_unique<JsonReporter>();
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = std::make_unique<ColoredTextReporter>();
    } else {
      state.reporter = std::make_unique<PlainTextReporter>();
    }
  } else {
    return false;
  }
  return true;
}
}  // namespace detail

inline auto DefaultInitializer::init(std::string_view arg0, const std::vector<std::string> &args)
    -> void {
  auto &state = this->state();

  detail::detect_reporter(state);

  auto parsed_args = cmd_args::ParsedArgs(args);
  auto reader_trace_level = trace::Level::STACK_ONLY;

  for (const auto &[key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(state, value)) {
        panic(cplib::format("Unknown {} option: {}", key, value));
      }
    } else if (key == "reader-trace-level") {
      auto level = var::u8("reader-trace-level", static_cast<std::uint8_t>(trace::Level::NONE),
                           static_cast<std::uint8_t>(trace::Level::FULL))
                       .parse(value);
      reader_trace_level = trace::Level(level);
    } else {
      panic("Unknown command-line argument variable: " + key);
    }
  }

  for (const auto &flag : parsed_args.flags) {
    if (flag == "help") {
      detail::print_help_message(arg0);
    } else {
      panic("Unknown command-line argument flag: " + flag);
    }
  }

  if (parsed_args.ordered.size() > 1) {
    panic("Program must be run with the following arguments:\n  " +
          std::string(detail::ARGS_USAGE));
  }

  if (parsed_args.ordered.empty()) {
    set_inf_fileno(fileno(stdin), reader_trace_level);
  } else {
    set_inf_path(parsed_args.ordered[0], reader_trace_level);
  }
}
// /Impl DefaultInitializer }}}

// Impl reporters {{{
namespace detail {
inline auto status_to_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "Internal Error";
    case Report::Status::VALID:
      return "Valid";
    case Report::Status::INVALID:
      return "Invalid";
    default:
      panic(cplib::format("Unknown validator report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto status_to_colored_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "\x1b[0;35mInternal Error\x1b[0m";
    case Report::Status::VALID:
      return "\x1b[0;32mValid\x1b[0m";
    case Report::Status::INVALID:
      return "\x1b[0;31mInvalid\x1b[0m";
    default:
      panic(cplib::format("Unknown validator report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto trait_status_to_json(const std::map<std::string, bool> &traits) -> json::Value {
  json::Map map;
  for (const auto &[k, v] : traits) {
    map.emplace(k, json::Value(v));
  }
  return json::Value(map);
}

inline auto print_trace_tree(const trace::TraceTreeNode<var::ReaderTrace> *node, std::size_t depth,
                             std::size_t &n_remaining_node, bool colored_output, std::ostream &os)
    -> void {
  if (!node || depth >= 8 || (node->tags.contains("#hidden"))) {
    return;
  }

  if (depth) {
    --n_remaining_node;

    // indentation
    for (std::size_t i = 0; i < depth - 1; ++i) {
      os << "  ";
    }
    os << "- ";

    // name
    if (colored_output) {
      os << "\x1b[0;33m";
    }
    os << node->trace.var_name;
    if (colored_output) {
      os << "\x1b[0m";
    }

    // value
    if (node->tags.contains("#v")) {
      os << " = " << cplib::compress(node->tags.at("#v").to_string());
    }
    os << '\n';
  }

  std::size_t n_visible_children = 0;
  for (const auto &child : node->get_children()) {
    if (!child->tags.contains("#hidden")) {
      ++n_visible_children;
    }
  }

  for (const auto &child : node->get_children()) {
    if (child->tags.contains("#hidden")) {
      continue;
    }
    if (!n_remaining_node) {
      for (std::size_t i = 0; i < depth; ++i) {
        os << "  ";
      }
      os << "- ... and " << n_visible_children << " more\n";
      break;
    }
    --n_visible_children;
    print_trace_tree(child.get(), depth + 1, n_remaining_node, colored_output, os);
  }
}
}  // namespace detail

inline auto JsonReporter::report(const Report &report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"message", json::Value(report.message)},
  };

  if (!trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(trace_stacks_.size());
    std::ranges::transform(trace_stacks_, std::back_inserter(trace_stacks),
                           [](auto &s) -> json::Value { return json::Value(s.to_json()); });
    map.emplace("reader_trace_stacks", trace_stacks);
  }

  if (!trait_status_.empty()) {
    map.emplace("traits", detail::trait_status_to_json(trait_status_));
  }

  if (trace_tree_) {
    if (auto json = trace_tree_->to_json(); json.has_value()) {
      map.emplace("reader_trace_tree", std::move(*json));
    }
  }

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report &report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << detail::status_to_title_string(report.status) << ".\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto &stack : trace_stacks_) {
      for (const auto &line : stack.to_plain_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (report.status == Report::Status::VALID && !trait_status_.empty()) {
    stream << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (const auto &[name, satisfaction] : trait_status_) {
      if (satisfaction) {
        satisfied.emplace_back(name);
      } else {
        dissatisfied.emplace_back(name);
      }
    }

    for (const auto &name : satisfied) {
      stream << "+ " << cplib::detail::hex_encode(name) << '\n';
    }
    for (const auto &name : dissatisfied) {
      stream << "- " << cplib::detail::hex_encode(name) << '\n';
    }
  }

  if (trace_tree_) {
    stream << "\nReader trace tree:\n";
    std::size_t n_remaining_node = 32;
    detail::print_trace_tree(trace_tree_, 0, n_remaining_node, false, stream);
  }

  return report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report &report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << detail::status_to_colored_title_string(report.status) << ".\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto &stack : trace_stacks_) {
      for (const auto &line : stack.to_colored_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (report.status == Report::Status::VALID && !trait_status_.empty()) {
    stream << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (const auto &[name, satisfaction] : trait_status_) {
      if (satisfaction) {
        satisfied.emplace_back(name);
      } else {
        dissatisfied.emplace_back(name);
      }
    }

    for (const auto &name : satisfied) {
      stream << "\x1b[0;32m+\x1b[0m " << name << '\n';
    }
    for (const auto &name : dissatisfied) {
      stream << "\x1b[0;31m-\x1b[0m " << name << '\n';
    }
  }

  if (trace_tree_) {
    stream << "\nReader trace tree:\n";
    std::size_t n_remaining_node = 32;
    detail::print_trace_tree(trace_tree_, 0, n_remaining_node, true, stream);
  }

  return report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE;
}
// /Impl reporters }}}
}  // namespace cplib::validator
// --- End embedded: validator.i.hpp ---

#endif
// --- End embedded: validator.hpp ---
// --- Start embedded: generator.hpp ---
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

#ifndef CPLIB_GENERATOR_HPP_
#define CPLIB_GENERATOR_HPP_

#include <any>  
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>


namespace cplib::generator {
/**
 * `Report` represents a report showing when generator exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  struct Status {
   public:
    enum Value : std::uint8_t {
      /// Indicates an internal error occurred.
      INTERNAL_ERROR,
      /// Indicates the program runs normally.
      OK,
    };

    /**
     * Default constructor for Status.
     */
    Status() = default;

    /**
     * Constructor for Status with a given value.
     *
     * @param value The value of the status.
     */
    constexpr Status(Value value);  // NOLINT(google-explicit-constructor)

    /**
     * Implicit conversion operator to Value.
     *
     * @return The value of the status.
     */
    constexpr operator Value() const;  // NOLINT(google-explicit-constructor)

    /**
     * Deleted conversion operator to bool.
     *
     * @return Deleted conversion operator to bool.
     */
    explicit operator bool() const = delete;

    /**
     * Get the string representation of the status.
     *
     * @return The string representation of the status.
     */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view;

   private:
    Value value_;
  };

  /// The status of the report.
  Status status;

  /// The message associated with the report.
  std::string message;

  /**
   * Constructor for Report.
   * @param status The status of the report.
   * @param message The message associated with the report.
   */
  Report(Status status, std::string message);
};

struct State;

/**
 * `Initializer` used to initialize the state.
 */
struct Initializer {
 public:
  virtual ~Initializer() = 0;

  auto set_state(State &state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string> &args) -> void = 0;

 protected:
  auto state() -> State &;

 private:
  State *state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report &report) -> int = 0;
};

struct State {
 public:
  /// The parser function of a flag type (`--flag`) command-line argument.
  using FlagParser = std::function<auto(std::set<std::string> flag_args)->void>;

  /// The parser function of a variable type (`--var=value`) command-line argument.
  using VarParser = std::function<auto(std::map<std::string, std::string> var_args)->void>;

  /// Random number generator.
  Random rnd;

  /// Initializer parses command-line arguments and initializes `generator::State`
  std::unique_ptr<Initializer> initializer;

  /// Reporter reports the given `generator::Report` and exits the program.
  std::unique_ptr<Reporter> reporter;

  /// Names of the flag type (`--flag`) command-line arguments required by the generator.
  std::vector<std::string> required_flag_args;

  /// Names of the variable type (`--var=value`) command-line arguments required by the generator.
  std::vector<std::string> required_var_args;

  /// Functions to parse flag type command-line arguments.
  std::vector<FlagParser> flag_parsers;

  /// Functions to parse variable type command-line arguments.
  std::vector<VarParser> var_parsers;

  /**
   * Constructs a new `State` object with the given initializer.
   *
   * @param initializer The initializer.
   */
  explicit State(std::unique_ptr<Initializer> initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Quits the generator with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(const Report &report) -> void;

  /**
   * Quits the generator with the `report::Status::OK` status.
   */
  [[noreturn]] auto quit_ok() -> void;

 private:
  /// Whether the program has exited.
  bool exited_{false};
};

/**
 * Default initializer of generator.
 */
struct DefaultInitializer : Initializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param arg0 The name of the program.
   * @param args The command-line arguments.
   */
  auto init(std::string_view arg0, const std::vector<std::string> &args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report &report) -> int override;
};

struct ArgumentsContext {
  State *state{};
  std::map<std::string, std::any> value_map{};

  explicit ArgumentsContext(State &state);
};

struct ArgValueTag {};

auto set_active_arguments_context(ArgumentsContext &context) -> void;
auto active_arguments_context() -> ArgumentsContext &;

struct FlagArg {
  using ResultType = bool;

  std::string name;
  ArgumentsContext *context;

  explicit FlagArg(std::string name);
  [[nodiscard]] auto operator|(ArgValueTag) const -> const ResultType &;
};

template <class T>
struct VarArg {
  using ResultType = typename T::Target;

  T var;
  ArgumentsContext *context;

  template <class... Args>
  explicit VarArg(Args &&...args);

  [[nodiscard]] auto operator|(ArgValueTag) const -> const ResultType &;
};

using MainFunc = auto (*)() -> void;

auto run_generator(State &state, int argc, char **argv, MainFunc main_func) -> int;

#define CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_)                              \
  auto cplib_generator_args_context_ = ::cplib::generator::ArgumentsContext(state_var_name_); \
  namespace cplib_generator_args_ {                                                           \
  using ::cplib::generator::FlagArg;                                                          \
  using ::cplib::generator::ArgValueTag;                                                      \
  template <class T>                                                                          \
  using Var = ::cplib::generator::VarArg<T>;                                                  \
  using Flag = ::cplib::generator::FlagArg;                                                   \
  }

#define CPLIB_REGISTER_GENERATOR_ARG_(arg)             \
  namespace cplib_generator_args_ {                    \
  const auto &arg | ::cplib::generator::ArgValueTag{}; \
  }

#define CPLIB_REGISTER_GENERATOR_ARGS_DEFER_(id) id CPLIB_REGISTER_GENERATOR_ARGS_EMPTY_()
#define CPLIB_REGISTER_GENERATOR_ARGS_EMPTY_()
#define CPLIB_REGISTER_GENERATOR_ARGS_AGAIN_() CPLIB_REGISTER_GENERATOR_ARGS_FOR_EACH_
#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_(...) __VA_ARGS__
#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_(...)                                  \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_(                                            \
      CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_(CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_( \
          CPLIB_REGISTER_GENERATOR_ARGS_EXPAND1_(__VA_ARGS__))))
#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_(...)                                  \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_(                                            \
      CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_(CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_( \
          CPLIB_REGISTER_GENERATOR_ARGS_EXPAND2_(__VA_ARGS__))))
#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND4_(...)                                  \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_(                                            \
      CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_(CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_( \
          CPLIB_REGISTER_GENERATOR_ARGS_EXPAND3_(__VA_ARGS__))))

#define CPLIB_REGISTER_GENERATOR_ARGS_FOR_EACH_(arg, ...) \
  CPLIB_REGISTER_GENERATOR_ARG_(arg)                      \
  __VA_OPT__(                                             \
      CPLIB_REGISTER_GENERATOR_ARGS_DEFER_(CPLIB_REGISTER_GENERATOR_ARGS_AGAIN_)()(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_(...)          \
  __VA_OPT__(CPLIB_REGISTER_GENERATOR_ARGS_EXPAND4_( \
      CPLIB_REGISTER_GENERATOR_ARGS_FOR_EACH_(__VA_ARGS__)))

/**
 * Macro to register generator with custom initializer.
 *
 * @param state_var_name_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 * @param args_namespace_name_ The name of the command-line arguments namespace.
 * @param ... The parsers of the command-line arguments.
 */
#define CPLIB_REGISTER_GENERATOR_OPT(state_var_name_, initializer_, args_namespace_name_, ...) \
  auto generator_main() -> void;                                                               \
  auto state_var_name_ = ::cplib::generator::State(                                            \
      ::std::unique_ptr<::cplib::generator::Initializer>(new initializer_));                   \
  CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_);                                    \
  CPLIB_REGISTER_GENERATOR_ARGS_(__VA_ARGS__);                                                 \
  namespace args_namespace_name_ = ::cplib_generator_args_;                                    \
  auto main(int argc, char **argv) -> int {                                                    \
    return ::cplib::generator::run_generator(state_var_name_, argc, argv, generator_main);     \
  }

#ifndef CPLIB_GENERATOR_DEFAULT_INITIALIZER
#define CPLIB_GENERATOR_DEFAULT_INITIALIZER ::cplib::generator::DefaultInitializer()
#endif

/**
 * Macro to register generator with default initializer.
 *
 * @param var_name_ The variable name of state object to be initialized.
 * @param args_namespace_name_ The name of the command-line arguments namespace.
 * @param ... The parsers of the command-line arguments.
 */
#define CPLIB_REGISTER_GENERATOR(var_name_, args_namespace_name_, ...)         \
  CPLIB_REGISTER_GENERATOR_OPT(var_name_, CPLIB_GENERATOR_DEFAULT_INITIALIZER, \
                               args_namespace_name_ __VA_OPT__(, ) __VA_ARGS__)
}  // namespace cplib::generator

// --- Start embedded: generator.i.hpp ---
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


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace cplib::generator {
inline constexpr Report::Status::Status(Value value) : value_(value) {}

inline constexpr Report::Status::operator Value() const { return value_; }

inline constexpr auto Report::Status::to_string() const -> std::string_view {
  switch (value_) {
    case INTERNAL_ERROR:
      return "internal_error";
    case OK:
      return "ok";
    default:
      panic(cplib::format("Unknown generator report status: {}", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, std::string message)
    : status(status), message(std::move(message)) {}

inline Initializer::~Initializer() = default;

inline auto Initializer::set_state(State &state) -> void { state_ = &state; };

inline auto Initializer::state() -> State & { return *state_; };

inline Reporter::~Reporter() = default;

// Impl State {{{
inline State::State(std::unique_ptr<Initializer> initializer)
    : rnd(),
      initializer(std::move(initializer)),
      reporter(std::make_unique<JsonReporter>()),
      required_flag_args(),
      required_var_args(),
      flag_parsers(),
      var_parsers() {
  this->initializer->set_state(*this);
  cplib::detail::panic_impl = [this](std::string_view msg) -> void {
    quit(Report(Report::Status::INTERNAL_ERROR, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::GENERATOR;
}

inline State::~State() {
  if (!exited_) panic("Generator must exit by calling method `State::quit*`");
}

inline auto State::quit(const Report &report) -> void {
  exited_ = true;

  auto exit_code = reporter->report(report);
  std::exit(exit_code);
}

inline auto State::quit_ok() -> void { quit(Report(Report::Status::OK, "")); }
// /Impl State }}}

inline ArgumentsContext::ArgumentsContext(State &state) : state(&state), value_map() {
  set_active_arguments_context(*this);
}

namespace detail {
inline ArgumentsContext *active_arguments_context_{};
}

inline auto set_active_arguments_context(ArgumentsContext &context) -> void {
  detail::active_arguments_context_ = &context;
}

inline auto active_arguments_context() -> ArgumentsContext & {
  if (!detail::active_arguments_context_) {
    panic("Generator arguments context is not initialized");
  }
  return *detail::active_arguments_context_;
}

inline FlagArg::FlagArg(std::string name)
    : name(std::move(name)), context(&active_arguments_context()) {
  context->state->required_flag_args.emplace_back(this->name);
  auto arg_name = this->name;
  auto *arg_context = context;
  context->state->flag_parsers.emplace_back(
      [arg_name, arg_context](const std::set<std::string> &flag_args) -> void {
        *std::any_cast<ResultType>(&arg_context->value_map[arg_name]) =
            static_cast<ResultType>(flag_args.count(arg_name));
      });
}

inline auto FlagArg::operator|(ArgValueTag) const -> const ResultType & {
  return *std::any_cast<ResultType>(&(context->value_map[name] = ResultType{}));
}

template <class T>
template <class... Args>
VarArg<T>::VarArg(Args &&...args)
    : var(std::forward<Args>(args)...), context(&active_arguments_context()) {
  context->state->required_var_args.emplace_back(var.name());
  auto arg_var = this->var;
  auto *arg_context = context;
  context->state->var_parsers.emplace_back(
      [arg_var, arg_context](const std::map<std::string, std::string> &var_args) -> void {
        auto arg_name = std::string(arg_var.name());
        *std::any_cast<ResultType>(&arg_context->value_map[arg_name]) =
            arg_var.parse(var_args.at(arg_name));
      });
}

template <class T>
auto VarArg<T>::operator|(ArgValueTag) const -> const ResultType & {
  return *std::any_cast<ResultType>(&(context->value_map[std::string(var.name())] = ResultType{}));
}

namespace detail {
inline auto collect_args(int argc, char **argv) -> std::vector<std::string> {
  std::vector<std::string> args;
  args.reserve(argc);
  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }
  return args;
}
}  // namespace detail

inline auto run_generator(State &state, int argc, char **argv, MainFunc main_func) -> int {
  auto args = detail::collect_args(argc, argv);
  state.initializer->init(argv[0], args);
  main_func();
  return 0;
}

// Impl DefaultInitializer {{{
namespace detail {
inline auto parse_arg(std::string_view arg) -> std::pair<std::string, std::optional<std::string>> {
  constexpr std::string_view prefix = "--";
  if (arg.compare(0, prefix.size(), prefix)) panic("Unknown option: " + std::string(arg));
  arg.remove_prefix(prefix.size());

  auto assign_pos = arg.find_first_of('=');
  if (assign_pos == std::string_view::npos) return {std::string(arg), std::nullopt};
  return {std::string(arg.substr(0, assign_pos)), std::string(arg.substr(assign_pos + 1))};
}

inline auto print_help_message(std::string_view program_name, std::string_view args_usage) -> void {
  std::string msg =
      cplib::format(CPLIB_STARTUP_TEXT
                    "\n"
                    "Usage:\n"
                    "  {} {}\n"
                    "\n"
                    "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
                    "enable colors",
                    program_name, args_usage);
  panic(msg);
}

inline auto detect_reporter(State &state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = std::make_unique<JsonReporter>();
  } else if (cplib::detail::has_colors()) {
    state.reporter = std::make_unique<ColoredTextReporter>();
  } else {
    state.reporter = std::make_unique<PlainTextReporter>();
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State &state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = std::make_unique<JsonReporter>();
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = std::make_unique<ColoredTextReporter>();
    } else {
      state.reporter = std::make_unique<PlainTextReporter>();
    }
  } else {
    return false;
  }
  return true;
}

inline auto validate_required_arguments(const State &state,
                                        const std::map<std::string, std::string> &var_args)
    -> void {
  for (const auto &var : state.required_var_args) {
    if (!var_args.count(var)) panic("Missing variable: " + var);
  }
}

inline auto get_args_usage(const State &state) {
  std::vector<std::string> builder;
  builder.reserve(state.required_flag_args.size() + state.required_var_args.size());
  for (const auto &arg : state.required_flag_args) builder.emplace_back("[--" + arg + "]");
  for (const auto &arg : state.required_var_args) builder.emplace_back("--" + arg + "=<value>");
  builder.emplace_back("[--report-format={auto|json|text}]");

  return join(builder.begin(), builder.end(), ' ');
}

inline auto set_binary_mode() {
  /*
    We recommend using binary mode on Windows. However, Codeforces Polygon doesn’t think so.
    Since the only Online Judge that uses Windows seems to be Codeforces, make it happy.
  */
#if defined(ON_WINDOWS) && !defined(ONLINE_JUDGE)
  _setmode(fileno(stdout), _O_BINARY);
#endif
}
}  // namespace detail

inline auto DefaultInitializer::init(std::string_view arg0, const std::vector<std::string> &args)
    -> void {
  auto &state = this->state();

  detail::detect_reporter(state);

  // required args are initially unordered, sort them to ensure subsequent binary_search is correct
  std::ranges::sort(state.required_flag_args);
  std::ranges::sort(state.required_var_args);

  auto parsed_args = cmd_args::ParsedArgs(args);
  auto args_usage = detail::get_args_usage(state);
  std::set<std::string> flag_args;
  std::map<std::string, std::string> var_args;

  for (const auto &[key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(state, value)) {
        panic(cplib::format("Unknown {} option: {}", key, value));
      }
    } else {
      if (!std::ranges::binary_search(state.required_var_args, key)) {
        panic("Unknown command-line argument variable: " + key);
      }
      if (auto it = var_args.find(key); it != var_args.end()) {
        it->second.push_back(' ');
        it->second += value;
      } else {
        var_args[key] = value;
      }
    }
  }

  for (const auto &flag : parsed_args.flags) {
    if (flag == "help") {
      detail::print_help_message(arg0, args_usage);
    } else {
      if (!std::ranges::binary_search(state.required_flag_args, flag)) {
        panic("Unknown command-line argument flag: " + flag);
      }
      flag_args.emplace(flag);
    }
  }

  detail::validate_required_arguments(state, var_args);

  for (const auto &parser : state.flag_parsers) parser(flag_args);
  for (const auto &parser : state.var_parsers) parser(var_args);

  // Unsynchronize to speed up std::cout output.
  std::ios_base::sync_with_stdio(false);

  detail::set_binary_mode();

  state.rnd.reseed(args);
}
// /Impl DefaultInitializer }}}

// Impl reporters {{{
namespace detail {
inline auto status_to_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "Internal Error";
    case Report::Status::OK:
      return "OK";
    default:
      panic(cplib::format("Unknown generator report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto status_to_colored_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "\x1b[0;35mInternal Error\x1b[0m";
    case Report::Status::OK:
      return "\x1b[0;32mOK\x1b[0m";
    default:
      panic(cplib::format("Unknown generator report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto JsonReporter::report(const Report &report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"message", json::Value(report.message)},
  };

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report &report) -> int {
  std::ostream stream(std::clog.rdbuf());

  if (report.status == Report::Status::OK && report.message.empty()) {
    // Do nothing when the report is OK and message is empty.
    std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  stream << detail::status_to_title_string(report.status) << ".\n" << report.message << '\n';

  return report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report &report) -> int {
  std::ostream stream(std::clog.rdbuf());

  if (report.status == Report::Status::OK && report.message.empty()) {
    // Do nothing when the report is OK and message is empty.
    std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  stream << detail::status_to_colored_title_string(report.status) << ".\n"
         << report.message << '\n';

  return report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
// /Impl reporters }}}
}  // namespace cplib::generator
// --- End embedded: generator.i.hpp ---

#endif
// --- End embedded: generator.hpp ---

// clang-format on

#endif  // CPLIB_HPP_
