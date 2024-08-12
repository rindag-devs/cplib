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
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <ios>
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

inline Reader::Trace::Trace(std::string var_name, io::Position pos)
    : var_name(std::move(var_name)), pos(pos) {}

[[nodiscard]] inline auto Reader::Trace::to_json_incomplete() const -> std::unique_ptr<json::Map> {
  std::map<std::string, std::unique_ptr<json::Value>> map;
  map.emplace("var_name", std::make_unique<json::String>(var_name));
  map.emplace("pos", pos.to_json());

  return std::make_unique<json::Map>(std::move(map));
}

[[nodiscard]] inline auto Reader::Trace::to_json_complete() const -> std::unique_ptr<json::Map> {
  std::map<std::string, std::unique_ptr<json::Value>> map;
  map.emplace("n", std::make_unique<json::String>(var_name));
  map.emplace("b", std::move(pos.to_json()->inner.at("byte")));
  map.emplace("l", std::make_unique<json::Int>(byte_length));

  return std::make_unique<json::Map>(std::move(map));
}

[[nodiscard]] inline auto Reader::TraceStack::to_json() const -> std::unique_ptr<json::Map> {
  std::map<std::string, std::unique_ptr<json::Value>> map;
  std::vector<std::unique_ptr<json::Value>> stack_list;

  stack_list.reserve(stack.size());
  for (const auto& trace : stack) {
    stack_list.emplace_back(trace.to_json_incomplete());
  }

  map.emplace("stack", std::make_unique<json::List>(std::move(stack_list)));
  map.emplace("fatal", std::make_unique<json::Bool>(fatal));
  return std::make_unique<json::Map>(std::move(map));
}

[[nodiscard]] inline auto Reader::TraceStack::to_plain_text_lines() const
    -> std::vector<std::string> {
  std::vector<std::string> lines;
  lines.reserve(stack.size() + 1);

  auto stream_line = std::string("Stream: ") + stream;
  if (fatal) {
    stream_line += " [fatal]";
  }
  lines.emplace_back(stream_line);

  std::size_t id = 0;
  for (const auto& trace : stack) {
    auto line = cplib::format("#%zu: %s @ line %zu, col %zu, byte %zu", id, trace.var_name.c_str(),
                              trace.pos.line + 1, trace.pos.col + 1, trace.pos.byte + 1);
    ++id;
    lines.emplace_back(std::move(line));
  }

  return lines;
}

[[nodiscard]] inline auto Reader::TraceStack::to_colored_text_lines() const
    -> std::vector<std::string> {
  std::vector<std::string> lines;
  lines.reserve(stack.size() + 1);

  auto stream_line = std::string("Stream: \x1b[0;33m") + stream + "\x1b[0m";
  if (fatal) {
    stream_line += " \x1b[0;31m[fatal]\x1b[0m";
  }
  lines.emplace_back(stream_line);

  std::size_t id = 0;
  for (const auto& trace : stack) {
    auto line = cplib::format(
        "#%zu: \x1b[0;33m%s\x1b[0m @ line \x1b[0;33m%zu\x1b[0m, col \x1b[0;33m%zu\x1b[0m, byte "
        "\x1b[0;33m%zu\x1b[0m",
        id, trace.var_name.c_str(), trace.pos.line + 1, trace.pos.col + 1, trace.pos.byte + 1);
    ++id;
    lines.emplace_back(std::move(line));
  }

  return lines;
}

inline Reader::TraceTreeNode::TraceTreeNode(Trace trace) : trace(std::move(trace)) {}

[[nodiscard]] inline auto Reader::TraceTreeNode::get_children() const
    -> const std::vector<std::unique_ptr<TraceTreeNode>>& {
  return children_;
}

[[nodiscard]] inline auto Reader::TraceTreeNode::get_parent() -> TraceTreeNode* { return parent_; }

[[nodiscard]] inline auto Reader::TraceTreeNode::to_json() const -> std::unique_ptr<json::Map> {
  std::map<std::string, std::unique_ptr<json::Value>> map;

  if (json_tag && json_tag->inner.count("#hidden")) {
    return nullptr;
  }

  map.emplace("trace", trace.to_json_complete());

  if (json_tag) {
    map.emplace("tag", json_tag->clone());
  }

  const auto& children = get_children();
  std::vector<std::unique_ptr<json::Value>> children_list;
  children_list.reserve(children.size());
  for (const auto& child : children) {
    auto child_value = child->to_json();
    if (child_value) {
      children_list.emplace_back(std::move(child_value));
    }
  }
  if (!children_list.empty()) {
    map.emplace("children", std::make_unique<json::List>(std::move(children_list)));
  }

  return std::make_unique<json::Map>(std::move(map));
}

inline auto Reader::TraceTreeNode::add_child(std::unique_ptr<TraceTreeNode> child)
    -> std::unique_ptr<TraceTreeNode>& {
  child->parent_ = this;
  return children_.emplace_back(std::move(child));
}

