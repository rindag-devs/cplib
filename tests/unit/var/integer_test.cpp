#include <string>

#include "../test_utils.hpp"
#include "cplib.hpp"
#include "gtest/gtest.h"

using cplib_test::make_test_reader;
using cplib_test::TestExitException;

TEST(VarIntTest, ReadsStandardTypes) {
  auto reader = make_test_reader("127 -128 255 12345 -32768");

  EXPECT_EQ(reader.read(cplib::var::i8("v1")), 127);
  EXPECT_EQ(reader.read(cplib::var::i8("v2")), -128);
  EXPECT_EQ(reader.read(cplib::var::u8("v3")), 255);
  EXPECT_EQ(reader.read(cplib::var::i32("v4")), 12345);
  EXPECT_EQ(reader.read(cplib::var::i16("v5")), -32768);
}

TEST(VarIntTest, BoundsChecking) {
  auto reader = make_test_reader("10 5");
  // Expect [0, 10]
  EXPECT_EQ(reader.read(cplib::var::i32("v1", 0, 10)), 10);

  // 5 is out of range [6, 10]
  EXPECT_THROW(
      {
        try {
          reader.read(cplib::var::i32("v2", 6, 10));
        } catch (const TestExitException& e) {
          EXPECT_TRUE(std::string(e.what()).find(">= 6") != std::string::npos);
          throw;
        }
      },
      TestExitException);
}

TEST(VarIntTest, FormatErrors) {
  {
    auto reader = make_test_reader("abc");
    EXPECT_THROW(reader.read(cplib::var::i32("v")), TestExitException);
  }
  {
    auto reader = make_test_reader("12.5");
    // Should fail because .5 remains or isn't an integer
    EXPECT_THROW(reader.read(cplib::var::i32("v")), TestExitException);
  }
}

TEST(VarIntTest, TypeLimitOverflow) {
  // 256 fits in int, but not uint8
  auto reader = make_test_reader("256");
  EXPECT_THROW(reader.read(cplib::var::u8("v")), TestExitException);
}
