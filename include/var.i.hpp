/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

/* cplib_embed_ignore start */
#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
#include "var.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_VAR_HPP_
#error "Must be included from var.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

/* cplib_embed_ignore start */
#include "io.hpp"
#include "json.hpp"
#include "pattern.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::var {
[[nodiscard]] inline auto Reader::TraceStack::to_json() const -> std::unique_ptr<cplib::json::Map> {
  std::map<std::string, std::unique_ptr<cplib::json::Value>> map;
  std::vector<std::unique_ptr<cplib::json::Value>> stack_list;

  for (const auto& trace : stack) {
    std::map<std::string, std::unique_ptr<cplib::json::Value>> trace_map;
    trace_map.insert({"var_name", std::make_unique<cplib::json::String>(trace.var_name)});
    trace_map.insert({"line_num", std::make_unique<cplib::json::Int>(trace.line_num)});
    trace_map.insert({"col_num", std::make_unique<cplib::json::Int>(trace.col_num)});
    trace_map.insert({"byte_num", std::make_unique<cplib::json::Int>(trace.byte_num)});
    stack_list.push_back(std::make_unique<cplib::json::Map>(std::move(trace_map)));
  }

  map.insert({"stack", std::make_unique<cplib::json::List>(std::move(stack_list))});
  map.insert({"stream_name", std::make_unique<cplib::json::String>(stream_name)});
  return std::make_unique<cplib::json::Map>(std::move(map));
}

[[nodiscard]] inline auto Reader::TraceStack::to_plain_text_lines() const
    -> std::vector<std::string> {
  std::vector<std::string> lines;

  lines.push_back(std::string("Stream: ") + stream_name);

  std::size_t id = 0;
  for (const auto& trace : stack) {
    auto line = cplib::format("#%zu: %s @ line %zu, col %zu, byte %zu", id, trace.var_name.c_str(),
                              trace.line_num + 1, trace.col_num + 1, trace.byte_num + 1);
    ++id;
    lines.push_back(std::move(line));
  }

  return lines;
}

[[nodiscard]] inline auto Reader::TraceStack::to_colored_text_lines() const
    -> std::vector<std::string> {
  std::vector<std::string> lines;

  lines.push_back(std::string("Stream: \x1b[0;33m") + stream_name + "\x1b[0m");

  std::size_t id = 0;
  for (const auto& trace : stack) {
    auto line = cplib::format(
        "#%zu: \x1b[0;33m%s\x1b[0m @ line \x1b[0;33m%zu\x1b[0m, col \x1b[0;33m%zu\x1b[0m, byte "
        "\x1b[0;33m%zu\x1b[0m",
        id, trace.var_name.c_str(), trace.line_num + 1, trace.col_num + 1, trace.byte_num + 1);
    ++id;
    lines.push_back(std::move(line));
  }

  return lines;
}

inline Reader::Reader(std::unique_ptr<io::InStream> inner, FailFunc fail_func)
    : inner_(std::move(inner)), fail_func_(std::move(fail_func)) {}

inline auto Reader::inner() -> io::InStream& { return *inner_; }

inline auto Reader::fail(std::string_view message) -> void {
  fail_func_(message, {traces_, std::string(inner().name())});
  std::exit(EXIT_FAILURE);
}

template <class T, class D>
inline auto Reader::read(const Var<T, D>& v) -> T {
  traces_.emplace_back(
      Trace{std::string(v.name()), inner().line_num(), inner().col_num(), inner().byte_num()});
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
                             Reader::FailFunc fail_func) -> var::Reader {
  auto buf = std::make_unique<std::filebuf>();
  if (!buf->open(path.data(), std::ios::binary | std::ios::in)) {
    panic(format("Can not open file `%s` as input stream", path.data()));
  }
  return var::Reader(std::make_unique<io::InStream>(std::move(buf), std::move(name), strict),
                     std::move(fail_func));
}

// Open `stdin` as input stream and create a `var::Reader`
inline auto make_stdin_reader(std::string name, bool strict, Reader::FailFunc fail_func)
    -> var::Reader {
  auto buf = std::make_unique<io::detail::FdInBuf>(fileno(stdin));
  var::Reader reader(std::make_unique<io::InStream>(std::move(buf), std::move(name), strict),
                     std::move(fail_func));
  /* FIXME: Under msvc stdin/stdout is an lvalue, cannot prevent users from using stdio. */
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
  auto reader = Reader(std::make_unique<io::InStream>(std::move(buf), "str", true),
                       [&](std::string_view msg, const var::Reader::TraceStack&) {
                         panic(std::string("Var::parse failed") + msg.data());
                       });
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

inline YesNo::YesNo() : YesNo(std::string(detail::VAR_DEFAULT_NAME)) {}

inline YesNo::YesNo(std::string name) : Var<bool, YesNo>(std::move(name)) {}

inline auto YesNo::read_from(Reader& in) const -> bool {
  auto token = in.inner().read_token();
  auto lower_token = in.inner().read_token();
  std::transform(lower_token.begin(), lower_token.end(), lower_token.begin(), ::tolower);

  if (lower_token == "yes") {
    return true;
  } else if (lower_token == "no") {
    return false;
  } else {
    panic("Expected `Yes` or `No`, got " + token);
  }
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
