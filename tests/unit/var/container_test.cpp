#include <tuple>
#include <utility>
#include <vector>

#include "cplib.hpp"
#include "gtest/gtest.h"
#include "test_utils.hpp"

using cplib_test::make_test_reader;
using cplib_test::TestExitException;

TEST(VarContainerTest, Vec) {
  auto reader = make_test_reader("3 10 20 30");
  auto n = reader.read(cplib::var::i32("n"));
  auto v = reader.read(cplib::var::Vec(cplib::var::i32("v"), n));

  ASSERT_EQ(v.size(), 3);
  EXPECT_EQ(v, std::vector<int>({10, 20, 30}));
}

TEST(VarContainerTest, VecWithSeparator) {
  // Strict mode usually requires exact separators.
  // Let's test standard space separated logic first.
  auto reader = make_test_reader("1,2,3");
  // Note: Vec consumes separator *between* elements.
  // We need to handle the comma.
  // If we assume non-strict stream, spaces are skipped, but commas must be explicitly read if
  // specified.
  auto v = reader.read(cplib::var::Vec(cplib::var::i32("v"), 3, cplib::var::Separator(',')));
  EXPECT_EQ(v, std::vector<int>({1, 2, 3}));
}

TEST(VarContainerTest, Mat) {
  // 2x2 matrix
  auto reader = make_test_reader("1 2 3 4");
  auto m = reader.read(cplib::var::Mat(cplib::var::i32("m"), 2, 2));

  ASSERT_EQ(m.size(), 2);
  ASSERT_EQ(m[0].size(), 2);
  EXPECT_EQ(m[0][0], 1);
  EXPECT_EQ(m[0][1], 2);
  EXPECT_EQ(m[1][0], 3);
  EXPECT_EQ(m[1][1], 4);
}

TEST(VarContainerTest, Pair) {
  auto reader = make_test_reader("10 hello");
  auto p =
      reader.read(cplib::var::Pair(std::make_pair(cplib::var::i32("k"), cplib::var::String("v"))));
  EXPECT_EQ(p.first, 10);
  EXPECT_EQ(p.second, "hello");
}

TEST(VarContainerTest, Tuple) {
  auto reader = make_test_reader("1 3.14 str");
  auto t = reader.read(cplib::var::Tuple(
      std::make_tuple(cplib::var::i32(), cplib::var::f64(), cplib::var::String())));
  EXPECT_EQ(std::get<0>(t), 1);
  EXPECT_NEAR(std::get<1>(t), 3.14, 1e-9);
  EXPECT_EQ(std::get<2>(t), "str");
}
