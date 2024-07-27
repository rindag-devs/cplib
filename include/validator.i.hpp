/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

/* cplib_embed_ignore start */
#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
#include "validator.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_VALIDATOR_HPP_
#error "Must be included from validator.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/* cplib_embed_ignore start */
#include "cmd_args.hpp"
#include "io.hpp"
#include "json.hpp"
#include "macros.hpp"
#include "utils.hpp"
#include "var.hpp"
/* cplib_embed_ignore end */

namespace cplib::validator {
inline constexpr Report::Status::Status(Value value) : value_(value) {}

inline constexpr Report::Status::operator Value() const { return value_; }

inline constexpr auto Report::Status::to_string() const -> std::string_view {
  switch (value_) {
    case INTERNAL_ERROR:
      return "internal_error";
    case VALID:
      return "valid";
    case INVALID:
      return "invalid";
    default:
      panic(format("Unknown validator report status: %d", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, std::string message)
    : status(status), message(std::move(message)) {}

inline Trait::Trait(std::string name, CheckFunc check_func)
    : Trait(std::move(name), std::move(check_func), {}) {}

inline Trait::Trait(std::string name, CheckFunc check_func, std::vector<std::string> dependencies)
    : name(std::move(name)),
      check_func(std::move(check_func)),
      dependencies(std::move(dependencies)) {}

inline Initializer::~Initializer() = default;

inline auto Initializer::set_state(State& state) -> void { state_ = &state; };

inline auto Initializer::set_inf_fileno(int fileno) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", true,
      [this](std::string_view msg, const cplib::var::Reader::TraceStack& traces) {
        state_->reporter->attach_trace_stack(traces);
        state_->quit_invalid(msg);
      });
}

inline auto Initializer::set_inf_path(std::string_view path) -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", true,
      [this](std::string_view msg, const cplib::var::Reader::TraceStack& traces) {
        state_->reporter->attach_trace_stack(traces);
        state_->quit_invalid(msg);
      });
}

inline Reporter::~Reporter() = default;

inline auto Reporter::attach_trace_stack(const cplib::var::Reader::TraceStack& trace_stack)
    -> void {
  trace_stack_ = trace_stack;
}

inline auto Reporter::attach_trait_status(const std::map<std::string, bool>& trait_status) -> void {
  trait_status_ = trait_status;
}

// Impl State {{{
namespace detail {
/**
 * In topological sorting, `callback` is called every time a new node is reached.
 * If `fn` returns false, nodes reachable by the current node will no longer be visited.
 */
inline auto topo_sort(const std::vector<std::vector<size_t>>& edges,
                      const std::function<auto(size_t)->bool>& callback) -> void {
  std::vector<size_t> degree(edges.size(), 0);

  for (const auto& edge : edges) {
    for (auto to : edge) ++degree[to];
  }

  std::queue<size_t> queue;

  for (size_t i = 0; i < edges.size(); ++i) {
    if (degree[i] == 0) queue.push(i);
  }

  while (!queue.empty()) {
    auto front = queue.front();
    queue.pop();
    if (!callback(front)) continue;
    for (auto to : edges[front]) {
      --degree[to];
      if (!degree[to]) queue.push(to);
    }
  }
}

inline auto build_edges(std::vector<Trait>& traits)
    -> std::optional<std::vector<std::vector<size_t>>> {
  // Check duplicate name
  std::sort(traits.begin(), traits.end(),
            [](const Trait& x, const Trait& y) { return x.name < y.name; });
  if (std::unique(traits.begin(), traits.end(), [](const Trait& x, const Trait& y) {
        return x.name == y.name;
      }) != traits.end()) {
    // Found duplicate name
    return std::nullopt;
  }

  std::vector<std::vector<size_t>> edges(traits.size());

  for (size_t i = 0; i < traits.size(); ++i) {
    auto& trait = traits[i];
    // Check duplicate dependencies
    std::sort(trait.dependencies.begin(), trait.dependencies.end());
    if (std::unique(trait.dependencies.begin(), trait.dependencies.end()) !=
        trait.dependencies.end()) {
      // Found duplicate dependencies
      return std::nullopt;
    }

    for (const auto& dep : trait.dependencies) {
      auto dep_id =
          std::lower_bound(traits.begin(), traits.end(), dep,
                           [](const Trait& x, const std::string& y) { return x.name < y; }) -
          traits.begin();
      edges[dep_id].emplace_back(i);
    }
  }

  return edges;
}

inline auto have_loop(const std::vector<std::vector<size_t>>& edges) -> bool {
  std::vector<uint8_t> visited(edges.size(), 0);  // Never use std::vector<bool>

  topo_sort(edges, [&](size_t node) {
    visited[node] = 1;
    return true;
  });

  for (auto v : visited) {
    if (!v) return true;
  }
  return false;
}

inline auto validate_traits(const std::vector<Trait>& traits,
                            const std::vector<std::vector<std::size_t>>& edges)
    -> std::map<std::string, bool> {
  std::map<std::string, bool> results;
  for (const auto& trait : traits) results[trait.name] = false;

  topo_sort(edges, [&](std::size_t id) {
    auto& node = traits[id];
    auto result = node.check_func();
    results.at(node.name) = result;
    return result;
  });

  return results;
}
}  // namespace detail

