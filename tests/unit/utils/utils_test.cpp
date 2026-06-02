#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cplib.hpp"
#include "gtest/gtest.h"

TEST(UtilsTest, Split) {
  auto parts = cplib::split("a,b,c", ',');
  ASSERT_EQ(parts.size(), 3);
  EXPECT_EQ(parts[0], "a");
  EXPECT_EQ(parts[1], "b");
  EXPECT_EQ(parts[2], "c");
}

TEST(UtilsTest, Tokenize) {
  // Tokenize ignores empty strings between separators
  auto parts = cplib::tokenize(",a,,b,", ',');
  ASSERT_EQ(parts.size(), 2);
  EXPECT_EQ(parts[0], "a");
  EXPECT_EQ(parts[1], "b");
}

TEST(UtilsTest, Join) {
  std::vector<std::string> v = {"x", "y", "z"};
  EXPECT_EQ(cplib::join(v.begin(), v.end(), "-"), "x-y-z");
}

TEST(UtilsTest, Trim) {
  EXPECT_EQ(cplib::trim("  abc  "), "abc");
  EXPECT_EQ(cplib::trim("abc"), "abc");
  EXPECT_EQ(cplib::trim("   "), "");
}

TEST(UtilsTest, FloatEquals) {
  EXPECT_TRUE(cplib::float_equals(1.0, 1.000000001, 1e-8));
  EXPECT_FALSE(cplib::float_equals(1.0, 1.1, 1e-8));
}

TEST(UtilsTest, UniqueFunctionPanicsWhenEmpty) {
  auto old_handler = std::move(cplib::detail::panic_impl);
  cplib::detail::panic_impl = [](std::string_view message) -> void {
    throw std::runtime_error(std::string(message));
  };

  cplib::UniqueFunction<auto()->int> fn(nullptr);
  EXPECT_FALSE(static_cast<bool>(fn));
  EXPECT_THROW(static_cast<void>(fn()), std::runtime_error);

  cplib::detail::panic_impl = std::move(old_handler);
}
