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
  "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023-2025\n"

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
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifdef CPLIB_USE_FMT_LIB
#define FMT_HEADER_ONLY
#include "fmt/base.h"
#include "fmt/core.h"
#else
#include <format>
#endif

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
#ifdef CPLIB_USE_FMT_LIB
template <class... Args>
[[nodiscard]] auto format(fmt::format_string<Args...> fmt, Args &&...args) -> std::string;

template <typename T>
concept formattable = requires(T &v, fmt::format_context ctx) {
  fmt::formatter<std::remove_cvref_t<T> >().format(v, ctx);
};
#else
template <class... Args>
[[nodiscard]] auto format(std::format_string<Args...> fmt, Args &&...args) -> std::string;

template <typename T>
concept formattable = requires(T &v, std::format_context ctx) {
  std::formatter<std::remove_cvref_t<T> >().format(v, ctx);
};
#endif

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_UTILS_HPP_
#error "Must be included from utils.hpp"
#endif
#endif


#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
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

// Impl get_work_mode {{{
namespace detail {
inline WorkMode work_mode = WorkMode::NONE;
}

inline auto get_work_mode() -> WorkMode { return detail::work_mode; }
// /Impl get_work_mode }}}
}  // namespace cplib
  

#endif

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
rand() CPLIB_RAND_THROW_STATEMENT -> int {
  cplib::panic("Don not use `rand`, use `rnd.next` instead");
}

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((error("Don not use `srand`, you should use `cplib::Random` for random generator")))
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4273)
#endif
inline auto
srand(unsigned int) CPLIB_RAND_THROW_STATEMENT -> void {
  cplib::panic("Don not use `srand`, you should use `cplib::Random` for random generator");
}

#endif

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
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace cplib::json {

struct Value;

using String = std::string;
using Int = std::int64_t;
using Real = double;
using Bool = bool;
using List = std::vector<Value>;
using Map = std::map<std::string, Value>;

struct Value {
  std::variant<String, Int, Real, Bool, List, Map> inner;

  [[nodiscard]] auto to_string() const -> std::string;

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
};

}  // namespace cplib::json

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


#include <string_view>
#include <type_traits>
#include <variant>
#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_JSON_HPP_
#error "Must be included from json.hpp"
#endif
#endif


#include <ios>
#include <sstream>
#include <string>
#include <utility>





namespace cplib::json {

namespace detail {
inline auto encode_string(std::string_view inner) -> std::string {
  std::stringbuf buf(std::ios_base::out);
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
  return buf.str();
}

inline auto encode_list(const List &inner) -> std::string {
  std::stringbuf buf(std::ios_base::out);
  buf.sputc('[');
  if (!inner.empty()) {
    auto it = inner.begin();
    auto tmp = it->to_string();
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      tmp = it->to_string();
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    }
  }
  buf.sputc(']');
  return buf.str();
}

inline auto encode_map(const Map &inner) -> std::string {
  std::stringbuf buf(std::ios_base::out);
  buf.sputc('{');
  if (!inner.empty()) {
    auto it = inner.begin();
    buf.sputc('\"');
    auto tmp = it->first;
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    buf.sputc('\"');
    buf.sputc(':');
    tmp = it->second.to_string();
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      buf.sputc('\"');
      tmp = it->first;
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
      buf.sputc('\"');
      buf.sputc(':');
      tmp = it->second.to_string();
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    }
  }
  buf.sputc('}');
  return buf.str();
}
}  // namespace detail

[[nodiscard]] inline auto Value::to_string() const -> std::string {
  return std::visit(
      [](const auto &arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, String>) {
          return detail::encode_string(arg);
        } else if constexpr (std::is_same_v<T, Int>) {
          return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, Real>) {
          return cplib::format("{:.10g}", arg);
        } else if constexpr (std::is_same_v<T, Bool>) {
          return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, List>) {
          return detail::encode_list(arg);
        } else if constexpr (std::is_same_v<T, Map>) {
          return detail::encode_map(arg);
        } else {
          panic("JSON value to string failed: unknown type");
        }
      },
      inner);
}

[[nodiscard]] auto Value::is_string() const -> bool {
  return std::holds_alternative<String>(inner);
}
[[nodiscard]] auto Value::is_int() const -> bool { return std::holds_alternative<Int>(inner); }
[[nodiscard]] auto Value::is_real() const -> bool { return std::holds_alternative<Real>(inner); }
[[nodiscard]] auto Value::is_bool() const -> bool { return std::holds_alternative<Bool>(inner); }
[[nodiscard]] auto Value::is_list() const -> bool { return std::holds_alternative<List>(inner); }
[[nodiscard]] auto Value::is_map() const -> bool { return std::holds_alternative<Map>(inner); }

[[nodiscard]] auto Value::as_string() -> String & { return std::get<String>(inner); }
[[nodiscard]] auto Value::as_string() const -> const String & { return std::get<String>(inner); }

[[nodiscard]] auto Value::as_int() -> Int & { return std::get<Int>(inner); }
[[nodiscard]] auto Value::as_int() const -> const Int & { return std::get<Int>(inner); }

[[nodiscard]] auto Value::as_real() -> Real & { return std::get<Real>(inner); }
[[nodiscard]] auto Value::as_real() const -> const Real & { return std::get<Real>(inner); }

[[nodiscard]] auto Value::as_bool() -> Bool & { return std::get<Bool>(inner); }
[[nodiscard]] auto Value::as_bool() const -> const Bool & { return std::get<Bool>(inner); }

[[nodiscard]] auto Value::as_list() -> List & { return std::get<List>(inner); }
[[nodiscard]] auto Value::as_list() const -> const List & { return std::get<List>(inner); }

[[nodiscard]] auto Value::as_map() -> Map & { return std::get<Map>(inner); }
[[nodiscard]] auto Value::as_map() const -> const Map & { return std::get<Map>(inner); }

}  // namespace cplib::json
  

#endif

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

#include <memory>
#include <string>
#include <string_view>
#include <utility>

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_PATTERN_HPP_
#error "Must be included from pattern.hpp"
#endif
#endif


#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "regex.h"





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
  // In function `regexec`, a match anywhere within the string is considered successful unless the
  // regular expression contains `^` or `$`.
  if (int err = regcomp(&re_->first, ("^" + src_ + "$").c_str(), REG_EXTENDED | REG_NOSUB); err) {
    auto err_msg = detail::get_regex_err_msg(err, &re_->first);
    panic("Pattern constructor failed: " + err_msg);
  }
  re_->second = true;
}

inline auto Pattern::match(std::string_view s) const -> bool {
  int result = regexec(&re_->first, std::string(s).c_str(), 0, nullptr, 0);

  if (!result) return true;

  if (result == REG_NOMATCH) return false;

  auto err_msg = detail::get_regex_err_msg(result, &re_->first);
  panic("Pattern match failed: " + err_msg);
  return false;
}

