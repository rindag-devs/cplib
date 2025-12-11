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
#include <ranges>
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
                       trace::Level::NONE, [](const Reader&, std::string_view msg) -> void {
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
  const char* first = token.data();
  const char* last = token.data() + token.length();
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
  }

  return result;
}

inline YesNo::YesNo() : YesNo(std::string(detail::VAR_DEFAULT_NAME)) {}

inline YesNo::YesNo(std::string name) : Var<bool, YesNo>(std::move(name)) {}

inline auto YesNo::read_from(Reader& in) const -> bool {
  auto word = in.inner().read_word();
  auto lower_token = word;
  std::ranges::transform(lower_token, lower_token.begin(), ::tolower);

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

inline auto String::read_from(Reader& in) const -> std::string {
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

inline auto Separator::read_from(Reader& in) const -> std::nullopt_t {
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
  } else if (std::isspace(s)) {
    auto got = in.inner().read();
    if (!std::isspace(got)) {
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
inline auto Vec<T>::read_from(Reader& in) const -> std::vector<typename T::Var::Target> {
  std::vector<typename T::Var::Target> result(len);
  for (std::size_t i = 0; i < len; ++i) {
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
  for (std::size_t i = 0; i < len0; ++i) {
    if (i > 0) in.read(sep0);
    auto name_prefix = std::to_string(i) + "_";
    for (std::size_t j = 0; j < len1; ++j) {
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
  // Delegate to the implementation helper with a generated index sequence.
  return read_from_impl(in, std::index_sequence_for<T...>{});
}

template <class... T>
template <std::size_t... Is>
inline auto Tuple<T...>::read_from_impl(Reader& in, std::index_sequence<Is...>) const
    -> std::tuple<typename T::Var::Target...> {
  // Create the result tuple that will be populated.
  std::tuple<typename T::Var::Target...> result;
  std::size_t cnt = 0;
  auto renamed_inc = [&cnt](auto var) { return var.renamed(std::to_string(cnt++)); };

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
inline auto ExtVar<T>::read_from(Reader& in) const -> T {
  return inner_function_(in);
}

template <class T>
template <std::ranges::range Range, class... Args>
inline ExtVec<T>::ExtVec(std::string name, Range&& range, Separator sep, Args... args)
  requires Readable<T, Args..., std::ranges::range_value_t<Range>>
    : Var<std::vector<T>, ExtVec<T>>(std::move(name)),
      inner_function_([range = std::forward<Range>(range), sep,
                       captured_args = std::make_tuple(std::forward<Args>(args)...)](
                          Reader& in) -> std::vector<T> {
        std::vector<T> result;
        if constexpr (std::ranges::sized_range<Range>) {
          result.reserve(std::ranges::size(range));
        }

        std::size_t i = 0;
        for (const auto& range_element : range) {
          if (i > 0) {
            in.read(sep);
          }

          // Use std::apply to construct an ExtVar for each element.
          // This unpacks the tuple of fixed arguments and passes them, along with the
          // current range_element, to the ExtVar constructor. This correctly sets up
          // the call to T::read with all necessary arguments and handles tracing.
          auto element = std::apply(
              [&](auto&&... fixed_args) {
                return in.read(ExtVar<T>(std::to_string(i),
                                         std::forward<decltype(fixed_args)>(fixed_args)...,
                                         range_element));
              },
              captured_args);

          result.push_back(std::move(element));
          ++i;
        }
        return result;
      }) {}

template <class T>
inline auto ExtVec<T>::read_from(Reader& in) const -> std::vector<T> {
  return inner_function_(in);
}
}  // namespace cplib::var
