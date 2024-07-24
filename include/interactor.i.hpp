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

#include <cstdio>    // for fileno, stderr, stdin, stdout
#include <cstdlib>   // for exit, EXIT_FAILURE, EXIT_SUCCESS
#include <iomanip>   // for operator<<, setprecision
#include <iostream>  // for basic_istream, clog, cin, wcin, cerr, cout, wcerr
#include <utility>   // for move

/* cplib_embed_ignore start */
#include "io.hpp"  // for InStream, InStream::seek_eof
#include "json.hpp"
#include "macros.hpp"  // for isatty, CPLIB_VERSION
#include "utils.hpp"   // for panic, format, has_colors, json_string_encode
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

inline Reporter::~Reporter() = default;

inline auto Reporter::attach_trace_stack(const cplib::var::Reader::TraceStack& trace_stack)
    -> void {
  trace_stack_ = trace_stack;
}

// Impl State {{{
inline State::State(Initializer initializer)
    : rnd(),
      inf(var::Reader(nullptr, {})),
      from_user(var::Reader(nullptr, {})),
      to_user(std::ostream(nullptr)),
      initializer(std::move(initializer)),
      reporter(std::make_unique<JsonReporter>()) {
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

  state.inf = var::detail::make_file_reader(
      inf_path, "inf", false,
      [&state](std::string_view msg, const cplib::var::Reader::TraceStack& traces) {
        state.reporter->attach_trace_stack(traces);
        panic(msg);
      });

  state.from_user = var::detail::make_stdin_reader(
      "from_user", false,
      [&state](std::string_view msg, const cplib::var::Reader::TraceStack& traces) {
        state.reporter->attach_trace_stack(traces);
        state.quit_wa(msg);
      });

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

inline auto JsonReporter::report(const Report& report) -> void {
  std::map<std::string, std::unique_ptr<cplib::json::Value>> map;
  map.insert(
      {"status", std::make_unique<cplib::json::String>(std::string(report.status.to_string()))});
  map.insert({"score", std::make_unique<cplib::json::Real>(report.score)});
  map.insert({"message", std::make_unique<cplib::json::String>(report.message)});

  if (trace_stack_.has_value()) {
    map.insert({"reader_trace_stack", trace_stack_->to_json()});
  }

  std::clog << cplib::json::Map(std::move(map)).to_string() << '\n';
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
