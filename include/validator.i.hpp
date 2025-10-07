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
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
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
#include "trace.hpp"
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
      panic(cplib::format("Unknown validator report status: {}", static_cast<int>(value_)));
      return "unknown";
  }
}

inline Report::Report(Report::Status status, std::string message)
    : status(status), message(std::move(message)) {}

inline Trait::Trait(std::string name, CheckFunc check_func)
    : Trait(std::move(name), std::move(check_func), {}) {}

inline Trait::Trait(std::string name, CheckFunc check_func,
                    std::map<std::string, bool> dependencies)
    : name(std::move(name)),
      check_func(std::move(check_func)),
      dependencies(std::move(dependencies)) {}

inline Initializer::~Initializer() = default;

inline auto Initializer::set_state(State& state) -> void { state_ = &state; };

inline auto Initializer::state() -> State& { return *state_; };

inline auto Initializer::set_inf_fileno(int fileno, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_fileno(
      fileno, "inf", true, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_invalid(msg);
      });
}

inline auto Initializer::set_inf_path(std::string_view path, trace::Level trace_level) -> void {
  state_->inf = var::detail::make_reader_by_path(
      path, "inf", true, trace_level,
      [this, trace_level](const var::Reader& reader, std::string_view msg) {
        if (trace_level >= trace::Level::STACK_ONLY) {
          state_->reporter->attach_trace_stack(reader.make_trace_stack(true));
        }
        state_->quit_invalid(msg);
      });
}

inline Reporter::~Reporter() = default;

inline auto Reporter::attach_trace_stack(trace::TraceStack<var::ReaderTrace> trace_stack) -> void {
  trace_stacks_.emplace_back(std::move(trace_stack));
}

inline auto Reporter::attach_trace_tree(const trace::TraceTreeNode<var::ReaderTrace>* root)
    -> void {
  if (!root) {
    panic("Reporter::attach_trace_tree failed: Trace tree root pointer is nullptr");
  }

  trace_tree_ = root;
}

inline auto Reporter::attach_trait_status(const std::map<std::string, bool>& trait_status) -> void {
  trait_status_ = trait_status;
}

// Impl State {{{
namespace detail {
/**
 * In topological sorting, `callback` is called every time a new node is reached.
 *
 * If `follow_unmatched_edge` is false, outgoing edges with a value different from the value
 * returned by the callback for the current node will not be visited.
 */
inline auto topo_sort(const std::vector<std::vector<std::pair<std::size_t, bool>>>& edges,
                      const bool follow_unmatched_edge,
                      const std::function<auto(std::size_t)->bool>& callback) -> void {
  std::vector<std::size_t> degree(edges.size(), 0);

  for (const auto& edge : edges) {
    for (auto [to, v] : edge) ++degree[to];
  }

  std::queue<std::size_t> queue;

  for (std::size_t i = 0; i < edges.size(); ++i) {
    if (degree[i] == 0) queue.push(i);
  }

  while (!queue.empty()) {
    auto front = queue.front();
    queue.pop();
    auto result = callback(front);
    for (auto [to, v] : edges[front]) {
      if (!follow_unmatched_edge && v != result) continue;
      --degree[to];
      if (!degree[to]) queue.push(to);
    }
  }
}

// Returns std::nullopt if failed
inline auto build_edges(std::vector<Trait>& traits)
    -> std::optional<std::vector<std::vector<std::pair<std::size_t, bool>>>> {
  // Check duplicate name
  std::ranges::sort(traits, [](const Trait& x, const Trait& y) { return x.name < y.name; });
  if (std::ranges::unique(traits, [](const Trait& x, const Trait& y) {
        return x.name == y.name;
      }).end() != traits.end()) {
    // Found duplicate name
    return std::nullopt;
  }

  std::vector<std::vector<std::pair<std::size_t, bool>>> edges(traits.size());

  for (std::size_t i = 0; i < traits.size(); ++i) {
    auto& trait = traits[i];
    for (const auto& [name, value] : trait.dependencies) {
      auto it = std::ranges::lower_bound(traits, name, std::less{}, &Trait::name);
      // IMPORTANT: Check if the dependency was actually found and is an exact match.
      if (it == traits.end() || it->name != name) {
        return std::nullopt;
      }
      auto dep_id = it - traits.begin();
      edges[dep_id].emplace_back(i, value);
    }
  }

  return edges;
}

inline auto have_loop(const std::vector<std::vector<std::pair<std::size_t, bool>>>& edges) -> bool {
  std::vector<std::uint8_t> visited(edges.size(), 0);  // Never use std::vector<bool>

  topo_sort(edges, true, [&](std::size_t node) {
    visited[node] = 1;
    return true;
  });

  for (auto v : visited) {
    if (!v) return true;
  }
  return false;
}

inline auto validate_traits(const std::vector<Trait>& traits,
                            const std::vector<std::vector<std::pair<std::size_t, bool>>>& edges)
    -> std::map<std::string, bool> {
  std::map<std::string, bool> results;

  topo_sort(edges, true, [&](std::size_t id) {
    auto& node = traits[id];
    auto result = node.check_func();
    results.emplace(node.name, result);
    return result;
  });

  return results;
}
}  // namespace detail

