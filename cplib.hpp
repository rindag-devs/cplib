/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_HPP_
#define CPLIB_HPP_

// clang-format off

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_MACROS_HPP_
#define CPLIB_MACROS_HPP_

#define CPLIB_VERSION "0.0.1-SNAPSHOT"

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
#define ON_WINDOWS
#if defined(_MSC_VER) && _MSC_VER > 1400
#pragma warning(disable : 4127)
#pragma warning(disable : 4146)
#pragma warning(disable : 4458)
#endif
#else
#include <unistd.h>  
#endif

#if defined(__GNUC__)
#define CPLIB_PRINTF_LIKE(n, m) __attribute__((format(printf, n, m)))
#else
#define CPLIB_PRINTF_LIKE(n, m) /* If only */
#endif                          /* __GNUC__ */

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_UTILS_HPP_
#define CPLIB_UTILS_HPP_

#include <iosfwd>       // for nullptr_t
#include <memory>       // for unique_ptr
#include <string>       // for string
#include <string_view>  // for string_view
#include <vector>       // for vector


  // for CPLIB_PRINTF_LIKE


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
 * Format string using printf-like syntax.
 *
 * @param fmt The format string.
 * @param ... The variadic arguments to be formatted.
 * @return The formatted string.
 */
CPLIB_PRINTF_LIKE(1, 2) auto format(const char *fmt, ...) -> std::string;

/**
 * Determine whether the two floating-point values are equals within the accuracy range.
 *
 * @tparam T The type of the values.
 * @param expected The expected floating-point value.
 * @param result The actual floating-point value.
 * @param max_err The maximum allowable error.
 * @return True if the values are equal within the accuracy range, false otherwise.
 */
template <class T>
auto float_equals(T expected, T result, T max_err) -> bool;

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
 * Concatenate the values between [first,last) into a string through separator.
 *
 * @tparam It The type of the iterator.
 * @param first Iterator to the first value.
 * @param last Iterator to the last value (exclusive).
 * @param separator The separator character.
 * @return The concatenated string.
 */
template <class It>
auto join(It first, It last, char separator) -> std::string;

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
class UniqueFunction;

/**
 * Template specialization for Ret(Args...) type.
 *
 * @tparam Ret The return type of the stored function.
 * @tparam Args The argument types of the stored function.
 */
template <typename Ret, typename... Args>
class UniqueFunction<Ret(Args...)> {
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
 * `WorkMode` indicates the current mode of cplib.
 */
enum class WorkMode {
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

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_UTILS_HPP_
#error "Must be included from utils.hpp"
#endif
#endif


#include <algorithm>    // for min, max
#include <cctype>       // for isspace, isprint
#include <cmath>        // for isinf, isnan
#include <cstdarg>      // for va_list, va_end, va_copy, va_start
#include <cstdio>       // for vsnprintf, fileno, stderr
#include <cstdlib>      // for getenv, abs, exit, EXIT_FAILURE
#include <iostream>     // for basic_ostream, operator<<, clog
#include <memory>       // for allocator, make_unique
#include <string>       // for basic_string, string, char_traits, operator+
#include <string_view>  // for string_view, operator<<, basic_string_view
#include <utility>      // for forward, move
#include <vector>       // for vector


  // for isatty, CPLIB_PRINTF_LIKE


namespace cplib {
namespace detail {
inline auto has_colors() -> bool {
  // https://bixense.com/clicolors/
  if (std::getenv("NO_COLOR") != nullptr) return false;
  if (std::getenv("CLICOLOR_FORCE") != nullptr) return true;
  return isatty(fileno(stderr));
}

inline auto json_string_encode(int c) -> std::string {
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
    return format("\\u%04x", static_cast<int>(c));
  } else {
    return {static_cast<char>(c)};
  }
}

inline auto json_string_encode(std::string_view s) -> std::string {
  std::string result;
  for (auto c : s) result += json_string_encode(c);
  return result;
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
    return format("\\x%02x", static_cast<int>(c));
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
  std::clog << "Unrecoverable error: " << s << '\n';
  exit(EXIT_FAILURE);
};
}  // namespace detail

inline auto panic(std::string_view message) -> void {
  detail::panic_impl(message);
  exit(EXIT_FAILURE);  // Usually unnecessary, but in special cases to prevent problems.
}
// /Impl panic }}}

// Impl format {{{
namespace detail {
inline auto string_vsprintf(const char* format, std::va_list args) -> std::string {
  std::va_list tmp_args;    // unfortunately you cannot consume a va_list twice
  va_copy(tmp_args, args);  // so we have to copy it
  const int required_len = std::vsnprintf(nullptr, 0, format, tmp_args);
  va_end(tmp_args);

  std::string buf(required_len, '\0');
  if (std::vsnprintf(&buf[0], required_len + 1, format, args) < 0) {
    panic("string_vsprintf encoding error");
    return "";
  }
  return buf;
}
}  // namespace detail

CPLIB_PRINTF_LIKE(1, 2) inline auto format(const char* fmt, ...) -> std::string {
  std::va_list args;
  va_start(args, fmt);
  std::string str{detail::string_vsprintf(fmt, args)};
  va_end(args);
  return str;
}
// /Impl format }}}

template <class T>
inline auto float_equals(T expected, T result, T max_err) -> bool {
  if (bool x_nan = std::isnan(expected), y_nan = std::isnan(result); x_nan || y_nan) {
    return x_nan && y_nan;
  }
  if (bool x_inf = std::isinf(expected), y_inf = std::isinf(result); x_inf || y_inf) {
    return x_inf && y_inf && (expected > 0) == (result > 0);
  }

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

template <typename It>
inline auto join(It first, It last, char separator) -> std::string {
  std::string result;
  bool repeated = false;
  for (It i = first; i != last; ++i) {
    if (repeated) {
      result.push_back(separator);
    } else {
      repeated = true;
    }
    result += *i;
  }
  return result;
}

inline auto split(std::string_view s, char separator) -> std::vector<std::string> {
  std::vector<std::string> result;
  std::string item;
  for (char i : s) {
    if (i == separator) {
      result.push_back(item);
      item.clear();
    } else {
      item.push_back(i);
    }
  }
  result.push_back(item);
  return result;
}

inline auto tokenize(std::string_view s, char separator) -> std::vector<std::string> {
  std::vector<std::string> result;
  std::string item;
  for (char i : s) {
    if (i == separator) {
      if (!item.empty()) result.push_back(item);
      item.clear();
    } else {
      item.push_back(i);
    }
  }
  if (!item.empty()) result.push_back(item);
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

// Impl get_work_mode {{{
namespace detail {
inline WorkMode work_mode = WorkMode::NONE;
}

inline auto get_work_mode() -> WorkMode { return detail::work_mode; }
// /Impl get_work_mode }}}
}  // namespace cplib
  

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_NO_RAND_HPP_
#define CPLIB_NO_RAND_HPP_



#include <cstdlib>  // for rand, srand


  // for panic


#ifdef __GLIBC__
#define CPLIB_RAND_THROW_STATEMENT noexcept(true)
#else
#define CPLIB_RAND_THROW_STATEMENT
#endif

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((error("Don not use `rand`, use `rnd.next` instead")))
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4273)
#endif
inline auto
rand() CPLIB_RAND_THROW_STATEMENT->int {
  cplib::panic("Don not use `rand`, use `rnd.next` instead");
}

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((error("Don not use `srand`, you should use `cplib::Random` for random generator")))
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4273)
#endif
inline auto
srand(unsigned int) CPLIB_RAND_THROW_STATEMENT->void {
  cplib::panic("Don not use `srand`, you should use `cplib::Random` for random generator");
}

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_PATTERN_HPP_
#define CPLIB_PATTERN_HPP_

#include <regex.h>  // for regex_t

#include <memory>       // for shared_ptr
#include <string>       // for string, basic_string
#include <string_view>  // for string_view
#include <utility>      // for pair

namespace cplib {
/**
 * Regex pattern in POSIX-Extended style. Used for matching strings.
 *
 * Format see
 * `https://en.wikibooks.org/wiki/Regular_Expressions/POSIX-Extended_Regular_Expressions`.
 */
class Pattern {
 public:
  /**
   * Create pattern instance by string.
   *
   * @param src The source string representing the regex pattern.
   */
  explicit Pattern(std::string src);

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
  std::string src_;

  // `re->second` represents whether regex is compiled successfully
  std::shared_ptr<std::pair<regex_t, bool>> re_;
};
}  // namespace cplib

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_PATTERN_HPP_
#error "Must be included from pattern.hpp"
#endif
#endif


#include <regex.h>  // for regex_t, regerror, regcomp, regexec, regfree

#include <cstddef>      // for size_t
#include <memory>       // for allocator, __shared_ptr_access, shared_ptr
#include <string>       // for basic_string, char_traits, operator+, operato...
#include <string_view>  // for string_view
#include <utility>      // for pair, move


  // for panic


namespace cplib {
namespace detail {
inline auto get_regex_err_msg(int err_code, regex_t* re) -> std::string {
  std::size_t len = regerror(err_code, re, nullptr, 0);
  std::string buf(len, 0);
  regerror(err_code, re, buf.data(), len);
  return buf;
}
}  // namespace detail

inline Pattern::Pattern(std::string src)
    : src_(std::move(src)), re_(new std::pair<regex_t, bool>, [](std::pair<regex_t, bool>* p) {
        if (p->second) regfree(&p->first);
        delete p;
      }) {
  using namespace std::string_literals;
  // In function `regexec`, a match anywhere within the string is considered successful unless the
  // regular expression contains `^` or `$`.
  if (int err = regcomp(&re_->first, ("^"s + src_ + "$"s).c_str(), REG_EXTENDED | REG_NOSUB); err) {
    auto err_msg = detail::get_regex_err_msg(err, &re_->first);
    panic("pattern constructor failed: "s + err_msg);
  }
  re_->second = true;
}

inline auto Pattern::match(std::string_view s) const -> bool {
  using namespace std::string_literals;

  int result = regexec(&re_->first, s.data(), 0, nullptr, 0);

  if (!result) return true;

  if (result == REG_NOMATCH) return false;

  auto err_msg = detail::get_regex_err_msg(result, &re_->first);
  panic("pattern match failed: "s + err_msg);
  return false;
}

inline auto Pattern::src() const -> std::string_view { return src_; }
}  // namespace cplib
  

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_RANDOM_HPP_
#define CPLIB_RANDOM_HPP_

#include <cstdint>           // for uint32_t
#include <initializer_list>  // for initializer_list
#include <random>            // for mt19937_64
#include <type_traits>       // for enable_if

namespace cplib {
/**
 * Random number generator that provides various methods to generate random numbers and perform
 * random operations.
 */
class Random {
 public:
  using Engine = std::mt19937_64;

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
   * Construct a random generator with seed which generated by `argc` and `argv`
   *
   * @param argc The number of command-line arguments.
   * @param argv The array of command-line arguments.
   */
  explicit Random(int argc, char** argv);

  /**
   * Reseed the generator with a new seed.
   *
   * @param seed The new seed value.
   */
  auto reseed(std::uint64_t seed) -> void;

  /**
   * Reseed the generator with a seed generated by `argc` and `argv`.
   *
   * @param argc The number of command-line arguments.
   * @param argv The array of command-line arguments.
   */
  auto reseed(int argc, char** argv) -> void;

  /**
   * Get the engine used by the random generator.
   *
   * @return A reference to the engine.
   */
  auto engine() -> Engine&;

  /**
   * Generate a random integer in the range [from, to].
   *
   * @tparam T The type of the integer.
   * @param from The lower bound of the range.
   * @param to The upper bound of the range.
   * @return The randomly generated integer.
   */
  template <class T>
  auto next(T from, T to) -> typename std::enable_if<std::is_integral_v<T>, T>::type;

  /**
   * Generate a random floating-point number in the range [from, to].
   *
   * @tparam T The type of the floating-point number.
   * @param from The lower bound of the range.
   * @param to The upper bound of the range.
   * @return The randomly generated floating-point number.
   */
  template <class T>
  auto next(T from, T to) -> typename std::enable_if<std::is_floating_point_v<T>, T>::type;

  /**
   * Generate a random boolean value.
   *
   * @tparam T The type of the boolean.
   * @return The randomly generated boolean value.
   */
  template <class T>
  auto next() -> typename std::enable_if<std::is_same_v<T, bool>, bool>::type;

  /**
   * Generate a random boolean value with a given probability of being true.
   *
   * @tparam T The type of the boolean.
   * @param true_prob The probability of the boolean being true.
   * @return The randomly generated boolean value.
   */
  template <class T>
  auto next(double true_prob) -> typename std::enable_if<std::is_same_v<T, bool>, bool>::type;

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
  template <class It>
  auto choice(It first, It last) -> It;