inline constexpr Reader::Fragment::Direction::Direction(Value value) : value_(value) {}

inline constexpr Reader::Fragment::Direction::operator Value() const { return value_; }

inline constexpr auto Reader::Fragment::Direction::to_string() const -> std::string_view {
  switch (value_) {
    case Value::AFTER:
      return "after";
    case Value::AROUND:
      return "around";
    case Value::BEFORE:
      return "before";
    default:
      panic(format("Unknown file fragment direction: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Reader::Fragment::Fragment(std::string stream, io::Position pos, Direction dir)
    : stream(std::move(stream)), pos(pos), dir(dir) {}

inline auto Reader::Fragment::to_json() const -> std::unique_ptr<json::Map> {
  std::map<std::string, std::unique_ptr<json::Value>> map;

  map.emplace("pos", pos.to_json());
  map.emplace("dir", std::make_unique<json::String>(std::string(dir.to_string())));

  std::vector<std::unique_ptr<json::Value>> highlight_lines_json;
  highlight_lines_json.reserve(highlight_lines.size());
  for (auto line : highlight_lines) {
    highlight_lines_json.push_back(std::make_unique<json::Int>(line));
  }
  map.emplace("highlight_lines", std::make_unique<json::List>(std::move(highlight_lines_json)));

  return std::make_unique<json::Map>(std::move(map));
}

inline auto Reader::Fragment::to_plain_text() const -> std::string {
  auto dir_str = std::string(dir.to_string());
  dir_str[0] = static_cast<char>(std::toupper(dir_str[0]));
  return format("%s %s:%zu:%zu, byte %zu", dir_str.c_str(), stream.c_str(), pos.line + 1,
                pos.col + 1, pos.byte + 1);
}

inline auto Reader::Fragment::to_colored_text() const -> std::string {
  auto dir_str = std::string(dir.to_string());
  dir_str[0] = static_cast<char>(std::toupper(dir_str[0]));
  return format(
      "%s \x1b[0;33m%s\x1b[0m:\x1b[0;33m%zu\x1b[0m:\x1b[0;33m%zu\x1b[0m, byte \x1b[0;33m%zu\x1b[0m",
      dir_str.c_str(), stream.c_str(), pos.line + 1, pos.col + 1, pos.byte + 1);
}

inline Reader::Reader(std::unique_ptr<io::InStream> inner, Reader::TraceLevel trace_level,
                      FailFunc fail_func)
    : inner_(std::move(inner)),
      trace_level_(static_cast<Reader::TraceLevel>(
          std::min(static_cast<int>(trace_level), CPLIB_VAR_READER_TRACE_LEVEL_MAX))),
      fail_func_(std::move(fail_func)),
      trace_stack_(),
      trace_tree_root_(std::make_unique<TraceTreeNode>(Trace("<root>", io::Position()))),
      trace_tree_current_(trace_tree_root_.get()) {}

inline auto Reader::inner() -> io::InStream& { return *inner_; }

inline auto Reader::inner() const -> const io::InStream& { return *inner_; }

[[noreturn]] inline auto Reader::fail(std::string_view message) -> void {
  fail_func_(*this, message);
  std::exit(EXIT_FAILURE);
}

template <class T, class D>
inline auto Reader::read(const Var<T, D>& v) -> T {
  auto trace_level = get_trace_level();

  if (trace_level >= TraceLevel::STACK_ONLY) {
    Trace trace{std::string(v.name()), inner().pos()};
    trace_stack_.emplace_back(trace);

    if (trace_level >= TraceLevel::FULL) {
      auto& child = trace_tree_current_->add_child(std::make_unique<TraceTreeNode>(trace));
      trace_tree_current_ = child.get();
    }
  }

  auto result = v.read_from(*this);

  if (trace_level >= TraceLevel::STACK_ONLY) {
    trace_stack_.back().byte_length = inner().pos().byte - trace_stack_.back().pos.byte;
    trace_stack_.pop_back();

    if (trace_level >= TraceLevel::FULL) {
      trace_tree_current_->trace.byte_length =
          inner().pos().byte - trace_tree_current_->trace.pos.byte;
      trace_tree_current_ = trace_tree_current_->get_parent();
    }
  }
  return result;
}

template <class... T>
inline auto Reader::operator()(T... vars) -> std::tuple<typename T::Var::Target...> {
  return {read(vars)...};
}

[[nodiscard]] inline auto Reader::get_trace_level() const -> TraceLevel { return trace_level_; }

[[nodiscard]] inline auto Reader::make_trace_stack(bool fatal) const -> TraceStack {
  return {trace_stack_, std::string(inner_->name()), fatal};
}

[[nodiscard]] inline auto Reader::get_trace_tree() const -> const TraceTreeNode* {
  if (get_trace_level() < TraceLevel::FULL) {
    panic("Reader::get_trace_tree requires `TraceLevel::FULL`");
  }

  return trace_tree_root_.get();
}

inline auto Reader::attach_json_tag(std::string_view key, std::unique_ptr<json::Value> value) {
  if (get_trace_level() < TraceLevel::FULL) {
    panic("Reader::get_trace_tree requires `TraceLevel::FULL`");
  }

  if (!trace_tree_current_->json_tag) {
    trace_tree_current_->json_tag =
        std::make_unique<json::Map>(std::map<std::string, std::unique_ptr<json::Value>>{});
  }

  trace_tree_current_->json_tag->inner.emplace(key, std::move(value));
}

[[nodiscard]] auto Reader::make_fragment(Fragment::Direction dir) const -> Fragment {
  if (get_trace_level() >= TraceLevel::STACK_ONLY) {
    auto trace_stack = make_trace_stack(false);
    if (!trace_stack.stack.empty()) {
      auto pos = trace_stack.stack.back().pos;
      auto fragment = Fragment(std::string(inner().name()), pos, dir);
      fragment.highlight_lines = {pos.line};
      return fragment;
    }
  }

  auto pos = inner().pos();
  return Fragment(std::string(inner().name()), pos, dir);
}

namespace detail {
// Open the given file and create a `var::Reader`
inline auto make_reader_by_path(std::string_view path, std::string name, bool strict,
                                Reader::TraceLevel trace_level,
                                Reader::FailFunc fail_func) -> var::Reader {
  auto buf = std::make_unique<io::InBuf>(path);
  return var::Reader(std::make_unique<io::InStream>(std::move(buf), std::move(name), strict),
                     trace_level, std::move(fail_func));
}

// Use file with givin fileno as input stream and create a `var::Reader`
inline auto make_reader_by_fileno(int fileno, std::string name, bool strict,
                                  Reader::TraceLevel trace_level,
                                  Reader::FailFunc fail_func) -> var::Reader {
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
                       Reader::TraceLevel::NONE, [](const Reader&, std::string_view msg) {
                         panic(std::string("Var::parse failed: ") + msg.data());
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

  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#v", std::make_unique<json::Int>(static_cast<std::uint64_t>(result)));
    in.attach_json_tag("#t", std::make_unique<json::String>("i"));
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
  enum struct State { SIGN, BEFORE_POINT, AFTER_POINT } state = State::SIGN;
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

  enum struct State {
    SIGN,
    BEFORE_POINT,
    AFTER_POINT,
    EXPONENT_SIGN,
    EXPONENT
  } state = State::SIGN;

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
  auto result = detail::parse_float<T>(token);

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

  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#v", std::make_unique<json::Real>(static_cast<double>(result)));
    in.attach_json_tag("#t", std::make_unique<json::String>("f"));
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
  auto result = detail::parse_strict_float<T>(token, &n_after_point);

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

  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#v", std::make_unique<json::Real>(static_cast<double>(result)));
    in.attach_json_tag("#t", std::make_unique<json::String>("sf"));
  }

  return result;
}

inline YesNo::YesNo() : YesNo(std::string(detail::VAR_DEFAULT_NAME)) {}

inline YesNo::YesNo(std::string name) : Var<bool, YesNo>(std::move(name)) {}

inline auto YesNo::read_from(Reader& in) const -> bool {
  auto token = in.inner().read_token();
  auto lower_token = in.inner().read_token();
  std::transform(lower_token.begin(), lower_token.end(), lower_token.begin(), ::tolower);

  bool result;
  if (lower_token == "yes") {
    result = true;
  } else if (lower_token == "no") {
    result = false;
  } else {
    panic("Expected `Yes` or `No`, got " + token);
  }

  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#v", std::make_unique<json::Bool>(result));
    in.attach_json_tag("#t", std::make_unique<json::String>("yn"));
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

  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#v", std::make_unique<json::String>(token));
    in.attach_json_tag("#t", std::make_unique<json::String>("s"));
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

  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#hidden", std::make_unique<json::Bool>(true));
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

  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#v", std::make_unique<json::String>(*line));
    in.attach_json_tag("#t", std::make_unique<json::String>("l"));
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
  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#t", std::make_unique<json::String>("v"));
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
  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#t", std::make_unique<json::String>("m"));
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
  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#t", std::make_unique<json::String>("p"));
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
  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#t", std::make_unique<json::String>("t"));
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
      inner([f, args...](Reader& in) { return f(in, args...); }) {}

template <class F>
inline auto FnVar<F>::read_from(Reader& in) const -> typename std::function<F>::result_type {
  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#t", std::make_unique<json::String>("F"));
  }
  return inner(in);
}

template <class T>
template <class... Args>
inline ExtVar<T>::ExtVar(std::string name, Args... args)
    : Var<T, ExtVar<T>>(std::move(name)),
      inner([args...](Reader& in) { return T::read(in, args...); }) {}

template <class T>
inline auto ExtVar<T>::read_from(Reader& in) const -> T {
  if (in.get_trace_level() >= Reader::TraceLevel::FULL) {
    in.attach_json_tag("#t", std::make_unique<json::String>("E"));
  }
  return inner(in);
}
}  // namespace cplib::var
