/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

/* cplib_embed_ignore start */
#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
#include "interactor.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_INTERACTOR_HPP_
#error "Must be included from interactor.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/* cplib_embed_ignore start */
#include "cmd_args.hpp"
#include "json.hpp"
#include "macros.hpp"
#include "utils.hpp"
#include "var.hpp"
/* cplib_embed_ignore end */

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

inline Initializer::~Initializer() = default;

inline auto Initializer::set_state(State& state) -> void { state_ = &state; };

inline auto Initializer::set_inf_fileno(int fileno, var::Reader::TraceLevel trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.get_trace_stack());
        }
        panic(msg);
      });
}

inline auto Initializer::set_from_user_fileno(int fileno, var::Reader::TraceLevel trace_level)
    -> void {
  state_->from_user = var::detail::make_reader_by_fileno(
      fileno, "from_user", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.get_trace_stack());
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_to_user_fileno(int fileno) -> void {
  var::detail::make_ostream_by_fileno(fileno, state_->to_user_buf, state_->to_user);
}

inline auto Initializer::set_inf_path(std::string_view path, var::Reader::TraceLevel trace_level)
    -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.get_trace_stack());
        }
        panic(msg);
      });
}

inline auto Initializer::set_from_user_path(std::string_view path,
                                            var::Reader::TraceLevel trace_level) -> void {
  state_->from_user = var::detail::make_reader_by_path(
      path, "from_user", false, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= var::Reader::TraceLevel::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.get_trace_stack());
        }
        state_->quit_wa(msg);
      });
}

inline auto Initializer::set_to_user_path(std::string_view path) -> void {
  var::detail::make_ostream_by_path(path, state_->to_user_buf, state_->to_user);
}

inline Reporter::~Reporter() = default;

inline auto Reporter::attach_trace_stack(const var::Reader::TraceStack& trace_stack) -> void {
  trace_stack_ = trace_stack;
}

// Impl State {{{
inline State::State(std::unique_ptr<Initializer> initializer)
    : rnd(),
      inf(var::Reader(nullptr, var::Reader::TraceLevel::NONE, {})),
      from_user(var::Reader(nullptr, var::Reader::TraceLevel::NONE, {})),
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

  reporter->report(report);

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

inline auto DefaultInitializer::init(std::string_view argv0, const std::vector<std::string>& args)
    -> void {
  detail::detect_reporter(*state_);

  auto parsed_args = cmd_args::ParsedArgs(args);

  for (const auto& [key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(*state_, value)) {
        panic(format("Unknown %s option: %s", key.c_str(), value.c_str()));
      }
    } else {
      panic("Unknown command-line argument variable: " + key);
    }
  }

  for (const auto& flag : parsed_args.flags) {
    if (flag == "help") {
      detail::print_help_message(argv0);
    } else {
      panic("Unknown command-line argument flag: " + flag);
    }
  }

  if (parsed_args.ordered.size() != 1) {
    panic("Program must be run with the following arguments:\n  " +
          std::string(detail::ARGS_USAGE));
  }
  auto inf_path = parsed_args.ordered[0];

  set_inf_path(inf_path, var::Reader::TraceLevel::STACK_ONLY);
  set_from_user_fileno(fileno(stdin), var::Reader::TraceLevel::STACK_ONLY);
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

inline auto JsonReporter::report(const Report& report) -> void {
  std::map<std::string, std::unique_ptr<json::Value>> map;
  map.emplace("status", std::make_unique<json::String>(std::string(report.status.to_string())));
  map.emplace("score", std::make_unique<json::Real>(report.score));
  map.emplace("message", std::make_unique<json::String>(report.message));

  if (trace_stack_.has_value()) {
    map.emplace("reader_trace_stack", trace_stack_->to_json());
  }

  std::clog << json::Map(std::move(map)).to_string() << '\n';
  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto PlainTextReporter::report(const Report& report) -> void {
  std::clog << std::fixed << std::setprecision(2)
            << detail::status_to_title_string(report.status).c_str() << ", scores "
            << report.score * 100.0 << " of 100.\n";

  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    std::clog << report.message << '\n';
  }

  if (trace_stack_.has_value()) {
    std::clog << "\nReader trace stack (most recent variable last):\n";
    for (const auto& line : trace_stack_->to_plain_text_lines()) {
      std::clog << "  " << line << '\n';
    }
  }

  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto ColoredTextReporter::report(const Report& report) -> void {
  std::clog << std::fixed << std::setprecision(2)
            << detail::status_to_colored_title_string(report.status).c_str()
            << ", scores \x1b[0;33m" << report.score * 100.0 << "\x1b[0m of 100.\n";
  if (report.status != Report::Status::ACCEPTED || !report.message.empty()) {
    std::clog << report.message << '\n';
  }

  if (trace_stack_.has_value()) {
    std::clog << "\nReader trace stack (most recent variable last):\n";
    for (const auto& line : trace_stack_->to_colored_text_lines()) {
      std::clog << "  " << line << '\n';
    }
  }

  std::exit(report.status == Report::Status::ACCEPTED ? EXIT_SUCCESS : EXIT_FAILURE);
}
// /Impl reporters }}}
}  // namespace cplib::interactor
