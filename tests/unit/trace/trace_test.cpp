#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cplib.hpp"
#include "gtest/gtest.h"

namespace {

struct PanicRestorer {
  cplib::UniqueFunction<auto(std::string_view)->void> old_handler;

  PanicRestorer() : old_handler(std::move(cplib::detail::panic_impl)) {}

  ~PanicRestorer() { cplib::detail::panic_impl = std::move(old_handler); }
};

struct SampleTrace {
  std::string name;
  int value;

  SampleTrace(std::string name, int value) : name(std::move(name)), value(value) {}

  [[nodiscard]] auto node_name() const -> std::string { return name; }

  [[nodiscard]] auto to_plain_text() const -> std::string {
    return name + "=" + std::to_string(value);
  }

  [[nodiscard]] auto to_colored_text() const -> std::string {
    return "\x1b[32m" + name + "=" + std::to_string(value) + "\x1b[0m";
  }

  [[nodiscard]] auto to_stack_json() const -> cplib::json::Value {
    cplib::json::Map map;
    map.emplace("name", cplib::json::Value(name));
    map.emplace("value", cplib::json::Value(static_cast<cplib::json::Int>(value)));
    return cplib::json::Value(map);
  }

  [[nodiscard]] auto to_tree_json() const -> cplib::json::Value { return to_stack_json(); }
};

struct TraceHarness : cplib::trace::Traced<SampleTrace> {
  using cplib::trace::Traced<SampleTrace>::pop_trace;
  using cplib::trace::Traced<SampleTrace>::push_trace;
  using cplib::trace::Traced<SampleTrace>::set_current_trace;

  explicit TraceHarness(cplib::trace::Level level)
      : cplib::trace::Traced<SampleTrace>(level, SampleTrace{"root", 0}) {}
};

auto install_throwing_panic_handler() -> void {
  cplib::detail::panic_impl = [](std::string_view message) -> void {
    throw std::runtime_error(std::string(message));
  };
}

auto require_json(std::optional<cplib::json::Raw> value) -> cplib::json::Raw {
  if (!value.has_value()) {
    throw std::runtime_error("expected visible trace JSON");
  }
  return std::move(value).value();
}

}  // namespace

TEST(TraceStackTest, SerializesFormats) {
  const cplib::trace::TraceStack<SampleTrace> non_fatal_stack{
      .stack = {SampleTrace{"root", 0}, SampleTrace{"child", 7}}, .fatal = false};

  EXPECT_EQ(non_fatal_stack.to_json().to_string(),
            R"({"fatal":false,"stack":[{"name":"root","value":0},{"name":"child","value":7}]})");
  EXPECT_EQ(non_fatal_stack.to_plain_text_lines(),
            (std::vector<std::string>{"#0: root=0", "#1: child=7"}));
  EXPECT_EQ(non_fatal_stack.to_colored_text_lines(),
            (std::vector<std::string>{"#0: \x1b[32mroot=0\x1b[0m", "#1: \x1b[32mchild=7\x1b[0m"}));
  EXPECT_EQ(non_fatal_stack.to_plain_text_compact(), "root/child=7");
  EXPECT_EQ(non_fatal_stack.to_colored_text_compact(),
            "root\x1b[0;90m/\x1b[0m\x1b[32mchild=7\x1b[0m");

  const cplib::trace::TraceStack<SampleTrace> fatal_stack{
      .stack = {SampleTrace{"root", 0}, SampleTrace{"child", 7}}, .fatal = true};

  EXPECT_EQ(fatal_stack.to_plain_text_lines(),
            (std::vector<std::string>{"[fatal]", "#0: root=0", "#1: child=7"}));
  EXPECT_EQ(fatal_stack.to_colored_text_lines(),
            (std::vector<std::string>{"\x1b[0;31m[fatal]\x1b[0m", "#0: \x1b[32mroot=0\x1b[0m",
                                      "#1: \x1b[32mchild=7\x1b[0m"}));
  EXPECT_EQ(fatal_stack.to_plain_text_compact(), "[fatal] root/child=7");
  EXPECT_EQ(fatal_stack.to_colored_text_compact(),
            "\x1b[0;31m[fatal]\x1b[0m root\x1b[0;90m/\x1b[0m\x1b[32mchild=7\x1b[0m");
}