inline auto Pattern::src() const -> std::string_view { return src_; }
}  // namespace cplib
  

#endif

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

#include <cstdint>
#include <initializer_list>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

namespace cplib {
/**
 * Random number generator that provides various methods to generate random numbers and perform
 * random operations.
 */
struct Random {
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
   * Construct a random generator with seed which generated command-line arguments.
   *
   * @param args The command-line arguments.
   */
  explicit Random(const std::vector<std::string>& args);

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
  auto reseed(const std::vector<std::string>& args) -> void;

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
  auto next(T from, T to) -> std::enable_if_t<std::is_integral_v<T>, T>;

  /**
   * Generate a random floating-point number in the range [from, to].
   *
   * @tparam T The type of the floating-point number.
   * @param from The lower bound of the range.
   * @param to The upper bound of the range.
   * @return The randomly generated floating-point number.
   */
  template <class T>
  auto next(T from, T to) -> std::enable_if_t<std::is_floating_point_v<T>, T>;

  /**
   * Generate a random boolean value.
   *
   * @tparam T The type of the boolean.
   * @return The randomly generated boolean value.
   */
  template <class T>
  auto next() -> std::enable_if_t<std::is_same_v<T, bool>, bool>;

  /**
   * Generate a random boolean value with a given probability of being true.
   *
   * @tparam T The type of the boolean.
   * @param true_prob The probability of the boolean being true.
   * @return The randomly generated boolean value.
   */
  template <class T>
  auto next(double true_prob) -> std::enable_if_t<std::is_same_v<T, bool>, bool>;

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_RANDOM_HPP_
#error "Must be included from random.hpp"
#endif
#endif


#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>





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

  if (l > r) panic("Rand_int_between failed: l must be <= r");

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
  if (l > r) panic("Rand_float_between failed: l must be <= r");

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
  

#endif

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_IO_HPP_
#error "Must be included from io.hpp"
#endif
#endif


#include <array>
#include <cassert>
#include <cctype>
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

InBuf::InBuf(int fd) : fd_(fd), need_close_(false) {
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

inline InStream::InStream(std::unique_ptr<std::streambuf> buf, std::string name, bool strict)
    : buf_(std::move(buf)), name_(std::move(name)), strict_(strict) {}

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
    if (write(fd_, &z, 1) != 1) {
      return EOF;
    }
  }
  return c;
}

inline auto OutBuf::xsputn(const char* s, std::streamsize num) -> std::streamsize {
  return write(fd_, s, num);
}

namespace detail {
// Open the givin file and create a `std::::ostream`
inline auto make_ostream_by_path(std::string_view path, std::unique_ptr<std::streambuf>& buf,
                                 std::ostream& stream) -> void {
  buf = std::make_unique<io::OutBuf>(path);
  stream.rdbuf(buf.get());
}

// Use file with givin fileno as output stream and create a `std::::ostream`
inline auto make_ostream_by_fileno(int fileno, std::unique_ptr<std::streambuf>& buf,
                                   std::ostream& stream) -> void {
  buf = std::make_unique<io::OutBuf>(fileno);
  stream.rdbuf(buf.get());
}
}  // namespace detail
}  // namespace cplib::io
  

#endif

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
concept Trace = requires(const T& t) {
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
  TraceTreeNode(const TraceTreeNode&) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const TraceTreeNode&) -> TraceTreeNode& = delete;

  /// Move constructor.
  TraceTreeNode(TraceTreeNode&&) noexcept = default;

  /// Move assignment operator.
  auto operator=(TraceTreeNode&&) noexcept -> TraceTreeNode& = default;

  /**
   * Get the children of the node.
   *
   * @return The children of the node.
   */
  [[nodiscard]] auto get_children() const -> const std::vector<std::unique_ptr<TraceTreeNode>>&;

  /**
   * Get the parent of the node.
   *
   * @return The parent of the node.
   */
  [[nodiscard]] auto get_parent() -> TraceTreeNode*;

  /**
   * Convert to JSON value.
   *
   * If node has tag `#hidden`, return `nullptr`.
   *
   * @return The JSON value or nullptr.
   */
  [[nodiscard]] auto to_json() const -> json::Map;

  /**
   * Convert a node to its child and return it again (as reference).
   *
   * @param child The child node.
   * @return The child node.
   */
  auto add_child(std::unique_ptr<TraceTreeNode> child) -> std::unique_ptr<TraceTreeNode>&;

 private:
  std::vector<std::unique_ptr<TraceTreeNode>> children_{};
  TraceTreeNode* parent_{nullptr};
};

/**
 * `Traced` represents a base class for objects that manage trace information.
 */
template <Trace T>
struct Traced {
 public:
  explicit Traced(Level trace_level, T root);
  virtual ~Traced() = 0;
  Traced(const Traced&) = delete;
  auto operator=(const Traced&) -> Traced& = delete;
  Traced(Traced&&) noexcept = default;
  auto operator=(Traced&&) noexcept -> Traced& = default;

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
  [[nodiscard]] auto get_trace_tree() const -> const TraceTreeNode<T>*;

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
  auto get_current_trace() const -> const T&;

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
  TraceTreeNode<T>* trace_tree_current_;
};

}  // namespace cplib::trace

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_TRACE_HPP_
#error "Must be included from trace.hpp"
#endif
#endif