inline State::State(std::unique_ptr<Initializer> initializer)
    : rnd(),
      inf(var::Reader(nullptr, {})),
      initializer(std::move(initializer)),
      reporter(std::make_unique<JsonReporter>()),
      traits_(),
      trait_edges_() {
  this->initializer->set_state(*this);
  cplib::detail::panic_impl = [this](std::string_view msg) {
    quit(Report(Report::Status::INTERNAL_ERROR, std::string(msg)));
  };
  cplib::detail::work_mode = WorkMode::VALIDATOR;
}

inline State::~State() {
  if (!exited_) panic("Validator must exit by calling method `State::quit*`");
}

inline auto State::traits(std::vector<Trait> traits) -> void {
  traits_ = std::move(traits);

  auto edges = detail::build_edges(traits_);
  if (!edges.has_value()) panic("Traits do not form a simple graph");

  if (detail::have_loop(*edges)) panic("Traits do not form a DAG");

  trait_edges_ = *edges;
}

inline auto State::quit(Report report) -> void {
  exited_ = true;

  if (report.status == Report::Status::VALID && !inf.inner().eof()) {
    report = Report(Report::Status::INVALID, "Extra content in the input file");
  }

  if (report.status == Report::Status::VALID) {
    reporter->attach_trait_status(detail::validate_traits(traits_, trait_edges_));
  }

  reporter->report(report);

  std::clog << "Unrecoverable error: Reporter didn't exit the program\n";
  std::exit(EXIT_FAILURE);
}

inline auto State::quit_valid() -> void { quit(Report(Report::Status::VALID, "")); }

inline auto State::quit_invalid(std::string_view message) -> void {
  quit(Report(Report::Status::INVALID, std::string(message)));
}
// /Impl State }}}

