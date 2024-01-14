/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
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

#include <algorithm>    // for binary_search, sort
#include <cstdio>       // for fileno, stderr
#include <cstdlib>      // for exit, EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>     // for basic_ostream, operator<<, clog, boolalpha
#include <map>          // for map, _Rb_tree_iterator, operator!=
#include <optional>     // for optional, nullopt
#include <set>          // for set
#include <string>       // for basic_string, char_traits, allocator, operator<
#include <string_view>  // for operator==, string_view, basic_string_view
#include <utility>      // for pair, move
#include <vector>       // for vector

/* cplib_embed_ignore start */
#include "macros.hpp"  // for isatty, CPLIB_VERSION
#include "utils.hpp"   // for panic, format, has_colors, join, json_string_...
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
      panic(format("Unknown generator report status: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, std::string message)
    : status(status), message(std::move(message)) {}

// Impl State {{{
inline State::State(Initializer initializer)
    : rnd(),
      initializer(std::move(initializer)),
      reporter(json_reporter),
      required_flag_args(),
      required_var_args(),
      flag_parsers(),
      var_parsers() {
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

  reporter(report);

  std::clog << "Unrecoverable error: Reporter didn't exit the program\n";
  std::exit(EXIT_FAILURE);
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
      format("cplib (CPLib) " CPLIB_VERSION
             "\n"
             "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
             "\n"
             "Usage:\n"
             "  %s %s\n"
             "\n"
             "Set environment variable `NO_COLOR=1` / `CLICOLOR_FORCE=1` to force disable / "
             "enable colors",
             program_name.data(), args_usage.data());
  panic(msg);
}

inline auto detect_reporter(State& state) -> void {
  if (!isatty(fileno(stderr))) {
    state.reporter = json_reporter;
  } else if (cplib::detail::has_colors()) {
    state.reporter = colored_text_reporter;
  } else {
    state.reporter = plain_text_reporter;
  }
}

// Set the report format of `state` according to the string `format`.
//
// Returns `false` if the `format` is invalid.
inline auto set_report_format(State& state, std::string_view format) -> bool {
  if (format == "auto") {
    detect_reporter(state);
  } else if (format == "json") {
    state.reporter = json_reporter;
  } else if (format == "text") {
    if (cplib::detail::has_colors()) {
      state.reporter = colored_text_reporter;
    } else {
      state.reporter = plain_text_reporter;
    }
  } else {
    return false;
  }
  return true;
}

inline auto parse_command_line_arguments(State& state, int argc, char** argv)
    -> std::pair<std::set<std::string>, std::map<std::string, std::string>> {
  std::set<std::string> flag_args;
  std::map<std::string, std::string> var_args;

  for (int i = 1; i < argc; ++i) {
    auto arg = std::string_view(argv[i]);
    if (constexpr std::string_view prefix = "--report-format=";
        !arg.compare(0, prefix.size(), prefix)) {
      arg.remove_prefix(prefix.size());
      if (!set_report_format(state, arg)) {
        panic(format("Unknown %s option: %s", prefix.data(), arg.data()));
      }
    } else {
      auto [name, value] = parse_arg(arg);
      if (!value.has_value()) {
        if (!std::binary_search(state.required_flag_args.begin(), state.required_flag_args.end(),
                                name)) {
          panic("Unknown flag: " + name);
        }
        flag_args.insert(name);
      } else {
        if (!std::binary_search(state.required_var_args.begin(), state.required_var_args.end(),
                                name)) {
          panic("Unknown variable: " + name);
        }
        if (auto it = var_args.find(name); it != var_args.end()) {
          it->second.push_back(' ');
          it->second += *value;
        } else {
          var_args[name] = *value;
        }
      }
    }
  }

  return {flag_args, var_args};
}

inline auto validate_required_arguments(const State& state,
                                        const std::map<std::string, std::string>& var_args)
    -> void {
  for (const auto& var : state.required_var_args) {
    if (!var_args.count(var)) panic("Missing variable: " + var);
  }
}

inline auto get_args_usage(const State& state) {
  using namespace std::string_literals;
  std::vector<std::string> builder;
  builder.reserve(state.required_flag_args.size() + state.required_var_args.size());
  for (const auto& arg : state.required_flag_args) builder.push_back("[--"s + arg + "]"s);
  for (const auto& arg : state.required_var_args) builder.push_back("--"s + arg + "=<value>"s);
  builder.push_back("[--report-format={auto|json|text}]"s);

  return join(builder.begin(), builder.end(), ' ');
}
}  // namespace detail

inline auto DefaultInitializer::operator()(State& state, int argc, char** argv) -> void {
  std::sort(state.required_flag_args.begin(), state.required_flag_args.end());
  std::sort(state.required_var_args.begin(), state.required_var_args.end());

  auto args_usage = detail::get_args_usage(state);

  detail::detect_reporter(state);

  if (argc > 1 && std::string_view("--help") == argv[1]) {
    detail::print_help_message(argv[0], args_usage);
  }

  auto [flag_args, var_args] = detail::parse_command_line_arguments(state, argc, argv);

  detail::validate_required_arguments(state, var_args);

  for (const auto& parser : state.flag_parsers) parser(flag_args);
  for (const auto& parser : state.var_parsers) parser(var_args);

  // Unsynchronize to speed up std::cout output.
  std::ios_base::sync_with_stdio(false);

  state.rnd.reseed(argc, argv);
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
      panic(format("Unknown generator report status: %d", static_cast<int>(status)));
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
      panic(format("Unknown generator report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}
}  // namespace detail

inline auto json_reporter(const Report& report) -> void {
  std::clog << std::boolalpha;

  std::clog << R"({"status": ")" << report.status.to_string() << R"(", "message": ")"
            << cplib::detail::json_string_encode(report.message) << "\"}";

  std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto plain_text_reporter(const Report& report) -> void {
  if (report.status == Report::Status::OK && report.message.empty()) {
    // Do nothing when the report is OK and message is empty.
    std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  std::clog << detail::status_to_title_string(report.status).c_str() << ".\n"
            << report.message << '\n';

  std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto colored_text_reporter(const Report& report) -> void {
  if (report.status == Report::Status::OK && report.message.empty()) {
    // Do nothing when the report is OK and message is empty.
    std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
  }

  std::clog << detail::status_to_colored_title_string(report.status).c_str() << ".\n"
            << report.message << '\n';

  std::exit(report.status == Report::Status::OK ? EXIT_SUCCESS : EXIT_FAILURE);
}
// /Impl reporters }}}
}  // namespace cplib::generator
