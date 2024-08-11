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

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

/* cplib_embed_ignore start */
#include "cmd_args.hpp"
#include "io.hpp"
#include "json.hpp"
#include "macros.hpp"
#include "utils.hpp"
#include "var.hpp"
/* cplib_embed_ignore end */

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

inline Initializer::~Initializer() = default;

inline auto Initializer::set_state(State& state) -> void { state_ = &state; };

inline auto Initializer::state() -> State& { return *state_; };

inline auto Initializer::set_inf_fileno(int fileno, var::Reader::TraceLevel trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
          state_->reporter->attach_fragment(
              reader.make_fragment(var::Reader::Fragment::Direction::AROUND));
        }
        panic(msg);
      });
}

inline auto Initializer::set_ouf_fileno(int fileno, var::Reader::TraceLevel trace_level) -> void {
  state_->ouf = var::detail::make_reader_by_fileno(
      fileno, "ouf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
          state_->reporter->attach_fragment(
              reader.make_fragment(var::Reader::Fragment::Direction::AROUND));
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_ans_fileno(int fileno, var::Reader::TraceLevel trace_level) -> void {
  state_->ans = var::detail::make_reader_by_fileno(
      fileno, "ans", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
          state_->reporter->attach_fragment(
              reader.make_fragment(var::Reader::Fragment::Direction::AROUND));
        }
        panic(msg);
      });
}

inline auto Initializer::set_inf_path(std::string_view path,
                                      var::Reader::TraceLevel trace_level) -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
          state_->reporter->attach_fragment(
              reader.make_fragment(var::Reader::Fragment::Direction::AROUND));
        }
        panic(msg);
      });
}

inline auto Initializer::set_ouf_path(std::string_view path,
                                      var::Reader::TraceLevel trace_level) -> void {
  state_->ouf = var::detail::make_reader_by_path(
      path, "ouf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
          state_->reporter->attach_fragment(
              reader.make_fragment(var::Reader::Fragment::Direction::AROUND));
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_ans_path(std::string_view path,
                                      var::Reader::TraceLevel trace_level) -> void {
  state_->ans = var::detail::make_reader_by_path(
      path, "ans", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
          state_->reporter->attach_fragment(
              reader.make_fragment(var::Reader::Fragment::Direction::AROUND));
        }
        panic(msg);
      });
}

inline Reporter::~Reporter() = default;

inline auto Reporter::attach_trace_stack(const var::Reader::TraceStack& trace_stack) -> void {
  trace_stacks_.emplace(std::string(trace_stack.stream), trace_stack);
}

inline auto Reporter::detach_trace_stack(const std::string& stream) -> void {
  trace_stacks_.erase(stream);
}

inline auto Reporter::attach_fragment(const var::Reader::Fragment& fragment) -> void {
  fragments_.emplace(std::string(fragment.stream), fragment);
}

inline auto Reporter::detach_fragment(const std::string& stream) -> void {
  fragments_.erase(stream);
}

// Impl State {{{

inline State::State(std::unique_ptr<Initializer> initializer)
    : rnd(),
      inf(var::Reader(nullptr, var::Reader::TraceLevel::NONE, {})),
      ouf(var::Reader(nullptr, var::Reader::TraceLevel::NONE, {})),
      ans(var::Reader(nullptr, var::Reader::TraceLevel::NONE, {})),
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
      format(CPLIB_STARTUP_TEXT
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

inline auto DefaultInitializer::init(std::string_view arg0,
                                     const std::vector<std::string>& args) -> void {
  auto& state = this->state();

  detail::detect_reporter(state);

  auto parsed_args = cmd_args::ParsedArgs(args);

  for (const auto& [key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(state, value)) {
        panic(format("Unknown %s option: %s", key.c_str(), value.c_str()));
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

  set_inf_path(inf_path, var::Reader::TraceLevel::STACK_ONLY);
  set_ouf_path(ouf_path, var::Reader::TraceLevel::STACK_ONLY);
  set_ans_path(ans_path, var::Reader::TraceLevel::STACK_ONLY);
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

inline auto JsonReporter::report(const Report& report) -> int {
  std::map<std::string, std::unique_ptr<json::Value>> map;
  map.emplace("status", std::make_unique<json::String>(std::string(report.status.to_string())));
  map.emplace("score", std::make_unique<json::Real>(report.score));
  map.emplace("message", std::make_unique<json::String>(report.message));

  if (!trace_stacks_.empty()) {
    std::map<std::string, std::unique_ptr<json::Value>> trace_stacks_map;
    for (const auto& [name, stack] : trace_stacks_) {
      trace_stacks_map.emplace(name, stack.to_json());
    }
    map.emplace("reader_trace_stacks", std::make_unique<json::Map>(std::move(trace_stacks_map)));
  }

  if (!fragments_.empty()) {
    std::map<std::string, std::unique_ptr<json::Value>> fragments_map;
    for (const auto& [name, file] : fragments_) {
      fragments_map.emplace(name, file.to_json());
    }
    map.emplace("reader_fragments", std::make_unique<json::Map>(std::move(fragments_map)));
  }

  std::ostream stream(std::clog.rdbuf());
  stream << json::Map(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2) << detail::status_to_title_string(report.status)
         << ", scores " << report.score * 100.0 << " of 100.\n";

  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& [_, stack] : trace_stacks_) {
      for (const auto& line : stack.to_plain_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (!fragments_.empty()) {
    stream << "\nFragments:\n";
    for (const auto& [_, file] : fragments_) {
      stream << "  - " << file.to_plain_text() << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << std::fixed << std::setprecision(2)
         << detail::status_to_colored_title_string(report.status) << ", scores \x1b[0;33m"
         << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& [_, stack] : trace_stacks_) {
      for (const auto& line : stack.to_colored_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (!fragments_.empty()) {
    stream << "\nFragments:\n";
    for (const auto& [_, file] : fragments_) {
      stream << "  - " << file.to_colored_text() << '\n';
    }
  }

  return report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE;
}
// /Impl reporters }}}
}  // namespace cplib::checker
