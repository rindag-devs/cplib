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
#include "evaluate.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_EVALUATE_HPP_
#error "Must be included from evaluate.hpp"
#endif
#endif
/* cplib_embed_ignore end */

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

/* cplib_embed_ignore start */
#include "json.hpp"
#include "trace.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

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
inline auto merge_message(const std::string& a, const std::string& b) -> std::string {
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
  if (other.named) {
    res.message = message;
  } else {
    res.message = detail::merge_message(message, other.message);
  }
  return res;
}

inline auto Result::operator+=(const Result& other) -> Result& {
  status = static_cast<Status::Value>(
      std::min(static_cast<Status::Value>(status), static_cast<Status::Value>(other.status)));
  score += other.score;
  if (!other.named) {
    message = detail::merge_message(message, other.message);
  }
  return *this;
}

inline auto Result::operator&(const Result& other) const -> Result {
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

inline auto Result::operator&=(const Result& other) -> Result& {
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

inline auto Evaluator::post_evaluate(Result& result) -> void {
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