// Impl DefaultInitializer {{{
namespace detail {
constexpr std::string_view ARGS_USAGE = "[<input_file>] [--report-format={auto|json|text}]";

inline auto print_help_message(std::string_view program_name) -> void {
  std::string msg =
      format("cplib (CPLib) " CPLIB_VERSION
             "\n"
             "https://github.com/rindag-devs/cplib/ by Rindag Devs, copyright(c) 2023\n"
             "\n"
             "Usage:\n"
             "  %s %s\n"
             "\n"
             "If <input_file> does not exist, stdin will be used as input\n"
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

inline auto DefaultInitializer::init(std::string_view argv0, const std::vector<std::string>& args)
    -> void {
  detail::detect_reporter(*state_);

  auto parsed_args = cplib::cmd_args::ParsedArgs(args);

  for (const auto& [key, value] : parsed_args.vars) {
    if (key == "report-format") {
      if (!detail::set_report_format(*state_, value)) {
        panic(format("Unknown %s option: %s", key.c_str(), value.c_str()));
      }
    } else {
      panic("Unknown command line argument variable: " + key);
    }
  }

  for (const auto& flag : parsed_args.flags) {
    if (flag == "help") {
      detail::print_help_message(argv0);
    } else {
      panic("Unknown command line argument flag: " + flag);
    }
  }

  if (parsed_args.ordered.size() > 1) {
    panic("Program must be run with the following arguments:\n  " +
          std::string(detail::ARGS_USAGE));
  }

  if (parsed_args.ordered.empty()) {
    set_inf_fileno(fileno(stdin));
  } else {
    set_inf_path(parsed_args.ordered[0]);
  }
}
// /Impl DefaultInitializer }}}

// Impl reporters {{{
namespace detail {
inline auto status_to_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "Internal Error";
    case Report::Status::VALID:
      return "Valid";
    case Report::Status::INVALID:
      return "Invalid";
    default:
      panic(format("Unknown validator report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto status_to_colored_title_string(Report::Status status) -> std::string {
  switch (status) {
    case Report::Status::INTERNAL_ERROR:
      return "\x1b[0;35mInternal Error\x1b[0m";
    case Report::Status::VALID:
      return "\x1b[0;32mValid\x1b[0m";
    case Report::Status::INVALID:
      return "\x1b[0;31mInvalid\x1b[0m";
    default:
      panic(format("Unknown validator report status: %d", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto trait_status_to_json(const std::map<std::string, bool>& traits)
    -> std::unique_ptr<cplib::json::Map> {
  std::map<std::string, std::unique_ptr<cplib::json::Value>> map;

  for (const auto& [k, v] : traits) {
    map.emplace(k, std::make_unique<cplib::json::Bool>(v));
  }

  return std::make_unique<cplib::json::Map>(std::move(map));
}
}  // namespace detail

inline auto JsonReporter::report(const Report& report) -> void {
  std::map<std::string, std::unique_ptr<cplib::json::Value>> map;
  map.emplace("status",
              std::make_unique<cplib::json::String>(std::string(report.status.to_string())));
  map.emplace("message", std::make_unique<cplib::json::String>(report.message));

  if (trace_stack_.has_value()) {
    map.emplace("reader_trace_stack", trace_stack_->to_json());
  }

  if (!trait_status_.empty()) {
    map.emplace("traits", detail::trait_status_to_json(trait_status_));
  }

  std::clog << cplib::json::Map(std::move(map)).to_string() << '\n';
  std::exit(report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto PlainTextReporter::report(const Report& report) -> void {
  std::clog << detail::status_to_title_string(report.status).c_str() << ".\n";

  if (report.status != Report::Status::VALID || !report.message.empty()) {
    std::clog << report.message << '\n';
  }

  if (trace_stack_.has_value()) {
    std::clog << "\nReader trace stack (most recent variable last):\n";
    for (const auto& line : trace_stack_->to_plain_text_lines()) {
      std::clog << "  " << line << '\n';
    }
  }

  if (report.status == Report::Status::VALID && !trait_status_.empty()) {
    std::clog << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (auto [name, satisfaction] : trait_status_) {
      if (satisfaction) {
        satisfied.emplace_back(name);
      } else {
        dissatisfied.emplace_back(name);
      }
    }

    for (const auto& name : satisfied) {
      std::clog << "+ " << cplib::detail::hex_encode(name) << '\n';
    }
    for (const auto& name : dissatisfied) {
      std::clog << "- " << cplib::detail::hex_encode(name) << '\n';
    }
  }

  std::exit(report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE);
}

inline auto ColoredTextReporter::report(const Report& report) -> void {
  std::clog << detail::status_to_colored_title_string(report.status).c_str() << ".\n";

  if (report.status != Report::Status::VALID || !report.message.empty()) {
    std::clog << report.message << '\n';
  }

  if (trace_stack_.has_value()) {
    std::clog << "\nReader trace stack (most recent variable last):\n";
    for (const auto& line : trace_stack_->to_colored_text_lines()) {
      std::clog << "  " << line << '\n';
    }
  }

  if (report.status == Report::Status::VALID && !trait_status_.empty()) {
    std::clog << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (auto [name, satisfaction] : trait_status_) {
      if (satisfaction) {
        satisfied.emplace_back(name);
      } else {
        dissatisfied.emplace_back(name);
      }
    }

    for (const auto& name : satisfied) {
      std::clog << "\x1b[0;32m+\x1b[0m " << name << '\n';
    }
    for (const auto& name : dissatisfied) {
      std::clog << "\x1b[0;31m-\x1b[0m " << name << '\n';
    }
  }

  std::exit(report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE);
}
// /Impl reporters }}}
}  // namespace cplib::validator