#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <map>
#include <memory>
#include <sstream>
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
  for (const auto& trace : stack) {
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
  for (const auto& trace : stack) {
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
  for (const auto& trace : stack) {
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
    -> const std::vector<std::unique_ptr<TraceTreeNode>>& {
  return children_;
}

template <Trace T>
[[nodiscard]] inline auto TraceTreeNode<T>::get_parent() -> TraceTreeNode* {
  return parent_;
}

template <Trace T>
[[nodiscard]] inline auto TraceTreeNode<T>::to_json() const -> json::Map {
  json::Map map;

  if (tags.count("#hidden")) {
    return {};
  }

  map.emplace("trace", trace.to_tree_json());
  if (!tags.empty()) {
    map.emplace("tags", tags);
  }

  const auto& children = get_children();
  json::List children_list;
  children_list.reserve(children.size());
  for (const auto& child : children) {
    auto child_value = child->to_json();
    if (!child_value.empty()) {
      children_list.emplace_back(std::move(child_value));
    }
  }
  if (!children_list.empty()) {
    map.emplace("children", std::move(children_list));
  }

  return map;
}

template <Trace T>
inline auto TraceTreeNode<T>::add_child(std::unique_ptr<TraceTreeNode> child)
    -> std::unique_ptr<TraceTreeNode>& {
  child->parent_ = this;
  return children_.emplace_back(std::move(child));
}

template <Trace T>
inline Traced<T>::Traced(Level trace_level, T root)
    : trace_level_(trace_level),
      active_traces_({root}),
      trace_tree_root_(std::make_unique<TraceTreeNode<T>>(std::move(root))),
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
[[nodiscard]] inline auto Traced<T>::get_trace_tree() const -> const TraceTreeNode<T>* {
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
inline auto Traced<T>::get_current_trace() const -> const T& {
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

  auto& child =
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


#endif

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
  double score;
  std::string message;

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
  constexpr auto operator<=>(const Result& other) const -> std::strong_ordering;

  /// Scales the score of the Result by a given factor.
  /// @param scale The scaling factor.
  /// @return A new Result object with the scaled score. Status and messages remain unchanged.
  [[nodiscard]] auto operator*(double scale) const -> Result;

  /// Scales the score of this Result by a given factor in-place. The message remains unchanged.
  /// @param scale The scaling factor.
  /// @return A reference to this Result object.
  auto operator*=(double scale) -> Result&;

  /// Combines two Results by adding their scores. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// @param other The Result to add.
  /// @return A new Result object representing the sum.
  [[nodiscard]] auto operator+(const Result& other) const -> Result;

  /// Combines this Result with another by adding scores in-place. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// @param other The Result to add.
  /// @return A reference to this Result object.
  auto operator+=(const Result& other) -> Result&;

  /// Combines two Results by taking the minimum of their scores. The message remains unchanged.
  /// The resulting status is the "worst" (minimum enum value) of the two statuses.
  /// This is typically used for "all or nothing" scenarios or combining results where
  /// the lowest score dictates the overall outcome.
  /// @param other The Result to combine with (min operation).
  /// @return A new Result object representing the minimum.
  [[nodiscard]] auto operator&(const Result& other) const -> Result;

  /// Combines this Result with another by taking the minimum of their scores in-place. The message
  /// remains unchanged. The resulting status is the "worst" (minimum enum value) of the two
  /// statuses.
  /// @param other The Result to combine with (min operation).
  /// @return A reference to this Result object.
  auto operator&=(const Result& other) -> Result&;

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
concept Evaluatable = requires(Evaluator& ev, const T& pans, const T& jans, Args&&... args) {
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
  using FailFunc = UniqueFunction<auto(const Evaluator&, std::string_view)->void>;
  using EvaluationHook = UniqueFunction<auto(const Evaluator&, const Result&)->void>;

  /**
   * Create an evaluator.
   */
  explicit Evaluator(trace::Level trace_level, FailFunc fail_func, EvaluationHook evaluation_hook);

  /// Copy constructor (deleted to prevent copying).
  Evaluator(const Evaluator&) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const Evaluator&) -> Evaluator& = delete;

  /// Move constructor.
  Evaluator(Evaluator&&) noexcept = default;

  /// Move assignment operator.
  auto operator=(Evaluator&&) noexcept -> Evaluator& = default;

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
  auto operator()(std::string_view var_name, const T& pans, const T& jans, Args... args) -> Result
    requires Evaluatable<T, Args...>;

  template <std::equality_comparable T>
  auto eq(std::string_view var_name, const T& pans, const T& jans) -> Result;

  template <std::floating_point T>
  auto approx(std::string_view var_name, const T& pans, const T& jans, const T& max_err) -> Result;

  template <class T>
  auto approx_abs(std::string_view var_name, const T& pans, const T& jans, const T& abs_err)
      -> Result
    requires std::is_arithmetic_v<T>;

 private:
  FailFunc fail_func_;
  EvaluationHook evaluation_hook_;

  auto pre_evaluate(std::string_view var_name) -> void;
  auto post_evaluate(const Result& result) -> void;
};

}  // namespace cplib::evaluate

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_EVALUATE_HPP_
#error "Must be included from evaluate.hpp"
#endif
#endif


#include <algorithm>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdio>
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

namespace detail {
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

inline constexpr auto Result::operator<=>(const Result& other) const -> std::strong_ordering {
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

inline auto Result::operator*=(double scale) -> Result& {
  score *= scale;
  return *this;
}

inline auto Result::operator+(const Result& other) const -> Result {
  Result res;
  res.status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  res.score = score + other.score;
  res.message = message;
  return res;
}

inline auto Result::operator+=(const Result& other) -> Result& {
  status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  score += other.score;
  return *this;
}

inline auto Result::operator&(const Result& other) const -> Result {
  Result res;
  res.status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  res.score = std::min(score, other.score);
  res.message = message;
  return res;
}

inline auto Result::operator&=(const Result& other) -> Result& {
  status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  score = std::min(score, other.score);
  return *this;
}

[[nodiscard]] inline auto Result::to_json() const -> json::Map {
  return {
      {"status", json::Value(json::String(status.to_string()))},
      {"score", json::Value(score)},
      {"message", json::Value(message)},
  };
}

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
    push_trace(trace);
  }
}

inline auto Evaluator::post_evaluate(const Result& result) -> void {
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
inline auto Evaluator::operator()(std::string_view var_name, const T& pans, const T& jans,
                                  Args... args) -> Result
  requires Evaluatable<T, Args...>
{
  pre_evaluate(var_name);
  Result result = T::evaluate(*this, pans, jans, std::forward<Args>(args)...);
  post_evaluate(result);
  return result;
}

template <std::equality_comparable T>
inline auto Evaluator::eq(std::string_view var_name, const T& pans, const T& jans) -> Result {
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
inline auto Evaluator::approx(std::string_view var_name, const T& pans, const T& jans,
                              const T& max_err) -> Result {
  pre_evaluate(var_name);
  Result result = Result::ac();
  if (!float_equals(jans, pans, max_err)) {
    T delta = float_delta(jans, pans);
    result = Result::wa(cplib::format(
        "`{}` is not approximately equal: expected {:.10g}, got {:.10g}, delta {:.10g}", var_name,
        jans, pans, delta));
  }
  post_evaluate(result);
  return result;
}

template <class T>
inline auto Evaluator::approx_abs(std::string_view var_name, const T& pans, const T& jans,
                                  const T& abs_err) -> Result
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
  

#endif

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
  using FailFunc = UniqueFunction<auto(const Reader&, std::string_view)->void>;

  /// Create a reader of input stream.
  explicit Reader(std::unique_ptr<io::InStream> inner, trace::Level trace_level,
                  FailFunc fail_func);

  /// Copy constructor (deleted to prevent copying).
  Reader(const Reader&) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const Reader&) -> Reader& = delete;

  /// Move constructor.
  Reader(Reader&&) noexcept = default;

  /// Move assignment operator.
  auto operator=(Reader&&) noexcept -> Reader& = default;

  /**
   * Get the inner wrapped input stream.
   *
   * @return Reference to the inner input stream.
   */
  [[nodiscard]] auto inner() -> io::InStream&;

  /**
   * Get the inner wrapped input stream.
   *
   * @return Reference to the inner input stream.
   */
  [[nodiscard]] auto inner() const -> const io::InStream&;

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
  auto read_from(Reader& in) const -> T override;
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
  auto read_from(Reader& in) const -> T override;
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
  auto read_from(Reader& in) const -> T override;
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
  auto read_from(Reader& in) const -> bool override;
};

/**
 * `String` is a variable reading template, indicating to read a whitespace separated string.
 */
struct String : Var<std::string, String> {
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
struct Separator : Var<std::nullopt_t, Separator> {
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
struct Line : Var<std::string, Line> {
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
  auto read_from(Reader& in) const -> std::vector<typename T::Var::Target> override;
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
  auto read_from(Reader& in) const -> std::vector<std::vector<typename T::Var::Target>> override;
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
  auto read_from(Reader& in) const
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
  auto read_from(Reader& in) const -> std::tuple<typename T::Var::Target...> override;
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

 private:
  /// The inner function.
  std::function<typename std::function<F>::result_type(Reader& in)> inner_function_;
};

// Defines the requirements for a type T to be "readable"
// with a static 'read' method that takes a Reader and additional arguments.
template <typename T, typename... Args>
concept Readable = requires(Reader& reader, Args&&... args) {
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
  explicit ExtVar(std::string name, Args... args)
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
  auto read_from(Reader& in) const -> T override;

 private:
  /// The inner function that encapsulates the call to T::read.
  std::function<T(Reader&)> inner_function_;
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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_VAR_HPP_
#error "Must be included from var.hpp"
#endif
#endif


#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <concepts>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <ios>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>









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

inline auto Reader::inner() -> io::InStream& { return *inner_; }

inline auto Reader::inner() const -> const io::InStream& { return *inner_; }

[[noreturn]] inline auto Reader::fail(std::string_view message) -> void {
  fail_func_(*this, message);
  std::exit(EXIT_FAILURE);
}

template <class T, class D>
inline auto Reader::read(const Var<T, D>& v) -> T {
  auto trace_level = get_trace_level();

  if (trace_level >= trace::Level::STACK_ONLY) {
    ReaderTrace trace{std::string(v.name()), inner().pos()};
    push_trace(trace);
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
  auto reader = Reader(std::make_unique<io::InStream>(std::move(buf), "str", true),
                       trace::Level::NONE, [](const Reader&, std::string_view msg) {
                         panic(std::string("Var::parse failed: ") + msg.data());
                       });
  T result = reader.read(*this);
  if (!reader.inner().eof()) {
    panic("Var::parse failed, extra characters in string");
  }
  return result;
}

template <class T, class D>
inline auto Var<T, D>::operator*(std::size_t len) const -> Vec<D> {
  return Vec<D>(*static_cast<const D*>(this), len);
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
inline auto Int<T>::read_from(Reader& in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected an integer, got EOF");
    } else {
      in.fail(cplib::format("Expected an integer, got whitespace `{}`",
                            cplib::detail::hex_encode(in.inner().seek())));
    }
  }

  T result{};
  const char* first = token.data();
  const char* last = token.data() + token.length();
  auto [ptr, ec] = std::from_chars(first, last, result);

  if (ec == std::errc::invalid_argument || ptr != last) {
    // * ec == std::errc::invalid_argument: String is not a valid integer format (e.g. "abc",
    //   "NaN", "Inf")
    // * ptr != last: The string is not fully parsed (for example "123abc")
    in.fail(cplib::format("Expected a integer, got `{}`", compress(token)));
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
    in.attach_tag("#t", json::Value("i"));
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
inline auto Float<T>::read_from(Reader& in) const -> T {
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
  const char* first = token.data();
  const char* last = token.data() + token.length();

  // `Float<T>` usually uses with non-strict streams, so it should support both fixed format and
  // scientific.
  auto [ptr, ec] = std::from_chars(first, last, result, std::chars_format::general);

  if (ec == std::errc::invalid_argument || ptr != last) {
    // * ec == std::errc::invalid_argument: String is not a valid floating point format (e.g. "abc",
    //   "NaN", "Inf")
    // * ptr != last: The string is not fully parsed (for example "123abc")
    in.fail(cplib::format("Expected a float, got `{}`", compress(token)));
  } else if (ec == std::errc::result_out_of_range) {
    // The parsing is successful, but the value exceeds the range of T
    in.fail(cplib::format("Float value `{}` is out of range for type `{}`", compress(token),
                          typeid(T).name()));
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
    in.attach_tag("#t", json::Value("f"));
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
inline auto StrictFloat<T>::read_from(Reader& in) const -> T {
  auto token = in.inner().read_token();

  if (token.empty()) {
    if (in.inner().eof()) {
      in.fail("Expected a strict float, got EOF");
    } else {
      in.fail(cplib::format("Expected a strict float, got whitespace `{}`",
                            cplib::detail::hex_encode(in.inner().seek())));
    }
  }

  T result;
  const char* first = token.data();
  const char* last = token.data() + token.length();

  // Use std::chars_format::fixed for strict float parsing (no scientific notation)
  auto [ptr, ec] = std::from_chars(first, last, result, std::chars_format::fixed);

  if (ec == std::errc::invalid_argument || ptr != last) {
    in.fail(cplib::format("Expected a strict float, got `{}`", compress(token)));
  } else if (ec == std::errc::result_out_of_range) {
    in.fail(cplib::format("Strict float value `{}` is out of range for type `{}`", compress(token),
                          typeid(T).name()));
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
    in.attach_tag("#t", json::Value("sf"));
  }

  return result;
}

inline YesNo::YesNo() : YesNo(std::string(detail::VAR_DEFAULT_NAME)) {}

inline YesNo::YesNo(std::string name) : Var<bool, YesNo>(std::move(name)) {}

inline auto YesNo::read_from(Reader& in) const -> bool {
  auto token = in.inner().read_token();
  auto lower_token = in.inner().read_token();
  std::ranges::transform(lower_token, lower_token.begin(), ::tolower);

  bool result;
  if (lower_token == "yes") {
    result = true;
  } else if (lower_token == "no") {
    result = false;
  } else {
    panic("Expected `Yes` or `No`, got " + token);
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#v", json::Value(result));
    in.attach_tag("#t", json::Value("yn"));
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
      in.fail(cplib::format("Expected a string, got whitespace `{}`",
                            cplib::detail::hex_encode(in.inner().seek())));
    }
  }

  if (pat.has_value() && !pat->match(token)) {
    in.fail(cplib::format("Expected a string matching `{}`, got `{}`", compress(pat->src()),
                          compress(token)));
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#v", json::Value(token));
    in.attach_tag("#t", json::Value("s"));
  }

  return token;
}

// Impl Separator {{{
inline Separator::Separator(char sep) : Separator(std::string(detail::VAR_DEFAULT_NAME), sep) {}

inline Separator::Separator(std::string name, char sep)
    : Var<std::nullopt_t, Separator>(std::move(name)), sep(sep) {}

inline auto Separator::read_from(Reader& in) const -> std::nullopt_t {
  if (in.inner().eof()) {
    in.fail(cplib::format("Expected a separator `{}`, got EOF", cplib::detail::hex_encode(sep)));
  }

  if (in.inner().is_strict()) {
    auto got = in.inner().read();
    if (got != sep) {
      in.fail(cplib::format("Expected a separator `{}`, got `{}`", cplib::detail::hex_encode(sep),
                            cplib::detail::hex_encode(got)));
    }
  } else if (std::isspace(sep)) {
    auto got = in.inner().read();
    if (!std::isspace(got)) {
      in.fail(cplib::format("Expected a separator `{}`, got `{}`", cplib::detail::hex_encode(sep),
                            cplib::detail::hex_encode(got)));
    }
  } else {
    in.inner().skip_blanks();
    auto got = in.inner().read();
    if (got != sep) {
      in.fail(cplib::format("Expected a separator `{}`, got `{}`", cplib::detail::hex_encode(sep),
                            cplib::detail::hex_encode(got)));
    }
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#hidden", json::Value(true));
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
    in.fail(cplib::format("Expected a line matching `{}`, got `{}`", compress(pat->src()),
                          compress(*line)));
  }

  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#v", json::Value(*line));
    in.attach_tag("#t", json::Value("l"));
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
  for (std::size_t i = 0; i < len; ++i) {
    if (i > 0) in.read(sep);
    result[i] = in.read(element.renamed(std::to_string(i)));
  }
  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#t", json::Value("v"));
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
  for (std::size_t i = 0; i < len0; ++i) {
    if (i > 0) in.read(sep0);
    auto name_prefix = std::to_string(i) + "_";
    for (std::size_t j = 0; j < len1; ++j) {
      if (j > 0) in.read(sep1);
      result[i][j] = in.read(element.renamed(name_prefix + std::to_string(j)));
    }
  }
  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#t", json::Value("m"));
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
  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#t", json::Value("p"));
  }
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
  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#t", json::Value("t"));
  }
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
      inner_function_(
          [captured_args = std::make_tuple(std::forward<Args>(args)...), f](Reader& in) {
            return std::apply(
                [&in, f](auto&&... unpacked_args) {
                  return f(in, std::forward<decltype(unpacked_args)>(unpacked_args)...);
                },
                captured_args);
          }) {}

template <class F>
inline auto FnVar<F>::read_from(Reader& in) const -> typename std::function<F>::result_type {
  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#t", json::Value("F"));
  }
  return inner_function_(in);
}

template <class T>
template <class... Args>
inline ExtVar<T>::ExtVar(std::string name, Args... args)
  requires Readable<T, Args...>
    : Var<T, ExtVar<T>>(std::move(name)),
      // Capture arguments into a tuple, then use std::apply to call T::read.
      // This is a robust way to handle variadic arguments within std::function.
      inner_function_([captured_args = std::make_tuple(std::forward<Args>(args)...)](Reader& in) {
        return std::apply(
            [&in](auto&&... unpacked_args) {
              // Call T::read with the Reader and the unpacked arguments
              return T::read(in, std::forward<decltype(unpacked_args)>(unpacked_args)...);
            },
            captured_args);
      }) {}

template <class T>
auto ExtVar<T>::read_from(Reader& in) const -> T {
  if (in.get_trace_level() >= trace::Level::FULL) {
    in.attach_tag("#t", json::Value("E"));
  }
  return inner_function_(in);
}
}  // namespace cplib::var
  

#endif

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
  explicit ParsedArgs(const std::vector<std::string>& args);

  [[nodiscard]] auto has_flag(std::string_view name) const -> bool;
};
}  // namespace cplib::cmd_args

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_CMD_ARGS_HPP_
#error "Must be included from cmd_args.hpp"
#endif
#endif


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

inline ParsedArgs::ParsedArgs(const std::vector<std::string>& args) {
  std::optional<std::string> last_flag;

  for (const auto& arg : args) {
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
  

#endif

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

  auto set_state(State& state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string>& args) -> void = 0;

 protected:
  auto state() -> State&;
  auto set_inf_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_ouf_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_ans_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_inf_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_ouf_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_ans_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_evaluator(trace::Level trace_level) -> void;

 private:
  State* state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report& report) -> int = 0;

  auto attach_reader_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void;
  auto attach_evaluator_trace_stack(trace::TraceStack<evaluate::EvaluatorTrace> trace_stack)
      -> void;

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
  auto init(std::string_view arg0, const std::vector<std::string>& args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Macro to register checker with custom initializer.
 *
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_CHECKER_OPT(input_struct_, output_struct_, initializer_)                    \
  static_assert(::cplib::var::Readable<input_struct_>, "`" #input_struct_ "` should be Readable"); \
  static_assert(::cplib::var::Readable<output_struct_, const input_struct_&>,                      \
                "`" #output_struct_ "` should be Readable");                                       \
  static_assert(::cplib::evaluate::Evaluatable<output_struct_, const input_struct_&>,              \
                "`" #output_struct_ "` should be Evaluatable");                                    \
  auto main(int argc, char** argv) -> int {                                                        \
    ::std::vector<::std::string> args;                                                             \
    args.reserve(argc);                                                                            \
    for (int i = 1; i < argc; ++i) {                                                               \
      args.emplace_back(argv[i]);                                                                  \
    }                                                                                              \
    auto state =                                                                                   \
        ::cplib::checker::State(::std::unique_ptr<decltype(initializer_)>(new initializer_));      \
    state.initializer->init(argv[0], args);                                                        \
    input_struct_ input{state.inf.read(::cplib::var::ExtVar<input_struct_>("input"))};             \
    output_struct_ output{state.ouf.read(::cplib::var::ExtVar<output_struct_>("output", input))};  \
    output_struct_ answer{state.ans.read(::cplib::var::ExtVar<output_struct_>("answer", input))};  \
    ::cplib::evaluate::Result result = state.evaluator("output", output, answer, input);           \
    ::cplib::checker::Report report{::cplib::checker::Report::Status(result.status), result.score, \
                                    ""};                                                           \
    state.quit(report);                                                                            \
    return 0;                                                                                      \
  }

/**
 * Macro to register checker with default initializer.
 */
#define CPLIB_REGISTER_CHECKER(input_struct_, output_struct_) \
  CPLIB_REGISTER_CHECKER_OPT(input_struct_, output_struct_, ::cplib::checker::DefaultInitializer())
}  // namespace cplib::checker

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



#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_CHECKER_HPP_
#error "Must be included from checker.hpp"
#endif
#endif


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

inline auto Initializer::set_state(State& state) -> void { state_ = &state; };

inline auto Initializer::state() -> State& { return *state_; };

inline auto Initializer::set_inf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_ouf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->ouf = var::detail::make_reader_by_fileno(
      fileno, "ouf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_ans_fileno(int fileno, trace::Level trace_level) -> void {
  state_->ans = var::detail::make_reader_by_fileno(
      fileno, "ans", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_inf_path(std::string_view path, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_ouf_path(std::string_view path, trace::Level trace_level) -> void {
  state_->ouf = var::detail::make_reader_by_path(
      path, "ouf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_ans_path(std::string_view path, trace::Level trace_level) -> void {
  state_->ans = var::detail::make_reader_by_path(
      path, "ans", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_reader_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_evaluator(trace::Level trace_level) -> void {
  state_->evaluator = evaluate::Evaluator(
      trace_level,
      [this, trace_level](const evaluate::Evaluator& evaluator, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_evaluator_trace_stack(evaluator.make_trace_stack(true));
        }
        panic(msg);
      },
      [this, trace_level](const evaluate::Evaluator& evaluator, const evaluate::Result& result) {
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

inline auto detect_reporter(State& state) -> void {
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
inline auto set_report_format(State& state, std::string_view format) -> bool {
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

inline auto DefaultInitializer::init(std::string_view arg0, const std::vector<std::string>& args)
    -> void {
  auto& state = this->state();

  detail::detect_reporter(state);

  auto parsed_args = cmd_args::ParsedArgs(args);

  for (const auto& [key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(state, value)) {
        panic(cplib::format("Unknown {} option: {}", key, value));
      }
    } else {
      panic("Unknown command-line argument variable: " + key);
    }
  }

  for (const auto& flag : parsed_args.flags) {
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

inline auto JsonReporter::report(const Report& report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"score", json::Value(report.score)},
      {"message", json::Value(report.message)},
  };

  if (!reader_trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(reader_trace_stacks_.size());
    std::ranges::transform(reader_trace_stacks_, std::back_inserter(trace_stacks),
                           [](const auto& s) { return json::Value(s.to_json()); });
    map.emplace("reader_trace_stacks", trace_stacks);
  }

  if (!evaluator_trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(evaluator_trace_stacks_.size());
    std::ranges::transform(evaluator_trace_stacks_, std::back_inserter(trace_stacks),
                           [](const auto& s) { return json::Value(s.to_json()); });
    map.emplace("evaluator_trace_stacks", trace_stacks);
  }

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2) << detail::status_to_title_string(report.status)
         << ", scores " << report.score * 100.0 << " of 100.\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!reader_trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& stack : reader_trace_stacks_) {
      for (const auto& line : stack.to_plain_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (!evaluator_trace_stacks_.empty()) {
    stream << "\nEvaluator trace stacks:\n";
    for (const auto& stack : evaluator_trace_stacks_) {
      stream << "  " << stack.to_plain_text_compact() << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2)
         << detail::status_to_colored_title_string(report.status) << ", scores \x1b[0;33m"
         << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!reader_trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& stack : reader_trace_stacks_) {
      for (const auto& line : stack.to_colored_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (!evaluator_trace_stacks_.empty()) {
    stream << "\nEvaluator trace stacks:\n";
    for (const auto& stack : evaluator_trace_stacks_) {
      stream << "  " << stack.to_colored_text_compact() << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}
// /Impl reporters }}}
}  // namespace cplib::checker
  

#endif

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

  auto set_state(State& state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string>& args) -> void = 0;

 protected:
  auto state() -> State&;

  auto set_inf_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_from_user_fileno(int fileno, trace::Level trace_level) -> void;
  auto set_to_user_fileno(int fileno) -> void;
  auto set_inf_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_from_user_path(std::string_view path, trace::Level trace_level) -> void;
  auto set_to_user_path(std::string_view path) -> void;

 private:
  State* state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report& report) -> int = 0;

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
  [[noreturn]] auto quit(const Report& report) -> void;

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
  auto init(std::string_view arg0, const std::vector<std::string>& args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Macro to register interactor with custom initializer.
 *
 * @param var_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 */
#define CPLIB_REGISTER_INTERACTOR_OPT(var_, initializer_)                                    \
  auto var_ =                                                                                \
      ::cplib::interactor::State(std::unique_ptr<decltype(initializer_)>(new initializer_)); \
  auto main(int argc, char** argv) -> int {                                                  \
    ::std::vector<::std::string> args;                                                       \
    args.reserve(argc);                                                                      \
    for (int i = 1; i < argc; ++i) {                                                         \
      args.emplace_back(argv[i]);                                                            \
    }                                                                                        \
    var_.initializer->init(argv[0], args);                                                   \
    auto interactor_main(void) -> void;                                                      \
    interactor_main();                                                                       \
    return 0;                                                                                \
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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_INTERACTOR_HPP_
#error "Must be included from interactor.hpp"
#endif
#endif


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

inline auto Initializer::set_state(State& state) -> void { state_ = &state; };

inline auto Initializer::state() -> State& { return *state_; };

inline auto Initializer::set_inf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        panic(msg);
      });
}

inline auto Initializer::set_from_user_fileno(int fileno, trace::Level trace_level) -> void {
  state_->from_user = var::detail::make_reader_by_fileno(
      fileno, "from_user", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
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
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
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
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
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

inline auto State::quit(const Report& report) -> void {
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

inline auto detect_reporter(State& state) -> void {
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
inline auto set_report_format(State& state, std::string_view format) -> bool {
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

inline auto DefaultInitializer::init(std::string_view arg0, const std::vector<std::string>& args)
    -> void {
  auto& state = this->state();

  detail::detect_reporter(state);

  auto parsed_args = cmd_args::ParsedArgs(args);

  for (const auto& [key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(state, value)) {
        panic(cplib::format("Unknown {} option: {}", key, value));
      }
    } else {
      panic("Unknown command-line argument variable: " + key);
    }
  }

  for (const auto& flag : parsed_args.flags) {
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

inline auto JsonReporter::report(const Report& report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"score", json::Value(report.score)},
      {"message", json::Value(report.message)},
  };

  if (!trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(trace_stacks_.size());
    std::ranges::transform(trace_stacks_, std::back_inserter(trace_stacks),
                           [](auto& s) { return json::Value(s.to_json()); });
    map.emplace("reader_trace_stacks", trace_stacks);
  }

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2) << detail::status_to_title_string(report.status)
         << ", scores " << report.score * 100.0 << " of 100.\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& stack : trace_stacks_) {
      for (const auto& line : stack.to_plain_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2)
         << detail::status_to_colored_title_string(report.status) << ", scores \x1b[0;33m"
         << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& stack : trace_stacks_) {
      for (const auto& line : stack.to_colored_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}
// /Impl reporters }}}
}  // namespace cplib::interactor
  

#endif

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

  auto set_state(State& state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string>& args) -> void = 0;

 protected:
  auto state() -> State&;

  auto set_inf_fileno(int fileno, trace::Level level) -> void;
  auto set_inf_path(std::string_view path, trace::Level level) -> void;

 private:
  State* state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report& report) -> int = 0;

  auto attach_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void;

  auto attach_trace_tree(const trace::TraceTreeNode<var::ReaderTrace>* root) -> void;

  auto attach_trait_status(const std::map<std::string, bool>& trait_status) -> void;

 protected:
  std::vector<trace::TraceStack<var::ReaderTrace>> trace_stacks_{};
  const trace::TraceTreeNode<var::ReaderTrace>* trace_tree_{};
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
  auto init(std::string_view arg0, const std::vector<std::string>& args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Macro to register validator with custom initializer.
 *
 * @param initializer_ The initializer function.
 * @param traits_func_ The function returns a list of traits.
 */
#define CPLIB_REGISTER_VALIDATOR_OPT(input_struct_, traits_func_, initializer_)                    \
  static_assert(::cplib::var::Readable<input_struct_>, "`" #input_struct_ "` should be Readable"); \
  auto main(int argc, char** argv) -> int {                                                        \
    ::std::vector<::std::string> args;                                                             \
    args.reserve(argc);                                                                            \
    for (int i = 1; i < argc; ++i) {                                                               \
      args.emplace_back(argv[i]);                                                                  \
    }                                                                                              \
    auto state =                                                                                   \
        ::cplib::validator::State(std::unique_ptr<decltype(initializer_)>(new initializer_));      \
    state.initializer->init(argv[0], args);                                                        \
    input_struct_ input{state.inf.read(::cplib::var::ExtVar<input_struct_>("input"))};             \
    std::function<auto(const input_struct_&)->::std::vector<::cplib::validator::Trait>>            \
        traits_func = traits_func_;                                                                \
    state.traits(traits_func(input));                                                              \
    state.quit_valid();                                                                            \
    return 0;                                                                                      \
  }

/**
 * Macro to register validator with default initializer.
 */
#define CPLIB_REGISTER_VALIDATOR(input_struct_, traits_func_) \
  CPLIB_REGISTER_VALIDATOR_OPT(input_struct_, traits_func_,   \
                               ::cplib::validator::DefaultInitializer())
}  // namespace cplib::validator

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_VALIDATOR_HPP_
#error "Must be included from validator.hpp"
#endif
#endif


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

inline auto Initializer::set_state(State& state) -> void { state_ = &state; };

inline auto Initializer::state() -> State& { return *state_; };

inline auto Initializer::set_inf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", true, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_invalid(msg);
      });
}

inline auto Initializer::set_inf_path(std::string_view path, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", true, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
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

inline auto Reporter::attach_trace_tree(const trace::TraceTreeNode<var::ReaderTrace>* root)
    -> void {
  if (!root) {
    panic("Reporter::attach_trace_tree failed: Trace tree root pointer is nullptr");
  }

  trace_tree_ = root;
}

inline auto Reporter::attach_trait_status(const std::map<std::string, bool>& trait_status) -> void {
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
inline auto topo_sort(const std::vector<std::vector<std::pair<std::size_t, bool>>>& edges,
                      const bool follow_unmatched_edge,
                      const std::function<auto(std::size_t)->bool>& callback) -> void {
  std::vector<std::size_t> degree(edges.size(), 0);

  for (const auto& edge : edges) {
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
inline auto build_edges(std::vector<Trait>& traits)
    -> std::optional<std::vector<std::vector<std::pair<std::size_t, bool>>>> {
  // Check duplicate name
  std::ranges::sort(traits, [](const Trait& x, const Trait& y) { return x.name < y.name; });
  if (std::ranges::unique(traits, [](const Trait& x, const Trait& y) {
        return x.name == y.name;
      }).end() != traits.end()) {
    // Found duplicate name
    return std::nullopt;
  }

  std::vector<std::vector<std::pair<std::size_t, bool>>> edges(traits.size());

  for (std::size_t i = 0; i < traits.size(); ++i) {
    auto& trait = traits[i];
    for (const auto& [name, value] : trait.dependencies) {
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

inline auto have_loop(const std::vector<std::vector<std::pair<std::size_t, bool>>>& edges) -> bool {
  std::vector<std::uint8_t> visited(edges.size(), 0);  // Never use std::vector<bool>

  topo_sort(edges, true, [&](std::size_t node) {
    visited[node] = 1;
    return true;
  });

  for (auto v : visited) {
    if (!v) return true;
  }
  return false;
}

inline auto validate_traits(const std::vector<Trait>& traits,
                            const std::vector<std::vector<std::pair<std::size_t, bool>>>& edges)
    -> std::map<std::string, bool> {
  std::map<std::string, bool> results;

  topo_sort(edges, true, [&](std::size_t id) {
    auto& node = traits[id];
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

// Impl DefaultInitializer {{{
namespace detail {
constexpr std::string_view ARGS_USAGE = "[<input_file>] [--report-format={auto|json|text}]";

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

inline auto detect_reporter(State& state) -> void {
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
inline auto set_report_format(State& state, std::string_view format) -> bool {
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

inline auto DefaultInitializer::init(std::string_view arg0, const std::vector<std::string>& args)
    -> void {
  auto& state = this->state();

  detail::detect_reporter(state);

  auto parsed_args = cmd_args::ParsedArgs(args);

  for (const auto& [key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(state, value)) {
        panic(cplib::format("Unknown {} option: {}", key, value));
      }
    } else {
      panic("Unknown command-line argument variable: " + key);
    }
  }

  for (const auto& flag : parsed_args.flags) {
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
    set_inf_fileno(fileno(stdin), trace::Level::FULL);
  } else {
    set_inf_path(parsed_args.ordered[0], trace::Level::FULL);
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

inline auto trait_status_to_json(const std::map<std::string, bool>& traits) -> json::Value {
  json::Map map;
  for (const auto& [k, v] : traits) {
    map.emplace(k, json::Value(v));
  }
  return json::Value(map);
}

inline auto print_trace_tree(const trace::TraceTreeNode<var::ReaderTrace>* node, std::size_t depth,
                             std::size_t& n_remaining_node, bool colored_output, std::ostream& os)
    -> void {
  if (!node || depth >= 8 || (node->tags.count("#hidden"))) {
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

    // type
    if (node->tags.count("#t")) {
      if (colored_output) {
        os << "\x1b[0;90m";
      }
      os << ": " << node->tags.at("#t").to_string();
      if (colored_output) {
        os << "\x1b[0m";
      }
    }

    // value
    if (node->tags.count("#v")) {
      os << " = " << node->tags.at("#v").to_string();
    }
    os << '\n';
  }

  std::size_t n_visible_children = 0;
  for (const auto& child : node->get_children()) {
    if (!!child->tags.count("#hidden")) {
      ++n_visible_children;
    }
  }

  for (const auto& child : node->get_children()) {
    if (child->tags.count("#hidden")) {
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

inline auto JsonReporter::report(const Report& report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"message", json::Value(report.message)},
  };

  if (!trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(trace_stacks_.size());
    std::ranges::transform(trace_stacks_, std::back_inserter(trace_stacks),
                           [](auto& s) { return json::Value(s.to_json()); });
    map.emplace("reader_trace_stacks", trace_stacks);
  }

  if (!trait_status_.empty()) {
    map.emplace("traits", detail::trait_status_to_json(trait_status_));
  }

  if (trace_tree_) {
    auto json = trace_tree_->to_json();
    if (json.count("children")) {
      map.emplace("reader_trace_tree", std::move(json.at("children")));
    }
  }

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << detail::status_to_title_string(report.status) << ".\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& stack : trace_stacks_) {
      for (const auto& line : stack.to_plain_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (report.status == Report::Status::VALID && !trait_status_.empty()) {
    stream << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (const auto& [name, satisfaction] : trait_status_) {
      if (satisfaction) {
        satisfied.emplace_back(name);
      } else {
        dissatisfied.emplace_back(name);
      }
    }

    for (const auto& name : satisfied) {
      stream << "+ " << cplib::detail::hex_encode(name) << '\n';
    }
    for (const auto& name : dissatisfied) {
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

inline auto ColoredTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << detail::status_to_colored_title_string(report.status) << ".\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& stack : trace_stacks_) {
      for (const auto& line : stack.to_colored_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (report.status == Report::Status::VALID && !trait_status_.empty()) {
    stream << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (const auto& [name, satisfaction] : trait_status_) {
      if (satisfaction) {
        satisfied.emplace_back(name);
      } else {
        dissatisfied.emplace_back(name);
      }
    }

    for (const auto& name : satisfied) {
      stream << "\x1b[0;32m+\x1b[0m " << name << '\n';
    }
    for (const auto& name : dissatisfied) {
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
  

#endif

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

  auto set_state(State& state) -> void;

  virtual auto init(std::string_view arg0, const std::vector<std::string>& args) -> void = 0;

 protected:
  auto state() -> State&;

 private:
  State* state_{};
};

/**
 * `Reporter` used to report and then exit the program.
 */
struct Reporter {
 public:
  virtual ~Reporter() = 0;

  [[nodiscard]] virtual auto report(const Report& report) -> int = 0;
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
struct DefaultInitializer : Initializer {
  /**
   * Initialize state according to default behavior.
   *
   * @param arg0 The name of the program.
   * @param args The command-line arguments.
   */
  auto init(std::string_view arg0, const std::vector<std::string>& args) -> void override;
};

/**
 * `JsonReporter` reports the given report in JSON format.
 */
struct JsonReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in plain text format for human reading.
 */
struct PlainTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

/**
 * Report the given report in colored text format for human reading.
 */
struct ColoredTextReporter : Reporter {
  [[nodiscard]] auto report(const Report& report) -> int override;
};

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
 * @param state_var_name_ The variable name of state object to be initialized.
 * @param initializer_ The initializer function.
 * @param args_namespace_name_ The name of the command-line arguments namespace.
 * @param ... The parsers of the command-line arguments.
 */
#define CPLIB_REGISTER_GENERATOR_OPT(state_var_name_, initializer_, args_namespace_name_, ...) \
  auto state_var_name_ =                                                                       \
      ::cplib::generator::State(std::unique_ptr<decltype(initializer_)>(new initializer_));    \
  CPLIB_PREPARE_GENERATOR_ARGS_NAMESPACE_(state_var_name_);                                    \
  CPLIB_REGISTER_GENERATOR_ARGS_(__VA_ARGS__);                                                 \
  namespace args_namespace_name_ = ::cplib_generator_args_;                                    \
  auto main(int argc, char** argv) -> int {                                                    \
    ::std::vector<::std::string> args;                                                         \
    args.reserve(argc);                                                                        \
    for (int i = 1; i < argc; ++i) {                                                           \
      args.emplace_back(argv[i]);                                                              \
    }                                                                                          \
    state_var_name_.initializer->init(argv[0], args);                                          \
    auto generator_main(void) -> void;                                                         \
    generator_main();                                                                          \
    return 0;                                                                                  \
  }

/**
 * Macro to register generator with default initializer.
 *
 * @param var_name_ The variable name of state object to be initialized.
 * @param args_namespace_name_ The name of the command-line arguments namespace.
 * @param ... The parsers of the command-line arguments.
 */
#define CPLIB_REGISTER_GENERATOR(var_name_, args_namespace_name_, ...)              \
  CPLIB_REGISTER_GENERATOR_OPT(var_name_, ::cplib::generator::DefaultInitializer(), \
                               args_namespace_name_, __VA_ARGS__)
}  // namespace cplib::generator

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


#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
  
#else
#ifndef CPLIB_GENERATOR_HPP_
#error "Must be included from generator.hpp"
#endif
#endif


#include <algorithm>
#include <cstdio>
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

inline auto Initializer::set_state(State& state) -> void { state_ = &state; };

inline auto Initializer::state() -> State& { return *state_; };

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

  auto exit_code = reporter->report(report);
  std::exit(exit_code);
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

inline auto detect_reporter(State& state) -> void {
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
inline auto set_report_format(State& state, std::string_view format) -> bool {
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

inline auto validate_required_arguments(const State& state,
                                        const std::map<std::string, std::string>& var_args)
    -> void {
  for (const auto& var : state.required_var_args) {
    if (!var_args.count(var)) panic("Missing variable: " + var);
  }
}

inline auto get_args_usage(const State& state) {
  std::vector<std::string> builder;
  builder.reserve(state.required_flag_args.size() + state.required_var_args.size());
  for (const auto& arg : state.required_flag_args) builder.emplace_back("[--" + arg + "]");
  for (const auto& arg : state.required_var_args) builder.emplace_back("--" + arg + "=<value>");
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

inline auto DefaultInitializer::init(std::string_view arg0, const std::vector<std::string>& args)
    -> void {
  auto& state = this->state();

  detail::detect_reporter(state);

  // required args are initially unordered, sort them to ensure subsequent binary_search is correct
  std::ranges::sort(state.required_flag_args);
  std::ranges::sort(state.required_var_args);

  auto parsed_args = cmd_args::ParsedArgs(args);
  auto args_usage = detail::get_args_usage(state);
  std::set<std::string> flag_args;
  std::map<std::string, std::string> var_args;

  for (const auto& [key, value] : parsed_args.vars) {
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

  for (const auto& flag : parsed_args.flags) {
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

  for (const auto& parser : state.flag_parsers) parser(flag_args);
  for (const auto& parser : state.var_parsers) parser(var_args);

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

inline auto JsonReporter::report(const Report& report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"message", json::Value(report.message)},
  };

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  if (report.status == Report::Status::OK && report.message.empty()) {
    // Do nothing when the report is OK and message is empty.
    std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  stream << detail::status_to_title_string(report.status) << ".\n" << report.message << '\n';

  return report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report& report) -> int {
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
  

#endif


// clang-format on

#endif  // CPLIB_HPP_