inline State::State(std::unique_ptr<Initializer> initializer)
    : rnd(),
      inf(var::Reader(nullptr, trace::Level::NONE, {})),
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

    if (inf.get_trace_level() >= trace::Level::FULL) {
      reporter->attach_trace_tree(inf.get_trace_tree());
    }
  }

  auto exit_code = reporter->report(report);
  std::exit(exit_code);
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
      cplib::format(CPLIB_STARTUP_TEXT
                    "Usage:\n"
                    "  {} {}\n"
                    "\n"
                    "If <input_file> does not exist, stdin will be used as input\n"
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

  if (parsed_args.ordered.size() > 1) {
    panic("Program must be run with the following arguments:\n  " +
          std::string(detail::ARGS_USAGE));
  }

  if (parsed_args.ordered.empty()) {
    set_inf_fileno(fileno(stdin), trace::Level::FULL);
  } else {
    set_inf_path(parsed_args.ordered[0], trace::Level::FULL);
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
      panic(cplib::format("Unknown validator report status: {}", static_cast<int>(status)));
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
      panic(cplib::format("Unknown validator report status: {}", static_cast<int>(status)));
      return "Unknown";
  }
}

inline auto trait_status_to_json(const std::map<std::string, bool>& traits) -> json::Value {
  json::Map map;
  for (const auto& [k, v] : traits) {
    map.emplace(k, json::Value(v));
  }
  return json::Value(map);
}

inline auto print_trace_tree(const trace::TraceTreeNode<var::ReaderTrace>* node, std::size_t depth,
                             std::size_t& n_remaining_node, bool colored_output, std::ostream& os)
    -> void {
  if (!node || depth >= 8 || (node->tags.count("#hidden"))) {
    return;
  }

  if (depth) {
    --n_remaining_node;

    // indentation
    for (std::size_t i = 0; i < depth - 1; ++i) {
      os << "  ";
    }
    os << "- ";

    // name
    if (colored_output) {
      os << "\x1b[0;33m";
    }
    os << node->trace.var_name;
    if (colored_output) {
      os << "\x1b[0m";
    }

    // type
    if (node->tags.count("#t")) {
      if (colored_output) {
        os << "\x1b[0;90m";
      }
      os << ": " << node->tags.at("#t").to_string();
      if (colored_output) {
        os << "\x1b[0m";
      }
    }

    // value
    if (node->tags.count("#v")) {
      os << " = " << cplib::compress(node->tags.at("#v").to_string());
    }
    os << '\n';
  }

  std::size_t n_visible_children = 0;
  for (const auto& child : node->get_children()) {
    if (!child->tags.count("#hidden")) {
      ++n_visible_children;
    }
  }

  for (const auto& child : node->get_children()) {
    if (child->tags.count("#hidden")) {
      continue;
    }
    if (!n_remaining_node) {
      for (std::size_t i = 0; i < depth; ++i) {
        os << "  ";
      }
      os << "- ... and " << n_visible_children << " more\n";
      break;
    }
    --n_visible_children;
    print_trace_tree(child.get(), depth + 1, n_remaining_node, colored_output, os);
  }
}
}  // namespace detail

