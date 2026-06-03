#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "cplib.hpp"
#include "gtest/gtest.h"

namespace {
struct PanicRestorer {
  cplib::UniqueFunction<auto(std::string_view)->void> old_handler;

  PanicRestorer() : old_handler(std::move(cplib::detail::panic_impl)) {}

  ~PanicRestorer() { cplib::detail::panic_impl = std::move(old_handler); }
};
}  // namespace

TEST(PatternTest, MatchesBoundedStringView) {
  cplib::Pattern pattern("abc");
  const std::string source = std::string("abc\0suffix", 10);
  const std::string_view prefix(source.data(), 3);

  EXPECT_TRUE(pattern.match(prefix));
  EXPECT_FALSE(pattern.match(std::string_view(source.data(), 4)));
}

TEST(PatternTest, InvalidPatternPanics) {
  PanicRestorer restorer;
  cplib::detail::panic_impl = [](std::string_view message) -> void {
    throw std::runtime_error(std::string(message));
  };

  EXPECT_THROW(static_cast<void>(cplib::Pattern("(")), std::runtime_error);
}

TEST(PatternTest, CopyConstructedPattern) {
  cplib::Pattern original("[a-z]+");
  cplib::Pattern copy = original;
  copy = cplib::Pattern("[0-9]+");

  EXPECT_TRUE(original.match("abc"));
  EXPECT_TRUE(copy.match("123"));
  EXPECT_FALSE(copy.match("xyz"));
}

TEST(PatternTest, CopyAssignedPatternRecompilesRegex) {
  cplib::Pattern target("[0-9]+");
  cplib::Pattern source("[a-z]+");

  target = source;

  EXPECT_TRUE(target.match("hello"));
  EXPECT_FALSE(target.match("123"));
  EXPECT_EQ(target.src(), source.src());
}

TEST(PatternTest, MoveConstructedPatternStillMatches) {
  cplib::Pattern source("[a-z]+");
  cplib::Pattern moved(std::move(source));

  EXPECT_TRUE(moved.match("world"));
}
