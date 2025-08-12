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
#include "generator.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_GENERATOR_HPP_
#error "Must be included from generator.hpp"
#endif
#endif
/* cplib_embed_ignore end */

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

/* cplib_embed_ignore start */
#include "cmd_args.hpp"
#include "json.hpp"
#include "macros.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

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