inline auto JsonReporter::report(const Report& report) -> int {
  json::Map map{
      {"status", json::Value(json::String(report.status.to_string()))},
      {"message", json::Value(report.message)},
  };

  if (!trace_stacks_.empty()) {
    json::List trace_stacks;
    trace_stacks.reserve(trace_stacks_.size());
    std::ranges::transform(trace_stacks_, std::back_inserter(trace_stacks),
                           [](auto& s) { return json::Value(s.to_json()); });
    map.emplace("reader_trace_stacks", trace_stacks);
  }

  if (!trait_status_.empty()) {
    map.emplace("traits", detail::trait_status_to_json(trait_status_));
  }

  if (trace_tree_) {
    auto json = trace_tree_->to_json();
    if (json.count("children")) {
      map.emplace("reader_trace_tree", std::move(json.at("children")));
    }
  }

  std::ostream stream(std::clog.rdbuf());
  stream << json::Value(std::move(map)).to_string() << '\n';
  return report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto PlainTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << detail::status_to_title_string(report.status) << ".\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& stack : trace_stacks_) {
      for (const auto& line : stack.to_plain_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (report.status == Report::Status::VALID && !trait_status_.empty()) {
    stream << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (const auto& [name, satisfaction] : trait_status_) {
      if (satisfaction) {
        satisfied.emplace_back(name);
      } else {
        dissatisfied.emplace_back(name);
      }
    }

    for (const auto& name : satisfied) {
      stream << "+ " << cplib::detail::hex_encode(name) << '\n';
    }
    for (const auto& name : dissatisfied) {
      stream << "- " << cplib::detail::hex_encode(name) << '\n';
    }
  }

  if (trace_tree_) {
    stream << "\nReader trace tree:\n";
    std::size_t n_remaining_node = 32;
    detail::print_trace_tree(trace_tree_, 0, n_remaining_node, false, stream);
  }

  return report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE;
}

inline auto ColoredTextReporter::report(const Report& report) -> int {
  std::ostream stream(std::clog.rdbuf());

  stream << detail::status_to_colored_title_string(report.status) << ".\n";

  if (!report.message.empty()) {
    stream << report.message << '\n';
  }

  if (!trace_stacks_.empty()) {
    stream << "\nReader trace stacks (most recent variable last):";
    for (const auto& stack : trace_stacks_) {
      for (const auto& line : stack.to_colored_text_lines()) {
        stream << '\n' << "  " << line;
      }
      stream << '\n';
    }
  }

  if (report.status == Report::Status::VALID && !trait_status_.empty()) {
    stream << "\nTraits satisfactions:\n";

    std::vector<std::string> satisfied, dissatisfied;
    for (const auto& [name, satisfaction] : trait_status_) {
      if (satisfaction) {
        satisfied.emplace_back(name);
      } else {
        dissatisfied.emplace_back(name);
      }
    }

    for (const auto& name : satisfied) {
      stream << "\x1b[0;32m+\x1b[0m " << name << '\n';
    }
    for (const auto& name : dissatisfied) {
      stream << "\x1b[0;31m-\x1b[0m " << name << '\n';
    }
  }

  if (trace_tree_) {
    stream << "\nReader trace tree:\n";
    std::size_t n_remaining_node = 32;
    detail::print_trace_tree(trace_tree_, 0, n_remaining_node, true, stream);
  }

  return report.status == Report::Status::VALID ? EXIT_SUCCESS : EXIT_FAILURE;
}
// /Impl reporters }}}
}  // namespace cplib::validator