  /**
   * Return a random iterator from the given container.
   *
   * @tparam Container The type of the container.
   * @param container The container to choose from.
   * @return A random iterator from the container.
   */
  template <class Container>
  auto choice(Container& container) -> decltype(std::begin(container));

  /**
   * Return a random iterator from the given map container by utilizing the values of the map
   * container as weights for weighted random number generation.
   *
   * @tparam Map The type of the map container.
   * @param map The map container to choose from.
   * @return A random iterator from the map container.
   */
  template <class Map>
  auto weighted_choice(const Map& map) -> decltype(std::begin(map));

  /**
   * Shuffle the elements in the given range.
   *
   * @tparam RandomIt The type of the random access iterator.
   * @param first The beginning of the range.
   * @param last The end of the range.
   */
  template <class RandomIt>
  auto shuffle(RandomIt first, RandomIt last) -> void;

  /**
   * Shuffle the elements in the given container.
   *
   * @tparam Container The type of the container.
   * @param container The container to shuffle.
   */
  template <class Container>
  auto shuffle(Container& container) -> void;

 private:
  Engine engine_;
};
}  // namespace cplib

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_RANDOM_HPP_
#error "Must be included from random.hpp"
#endif
#endif


#include <algorithm>         // for shuffle
#include <cstdint>           // for uint64_t, uint32_t
#include <cstring>           // for strlen, size_t
#include <initializer_list>  // for initializer_list
#include <iterator>          // for distance, iterator_traits, next
#include <limits>            // for numeric_limits
#include <type_traits>       // for enable_if, make_unsigned





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
  using UnsignedT = typename std::make_unsigned<T>::type;

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

inline Random::Random(int argc, char** argv) : engine_() { reseed(argc, argv); }

inline auto Random::reseed(std::uint64_t seed) -> void { engine().seed(seed); }

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

  reseed(seed & ((1ull << 48) - 1));
}

inline auto Random::engine() -> Engine& { return engine_; }

template <class T>
inline auto Random::next(T from, T to) -> typename std::enable_if<std::is_integral_v<T>, T>::type {
  // Allow range from higher to lower
  if (from <= to) {
    return detail::rand_int_between<T>(engine(), from, to);
  }
  return detail::rand_int_between<T>(engine(), to, from);
}

template <class T>
inline auto Random::next(T from, T to) ->
    typename std::enable_if<std::is_floating_point_v<T>, T>::type {
  // Allow range from higher to lower
  if (from <= to) {
    return detail::rand_float_between<T>(engine(), from, to);
  }
  return detail::rand_float_between<T>(engine(), to, from);
}

template <class T>
inline auto Random::next() -> typename std::enable_if<std::is_same_v<T, bool>, bool>::type {
  return next<bool>(0.5);
}

template <class T>
inline auto Random::next(double true_prob) ->
    typename std::enable_if<std::is_same_v<T, bool>, bool>::type {
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
  

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_IO_HPP_
#define CPLIB_IO_HPP_

#include <cstdio>       // for size_t
#include <ios>          // for streambuf, basic_streambuf
#include <memory>       // for unique_ptr
#include <optional>     // for optional
#include <string>       // for string, basic_string
#include <string_view>  // for string_view


  // for UniqueFunction, UniqueFunction<>::UniqueFunct...


namespace cplib::io {
/**
 * An input stream class that provides various functionalities for reading and manipulating streams.
 */
class InStream {
 public:
  using FailFunc = UniqueFunction<auto(std::string_view)->void>;

  /**
   * Constructs an InStream object.
   *
   * @param buf A unique pointer to a stream buffer.
   * @param name The name of the stream.
   * @param strict Indicates if the stream is in strict mode.
   * @param fail_func A function that will be called when a failure occurs.
   */
  explicit InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict,
                    FailFunc fail_func);

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
   * Returns the current line number.
   *
   * @return The current line number as a size_t.
   */
  [[nodiscard]] auto line_num() const -> std::size_t;

  /**
   * Returns the current column number.
   *
   * @return The current column number as a size_t.
   */
  [[nodiscard]] auto col_num() const -> std::size_t;

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
   * Moves the pointer to the first non-space and non-tab character and calls [`eoln()`].
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
   * Ignores whitespaces in non-strict mode (strict mode is used in validators usually).
   *
   * @return The read token as a string.
   */
  auto read_token() -> std::string;

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

  /**
   * Quit program with an error.
   *
   * @param message The error message.
   */
  [[noreturn]] auto fail(std::string_view message) -> void;

 private:
  std::unique_ptr<std::streambuf> buf_;
  std::string name_;
  bool strict_;         // In strict mode, whitespace characters are not ignored
  FailFunc fail_func_;  // Calls when fail
  std::size_t line_num_{1};
  std::size_t col_num_{1};
};
}  // namespace cplib::io

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_IO_HPP_
#error "Must be included from io.hpp"
#endif
#endif


#include <array>        // for array
#include <cctype>       // for isspace
#include <cstdio>       // for EOF, size_t
#include <cstdlib>      // for exit, EXIT_FAILURE
#include <cstring>      // for memmove
#include <memory>       // for unique_ptr
#include <optional>     // for optional, nullopt
#include <string>       // for basic_string, string, char_traits
#include <string_view>  // for string_view
#include <utility>      // for move


  // for write, read
   // for UniqueFunction<>::operator(), format, panic


namespace cplib::io {
namespace detail {
// A stream buffer that writes on a file descriptor
//
// https://www.josuttis.com/cppcode/fdstream.html
class FdOutBuf : public std::streambuf {
 public:
  explicit FdOutBuf(int fd) : fd_(fd) {
#ifdef ON_WINDOWS
    _setmode(fd_, _O_BINARY);  // Sets file mode to binary
#endif
  }

 protected:
  // Write one character
  auto overflow(int_type c) -> int_type override {
    if (c != EOF) {
      auto z = static_cast<char>(c);
      if (write(fd_, &z, 1) != 1) {
        return EOF;
      }
    }
    return c;
  }
  // Write multiple characters
  auto xsputn(const char* s, std::streamsize num) -> std::streamsize override {
    return write(fd_, s, num);
  }

  int fd_;  // File descriptor
};

// A stream buffer that reads on a file descriptor
//
// https://www.josuttis.com/cppcode/fdstream.html
class FdInBuf : public std::streambuf {
 public:
  /**
   * Constructor
   * - Initialize file descriptor
   * - Initialize empty data buffer
   * - No putback area
   * => Force underflow()
   */
  explicit FdInBuf(int fd) : fd_(fd) {
#ifdef ON_WINDOWS
    _setmode(fd_, _O_BINARY);  // Sets file mode to binary
#endif
    setg(buf_.data() + PB_SIZE,   // Beginning of putback area
         buf_.data() + PB_SIZE,   // Read position
         buf_.data() + PB_SIZE);  // End position
  }

 protected:
  // Insert new characters into the buffer
  auto underflow() -> int_type override {
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
    std::ptrdiff_t num = read(fd_, buf_.data() + PB_SIZE, BUF_SIZE);
    if (num <= 0) {
      // Error or EOF
      return EOF;
    }

    // Reset buffer pointers
    setg(buf_.data() + (PB_SIZE - num_putback),  // Beginning of putback area
         buf_.data() + PB_SIZE,                  // Read position
         buf_.data() + PB_SIZE + num);           // End of buffer

    // Return next character
    return traits_type::to_int_type(*gptr());
  }

  int fd_;
  /**
   * Data buffer:
   * - At most, pbSize characters in putback area plus
   * - At most, bufSize characters in ordinary read buffer
   */
  static constexpr int PB_SIZE = 4;           // Size of putback area
  static constexpr int BUF_SIZE = 65536;      // Size of the data buffer
  std::array<char, BUF_SIZE + PB_SIZE> buf_;  // Data buffer
};
}  // namespace detail

inline InStream::InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict,
                          FailFunc fail_func)
    : buf_(std::move(buf)),
      name_(std::move(name)),
      strict_(strict),
      fail_func_(std::move(fail_func)) {}

inline auto InStream::name() const -> std::string_view { return name_; }

inline auto InStream::skip_blanks() -> void {
  while (true) {
    if (int c = seek(); c == EOF || !std::isspace(c)) break;
    read();
  }
}

inline auto InStream::seek() -> int { return buf_->sgetc(); }

inline auto InStream::read() -> int {
  int c = buf_->sbumpc();
  if (c == EOF) return EOF;
  if (c == '\n') {
    ++line_num_, col_num_ = 1;
  } else {
    ++col_num_;
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
  if (line_num() != 1 || col_num() != 1) {
    panic(format("Can't set strict mode of `%s` when not at the beginning of the file",
                 name().data()));
  }
  strict_ = b;
}

inline auto InStream::line_num() const -> std::size_t { return line_num_; }

inline auto InStream::col_num() const -> std::size_t { return col_num_; }

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
    if (int c = seek(); c == EOF || std::isspace(c)) break;
    token.push_back(static_cast<char>(read()));
  }
  return token;
}

inline auto InStream::read_line() -> std::optional<std::string> {
  std::string line;
  if (eof()) return std::nullopt;
  while (true) {
    int c = read();
    if (c == EOF || c == '\n') break;
    line.push_back(static_cast<char>(c));
  }
  return line;
}

inline auto InStream::fail(std::string_view message) -> void {
  fail_func_(message);
  exit(EXIT_FAILURE);  // Usually unnecessary, but in sepial cases to prevent problems.
}
}  // namespace cplib::io
  

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_VAR_HPP_
#define CPLIB_VAR_HPP_

#include <cstdint>      // for int16_t, int32_t, int64_t, int8_t, uint16_t
#include <cstdio>       // for size_t
#include <functional>   // for function
#include <iterator>     // for pair
#include <memory>       // for unique_ptr, allocator
#include <optional>     // for optional, nullopt_t
#include <string>       // for string, basic_string
#include <string_view>  // for string_view
#include <tuple>        // for tuple
#include <utility>      // for pair
#include <variant>      // for tuple
#include <vector>       // for vector


       // for InStream
  // for Pattern


namespace cplib::var {
template <class T, class D>
class Var;

/**
 * `Reader` represents a traced input stream with line and column information.
 */
class Reader {
 public:
  /**
   * `Trace` represents trace information for a variable.
   */
  struct Trace {
    std::string var_name;
    std::size_t line_num;
    std::size_t col_num;
  };

  /**
   * Create a root reader of input stream.
   *
   * @param inner The inner input stream to wrap.
   */
  explicit Reader(std::unique_ptr<io::InStream> inner);

  /// Copy constructor (deleted to prevent copying).
  Reader(const Reader&) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const Reader&) -> Reader& = delete;

  /// Move constructor.
  Reader(Reader&&) = default;

  /// Move assignment operator.
  auto operator=(Reader&&) -> Reader& = default;

  /**
   * Get the inner wrapped input stream.
   *
   * @return Reference to the inner input stream.
   */
  auto inner() -> io::InStream&;

  /**
   * Call `Instream::fail` of the wrapped input stream.
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
  auto read(const Var<T, D>& v) -> T;

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
  std::vector<Trace> traces_;
};

template <class T>
class Vec;

/**
 * `Var` describes a "variable reading template".
 *
 * @tparam T The target type of the variable reading template.
 * @tparam D The derived class of the variable reading template. When other classes inherit from
 * `Var`, this template parameter should be the class itself.
 */
template <class T, class D>
class Var {
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
  virtual auto read_from(Reader& in) const -> T = 0;

 protected:
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

 private:
  std::string name_;
};

/**
 * `Int` is a variable reading template, indicating to read an integer in a given range in decimal
 * form.
 *
 * @tparam T The target type of the variable reading template.
 */
template <class T>
class Int : public Var<T, Int<T>> {
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
  auto read_from(Reader& in) const -> T override;
};

/**
 * `Float` is a variable reading template, indicating to read a floating-point number in a given
 * range in fixed form or scientific form.
 */
template <class T>
class Float : public Var<T, Float<T>> {
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
  auto read_from(Reader& in) const -> T override;
};

/**
 * `StrictFloat` is a variable reading template, indicating to read a floating-point number in a
 * given range in fixed for with digit count restrictions.
 */
template <class T>
class StrictFloat : public Var<T, StrictFloat<T>> {
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
  auto read_from(Reader& in) const -> T override;
};

/**
 * `String` is a variable reading template, indicating to read a whitespace separated string.
 */
class String : public Var<std::string, String> {
 public:
  std::optional<Pattern> pat;

