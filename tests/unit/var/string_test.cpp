#include "cplib.hpp"
#include "gtest/gtest.h"
#include "test_utils.hpp"

using cplib::Pattern;
using cplib_test::make_test_reader;
using cplib_test::TestExitException;

TEST(VarStringTest, ReadToken) {
  auto reader = make_test_reader("hello world");
  EXPECT_EQ(reader.read(cplib::var::String("s1")), "hello");
  EXPECT_EQ(reader.read(cplib::var::String("s2")), "world");
}

TEST(VarStringTest, PatternMatching) {
  auto reader = make_test_reader("abc 123");
  // Matches [a-z]+
  EXPECT_EQ(reader.read(cplib::var::String("s1", Pattern("[a-z]+"))), "abc");

  // 123 does not match [a-z]+
  EXPECT_THROW(reader.read(cplib::var::String("s2", Pattern("[a-z]+"))), TestExitException);
}

TEST(VarStringTest, ReadLine) {
  auto reader = make_test_reader("Line 1\nLine 2");
  EXPECT_EQ(reader.read(cplib::var::String(cplib::var::String::Mode::LINE)), "Line 1");
  EXPECT_EQ(reader.read(cplib::var::String(cplib::var::String::Mode::LINE)), "Line 2");
}

TEST(VarStringTest, ReadWord) {
  auto reader = make_test_reader("word1|WORD2", true);  // strict
  EXPECT_EQ(reader.read(cplib::var::String(cplib::var::String::Mode::WORD)), "word1");
  reader.read(cplib::var::Separator("sep", '|'));
  EXPECT_EQ(reader.read(cplib::var::String(cplib::var::String::Mode::WORD)), "WORD2");
}

TEST(VarYesNoTest, CaseInsensitive) {
  auto reader = make_test_reader("Yes no YES NO yEs");
  EXPECT_TRUE(reader.read(cplib::var::YesNo()));
  EXPECT_FALSE(reader.read(cplib::var::YesNo()));
  EXPECT_TRUE(reader.read(cplib::var::YesNo()));
  EXPECT_FALSE(reader.read(cplib::var::YesNo()));
  EXPECT_TRUE(reader.read(cplib::var::YesNo()));
}

TEST(VarYesNoTest, InvalidInput) {
  auto reader = make_test_reader("Yeah");
  EXPECT_THROW(reader.read(cplib::var::YesNo()), TestExitException);
}

TEST(VarSeparatorTest, ExplicitCheck) {
  // Strict mode: requires exact character matches
  auto reader = make_test_reader("1,2", /*strict=*/true);
  EXPECT_EQ(reader.read(cplib::var::i32()), 1);
  // Reads comma
  reader.read(cplib::var::Separator("sep", ','));
  EXPECT_EQ(reader.read(cplib::var::i32()), 2);
}

TEST(VarSeparatorTest, WhitespaceSkipNonStrict) {
  // Non-strict mode skips whitespace automatically usually, but Separator checks specific char
  auto reader = make_test_reader("1 ; 2");
  reader.read(cplib::var::i32());
  // Should skip space and find ';'
  reader.read(cplib::var::Separator("sep", ';'));
  EXPECT_EQ(reader.read(cplib::var::i32()), 2);
}
