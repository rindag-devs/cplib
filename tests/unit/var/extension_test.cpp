#include <vector>

#include "cplib.hpp"
#include "gtest/gtest.h"
#include "test_utils.hpp"

using cplib_test::make_test_reader;

struct CustomPoint {
  int x, y;

  // Context-aware read: takes bounds for x and y
  static auto read(cplib::var::Reader& in, int max_val) -> CustomPoint {
    auto x = in.read(cplib::var::i32("x", -max_val, max_val));
    auto y = in.read(cplib::var::i32("y", -max_val, max_val));
    return {.x = x, .y = y};
  }
};

TEST(VarExtensionTest, ExtVar) {
  auto reader = make_test_reader("10 -5");
  // Pass context 100
  auto p = reader.read(cplib::var::ExtVar<CustomPoint>("p", 100));
  EXPECT_EQ(p.x, 10);
  EXPECT_EQ(p.y, -5);
}

TEST(VarExtensionTest, ExtVec) {
  auto reader = make_test_reader("1 1 2 2 3 3");
  std::vector<int> bounds = {10, 10, 10};  // Range driving the vector loop

  // Reads a CustomPoint for each element in bounds, passing the bound value to read()
  // Note: ExtVec passes the range element as the LAST argument to T::read.
  auto points = reader.read(cplib::var::ExtVec<CustomPoint>("points", bounds, cplib::var::space));

  ASSERT_EQ(points.size(), 3);
  EXPECT_EQ(points[0].x, 1);
  EXPECT_EQ(points[2].x, 3);
}

TEST(VarExtensionTest, FnVar) {
  auto reader = make_test_reader("42");

  auto custom_logic = [](cplib::var::Reader& in, int multiplier) {
    int val = in.read(cplib::var::i32());
    return val * multiplier;
  };

  int result = reader.read(cplib::var::FnVar<int(cplib::var::Reader&, int)>("f", custom_logic, 2));
  EXPECT_EQ(result, 84);
}