  /**
   * Default constructor.
   */
  explicit String();

  /**
   * Constructor with pattern parameter.
   *
   * @param pat The pattern of the String variable.
   */
  explicit String(Pattern pat);

  /**
   * Constructor with name parameter.
   *
   * @param name The name of the String variable.
   */
  explicit String(std::string name);

  /**
   * Constructor with pattern and name parameters.
   *
   * @param name The name of the String variable.
   * @param pat The pattern of the String variable.
   * */
  explicit String(std::string name, Pattern pat);

  /**
   * Read the value from the input reader.
   *
   * @param in The input reader.
   * @return The value read from the input reader.
   */
  auto read_from(Reader& in) const -> std::string override;
};

/**
 * `Separator` is a variable reading template, indicating to read a character as a separator.
 *
 * - If it is in strict mode, read exact one character determine whether it is same as `sep`.
 * - Otherwise, if `std::isspace(sep)`, read the next consecutive whitespaces.
 * - Otherwise, try skipping blanks and read exact one character `sep`.
 */
class Separator : public Var<std::nullopt_t, Separator> {
 public:
  char sep;

  /**
   * Constructs a `Separator` object with the specified separator character.
   *
   * @param sep The separator character.
   */
  explicit Separator(char sep);

  /**
   * Constructs a `Separator` object with the specified separator character and name.
   *
   * @param name The name of the `Separator`.
   * @param sep The separator character.
   */
  explicit Separator(std::string name, char sep);

  /**
   * Reads the separator character from the input reader.
   *
   * @param in The input reader.
   * @return `std::nullopt` to indicate that no value is read.
   */
  auto read_from(Reader& in) const -> std::nullopt_t override;
};

/**
 * `Line` is a variable reading template, indicating to read a end-of-line separated string.
 */
class Line : public Var<std::string, Line> {
 public:
  std::optional<Pattern> pat;

  /**
   * Constructs a `Line` object.
   */
  explicit Line();

  /**
   * Constructs a `Line` object with the specified pattern.
   *
   * @param pat The pattern to match for the line.
   */
  explicit Line(Pattern pat);

  /**
   * Constructs a `Line` object with the specified name.
   *
   * @param name The name of the `Line`.
   */
  explicit Line(std::string name);

  /**
   * Constructs a `Line` object with the specified pattern and name.
   *
   * @param name The name of the `Line`.
   * @param pat The pattern to match for the line.
   */
  explicit Line(std::string name, Pattern pat);

  /**
   * Reads the line from the input reader.
   *
   * @param in The input reader.
   * @return The read line as a string.
   */
  auto read_from(Reader& in) const -> std::string override;
};

/**
 * `Vec` is a variable reading template, used to read a fixed length of variables of the same type
 * and return them as `std::vector`.
 *
 * @tparam T The type of the inner variable reading template.
 */
template <class T>
class Vec : public Var<std::vector<typename T::Var::Target>, Vec<T>> {
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
  auto read_from(Reader& in) const -> std::vector<typename T::Var::Target> override;
};

/**
 * `Mat` is a variable reading template used to read a fixed-size variable matrix. The elements in
 * the matrix have the same type.
 *
 * @tparam T The type of the inner variable reading template.
 */
template <class T>
class Mat : public Var<std::vector<std::vector<typename T::Var::Target>>, Mat<T>> {
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
  auto read_from(Reader& in) const -> std::vector<std::vector<typename T::Var::Target>> override;
};

/**
 * `Pair` is a variable reading template that reads two variables separated by a given separator and
 * returns them as `std::pair`.
 *
 * @tparam F The type of the first variable reading template.
 * @tparam S The type of the second variable reading template.
 */
template <class F, class S>
class Pair : public Var<std::pair<typename F::Var::Target, typename S::Var::Target>, Pair<F, S>> {
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
  auto read_from(Reader& in) const
      -> std::pair<typename F::Var::Target, typename S::Var::Target> override;
};

/**
 * Tuple of variables.
 *
 * @tparam T The type of the variable reading templates.
 */
template <class... T>
class Tuple : public Var<std::tuple<typename T::Var::Target...>, Tuple<T...>> {
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
  auto read_from(Reader& in) const -> std::tuple<typename T::Var::Target...> override;
};

/**
 * `FnVar` is used to wrap a function into a variable reading template.
 *
 * The type of the first parameter of the wrapped function must be `cplib::var::Reader`, and
 * subsequent parameters (which may not exist) are arbitrary.
 *
 * @tparam F The type of the function.
 */
template <class F>
class FnVar : public Var<typename std::function<F>::result_type, FnVar<F>> {
 public:
  /// The inner function.
  std::function<typename std::function<F>::result_type(Reader& in)> inner;

  /**
   * Constructor.
   *
   * @param name The name of the variable to be read.
   * @param f The function object.
   * @param args The second to last arguments to the function, the count and type must match the
   * parameters of `f`.
   */
  template <class... Args>
  FnVar(std::string name, std::function<F> f, Args... args);

  /**
   * Read from reader.
   *
   * Use `in` as the first argument, together with subsequent arguments passed in the constructor,
   * to call the wrapped function.
   *
   * @param in The reader object.
   * @return The result of the function.
   */
  auto read_from(Reader& in) const -> typename std::function<F>::result_type override;
};

/**
 * For a type `T` which has implemented `static auto T::read(var::Reader&, ...) -> T`,
 * `ExtVar<T>` provides a variable template for creating `T` by calling `T::read`.
 *
 * @tparam T The type of the variable, which must have a static `auto T::read(var::Reader&, ...) ->
 * T`
 */
template <class T>
class ExtVar : public Var<T, ExtVar<T>> {
 public:
  /// The inner function.
  std::function<auto(Reader& in)->T> inner;

  /**
   * Constructor.
   *
   * @param name The name of the variable to be read.
   * @param args The second to last arguments to the function `T::read`.
   */
  template <class... Args>
  explicit ExtVar(std::string name, Args... args);

  /**
   * Read from reader.
   *
   * Use `in` as the first argument, together with subsequent arguments passed in the constructor,
   * to call `T::read`.
   *
   * @param in The reader object.
   * @return The result of `T::read`.
   */
  auto read_from(Reader& in) const -> T override;
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

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_VAR_HPP_
#error "Must be included from var.hpp"
#endif
#endif


#include <cctype>        // for isspace, isdigit
#include <charconv>      // for from_chars
#include <cmath>         // for isnan, pow
#include <cstdio>        // for size_t, fileno, stdin, stdout
#include <fstream>       // for basic_istream, basic_ostream, basic_filebuf
#include <functional>    // for function
#include <iostream>      // for cin, cerr, cout
#include <limits>        // for numeric_limits
#include <memory>        // for make_unique, unique_ptr, allocator
#include <optional>      // for optional, nullopt, nullopt_t
#include <sstream>       // for stringbuf
#include <string>        // for basic_string, string, char_traits, to_string
#include <string_view>   // for string_view
#include <system_error>  // for errc
#include <tuple>         // for tuple, apply
#include <utility>       // for move, pair
#include <variant>       // for tuple
#include <vector>        // for vector







namespace cplib::var {
inline Reader::Reader(std::unique_ptr<io::InStream> inner)
    : inner_(std::move(inner)), traces_({}) {}

inline auto Reader::inner() -> io::InStream& { return *inner_; }

inline auto Reader::fail(std::string_view message) -> void {
  using namespace std::string_literals;
  std::string result = "read error: "s + std::string(message);
  size_t depth = 0;
  for (auto it = traces_.rbegin(); it != traces_.rend(); ++it) {
    result += format("\n  #%zu: %s @ %s:%zu:%zu", depth, it->var_name.c_str(),
                     inner().name().data(), it->line_num, it->col_num);
    ++depth;
  }
  inner_->fail(result);
}

template <class T, class D>
inline auto Reader::read(const Var<T, D>& v) -> T {
  traces_.emplace_back(Trace{std::string(v.name()), inner_->line_num(), inner_->col_num()});
  auto result = v.read_from(*this);
  traces_.pop_back();
  return result;
}

template <class... T>
inline auto Reader::operator()(T... vars) -> std::tuple<typename T::Var::Target...> {
  return {read(vars)...};
}

namespace detail {
// Open the given file and create a `var::Reader`
inline auto make_file_reader(std::string_view path, std::string name, bool strict,
                             io::InStream::FailFunc fail_func) -> var::Reader {
  auto buf = std::make_unique<std::filebuf>();
  if (!buf->open(path.data(), std::ios::binary | std::ios::in)) {
    panic(format("Can not open file `%s` as input stream", path.data()));
  }
  return var::Reader(std::make_unique<io::InStream>(std::move(buf), std::move(name), strict,
                                                    std::move(fail_func)));
}

// Open `stdin` as input stream and create a `var::Reader`
inline auto make_stdin_reader(std::string name, bool strict, io::InStream::FailFunc fail_func)
    -> var::Reader {
  auto buf = std::make_unique<io::detail::FdInBuf>(fileno(stdin));
  var::Reader reader(std::make_unique<io::InStream>(std::move(buf), std::move(name), strict,
                                                    std::move(fail_func)));
  /* FIXME: Under msvc stdin/stdout is an lvalue, cannot prevent users from using stdio. */
  // stdin = nullptr;
  std::cin.rdbuf(nullptr);
  std::cin.tie(nullptr);
  return reader;
}

// Open `stdout` as output stream and create a `std::::ostream`
inline auto make_stdout_ostream(std::unique_ptr<std::streambuf>& buf, std::ostream& stream)
    -> void {
  buf = std::make_unique<io::detail::FdOutBuf>(fileno(stdout));
  stream.rdbuf(buf.get());
  /* FIXME: Under msvc stdin/stdout is an lvalue, cannot prevent users from using stdio. */
  // stdout = nullptr;
  std::cout.rdbuf(nullptr);
  std::cin.tie(nullptr);
  std::cerr.tie(nullptr);
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
  D clone = *static_cast<const D*>(this);
  return clone;
}

template <class T, class D>
inline auto Var<T, D>::renamed(std::string_view name) const -> D {
  D clone = *static_cast<const D*>(this);
  clone.name_ = name;
  return clone;
}

template <class T, class D>
inline auto Var<T, D>::parse(std::string_view s) const -> T {
  auto buf = std::make_unique<std::stringbuf>(std::string(s), std::ios_base::in);
  auto reader = Reader(std::make_unique<io::InStream>(std::move(buf), "str", true,
                                                      [&](std::string_view s) { panic(s); }));
  T result = reader.read(*this);
  if (!reader.inner().eof()) {
    panic("Var::parse failed, extra characters in string");
  }
  return result;
}

template <class T, class D>
inline auto Var<T, D>::operator*(size_t len) const -> Vec<D> {
  return Vec<D>(*static_cast<const D*>(this), len);
}

template <class T, class D>
inline Var<T, D>::Var() : name_(std::string(detail::VAR_DEFAULT_NAME)) {}

template <class T, class D>
inline Var<T, D>::Var(std::string name) : name_(std::move(name)) {}

template <class T>
inline Int<T>::Int() : Int<T>(std::string(detail::VAR_DEFAULT_NAME), std::nullopt, std::nullopt) {}

template <class T>
inline Int<T>::Int(std::string name) : Int<T>(std::move(name), std::nullopt, std::nullopt) {}

template <class T>
inline Int<T>::Int(std::optional<T> min, std::optional<T> max)
    : Int<T>(std::string(detail::VAR_DEFAULT_NAME), std::move(min), std::move(max)) {}

template <class T>
inline Int<T>::Int(std::string name, std::optional<T> min, std::optional<T> max)
    : Var<T, Int<T>>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <class T>
inline auto Int<T>::read_from(Reader& in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected an integer, got EOF");
    } else {
      in.fail(format("Expected an integer, got whitespace `%s`",
                     cplib::detail::hex_encode(in.inner().seek()).c_str()));
    }
  }

  T result{};
  auto [ptr, ec] = std::from_chars(token.c_str(), token.c_str() + token.size(), result);

  if (ec != std::errc() || ptr != token.c_str() + token.size()) {
    in.fail(format("Expected an integer, got `%s`", compress(token).c_str()));
  }

  if (min.has_value() && result < *min) {
    in.fail(format("Expected an integer >= %s, got `%s`", std::to_string(*min).c_str(),
                   compress(token).c_str()));
  }

  if (max.has_value() && result > *max) {
    in.fail(format("Expected an integer <= %s, got `%s`", std::to_string(*max).c_str(),
                   compress(token).c_str()));
  }

