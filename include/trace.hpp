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

#ifndef CPLIB_TRACE_HPP_
#define CPLIB_TRACE_HPP_

#include <concepts>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

/* cplib_embed_ignore start */
#include "json.hpp"
/* cplib_embed_ignore end */

namespace cplib::trace {

/**
 * Trace level.
 */
enum struct Level : std::uint8_t {
  NONE,        /// Do not trace.
  STACK_ONLY,  /// Enable trace stack only.
  FULL,        /// Trace the whole input stream. Enable trace stack and trace tree.
};

template <typename T>
concept Trace = requires(const T& t) {
  { t.node_name() } -> std::same_as<std::string>;
  { t.to_plain_text() } -> std::same_as<std::string>;
  { t.to_colored_text() } -> std::same_as<std::string>;
  { t.to_stack_json() } -> std::same_as<json::Value>;
  { t.to_tree_json() } -> std::same_as<json::Value>;
} && std::copyable<T>;

/**
 * `TraceStack` represents a stack of trace.
 */
template <Trace T>
struct TraceStack {
  std::vector<T> stack;
  bool fatal;

  /// Convert to JSON map.
  [[nodiscard]] auto to_json() const -> json::Value;

  /// Convert to human-friendly plain text.
  /// Each line does not contain the trailing `\n` character.
  [[nodiscard]] auto to_plain_text_lines() const -> std::vector<std::string>;

  /// Convert to human-friendly colored text (ANSI escape color).
  /// Each line does not contain the trailing `\n` character.
  [[nodiscard]] auto to_colored_text_lines() const -> std::vector<std::string>;

  /// Convert to human-friendly plain text, using compact (one line) style.
  [[nodiscard]] auto to_plain_text_compact() const -> std::string;

  /// Convert to human-friendly colored text (ANSI escape color), using compact (one line) style.
  [[nodiscard]] auto to_colored_text_compact() const -> std::string;
};

/**
 * `TraceTreeNode` represents a node of trace tree.
 */
template <Trace T>
struct TraceTreeNode {
 public:
  T trace;
  json::Map tags{};

  /**
   * Create a TraceTreeNode with trace.
   *
   * @param trace The trace of the node.
   */
  explicit TraceTreeNode(T trace);

  /// Copy constructor (deleted to prevent copying).
  TraceTreeNode(const TraceTreeNode&) = delete;

  /// Copy assignment operator (deleted to prevent copying).
  auto operator=(const TraceTreeNode&) -> TraceTreeNode& = delete;

  /// Move constructor.
  TraceTreeNode(TraceTreeNode&&) noexcept = default;

  /// Move assignment operator.
  auto operator=(TraceTreeNode&&) noexcept -> TraceTreeNode& = default;

  /**
   * Get the children of the node.
   *
   * @return The children of the node.
   */
  [[nodiscard]] auto get_children() const -> const std::vector<std::unique_ptr<TraceTreeNode>>&;

  /**
   * Get the parent of the node.
   *
   * @return The parent of the node.
   */
  [[nodiscard]] auto get_parent() -> TraceTreeNode*;

  /**
   * Convert to JSON value.
   *
   * If node has tag `#hidden`, return `nullptr`.
   *
   * @return The JSON value or nullptr.
   */
  [[nodiscard]] auto to_json() const -> json::Map;

  /**
   * Convert a node to its child and return it again (as reference).
   *
   * @param child The child node.
   * @return The child node.
   */
  auto add_child(std::unique_ptr<TraceTreeNode> child) -> std::unique_ptr<TraceTreeNode>&;

 private:
  std::vector<std::unique_ptr<TraceTreeNode>> children_{};
  TraceTreeNode* parent_{nullptr};
};

/**
 * `Traced` represents a base class for objects that manage trace information.
 */
template <Trace T>
struct Traced {
 public:
  explicit Traced(Level trace_level, T root);
  virtual ~Traced() = 0;
  Traced(const Traced&) = delete;
  auto operator=(const Traced&) -> Traced& = delete;
  Traced(Traced&&) noexcept = default;
  auto operator=(Traced&&) noexcept -> Traced& = default;

  /**
   * Get the trace level.
   */
  [[nodiscard]] auto get_trace_level() const -> Level;

  /**
   * Make a trace stack from the current trace.
   *
   * Only available when `TraceLevel::STACK_ONLY` or higher is set.
   * Otherwise, an error will be panicked.
   */
  [[nodiscard]] auto make_trace_stack(bool fatal) const -> TraceStack<T>;

  /**
   * Get the trace tree.
   *
   * Only available when `TraceLevel::FULL` is set.
   * Otherwise, an error will be panicked.
   */
  [[nodiscard]] auto get_trace_tree() const -> const TraceTreeNode<T>*;

  /**
   * Attach a tag to the current trace.
   *
   * @param key The tag key.
   * @param value The tag value.
   */
  auto attach_tag(std::string_view key, json::Value value) -> void;

  /**
   * Get current trace.
   *
   * Only available when `TraceLevel::STACK_ONLY` or higher is set.
   * Otherwise, an error will be panicked.
   */
  auto get_current_trace() const -> const T&;

 protected:
  /**
   * Set current trace.
   *
   * Only available when `TraceLevel::STACK_ONLY` or higher is set.
   * Otherwise, an error will be panicked.
   */
  auto set_current_trace(T trace) -> void;

  /**
   * Push a new trace onto the stack/tree.
   *
   * @param t The trace to push.
   */
  auto push_trace(T trace) -> void;

  /**
   * Pop the current trace from the stack/tree.
   * This should be called when the scope of the trace ends.
   */
  auto pop_trace() -> void;

 private:
  trace::Level trace_level_;
  std::vector<T> active_traces_;
  std::unique_ptr<TraceTreeNode<T>> trace_tree_root_;
  TraceTreeNode<T>* trace_tree_current_;
};

}  // namespace cplib::trace

#include "trace.i.hpp"

#endif
