#include <memory>
#include <sstream>
#include <string>

#include "cplib.hpp"
#include "gtest/gtest.h"
#include "test_utils.hpp"

using cplib::io::InStream;
using cplib_test::make_test_reader;
using cplib_test::TestExitException;

TEST(IoTest, StrictModeWhitespace) {
  std::string content = "   token";
  auto buf = std::make_unique<std::stringbuf>(content);
  // Strict = true
  InStream stream(std::make_unique<std::stringbuf>(content), "test", true);
  // In strict mode, if first char is space, returns empty string.
  EXPECT_EQ(stream.read_token(), "");
}

TEST(IoTest, NonStrictSkipping) {
  std::string content = "   token";
  InStream stream(std::make_unique<std::stringbuf>(content), "test", false);
  EXPECT_EQ(stream.read_token(), "token");
}

TEST(IoTest, PositionTracking) {
  // Line 0: 012\n
  // Line 1: 45
  std::string content = "abc\nde";
  InStream stream(std::make_unique<std::stringbuf>(content), "test", false);

  EXPECT_EQ(stream.read(), 'a');  // byte 1, col 1
  EXPECT_EQ(stream.pos().line, 0);
  EXPECT_EQ(stream.pos().col, 1);

  stream.read();  // b
  stream.read();  // c
  stream.read();  // \n

  EXPECT_EQ(stream.pos().line, 1);
  EXPECT_EQ(stream.pos().col, 0);

  EXPECT_EQ(stream.read(), 'd');
}

TEST(IoTest, EofCheck) {
  InStream stream(std::make_unique<std::stringbuf>(""), "test", false);
  EXPECT_TRUE(stream.eof());
  EXPECT_TRUE(stream.seek_eof());
}