  return result;
}

namespace detail {
inline constexpr std::size_t MAX_N_SIGNIFICANT = 19;
inline constexpr std::int64_t MAX_EXPONENT = 32767;

inline constexpr auto char_to_lower(const int c) -> int {
  return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

inline constexpr auto equals_ignore_case(std::string_view s1, std::string_view s2) -> bool {
  if (s1.size() != s2.size()) return false;
  for (auto it1 = s1.begin(), it2 = s2.begin(); it1 != s1.end(); ++it1, ++it2) {
    if (char_to_lower(*it1) != char_to_lower(*it2)) return false;
  }
  return true;
}

template <class T>
inline constexpr auto powi(T x, std::int64_t y) -> T {
  T res{1};
  while (y) {
    if (y & 1) res *= x;
    x *= x;
    y >>= 1;
  }
  return res;
}

template <class T>
inline constexpr auto create_float(std::int64_t sign, std::int64_t before_point,
                                   std::int64_t after_point, std::int64_t exponent_sign,
                                   std::int64_t exponent, std::size_t n_after_point,
                                   std::size_t n_tailing_zero) -> T {
  T result = static_cast<T>(before_point);
  if (n_tailing_zero != 0) {
    result *= powi<T>(10.0, n_tailing_zero);
  }
  if (after_point != 0) {
    result += after_point * powi<T>(0.1, n_after_point);
  }
  if (exponent != 0) {
    if (exponent_sign > 0) {
      result *= powi<T>(10.0, exponent);
    } else {
      result *= powi<T>(0.1, exponent);
    }
  }
  return sign * result;
}

template <class T>
inline constexpr auto parse_strict_float(std::string_view s, std::size_t* n_after_point_out) -> T {
  enum class State { SIGN, BEFORE_POINT, AFTER_POINT } state = State::SIGN;
  std::size_t n_significant = 0, n_after_point = 0, n_tailing_zero = 0;
  std::int64_t sign = 1, before_point = 0, after_point = 0;

  for (auto c : s) {
    if (state == State::SIGN) {
      state = State::BEFORE_POINT;
      if (c == '-') {
        sign = -1;
        continue;
      }
    }
    if (state <= State::BEFORE_POINT && c == '.') {
      if (n_significant == 0) {
        // .abc
        return std::numeric_limits<T>::quiet_NaN();
      }
      state = State::AFTER_POINT;
      continue;
    }
    if (!isdigit(c)) {
      return std::numeric_limits<T>::quiet_NaN();
    }
    if (state <= State::BEFORE_POINT && before_point == 0 && n_significant > 0) {
      // 0a.bcd
      return std::numeric_limits<T>::quiet_NaN();
    }
    if (n_significant >= MAX_N_SIGNIFICANT) {
      if (state <= State::BEFORE_POINT) {
        ++n_tailing_zero;
      } else {
        ++n_after_point;
      }
      continue;
    }
    ++n_significant;
    if (state <= State::BEFORE_POINT) {
      before_point *= 10;
      before_point += c ^ '0';
    } else {
      ++n_after_point;
      after_point *= 10;
      after_point += c ^ '0';
    }
  }

  if (n_after_point_out) {
    *n_after_point_out = n_after_point;
  }

  return create_float<T>(sign, before_point, after_point, 1, 0, n_after_point, n_tailing_zero);
}

template <class T>
inline constexpr auto parse_float(std::string_view s) -> T {
  if (equals_ignore_case(s, "inf") || equals_ignore_case(s, "infinity")) {
    return std::numeric_limits<T>::infinity();
  }
  if (equals_ignore_case(s, "-inf") || equals_ignore_case(s, "-infinity")) {
    return -std::numeric_limits<T>::infinity();
  }

  enum class State { SIGN, BEFORE_POINT, AFTER_POINT, EXPONENT_SIGN, EXPONENT } state = State::SIGN;

  std::size_t n_significant = 0, n_after_point = 0, n_tailing_zero = 0;
  std::int64_t sign = 1, before_point = 0, after_point = 0, exponent_sign = 1, exponent = 0;

  for (auto c : s) {
    if (state == State::SIGN) {
      state = State::BEFORE_POINT;
      if (c == '-') {
        sign = -1;
        continue;
      } else if (c == '+') {
        continue;
      }
    }
    if (state == State::EXPONENT_SIGN) {
      state = State::EXPONENT;
      if (c == '-') {
        exponent_sign = -1;
        continue;
      } else if (c == '+') {
        continue;
      }
    }
    if (state <= State::BEFORE_POINT && c == '.') {
      state = State::AFTER_POINT;
      continue;
    }
    if (state < State::EXPONENT && (c == 'e' || c == 'E')) {
      state = State::EXPONENT_SIGN;
      continue;
    }
    if (!isdigit(c)) {
      return std::numeric_limits<T>::quiet_NaN();
    }
    if (state >= State::EXPONENT_SIGN) {
      exponent *= 10;
      exponent += c ^ '0';
      if (exponent > MAX_EXPONENT) {
        if (exponent_sign < 0) {
          return sign * T(0);
        }
        return sign * std::numeric_limits<T>::infinity();
      }
      continue;
    }
    if (n_significant >= MAX_N_SIGNIFICANT) {
      if (state <= State::BEFORE_POINT) {
        ++n_tailing_zero;
      } else {
        ++n_after_point;
      }
      continue;
    }
    ++n_significant;
    if (state <= State::BEFORE_POINT) {
      before_point *= 10;
      before_point += c ^ '0';
    } else {
      ++n_after_point;
      after_point *= 10;
      after_point += c ^ '0';
    }
  }

  return create_float<T>(sign, before_point, after_point, exponent_sign, exponent, n_after_point,
                         n_tailing_zero);
}
}  // namespace detail

template <class T>
inline Float<T>::Float()
    : Float<T>(std::string(detail::VAR_DEFAULT_NAME), std::nullopt, std::nullopt) {}

template <class T>
inline Float<T>::Float(std::string name) : Float<T>(std::move(name), std::nullopt, std::nullopt) {}

template <class T>
inline Float<T>::Float(std::optional<T> min, std::optional<T> max)
    : Float<T>(std::string(detail::VAR_DEFAULT_NAME), std::move(min), std::move(max)) {}

template <class T>
inline Float<T>::Float(std::string name, std::optional<T> min, std::optional<T> max)
    : Var<T, Float<T>>(std::move(name)), min(std::move(min)), max(std::move(max)) {}

template <class T>
inline auto Float<T>::read_from(Reader& in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected a float, got EOF");
    } else {
      in.fail(format("Expected a float, got whitespace `%s`",
                     cplib::detail::hex_encode(in.inner().seek()).c_str()));
    }
  }

  // `Float<T>` usually uses with non-strict streams, so it should support both fixed format and
  // scientific.
  T result = detail::parse_float<T>(token);

  if (std::isnan(result)) {
    in.fail(format("Expected a float, got `%s`", compress(token).c_str()));
  }

  if (min.has_value() && result < *min) {
    in.fail(format("Expected a float >= %s, got `%s`", std::to_string(*min).c_str(),
                   compress(token).c_str()));
  }

  if (max.has_value() && result > *max) {
    in.fail(format("Expected a float <= %s, got `%s`", std::to_string(*max).c_str(),
                   compress(token).c_str()));
  }

  return result;
}

template <class T>
inline StrictFloat<T>::StrictFloat(T min, T max, size_t min_n_digit, size_t max_n_digit)
    : StrictFloat<T>(std::string(detail::VAR_DEFAULT_NAME), min, max, min_n_digit, max_n_digit) {}

template <class T>
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

template <class T>
inline auto StrictFloat<T>::read_from(Reader& in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected a strict float, got EOF");
    } else {
      in.fail(format("Expected a strict float, got whitespace `%s`",
                     cplib::detail::hex_encode(in.inner().seek()).c_str()));
    }
  }

  std::size_t n_after_point;
  T result = detail::parse_strict_float<T>(token, &n_after_point);

  if (std::isnan(result)) {
    in.fail(format("Expected a strict float, got `%s`", compress(token).c_str()));
  }

  if (n_after_point < min_n_digit) {
    in.fail(
        format("Expected a strict float with >= %zu digits after point, got `%s` with %zu digits "
               "after point",
               min_n_digit, compress(token).c_str(), n_after_point));
  }

  if (n_after_point > max_n_digit) {
    in.fail(
        format("Expected a strict float with <= %zu digits after point, got `%s` with %zu digits "
               "after point",
               max_n_digit, compress(token).c_str(), n_after_point));
  }

  if (result < min) {
    in.fail(format("Expected a strict float >= %s, got `%s`", std::to_string(min).c_str(),
                   compress(token).c_str()));
  }

  if (result > max) {
    in.fail(format("Expected a strict float <= %s, got `%s`", std::to_string(max).c_str(),
                   compress(token).c_str()));
  }

  return result;
}

inline String::String() : String(std::string(detail::VAR_DEFAULT_NAME)) {}

inline String::String(Pattern pat)
    : String(std::string(detail::VAR_DEFAULT_NAME), std::move(pat)) {}

inline String::String(std::string name)
    : Var<std::string, String>(std::move(name)), pat(std::nullopt) {}

inline String::String(std::string name, Pattern pat)
    : Var<std::string, String>(std::move(name)), pat(std::move(pat)) {}

inline auto String::read_from(Reader& in) const -> std::string {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected a string, got EOF");
    } else {
      in.fail(format("Expected a string, got whitespace `%s`",
                     cplib::detail::hex_encode(in.inner().seek()).c_str()));
    }
  }

  if (pat.has_value() && !pat->match(token)) {
    in.fail(format("Expected a string matching `%s`, got `%s`", compress(pat->src()).data(),
                   compress(token).c_str()));
  }

  return token;
}

// Impl Separator {{{
inline Separator::Separator(char sep) : Separator(std::string(detail::VAR_DEFAULT_NAME), sep) {}

inline Separator::Separator(std::string name, char sep)
    : Var<std::nullopt_t, Separator>(std::move(name)), sep(sep) {}

inline auto Separator::read_from(Reader& in) const -> std::nullopt_t {
  if (in.inner().eof()) {
    in.fail(format("Expected a separator `%s`, got EOF", cplib::detail::hex_encode(sep).c_str()));
  }

  if (in.inner().is_strict()) {
    auto got = in.inner().read();
    if (got != sep) {
      in.fail(format("Expected a separator `%s`, got `%s`", cplib::detail::hex_encode(sep).c_str(),
                     cplib::detail::hex_encode(got).c_str()));
    }
  } else if (std::isspace(sep)) {
    auto got = in.inner().read();
    if (!std::isspace(got)) {
      in.fail(format("Expected a separator `%s`, got `%s`", cplib::detail::hex_encode(sep).c_str(),
                     cplib::detail::hex_encode(got).c_str()));
    }
  } else {
    in.inner().skip_blanks();
    auto got = in.inner().read();
    if (got != sep) {
      in.fail(format("Expected a separator `%s`, got `%s`", cplib::detail::hex_encode(sep).c_str(),
                     cplib::detail::hex_encode(got).c_str()));
    }
  }

  return std::nullopt;
}
// /Impl Separator }}}

inline Line::Line() : Line(std::string(detail::VAR_DEFAULT_NAME)) {}

inline Line::Line(Pattern pat) : Line(std::string(detail::VAR_DEFAULT_NAME), std::move(pat)) {}

inline Line::Line(std::string name) : Var<std::string, Line>(std::move(name)), pat(std::nullopt) {}

inline Line::Line(std::string name, Pattern pat)
    : Var<std::string, Line>(std::move(name)), pat(std::move(pat)) {}

inline auto Line::read_from(Reader& in) const -> std::string {
  auto line = in.inner().read_line();

  if (!line.has_value()) {
    in.fail("Expected a line, got EOF");
  }

  if (pat.has_value() && pat->match(*line)) {
    in.fail(format("Expected a line matching `%s`, got `%s`", compress(pat->src()).data(),
                   compress(*line).c_str()));
  }

  return *line;
}

template <class T>
inline Vec<T>::Vec(T element, size_t len) : Vec<T>(element, len, var::space) {}

template <class T>
inline Vec<T>::Vec(T element, size_t len, Separator sep)
    : Var<std::vector<typename T::Var::Target>, Vec<T>>(std::string(element.name())),
      element(std::move(element)),
      len(len),
      sep(std::move(sep)) {}

template <class T>
inline auto Vec<T>::read_from(Reader& in) const -> std::vector<typename T::Var::Target> {
  std::vector<typename T::Var::Target> result(len);
  for (size_t i = 0; i < len; ++i) {
    if (i > 0) in.read(sep);
    result[i] = in.read(element.renamed(std::to_string(i)));
  }
  return result;
}

