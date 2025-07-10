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
#include "trace.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_TRACE_HPP_
#error "Must be included from trace.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/* cplib_embed_ignore start */
#include "json.hpp"
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::trace {

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_json() const -> json::Value {
  json::Map map;
  json::List stack_list;

  stack_list.reserve(stack.size());
  for (const auto& trace : stack) {
    stack_list.emplace_back(trace.to_stack_json());
  }

  map.emplace("stack", std::move(stack_list));
  map.emplace("fatal", fatal);
  return json::Value(map);
}

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_plain_text_lines() const -> std::vector<std::string> {
  std::vector<std::string> lines;
  lines.reserve(stack.size());

  if (fatal) {
    lines.emplace_back("[fatal]");
  }

  std::size_t id = 0;
  for (const auto& trace : stack) {
    auto line = cplib::format("#{}: {}", id, trace.to_plain_text());
    ++id;
    lines.emplace_back(std::move(line));
  }

  return lines;
}

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_colored_text_lines() const -> std::vector<std::string> {
  std::vector<std::string> lines;
  lines.reserve(stack.size());

  if (fatal) {
    lines.emplace_back("\x1b[0;31m[fatal]\x1b[0m");
  }

  std::size_t id = 0;
  for (const auto& trace : stack) {
    auto line = cplib::format("#{}: {}", id, trace.to_colored_text());
    ++id;
    lines.emplace_back(std::move(line));
  }

  return lines;
}

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_plain_text_compact() const -> std::string {
  std::stringbuf buf(std::ios_base::out);

  if (fatal) {
    constexpr std::string_view FATAL_TEXT = "[fatal] ";
    buf.sputn(FATAL_TEXT.data(), FATAL_TEXT.size());
  }

  for (auto it = stack.begin(); it != stack.end(); ++it) {
    if (std::next(it) == stack.end()) {
      auto node_text = it->to_plain_text();
      buf.sputn(node_text.c_str(), node_text.size());
      continue;
    }
    auto node_text = it->node_name();
    buf.sputn(node_text.c_str(), node_text.size());
    buf.sputc('/');
  }

  return buf.str();
}

template <Trace T>
[[nodiscard]] inline auto TraceStack<T>::to_colored_text_compact() const -> std::string {
  std::stringbuf buf(std::ios_base::out);

  if (fatal) {
    constexpr std::string_view FATAL_TEXT = "\x1b[0;31m[fatal]\x1b[0m ";
    buf.sputn(FATAL_TEXT.data(), FATAL_TEXT.size());
  }

  for (auto it = stack.begin(); it != stack.end(); ++it) {
    if (std::next(it) == stack.end()) {
      auto node_text = it->to_colored_text();
      buf.sputn(node_text.c_str(), node_text.size());
      continue;
    }
    auto node_text = it->node_name();
    buf.sputn(node_text.c_str(), node_text.size());
    constexpr std::string_view SLASH = "\x1b[0;90m/\x1b[0m";
    buf.sputn(SLASH.data(), SLASH.size());
  }

  return buf.str();
}

template <Trace T>
inline TraceTreeNode<T>::TraceTreeNode(T trace) : trace(std::move(trace)) {}

template <Trace T>
[[nodiscard]] inline auto TraceTreeNode<T>::get_children() const
    -> const std::vector<std::unique_ptr<TraceTreeNode>>& {
  return children_;
}

template <Trace T>
[[nodiscard]] inline auto TraceTreeNode<T>::get_parent() -> TraceTreeNode* {
  return parent_;
}

template <Trace T>
[[nodiscard]] inline auto TraceTreeNode<T>::to_json() const -> json::Map {
  json::Map map;

  if (tags.count("#hidden")) {
    return {};
  }

  map.emplace("trace", trace.to_tree_json());
  if (!tags.empty()) {
    map.emplace("tags", tags);
  }

  const auto& children = get_children();
  json::List children_list;
  children_list.reserve(children.size());
  for (const auto& child : children) {
    auto child_value = child->to_json();
    if (!child_value.empty()) {
      children_list.emplace_back(std::move(child_value));
    }
  }
  if (!children_list.empty()) {
    map.emplace("children", std::move(children_list));
  }

  return map;
}

template <Trace T>
inline auto TraceTreeNode<T>::add_child(std::unique_ptr<TraceTreeNode> child)
    -> std::unique_ptr<TraceTreeNode>& {
  child->parent_ = this;
  return children_.emplace_back(std::move(child));
}

template <Trace T>
inline Traced<T>::Traced(Level trace_level, T root)
    : trace_level_(trace_level),
      active_traces_({root}),
      trace_tree_root_(std::make_unique<TraceTreeNode<T>>(std::move(root))),
      trace_tree_current_(trace_tree_root_.get()) {}

template <Trace T>
inline Traced<T>::~Traced() = default;

template <Trace T>
[[nodiscard]] inline auto Traced<T>::get_trace_level() const -> Level {
  return trace_level_;
}

template <Trace T>
[[nodiscard]] inline auto Traced<T>::make_trace_stack(bool fatal) const -> TraceStack<T> {
  return {.stack = active_traces_, .fatal = fatal};
}

template <Trace T>
[[nodiscard]] inline auto Traced<T>::get_trace_tree() const -> const TraceTreeNode<T>* {
  if (get_trace_level() < Level::FULL) {
    panic("Traced::get_trace_tree requires `Level::FULL`");
  }

  return trace_tree_root_.get();
}

template <Trace T>
inline auto Traced<T>::attach_tag(std::string_view key, json::Value value) -> void {
  if (get_trace_level() < Level::FULL) {
    panic("Traced::attach_tag requires `Level::FULL`");
  }

  trace_tree_current_->tags.emplace(key, std::move(value));
}

template <Trace T>
inline auto Traced<T>::get_current_trace() const -> const T& {
  if (get_trace_level() < Level::STACK_ONLY) {
    panic("Traced::get_current_trace requires `Level::STACK_ONLY`");
  }

  return active_traces_.back();
}

template <Trace T>
inline auto Traced<T>::set_current_trace(T trace) -> void {
  if (get_trace_level() < Level::STACK_ONLY) {
    panic("Traced::set_current_trace requires `Level::STACK_ONLY`");
  }

  active_traces_.back() = trace;

  if (get_trace_level() < Level::FULL) {
    return;
  }
  trace_tree_current_->trace = std::move(trace);
}

template <Trace T>
inline auto Traced<T>::push_trace(T trace) -> void {
  if (trace_level_ < Level::STACK_ONLY) {
    return;
  }

  active_traces_.emplace_back(trace);

  if (trace_level_ < Level::FULL) {
    return;
  }

  auto& child =
      trace_tree_current_->add_child(std::make_unique<trace::TraceTreeNode<T>>(std::move(trace)));
  trace_tree_current_ = child.get();
}

template <Trace T>
inline auto Traced<T>::pop_trace() -> void {
  if (trace_level_ < Level::STACK_ONLY) {
    return;
  }

  if (active_traces_.size() <= 1) {
    panic("Pop trace failed: cannot pop the root element");
  }
  active_traces_.pop_back();

  if (trace_level_ < Level::FULL) {
    return;
  }

  trace_tree_current_ = trace_tree_current_->get_parent();
}

}  // namespace cplib::trace