TEST(TraceStackTest, EmptyStackFormats) {
  const cplib::trace::TraceStack<SampleTrace> stack{.stack = {}, .fatal = true};

  EXPECT_EQ(stack.to_json().to_string(), R"({"fatal":true,"stack":[]})");
  EXPECT_EQ(stack.to_plain_text_lines(), (std::vector<std::string>{"[fatal]"}));
  EXPECT_EQ(stack.to_colored_text_lines(), (std::vector<std::string>{"\x1b[0;31m[fatal]\x1b[0m"}));
  EXPECT_EQ(stack.to_plain_text_compact(), "[fatal] ");
  EXPECT_EQ(stack.to_colored_text_compact(), "\x1b[0;31m[fatal]\x1b[0m ");
}

TEST(TraceTreeNodeTest, ParentLinksAndJson) {
  cplib::trace::TraceTreeNode<SampleTrace> root(SampleTrace{"root", 0});
  root.tags.emplace("phase", cplib::json::Value("setup"));

  auto &left = root.add_child(
      std::make_unique<cplib::trace::TraceTreeNode<SampleTrace>>(SampleTrace{"left", 1}));
  auto *left_node = left.get();
  auto &right = root.add_child(
      std::make_unique<cplib::trace::TraceTreeNode<SampleTrace>>(SampleTrace{"right", 2}));
  auto *right_node = right.get();
  left_node->add_child(
      std::make_unique<cplib::trace::TraceTreeNode<SampleTrace>>(SampleTrace{"leaf", 3}));
  right_node->tags.emplace("#hidden", cplib::json::Value(true));

  ASSERT_EQ(root.get_children().size(), 2);
  EXPECT_EQ(left_node->get_parent(), &root);
  EXPECT_EQ(right_node->get_parent(), &root);
  ASSERT_EQ(left_node->get_children().size(), 1);
  EXPECT_EQ(left_node->get_children().front()->get_parent(), left_node);

  const auto raw_json = require_json(root.to_json());
  EXPECT_EQ(
      cplib::json::Value(raw_json).to_string(),
      R"({"trace":{"name":"root","value":0},"tags":{"phase":"setup"},"children":[{"trace":{"name":"left","value":1},"children":[{"trace":{"name":"leaf","value":3}}]}]})");
}

TEST(TraceTreeNodeTest, HiddenNodes) {
  cplib::trace::TraceTreeNode<SampleTrace> root(SampleTrace{"root", 0});
  root.add_child(
          std::make_unique<cplib::trace::TraceTreeNode<SampleTrace>>(SampleTrace{"hidden", 1}))
      ->tags.emplace("#hidden", cplib::json::Value(true));

  const auto visible_raw_json = require_json(root.to_json());
  EXPECT_EQ(cplib::json::Value(visible_raw_json).to_string(),
            R"({"trace":{"name":"root","value":0}})");

  root.tags.emplace("#hidden", cplib::json::Value(true));
  EXPECT_FALSE(root.to_json().has_value());
}

TEST(TracedTest, NoneLevelRejectsAccess) {
  PanicRestorer restorer;
  install_throwing_panic_handler();
  TraceHarness traced(cplib::trace::Level::NONE);

  EXPECT_EQ(traced.get_trace_level(), cplib::trace::Level::NONE);
  traced.push_trace(SampleTrace{"ignored", 1});
  traced.pop_trace();

  const auto stack = traced.make_trace_stack(false);
  ASSERT_EQ(stack.stack.size(), 1);
  EXPECT_EQ(stack.stack.front().name, "root");
  EXPECT_FALSE(stack.fatal);

  EXPECT_THROW(static_cast<void>(traced.get_current_trace()), std::runtime_error);
  EXPECT_THROW(traced.set_current_trace(SampleTrace{"new-root", 9}), std::runtime_error);
  EXPECT_THROW(static_cast<void>(traced.get_trace_tree()), std::runtime_error);
  EXPECT_THROW(traced.attach_tag("tag", cplib::json::Value(true)), std::runtime_error);
}