template <class T>
inline Mat<T>::Mat(T element, size_t len0, size_t len1)
    : Mat<T>(element, len0, len1, var::space, var::eoln) {}

template <class T>
inline Mat<T>::Mat(T element, size_t len0, size_t len1, Separator sep0, Separator sep1)
    : Var<std::vector<std::vector<typename T::Var::Target>>, Mat<T>>(std::string(element.name())),
      element(std::move(element)),
      len0(len0),
      len1(len1),
      sep0(std::move(sep0)),
      sep1(std::move(sep1)) {}

template <class T>
inline auto Mat<T>::read_from(Reader& in) const
    -> std::vector<std::vector<typename T::Var::Target>> {
  std::vector<std::vector<typename T::Var::Target>> result(
      len0, std::vector<typename T::Var::Target>(len1));
  for (size_t i = 0; i < len0; ++i) {
    if (i > 0) in.read(sep0);
    auto name_prefix = std::to_string(i) + "_";
    for (size_t j = 0; j < len1; ++j) {
      if (j > 0) in.read(sep1);
      result[i][j] = in.read(element.renamed(name_prefix + std::to_string(j)));
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
inline auto Pair<F, S>::read_from(Reader& in) const
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
inline auto Tuple<T...>::read_from(Reader& in) const -> std::tuple<typename T::Var::Target...> {
  return std::apply(
      [&in](const auto&... args) {
        size_t cnt = 0;
        auto renamed_inc = [&cnt](auto var) { return var.renamed(std::to_string(cnt++)); };
        return std::tuple{in.read(renamed_inc(args))...};
      },
      elements);
}

template <class F>
template <class... Args>
inline FnVar<F>::FnVar(std::string name, std::function<F> f, Args... args)
    : Var<typename std::function<F>::result_type, FnVar<F>>(std::move(name)),
      inner([f, args...](Reader& in) { return f(in, args...); }) {}

template <class F>
inline auto FnVar<F>::read_from(Reader& in) const -> typename std::function<F>::result_type {
  return inner(in);
}

template <class T>
template <class... Args>
inline ExtVar<T>::ExtVar(std::string name, Args... args)
    : Var<T, ExtVar<T>>(std::move(name)),
      inner([args...](Reader& in) { return T::read(in, args...); }) {}

template <class T>
inline auto ExtVar<T>::read_from(Reader& in) const -> T {
  return inner(in);
}
}  // namespace cplib::var
  

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_CHECKER_HPP_
#define CPLIB_CHECKER_HPP_

#include <string>       // for basic_string, string
#include <string_view>  // for string_view


  // for Random
   // for UniqueFunction, UniqueFunction<>::UniqueFunct...
     // for Reader


namespace cplib::checker {
/**
 * `Report` represents a report showing when checker exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  class Status {
   public:
    enum Value {
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

/**
 * Represents the state of the checker.
 */
class State {
 public:
  /// The type of function used to initialize the state.
  using Initializer = UniqueFunction<auto(State& state, int argc, char** argv)->void>;

  /// The type of function used for reporting.
  using Reporter = UniqueFunction<auto(const Report& report)->void>;

  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Output file reader.
  var::Reader ouf;

  /// Answer file reader.
  var::Reader ans;

  /// Initializer is a function parsing command line arguments and initializing `checker::State`
  Initializer initializer;

  /// Reporter is a function that reports the given `checker::Report` and exits the program.
  Reporter reporter;

  /**
   * Constructs a new `State` object with the given initializer function.
   *
   * @param initializer The initializer function.
   */
  explicit State(Initializer initializer);

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
struct DefaultInitializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param state The state object to be initialized.
   * @param argc The number of command line arguments.
   * @param argv The command line arguments.
   */
  auto operator()(State& state, int argc, char** argv) -> void;
};

/**
 * Report the given report in JSON format.
 *
 * @param report The report to be reported.
 */
auto json_reporter(const Report& report) -> void;

/**
 * Report the given report in plain text format for human reading.
 *
 * @param report The report to be reported.
 */
auto plain_text_reporter(const Report& report) -> void;

/**
 * Report the given report in colored text format for human reading.
 *
 * @param report The report to be reported.
 */
auto colored_text_reporter(const Report& report) -> void;

/**
 * Macro to register checker with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_CHECKER_OPT(var_, initializer_) \
  auto var_ = ::cplib::checker::State(initializer_);   \
  auto main(signed argc, char** argv)->signed {        \
    var_.initializer(var_, argc, argv);                \
    auto checker_main(void)->void;                     \
    checker_main();                                    \
    return 0;                                          \
  }

/**
 * Macro to register checker with default initializer.
 *
 * @param var The variable name of state object to be initialized.
 */
#define CPLIB_REGISTER_CHECKER(var) \
  CPLIB_REGISTER_CHECKER_OPT(var, ::cplib::checker::DefaultInitializer())
}  // namespace cplib::checker

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_CHECKER_HPP_
#error "Must be included from checker.hpp"
#endif
#endif


#include <array>        // for array
#include <cstdio>       // for fileno, stderr
#include <cstdlib>      // for exit, EXIT_FAILURE, EXIT_SUCCESS
#include <iomanip>      // for operator<<, setprecision
#include <iostream>     // for basic_ostream, operator<<, clog, fixed
#include <memory>       // for unique_ptr
#include <string>       // for basic_string, char_traits, allocator, string
#include <string_view>  // for string_view, operator==, basic_string_view
#include <utility>      // for move


      // for InStream, InStream::seek_eof
  // for isatty, CPLIB_VERSION
   // for panic, format, has_colors, json_string_encode


namespace cplib::checker {
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
      panic(format("Unknown checker report status: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, double score, std::string message)
    : status(status), score(score), message(std::move(message)) {}

// Impl State {{{
inline State::State(Initializer initializer)
    : rnd(),
      inf(var::Reader(nullptr)),
      ouf(var::Reader(nullptr)),
      ans(var::Reader(nullptr)),
      initializer(std::move(initializer)),
      reporter(json_reporter) {
  cplib::detail::panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, 0.0, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::CHECKER;
}

inline State::~State() {
  if (!exited_) panic("Checker must exit by calling method `State::quit*`");
}

inline auto State::disable_check_dirt() -> void { check_dirt_ = true; }

inline auto State::quit(Report report) -> void {
  exited_ = true;

  if (check_dirt_ &&
      (report.status == Report::Status::ACCEPTED ||
       report.status == Report::Status::PARTIALLY_CORRECT) &&
      !ouf.inner().seek_eof()) {
    report = Report(Report::Status::WRONG_ANSWER, 0.0, "Extra content in the output file");
  }

  reporter(report);

  std::clog << "Unrecoverable error: Reporter didn't exit the program\n";
  std::exit(EXIT_FAILURE);
}

inline auto State::quit_ac() -> void { quit(Report(Report::Status::ACCEPTED, 1.0, "")); }

inline auto State::quit_wa(std::string_view message) -> void {
  quit(Report(Report::Status::WRONG_ANSWER, 0.0, std::string(message)));
}

inline auto State::quit_pc(double points, std::string_view message) -> void {
  quit(Report(Report::Status::PARTIALLY_CORRECT, points, std::string(message)));
}
// /Impl State }}}

// Impl DefaultInitializer {{{
namespace detail {
constexpr std::string_view ARGS_USAGE =
    "<input_file> <output_file> <answer_file> [--report-format={auto|json|text}]";

inline auto print_help_message(std::string_view program_name) -> void {
  std::string msg =
      format("cplib (CPLib) " CPLIB_VERSION
             "\n"
             "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
             "\n"
             "Usage:\n"
             "  %s %s\n"
             "\n"
             "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
             "enable colors",
             program_name.data(), ARGS_USAGE.data());
  panic(msg);
}

inline auto detect_reporter(State& state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = json_reporter;
  } else if (cplib::detail::has_colors()) {
    state.reporter = colored_text_reporter;
  } else {
    state.reporter = plain_text_reporter;
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State& state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = json_reporter;
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = colored_text_reporter;
    } else {
      state.reporter = plain_text_reporter;
    }
  } else {
    return false;
  }
  return true;
}

inline auto parse_command_line_arguments(State& state, int argc, char** argv)
    -> std::array<std::string_view, 3> {
  if (argc < 4) {
    panic("Program must be run with the following arguments:\n  " + std::string(ARGS_USAGE));
  }

  for (int i = 4; i < argc; ++i) {
    auto arg = std::string_view(argv[i]);
    if (constexpr std::string_view prefix = "--report-format=";
        !arg.compare(0, prefix.size(), prefix)) {
      arg.remove_prefix(prefix.size());
      if (!set_report_format(state, arg)) {
        panic(format("Unknown %s option: %s", prefix.data(), arg.data()));
      }
    } else {
      panic("Unknown option: " + std::string(arg));
    }
  }

  return {argv[1], argv[2], argv[3]};
}
}  // namespace detail

inline auto DefaultInitializer::operator()(State& state, int argc, char** argv) -> void {
  detail::detect_reporter(state);

  if (argc > 1 && std::string_view("--help") == argv[1]) {
    detail::print_help_message(argv[0]);
  }

  auto [inf_path, ouf_path, ans_path] = detail::parse_command_line_arguments(state, argc, argv);

  state.inf = var::detail::make_file_reader(inf_path, "inf", false,
                                            [](std::string_view msg) { panic(msg); });
  state.ouf = var::detail::make_file_reader(ouf_path, "ouf", false,
                                            [&state](std::string_view msg) { state.quit_wa(msg); });
  state.ans = var::detail::make_file_reader(ans_path, "ans", false,
                                            [](std::string_view msg) { panic(msg); });
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
      panic(format("Unknown checker report status: %d", static_cast<int>(status)));
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
      panic(format("Unknown checker report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto json_reporter(const Report& report) -> void {
  auto msg = format(R"({"status": "%s", "score": %.3f, "message": "%s"})",
                    report.status.to_string().data(), report.score,
                    cplib::detail::json_string_encode(report.message).c_str());
  std::clog << msg << '\n';
  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto plain_text_reporter(const Report& report) -> void {
  std::clog << std::fixed << std::setprecision(2)
            << detail::status_to_title_string(report.status).c_str() << ", scores "
            << report.score * 100.0 << " of 100.\n";
  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    std::clog << report.message << '\n';
  }
  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto colored_text_reporter(const Report& report) -> void {
  std::clog << std::fixed << std::setprecision(2)
            << detail::status_to_colored_title_string(report.status).c_str()
            << ", scores \x1b[0;33m" << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    std::clog << report.message << '\n';
  }
  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}
// /Impl reporters }}}
}  // namespace cplib::checker
  

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_INTERACTOR_HPP_
#define CPLIB_INTERACTOR_HPP_

#include <memory>       // for unique_ptr
#include <ostream>      // for basic_ostream, ostream, streambuf
#include <string>       // for basic_string, string
#include <string_view>  // for string_view


  // for Random
   // for UniqueFunction, UniqueFunction<>::UniqueFunct...
     // for Reader


namespace cplib::interactor {
/**
 * `Report` represents a report showing when interactor exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  class Status {
   public:
    enum Value {
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

/**
 * Represents the state of the validator.
 */
class State {
 public:
  /// The type of function used to initialize the state.
  using Initializer = UniqueFunction<auto(State& state, int argc, char** argv)->void>;

  /// The type of function used for reporting.
  using Reporter = UniqueFunction<auto(const Report& report)->void>;

  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Reader used to read user program stdout.
  var::Reader from_user;

  /// Output stream used to send information to user program stdin.
  std::ostream to_user;

  /// Initializer is a function parsing command line arguments and initializing `interactor::State`
  Initializer initializer;

  /// Reporter is a function that reports the given `interactor::Report` and exits the program.
  Reporter reporter;

  /**
   * Constructs a new `State` object with the given initializer function.
   *
   * @param initializer The initializer function.
   */
  explicit State(Initializer initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Disables the check for redundant content of `from_user` stream.
   */
  auto disable_check_dirt() -> void;

  /**
   * Quits the interactor with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(Report report) -> void;

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

  /// Whether to check for redundant content in the `from_user` stream.
  bool check_dirt_{true};
};

/**
 * Default initializer of interactor.
 */
struct DefaultInitializer {
  std::unique_ptr<std::streambuf> to_user_buf;

  /**
   * Initialize state according to default behavior.
   *
   * @param state The state object to be initialized.
   * @param argc The number of command line arguments.
   * @param argv The command line arguments.
   */
  auto operator()(State& state, int argc, char** argv) -> void;
};

/**
 * Report the given report in JSON format.
 *
 * @param report The report to be reported.
 */
auto json_reporter(const Report& report) -> void;

/**
 * Report the given report in plain text format for human reading.
 *
 * @param report The report to be reported.
 */
auto plain_text_reporter(const Report& report) -> void;

/**
 * Report the given report in colored text format for human reading.
 *
 * @param report The report to be reported.
 */
auto colored_text_reporter(const Report& report) -> void;

/**
 * Macro to register interactor with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_INTERACTOR_OPT(var_, initializer_) \
  auto var_ = ::cplib::interactor::State(initializer_);   \
  auto main(signed argc, char** argv)->signed {           \
    var_.initializer(var_, argc, argv);                   \
    auto interactor_main(void)->void;                     \
    interactor_main();                                    \
    return 0;                                             \
  }

/**
 * Macro to register interactor with default initializer.
 *
 * @param var The variable name of state object to be initialized.
 */
#define CPLIB_REGISTER_INTERACTOR(var) \
  CPLIB_REGISTER_INTERACTOR_OPT(var, ::cplib::interactor::DefaultInitializer())
}  // namespace cplib::interactor

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_INTERACTOR_HPP_
#error "Must be included from interactor.hpp"
#endif
#endif


#include <cstdio>    // for fileno, stderr, stdin, stdout
#include <cstdlib>   // for exit, EXIT_FAILURE, EXIT_SUCCESS
#include <iomanip>   // for operator<<, setprecision
#include <iostream>  // for basic_istream, clog, cin, wcin, cerr, cout, wcerr
#include <utility>   // for move


      // for InStream, InStream::seek_eof
  // for isatty, CPLIB_VERSION
   // for panic, format, has_colors, json_string_encode


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
      panic(format("Unknown interactor report status: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, double score, std::string message)
    : status(status), score(score), message(std::move(message)) {}

// Impl State {{{
inline State::State(Initializer initializer)
    : rnd(),
      inf(var::Reader(nullptr)),
      from_user(var::Reader(nullptr)),
      to_user(std::ostream(nullptr)),
      initializer(std::move(initializer)),
      reporter(json_reporter) {
  cplib::detail::panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, 0.0, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::INTERACTOR;
}

inline State::~State() {
  if (!exited_) panic("Interactor must exit by calling method `State::quit*`");
}

inline auto State::disable_check_dirt() -> void { check_dirt_ = true; }

inline auto State::quit(Report report) -> void {
  exited_ = true;

  if (check_dirt_ &&
      (report.status == Report::Status::ACCEPTED ||
       report.status == Report::Status::PARTIALLY_CORRECT) &&
      !from_user.inner().seek_eof()) {
    report = Report(Report::Status::WRONG_ANSWER, 0.0, "Extra content in the user output");
  }

  reporter(report);

  std::clog << "Unrecoverable error: Reporter didn't exit the program\n";
  std::exit(EXIT_FAILURE);
}

inline auto State::quit_ac() -> void { quit(Report(Report::Status::ACCEPTED, 1.0, "")); }

inline auto State::quit_wa(std::string_view message) -> void {
  quit(Report(Report::Status::WRONG_ANSWER, 0.0, std::string(message)));
}

inline auto State::quit_pc(double points, std::string_view message) -> void {
  quit(Report(Report::Status::PARTIALLY_CORRECT, points, std::string(message)));
}
// /Impl State }}}

// Impl DefaultInitializer {{{
namespace detail {
constexpr std::string_view ARGS_USAGE = "<input_file> [--report-format={auto|json|text}]";

inline auto print_help_message(std::string_view program_name) -> void {
  std::string msg =
      format("cplib (CPLib) " CPLIB_VERSION
             "\n"
             "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
             "\n"
             "Usage:\n"
             "  %s %s\n"
             "\n"
             "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
             "enable colors",
             program_name.data(), ARGS_USAGE.data());
  panic(msg);
}

inline auto detect_reporter(State& state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = json_reporter;
  } else if (cplib::detail::has_colors()) {
    state.reporter = colored_text_reporter;
  } else {
    state.reporter = plain_text_reporter;
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State& state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = json_reporter;
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = colored_text_reporter;
    } else {
      state.reporter = plain_text_reporter;
    }
  } else {
    return false;
  }
  return true;
}

inline auto parse_command_line_arguments(State& state, int argc, char** argv) -> std::string_view {
  if (argc < 2) {
    panic("Program must be run with the following arguments:\n  " + std::string(ARGS_USAGE));
  }

  for (int i = 2; i < argc; ++i) {
    auto arg = std::string_view(argv[i]);
    if (constexpr std::string_view prefix = "--report-format=";
        !arg.compare(0, prefix.size(), prefix)) {
      arg.remove_prefix(prefix.size());
      if (!set_report_format(state, arg)) {
        panic(format("Unknown %s option: %s", prefix.data(), arg.data()));
      }
    } else {
      panic("Unknown option: " + std::string(arg));
    }
  }

  return argv[1];
}

// Disable stdin & stdout
inline auto disable_stdio() -> void {
  std::ios_base::sync_with_stdio(false);
  /* FIXME: Under msvc stdin/stdout is an lvalue, cannot prevent users from using stdio. */
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

inline auto DefaultInitializer::operator()(State& state, int argc, char** argv) -> void {
  detail::detect_reporter(state);

  if (argc > 1 && std::string_view("--help") == argv[1]) {
    detail::print_help_message(argv[0]);
  }

  auto inf_path = detail::parse_command_line_arguments(state, argc, argv);

  state.inf = var::detail::make_file_reader(inf_path, "inf", false,
                                            [](std::string_view msg) { panic(msg); });

  state.from_user = var::detail::make_stdin_reader(
      "from_user", false, [&state](std::string_view msg) { state.quit_wa(msg); });

  var::detail::make_stdout_ostream(to_user_buf, state.to_user);
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
      panic(format("Unknown interactor report status: %d", static_cast<int>(status)));
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
      panic(format("Unknown interactor report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto json_reporter(const Report& report) -> void {
  auto msg = format(R"({"status": "%s", "score": %.3f, "message": "%s"})",
                    report.status.to_string().data(), report.score,
                    cplib::detail::json_string_encode(report.message).c_str());
  std::clog << msg << '\n';
  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto plain_text_reporter(const Report& report) -> void {
  std::clog << std::fixed << std::setprecision(2)
            << detail::status_to_title_string(report.status).c_str() << ", scores "
            << report.score * 100.0 << " of 100.\n";
  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    std::clog << report.message << '\n';
  }
  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto colored_text_reporter(const Report& report) -> void {
  std::clog << std::fixed << std::setprecision(2)
            << detail::status_to_colored_title_string(report.status).c_str()
            << ", scores \x1b[0;33m" << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    std::clog << report.message << '\n';
  }
  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}
// /Impl reporters }}}
}  // namespace cplib::interactor
  

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_VALIDATOR_HPP_
#define CPLIB_VALIDATOR_HPP_

#include <cstddef>      // for size_t
#include <functional>   // for function
#include <map>          // for map
#include <string>       // for string, basic_string
#include <string_view>  // for string_view
#include <vector>       // for vector


  // for Random
   // for UniqueFunction, UniqueFunction<>::UniqueFunct...
     // for Reader


namespace cplib::validator {
/**
 * `Report` represents a report showing when validator exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  class Status {
   public:
    enum Value {
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

  /// The list of names of traits that this trait depends on.
  std::vector<std::string> dependencies;

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
   * @param dependencies The list of names of traits that this trait depends on.
   */
  Trait(std::string name, CheckFunc check_func, std::vector<std::string> dependencies);
};

class State {
 public:
  /// The type of function used to initialize the state.
  using Initializer = UniqueFunction<auto(State& state, int argc, char** argv)->void>;

  /// The type of function used for reporting.
  using Reporter = UniqueFunction<
      auto(const Report& report, const std::map<std::string, bool>& trait_status)->void>;

  /// Random number generator.
  Random rnd;

  /// Input file reader.
  var::Reader inf;

  /// Initializer is a function parsing command line arguments and initializing `validator::State`.
  Initializer initializer;

  /// Reporter is a function that reports the given `validator::Report` and exits the program.
  Reporter reporter;

  /**
   * Constructs a new `State` object with the given initializer function.
   *
   * @param initializer The initializer function.
   */
  explicit State(Initializer initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Sets the traits of the validator.
   *
   * **WARNING**: Calling this function multiple times will overwrite the last trait list.
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
  std::vector<std::vector<std::size_t>> trait_edges_;
};

/**
 * Default initializer of validator.
 */
struct DefaultInitializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param state The state object to be initialized.
   * @param argc The number of command line arguments.
   * @param argv The command line arguments.
   */
  auto operator()(State& state, int argc, char** argv) -> void;
};

/**
 * Report the given report in JSON format.
 *
 * @param report The report to be reported.
 * @param trait_status The status of each trait (satisfied or dissatisfied).
 */
auto json_reporter(const Report& report, const std::map<std::string, bool>& trait_status) -> void;

/**
 * Report the given report in plain text format.
 *
 * @param report The report to be reported.
 * @param trait_status The status of each trait (satisfied or dissatisfied).
 */
auto plain_text_reporter(const Report& report, const std::map<std::string, bool>& trait_status)
    -> void;

/**
 * Report the given report in colored text format.
 *
 * @param report The report to be reported.
 * @param trait_status The status of each trait (satisfied or dissatisfied).
 */
auto colored_text_reporter(const Report& report, const std::map<std::string, bool>& trait_status)
    -> void;

/**
 * Macro to register validator with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_VALIDATOR_OPT(var_, initializer_) \
  auto var_ = ::cplib::validator::State(initializer_);   \
  auto main(signed argc, char** argv)->signed {          \
    var_.initializer(var_, argc, argv);                  \
    auto validator_main(void)->void;                     \
    validator_main();                                    \
    return 0;                                            \
  }

/**
 * Macro to register validator with default initializer.
 *
 * @param var The variable name of state object to be initialized.
 */
#define CPLIB_REGISTER_VALIDATOR(var) \
  CPLIB_REGISTER_VALIDATOR_OPT(var, ::cplib::validator::DefaultInitializer())
}  // namespace cplib::validator

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_VALIDATOR_HPP_
#error "Must be included from validator.hpp"
#endif
#endif


#include <algorithm>    // for copy, max, sort, unique, fill_n, lower_...
#include <cstdint>      // for uint8_t
#include <cstdio>       // for fileno, stderr
#include <cstdlib>      // for exit, EXIT_FAILURE, EXIT_SUCCESS
#include <functional>   // for function
#include <iostream>     // for basic_ostream, operator<<, clog, boolalpha
#include <map>          // for map, operator!=, _Rb_tree_const_iterator
#include <memory>       // for unique_ptr
#include <optional>     // for optional, nullopt
#include <queue>        // for queue
#include <string>       // for basic_string, char_traits, string, allo...
#include <string_view>  // for string_view, operator==, basic_string_view
#include <utility>      // for move, pair
#include <vector>       // for vector


      // for InStream, InStream::eof
  // for isatty, CPLIB_VERSION
   // for panic, format, has_colors, hex_encode


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
      panic(format("Unknown validator report status: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, std::string message)
    : status(status), message(std::move(message)) {}

inline Trait::Trait(std::string name, CheckFunc check_func)
    : Trait(std::move(name), std::move(check_func), {}) {}

inline Trait::Trait(std::string name, CheckFunc check_func, std::vector<std::string> dependencies)
    : name(std::move(name)),
      check_func(std::move(check_func)),
      dependencies(std::move(dependencies)) {}

// Impl State {{{
namespace detail {
/**
 * In topological sorting, `callback` is called every time a new node is reached.
 * If `fn` returns false, nodes reachable by the current node will no longer be visited.
 */
inline auto topo_sort(const std::vector<std::vector<size_t>>& edges,
                      const std::function<auto(size_t)->bool>& callback) -> void {
  std::vector<size_t> degree(edges.size(), 0);

  for (const auto& edge : edges) {
    for (auto to : edge) ++degree[to];
  }

  std::queue<size_t> queue;

  for (size_t i = 0; i < edges.size(); ++i) {
    if (degree[i] == 0) queue.push(i);
  }

  while (!queue.empty()) {
    auto front = queue.front();
    queue.pop();
    if (!callback(front)) continue;
    for (auto to : edges[front]) {
      --degree[to];
      if (!degree[to]) queue.push(to);
    }
  }
}

inline auto build_edges(std::vector<Trait>& traits)
    -> std::optional<std::vector<std::vector<size_t>>> {
  // Check duplicate name
  std::sort(traits.begin(), traits.end(),
            [](const Trait& x, const Trait& y) { return x.name < y.name; });
  if (std::unique(traits.begin(), traits.end(), [](const Trait& x, const Trait& y) {
        return x.name == y.name;
      }) != traits.end()) {
    // Found duplicate name
    return std::nullopt;
  }

  std::vector<std::vector<size_t>> edges(traits.size());

  for (size_t i = 0; i < traits.size(); ++i) {
    auto& trait = traits[i];
    // Check duplicate dependencies
    std::sort(trait.dependencies.begin(), trait.dependencies.end());
    if (std::unique(trait.dependencies.begin(), trait.dependencies.end()) !=
        trait.dependencies.end()) {
      // Found duplicate dependencies
      return std::nullopt;
    }

    for (const auto& dep : trait.dependencies) {
      auto dep_id =
          std::lower_bound(traits.begin(), traits.end(), dep,
                           [](const Trait& x, const std::string& y) { return x.name < y; }) -
          traits.begin();
      edges[dep_id].push_back(i);
    }
  }

  return edges;
}

inline auto have_loop(const std::vector<std::vector<size_t>>& edges) -> bool {
  std::vector<uint8_t> visited(edges.size(), 0);  // Never use std::vector<bool>

  topo_sort(edges, [&](size_t node) {
    visited[node] = 1;
    return true;
  });

  for (auto v : visited) {
    if (!v) return true;
  }
  return false;
}

inline auto validate_traits(const std::vector<Trait>& traits,
                            const std::vector<std::vector<std::size_t>>& edges)
    -> std::map<std::string, bool> {
  std::map<std::string, bool> results;
  for (const auto& trait : traits) results[trait.name] = false;

  topo_sort(edges, [&](std::size_t id) {
    auto& node = traits[id];
    auto result = node.check_func();
    results.at(node.name) = result;
    return result;
  });

  return results;
}
}  // namespace detail

inline State::State(Initializer initializer)
    : rnd(),
      inf(var::Reader(nullptr)),
      initializer(std::move(initializer)),
      reporter(json_reporter),

      traits_(),
      trait_edges_() {
  cplib::detail::panic_impl = [this](std::string_view msg) {
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

  std::map<std::string, bool> trait_status;
  if (report.status == Report::Status::VALID) {
    trait_status = detail::validate_traits(traits_, trait_edges_);
  }

  reporter(report, trait_status);

  std::clog << "Unrecoverable error: Reporter didn't exit the program\n";
  std::exit(EXIT_FAILURE);
}

inline auto State::quit_valid() -> void { quit(Report(Report::Status::VALID, "")); }

inline auto State::quit_invalid(std::string_view message) -> void {
  quit(Report(Report::Status::INVALID, std::string(message)));
}
// /Impl State }}}

// Impl DefaultInitializer {{{
namespace detail {
constexpr std::string_view ARGS_USAGE = "[<input_file>] [--report-format={auto|json|text}]";

inline auto print_help_message(std::string_view program_name) -> void {
  std::string msg =
      format("cplib (CPLib) " CPLIB_VERSION
             "\n"
             "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
             "\n"
             "Usage:\n"
             "  %s %s\n"
             "\n"
             "If <input_file> does not exist, stdin will be used as input\n"
             "\n"
             "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
             "enable colors",
             program_name.data(), ARGS_USAGE.data());
  panic(msg);
}

inline auto detect_reporter(State& state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = json_reporter;
  } else if (cplib::detail::has_colors()) {
    state.reporter = colored_text_reporter;
  } else {
    state.reporter = plain_text_reporter;
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State& state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = json_reporter;
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = colored_text_reporter;
    } else {
      state.reporter = plain_text_reporter;
    }
  } else {
    return false;
  }
  return true;
}

inline auto parse_command_line_arguments(State& state, int argc, char** argv) -> std::string_view {
  std::string_view inf_path;
  int opts_args_start = 2;

  if (argc < 2 || argv[1][0] == '\0' || argv[1][0] == '-') {
    opts_args_start = 1;
  } else {
    inf_path = argv[1];
  }

  for (int i = opts_args_start; i < argc; ++i) {
    auto arg = std::string_view(argv[i]);
    if (constexpr std::string_view prefix = "--report-format=";
        !arg.compare(0, prefix.size(), prefix)) {
      arg.remove_prefix(prefix.size());
      if (!set_report_format(state, arg)) {
        panic(format("Unknown %s option: %s", prefix.data(), arg.data()));
      }
    } else {
      panic("Unknown option: " + std::string(arg));
    }
  }

  return inf_path;
}
}  // namespace detail

inline auto DefaultInitializer::operator()(State& state, int argc, char** argv) -> void {
  detail::detect_reporter(state);

  if (argc > 1 && std::string_view("--help") == argv[1]) {
    detail::print_help_message(argv[0]);
  }

  auto inf_path = detail::parse_command_line_arguments(state, argc, argv);

  std::unique_ptr<std::streambuf> inf_buf = nullptr;
  if (inf_path.empty()) {
    state.inf = var::detail::make_stdin_reader(
        "inf", true, [&state](std::string_view msg) { state.quit_invalid(msg); });
  } else {
    state.inf = var::detail::make_file_reader(
        inf_path, "inf", true, [&state](std::string_view msg) { state.quit_invalid(msg); });
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
      panic(format("Unknown validator report status: %d", static_cast<int>(status)));
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
      panic(format("Unknown validator report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto json_reporter(const Report& report, const std::map<std::string, bool>& trait_status)
    -> void {
  std::clog << std::boolalpha;

  std::clog << R"({"status": ")" << report.status.to_string() << R"(", "message": ")"
            << cplib::detail::json_string_encode(report.message) << "\"";

  if (report.status == Report::Status::VALID) {
    std::clog << ", \"traits\": {";
    bool is_first = true;
    for (auto [name, satisfaction] : trait_status) {
      if (is_first) {
        is_first = false;
      } else {
        std::clog << ", ";
      }
      std::clog << "\"" << cplib::detail::json_string_encode(name) << "\": " << satisfaction;
    }
    std::clog << '}';
  }

  std::clog << "}\n";

  std::exit(report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto plain_text_reporter(const Report& report,
                                const std::map<std::string, bool>& trait_status) -> void {
  std::clog << detail::status_to_title_string(report.status).c_str() << ".\n";

  if (report.status != Report::Status::VALID || !report.message.empty()) {
    std::clog << report.message << '\n';
  }

  if (report.status == Report::Status::VALID && !trait_status.empty()) {
    std::clog << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (auto [name, satisfaction] : trait_status) {
      if (satisfaction) {
        satisfied.push_back(name);
      } else {
        dissatisfied.push_back(name);
      }
    }

    for (const auto& name : satisfied) {
      std::clog << "+ " << cplib::detail::hex_encode(name) << '\n';
    }
    for (const auto& name : dissatisfied) {
      std::clog << "- " << cplib::detail::hex_encode(name) << '\n';
    }
  }

  std::exit(report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto colored_text_reporter(const Report& report,
                                  const std::map<std::string, bool>& trait_status) -> void {
  std::clog << detail::status_to_colored_title_string(report.status).c_str() << ".\n";

  if (report.status != Report::Status::VALID || !report.message.empty()) {
    std::clog << report.message << '\n';
  }

  if (report.status == Report::Status::VALID && !trait_status.empty()) {
    std::clog << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (auto [name, satisfaction] : trait_status) {
      if (satisfaction) {
        satisfied.push_back(name);
      } else {
        dissatisfied.push_back(name);
      }
    }

    for (const auto& name : satisfied) {
      std::clog << "\x1b[0;32m+\x1b[0m " << name << '\n';
    }
    for (const auto& name : dissatisfied) {
      std::clog << "\x1b[0;31m-\x1b[0m " << name << '\n';
    }
  }

  std::exit(report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE);
}
// /Impl reporters }}}
}  // namespace cplib::validator
  

#endif

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_GENERATOR_HPP_
#define CPLIB_GENERATOR_HPP_

#include <any>          // for any
#include <functional>   // for function
#include <map>          // for map
#include <set>          // for set
#include <string>       // for string, basic_string
#include <string_view>  // for string_view
#include <vector>       // for vector


  // for Random


namespace cplib::generator {
/**
 * `Report` represents a report showing when generator exits.
 */
struct Report {
  /**
   * `Status` represents the status of the report.
   */
  class Status {
   public:
    enum Value {
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

class State {
 public:
  /// The type of function used to initialize the state.
  using Initializer = std::function<auto(State& state, int argc, char** argv)->void>;

  /// The type of function used for reporting.
  using Reporter = std::function<auto(const Report& report)->void>;

  /// The parser function of a flag type (`--flag`) command line argument.
  using FlagParser = std::function<auto(std::set<std::string> flag_args)->void>;

  /// The parser function of a variable type (`--var=value`) command line argument.
  using VarParser = std::function<auto(std::map<std::string, std::string> var_args)->void>;

  /// Random number generator.
  Random rnd;

  /// Initializer is a function parsing command line arguments and initializing `generator::State`.
  Initializer initializer;

  /// Reporter is a function that reports the given `generator::Report` and exits the program.
  Reporter reporter;

  /// Names of the flag type (`--flag`) command line arguments required by the generator.
  std::vector<std::string> required_flag_args;

  /// Names of the variable type (`--var=value`) command line arguments required by the generator.
  std::vector<std::string> required_var_args;

  /// Functions to parse flag type command line arguments.
  std::vector<FlagParser> flag_parsers;

  /// Functions to parse variable type command line arguments.
  std::vector<VarParser> var_parsers;

  /**
   * Constructs a new `State` object with the given initializer function.
   *
   * @param initializer The initializer function.
   */
  explicit State(Initializer initializer);

  /**
   * Destroys the `State` object.
   */
  ~State();

  /**
   * Quits the generator with the given report.
   *
   * @param report The report to be reported.
   */
  [[noreturn]] auto quit(const Report& report) -> void;

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
struct DefaultInitializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param state The state object to be initialized.
   * @param argc The number of command line arguments.
   * @param argv The command line arguments.
   */
  auto operator()(State& state, int argc, char** argv) -> void;
};

/**
 * Report the given report in JSON format.
 *
 * @param report The report to be reported.
 */
auto json_reporter(const Report& report) -> void;

/**
 * Report the given report in plain text format for human reading.
 *
 * @param report The report to be reported.
 */
auto plain_text_reporter(const Report& report) -> void;

/**
 * Report the given report in colored text format for human reading.
 *
 * @param report The report to be reported.
 */
auto colored_text_reporter(const Report& report) -> void;

#define CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_)                                 \
  namespace cplib_generator_args_detail_ {                                                       \
  struct AsResultTag_ {};                                                                        \
                                                                                                 \
  std::map<std::string, std::any> value_map_;                                                    \
                                                                                                 \
  struct Flag {                                                                                  \
    using ResultType = bool;                                                                     \
    std::string name;                                                                            \
    explicit Flag(std::string name_) : name(std::move(name_)) {                                  \
      state_var_name_.required_flag_args.emplace_back(name);                                     \
      auto name = this->name;                                                                    \
      state_var_name_.flag_parsers.emplace_back([name](const std::set<std::string>& flag_args) { \
        value_map_[name] = static_cast<ResultType>(flag_args.count(name));                       \
      });                                                                                        \
    }                                                                                            \
    inline auto operator|(AsResultTag_) const -> const ResultType& {                             \
      return *std::any_cast<ResultType>(&(value_map_[name] = ResultType{}));                     \
    }                                                                                            \
  };                                                                                             \
                                                                                                 \
  template <class T>                                                                             \
  struct Var {                                                                                   \
    using ResultType = typename T::Target;                                                       \
    T var;                                                                                       \
    template <class... Args>                                                                     \
    explicit Var(Args... args) : var(std::forward<Args>(args)...) {                              \
      state_var_name_.required_var_args.emplace_back(var.name());                                \
      auto var = this->var;                                                                      \
      state_var_name_.var_parsers.emplace_back(                                                  \
          [var](const std::map<std::string, std::string>& var_args) {                            \
            auto name = std::string(var.name());                                                 \
            value_map_[name] = var.parse(var_args.at(name));                                     \
          });                                                                                    \
    }                                                                                            \
    inline auto operator|(AsResultTag_) const -> const ResultType& {                             \
      return *std::any_cast<ResultType>(&(value_map_[std::string(var.name())] = ResultType{}));  \
    }                                                                                            \
  };                                                                                             \
  }                                                                                              \
  namespace cplib_generator_args_ {                                                              \
  using ::cplib_generator_args_detail_::Flag;                                                    \
  using ::cplib_generator_args_detail_::Var;                                                     \
  }

#define CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(x) x

#define CPLIB_REGISTER_GENERATOR_ARGS_0_()

#define CPLIB_REGISTER_GENERATOR_ARGS_1_(arg)                       \
  namespace cplib_generator_args_ {                                 \
  const auto& arg | ::cplib_generator_args_detail_::AsResultTag_{}; \
  }

#define CPLIB_REGISTER_GENERATOR_ARGS_2_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_1_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_3_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_2_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_4_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_3_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_5_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_4_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_6_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_5_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_7_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_6_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_8_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_7_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_9_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)            \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_8_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_10_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_9_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_11_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_10_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_12_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_11_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_13_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_12_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_14_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_13_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_15_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_14_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_16_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_15_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_17_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_16_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_18_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_17_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_19_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_18_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_20_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_19_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_21_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_20_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_22_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_21_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_23_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_22_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_24_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_23_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_25_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_24_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_26_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_25_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_27_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_26_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_28_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_27_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_29_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_28_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_30_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_29_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_31_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_30_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_32_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_31_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_33_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_32_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_34_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_33_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_35_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_34_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_36_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_35_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_37_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_36_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_38_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_37_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_39_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_38_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_40_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_39_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_41_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_40_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_42_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_41_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_43_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_42_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_44_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_43_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_45_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_44_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_46_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_45_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_47_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_46_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_48_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_47_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_49_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_48_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_50_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_49_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_51_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_50_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_52_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_51_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_53_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_52_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_54_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_53_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_55_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_54_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_56_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_55_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_57_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_56_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_58_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_57_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_59_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_58_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_60_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_59_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_61_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_60_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_62_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_61_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_63_(arg0, ...) \
  CPLIB_REGISTER_GENERATOR_ARGS_1_(arg0)             \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_62_(__VA_ARGS__))

#define CPLIB_REGISTER_GENERATOR_ARGS_GET_NTH_ARG_(                                                \
    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, \
    _59, _60, _61, _62, _63, N, ...)                                                               \
  N

#define CPLIB_REGISTER_GENERATOR_ARGS_(...)                                                       \
  CPLIB_REGISTER_GENERATOR_ARGS_EXPAND_(CPLIB_REGISTER_GENERATOR_ARGS_GET_NTH_ARG_(               \
      dummy, ##__VA_ARGS__, CPLIB_REGISTER_GENERATOR_ARGS_63_, CPLIB_REGISTER_GENERATOR_ARGS_62_, \
      CPLIB_REGISTER_GENERATOR_ARGS_61_, CPLIB_REGISTER_GENERATOR_ARGS_60_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_59_, CPLIB_REGISTER_GENERATOR_ARGS_58_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_57_, CPLIB_REGISTER_GENERATOR_ARGS_56_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_55_, CPLIB_REGISTER_GENERATOR_ARGS_54_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_53_, CPLIB_REGISTER_GENERATOR_ARGS_52_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_51_, CPLIB_REGISTER_GENERATOR_ARGS_50_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_49_, CPLIB_REGISTER_GENERATOR_ARGS_48_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_47_, CPLIB_REGISTER_GENERATOR_ARGS_46_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_45_, CPLIB_REGISTER_GENERATOR_ARGS_44_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_43_, CPLIB_REGISTER_GENERATOR_ARGS_42_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_41_, CPLIB_REGISTER_GENERATOR_ARGS_40_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_39_, CPLIB_REGISTER_GENERATOR_ARGS_38_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_37_, CPLIB_REGISTER_GENERATOR_ARGS_36_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_35_, CPLIB_REGISTER_GENERATOR_ARGS_34_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_33_, CPLIB_REGISTER_GENERATOR_ARGS_32_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_31_, CPLIB_REGISTER_GENERATOR_ARGS_30_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_29_, CPLIB_REGISTER_GENERATOR_ARGS_28_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_27_, CPLIB_REGISTER_GENERATOR_ARGS_26_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_25_, CPLIB_REGISTER_GENERATOR_ARGS_24_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_23_, CPLIB_REGISTER_GENERATOR_ARGS_22_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_21_, CPLIB_REGISTER_GENERATOR_ARGS_20_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_19_, CPLIB_REGISTER_GENERATOR_ARGS_18_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_17_, CPLIB_REGISTER_GENERATOR_ARGS_16_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_15_, CPLIB_REGISTER_GENERATOR_ARGS_14_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_13_, CPLIB_REGISTER_GENERATOR_ARGS_12_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_11_, CPLIB_REGISTER_GENERATOR_ARGS_10_,                       \
      CPLIB_REGISTER_GENERATOR_ARGS_9_, CPLIB_REGISTER_GENERATOR_ARGS_8_,                         \
      CPLIB_REGISTER_GENERATOR_ARGS_7_, CPLIB_REGISTER_GENERATOR_ARGS_6_,                         \
      CPLIB_REGISTER_GENERATOR_ARGS_5_, CPLIB_REGISTER_GENERATOR_ARGS_4_,                         \
      CPLIB_REGISTER_GENERATOR_ARGS_3_, CPLIB_REGISTER_GENERATOR_ARGS_2_,                         \
      CPLIB_REGISTER_GENERATOR_ARGS_1_, CPLIB_REGISTER_GENERATOR_ARGS_0_)(__VA_ARGS__))

/**
 * Macro to register generator with custom initializer.
 *
 * @param var_name_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 * @param args_namespace_name_ The name of the command line arguments namespace.
 * @param ... The parsers of the command line arguments.
 */
#define CPLIB_REGISTER_GENERATOR_OPT(state_var_name_, initializer_, args_namespace_name_, ...) \
  auto state_var_name_ = ::cplib::generator::State(initializer_);                              \
  CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_);                                    \
  CPLIB_REGISTER_GENERATOR_ARGS_(__VA_ARGS__);                                                 \
  namespace args_namespace_name_ = ::cplib_generator_args_;                                    \
  auto main(signed argc, char** argv)->signed {                                                \
    state_var_name_.initializer(state_var_name_, argc, argv);                                  \
    auto generator_main(void)->void;                                                           \
    generator_main();                                                                          \
    return 0;                                                                                  \
  }

/**
 * Macro to register generator with default initializer.
 *
 * @param var_name_ The variable name of state object to be initialized.
 * @param args_namespace_name_ The name of the command line arguments namespace.
 * @param ... The parsers of the command line arguments.
 */
#define CPLIB_REGISTER_GENERATOR(var_name_, args_namespace_name_, ...)              \
  CPLIB_REGISTER_GENERATOR_OPT(var_name_, ::cplib::generator::DefaultInitializer(), \
                               args_namespace_name_, __VA_ARGS__)
}  // namespace cplib::generator

/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_GENERATOR_HPP_
#error "Must be included from generator.hpp"
#endif
#endif


#include <algorithm>    // for binary_search, sort
#include <cstdio>       // for fileno, stderr
#include <cstdlib>      // for exit, EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>     // for basic_ostream, operator<<, clog, boolalpha
#include <map>          // for map, _Rb_tree_iterator, operator!=
#include <optional>     // for optional, nullopt
#include <set>          // for set
#include <string>       // for basic_string, char_traits, allocator, operator<
#include <string_view>  // for operator==, string_view, basic_string_view
#include <utility>      // for pair, move
#include <vector>       // for vector


  // for isatty, CPLIB_VERSION
   // for panic, format, has_colors, join, json_string_...


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
      panic(format("Unknown generator report status: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, std::string message)
    : status(status), message(std::move(message)) {}

// Impl State {{{
inline State::State(Initializer initializer)
    : rnd(),
      initializer(std::move(initializer)),
      reporter(json_reporter),
      required_flag_args(),
      required_var_args(),
      flag_parsers(),
      var_parsers() {
  cplib::detail::panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::GENERATOR;
}

inline State::~State() {
  if (!exited_) panic("Generator must exit by calling method `State::quit*`");
}

inline auto State::quit(const Report& report) -> void {
  exited_ = true;

  reporter(report);

  std::clog << "Unrecoverable error: Reporter didn't exit the program\n";
  std::exit(EXIT_FAILURE);
}

inline auto State::quit_ok() -> void { quit(Report(Report::Status::OK, "")); }
// /Impl State }}}

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
      format("cplib (CPLib) " CPLIB_VERSION
             "\n"
             "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
             "\n"
             "Usage:\n"
             "  %s %s\n"
             "\n"
             "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
             "enable colors",
             program_name.data(), args_usage.data());
  panic(msg);
}

inline auto detect_reporter(State& state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = json_reporter;
  } else if (cplib::detail::has_colors()) {
    state.reporter = colored_text_reporter;
  } else {
    state.reporter = plain_text_reporter;
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State& state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = json_reporter;
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = colored_text_reporter;
    } else {
      state.reporter = plain_text_reporter;
    }
  } else {
    return false;
  }
  return true;
}

inline auto parse_command_line_arguments(State& state, int argc, char** argv)
    -> std::pair<std::set<std::string>, std::map<std::string, std::string>> {
  std::set<std::string> flag_args;
  std::map<std::string, std::string> var_args;

  for (int i = 1; i < argc; ++i) {
    auto arg = std::string_view(argv[i]);
    if (constexpr std::string_view prefix = "--report-format=";
        !arg.compare(0, prefix.size(), prefix)) {
      arg.remove_prefix(prefix.size());
      if (!set_report_format(state, arg)) {
        panic(format("Unknown %s option: %s", prefix.data(), arg.data()));
      }
    } else {
      auto [name, value] = parse_arg(arg);
      if (!value.has_value()) {
        if (!std::binary_search(state.required_flag_args.begin(), state.required_flag_args.end(),
                                name)) {
          panic("Unknown flag: " + name);
        }
        flag_args.insert(name);
      } else {
        if (!std::binary_search(state.required_var_args.begin(), state.required_var_args.end(),
                                name)) {
          panic("Unknown variable: " + name);
        }
        if (auto it = var_args.find(name); it != var_args.end()) {
          it->second.push_back(' ');
          it->second += *value;
        } else {
          var_args[name] = *value;
        }
      }
    }
  }

  return {flag_args, var_args};
}

inline auto validate_required_arguments(const State& state,
                                        const std::map<std::string, std::string>& var_args)
    -> void {
  for (const auto& var : state.required_var_args) {
    if (!var_args.count(var)) panic("Missing variable: " + var);
  }
}

inline auto get_args_usage(const State& state) {
  using namespace std::string_literals;
  std::vector<std::string> builder;
  builder.reserve(state.required_flag_args.size() + state.required_var_args.size());
  for (const auto& arg : state.required_flag_args) builder.push_back("[--"s + arg + "]"s);
  for (const auto& arg : state.required_var_args) builder.push_back("--"s + arg + "=<value>"s);
  builder.push_back("[--report-format={auto|json|text}]"s);

  return join(builder.begin(), builder.end(), ' ');
}
}  // namespace detail

inline auto DefaultInitializer::operator()(State& state, int argc, char** argv) -> void {
  std::sort(state.required_flag_args.begin(), state.required_flag_args.end());
  std::sort(state.required_var_args.begin(), state.required_var_args.end());

  auto args_usage = detail::get_args_usage(state);

  detail::detect_reporter(state);

  if (argc > 1 && std::string_view("--help") == argv[1]) {
    detail::print_help_message(argv[0], args_usage);
  }

  auto [flag_args, var_args] = detail::parse_command_line_arguments(state, argc, argv);

  detail::validate_required_arguments(state, var_args);

  for (const auto& parser : state.flag_parsers) parser(flag_args);
  for (const auto& parser : state.var_parsers) parser(var_args);

  // Unsynchronize to speed up std::cout output.
  std::ios_base::sync_with_stdio(false);

  state.rnd.reseed(argc, argv);
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
      panic(format("Unknown generator report status: %d", static_cast<int>(status)));
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
      panic(format("Unknown generator report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto json_reporter(const Report& report) -> void {
  std::clog << std::boolalpha;

  std::clog << R"({"status": ")" << report.status.to_string() << R"(", "message": ")"
            << cplib::detail::json_string_encode(report.message) << "\"}";

  std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto plain_text_reporter(const Report& report) -> void {
  if (report.status == Report::Status::OK && report.message.empty()) {
    // Do nothing when the report is OK and message is empty.
    std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  std::clog << detail::status_to_title_string(report.status).c_str() << ".\n"
            << report.message << '\n';

  std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto colored_text_reporter(const Report& report) -> void {
  if (report.status == Report::Status::OK && report.message.empty()) {
    // Do nothing when the report is OK and message is empty.
    std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  std::clog << detail::status_to_colored_title_string(report.status).c_str() << ".\n"
            << report.message << '\n';

  std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
}
// /Impl reporters }}}
}  // namespace cplib::generator
  

#endif


// clang-format on

#endif  // CPLIB_HPP_
