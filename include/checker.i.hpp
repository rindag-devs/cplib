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
#include "checker.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_CHECKER_HPP_
#error "Must be included from checker.hpp"
#endif
#endif
/* cplib_embed_ignore end */

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

/* cplib_embed_ignore start */
#include "cmd_args.hpp"
#include "evaluate.hpp"
#include "io.hpp"
#include "json.hpp"
#include "macros.hpp"
#include "trace.hpp"
#include "utils.hpp"
#include "var.hpp"
/* cplib_embed_ignore end */

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

inline auto Reporter::get_evaluator_trace_stacks() const
    -> const std::vector<trace::TraceStack<evaluate::EvaluatorTrace>>& {
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
