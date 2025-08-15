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

/* cplib_embed_ignore start */
#include "io.hpp"
#include "json.hpp"
#include "pattern.hpp"
#include "trace.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

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