TEST(TracedTest, StackOnlyNoTree) {
  PanicRestorer restorer;
  install_throwing_panic_handler();
  TraceHarness traced(cplib::trace::Level::STACK_ONLY);

  EXPECT_EQ(traced.get_current_trace().name, "root");
  traced.set_current_trace(SampleTrace{"renamed-root", 10});
  EXPECT_EQ(traced.get_current_trace().value, 10);

  traced.push_trace(SampleTrace{"child", 20});
  traced.push_trace(SampleTrace{"grandchild", 30});
  EXPECT_EQ(traced.get_current_trace().name, "grandchild");

  const auto stack = traced.make_trace_stack(true);
  ASSERT_EQ(stack.stack.size(), 3);
  EXPECT_TRUE(stack.fatal);
  EXPECT_EQ(stack.stack[0].name, "renamed-root");
  EXPECT_EQ(stack.stack[1].name, "child");
  EXPECT_EQ(stack.stack[2].name, "grandchild");

  traced.pop_trace();
  EXPECT_EQ(traced.get_current_trace().name, "child");
  traced.pop_trace();
  EXPECT_EQ(traced.get_current_trace().name, "renamed-root");
  EXPECT_THROW(traced.pop_trace(), std::runtime_error);
  EXPECT_THROW(static_cast<void>(traced.get_trace_tree()), std::runtime_error);
  EXPECT_THROW(traced.attach_tag("tag", cplib::json::Value(true)), std::runtime_error);
}

TEST(TracedTest, FullLevelTreeAndTags) {
  PanicRestorer restorer;
  install_throwing_panic_handler();
  TraceHarness traced(cplib::trace::Level::FULL);

  traced.attach_tag("stage", cplib::json::Value("root"));
  traced.push_trace(SampleTrace{"child", 1});
  traced.attach_tag("stage", cplib::json::Value("child"));
  traced.push_trace(SampleTrace{"grandchild", 2});
  traced.set_current_trace(SampleTrace{"renamed-grandchild", 3});
  traced.attach_tag("deep", cplib::json::Value(true));
  traced.pop_trace();
  traced.push_trace(SampleTrace{"sibling", 4});
  traced.attach_tag("#hidden", cplib::json::Value(true));

  const auto stack = traced.make_trace_stack(false);
  ASSERT_EQ(stack.stack.size(), 3);
  EXPECT_EQ(stack.stack[0].name, "root");
  EXPECT_EQ(stack.stack[1].name, "child");
  EXPECT_EQ(stack.stack[2].name, "sibling");

  const auto *tree = traced.get_trace_tree();
  ASSERT_NE(tree, nullptr);
  ASSERT_EQ(tree->get_children().size(), 1);
  ASSERT_EQ(tree->get_children().front()->get_children().size(), 2);
  EXPECT_EQ(tree->get_children().front()->get_children().back()->trace.name, "sibling");

  const auto raw_json = require_json(tree->to_json());
  EXPECT_EQ(
      cplib::json::Value(raw_json).to_string(),
      R"({"trace":{"name":"root","value":0},"tags":{"stage":"root"},"children":[{"trace":{"name":"child","value":1},"tags":{"stage":"child"},"children":[{"trace":{"name":"renamed-grandchild","value":3},"tags":{"deep":true}}]}]})");
}

TEST(TracedTest, MoveKeepsTree) {
  TraceHarness source(cplib::trace::Level::FULL);
  source.push_trace(SampleTrace{"child", 1});

  TraceHarness moved(std::move(source));

  EXPECT_EQ(moved.get_current_trace().name, "child");
  const auto *tree = moved.get_trace_tree();
  ASSERT_NE(tree, nullptr);
  ASSERT_EQ(tree->get_children().size(), 1);
  EXPECT_EQ(tree->get_children().front()->trace.name, "child");
}
