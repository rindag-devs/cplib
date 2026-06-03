#include <cmath>

#include "../test_utils.hpp"
#include "cplib.hpp"
#include "gtest/gtest.h"

using cplib_test::make_test_reader;
using cplib_test::TestExitException;

TEST(VarFloatTest, BasicFloat) {
  auto reader = make_test_reader("3.14159 -2e-3");

  auto f1 = reader.read(cplib::var::f64("f1"));
  EXPECT_NEAR(f1, 3.14159, 1e-9);

  auto f2 = reader.read(cplib::var::f64("f2"));
  EXPECT_NEAR(f2, -0.002, 1e-9);
}

TEST(VarFloatTest, FloatRange) {
  auto reader = make_test_reader("1.5");
  EXPECT_THROW(reader.read(cplib::var::f64("f", 0.0, 1.0)), TestExitException);
}

TEST(VarFloatTest, LongDoubleGeneral) {
  auto reader = make_test_reader("3.141592653589793238462643383279502884 -2e-3 inf nan");

  auto pi = reader.read(cplib::var::fext("pi"));
  EXPECT_LT(std::abs(pi - 3.141592653589793238462643383279502884L), 1e-18L);

  auto small = reader.read(cplib::var::fext("small"));
  EXPECT_LT(std::abs(small + 0.002L), 1e-18L);

  auto inf = reader.read(cplib::var::fext("inf"));
  EXPECT_TRUE(std::isinf(inf));

  auto nan = reader.read(cplib::var::fext("nan"));
  EXPECT_TRUE(std::isnan(nan));
}

TEST(VarStrictFloatTest, DecimalPlaces) {
  // strict format: 2 decimal places required
  // Input: 1.23 (ok), 1.2 (fail), 1.234 (fail)

  {
    auto reader = make_test_reader("1.23");
    double val = reader.read(cplib::var::f64s("v", -100, 100, 2, 2));
    EXPECT_NEAR(val, 1.23, 1e-9);
  }
  {
    auto reader = make_test_reader("1.2");
    EXPECT_THROW(reader.read(cplib::var::f64s("v", -100, 100, 2, 2)), TestExitException);
  }
  {
    auto reader = make_test_reader("1.234");
    EXPECT_THROW(reader.read(cplib::var::f64s("v", -100, 100, 2, 2)), TestExitException);
  }
}

TEST(VarStrictFloatTest, ScientificNotationRejection) {
  auto reader = make_test_reader("1e2");
  // Strict float usually rejects scientific notation if parsing uses chars_format::fixed
  EXPECT_THROW(reader.read(cplib::var::f64s("v", 0, 200, 0, 10)), TestExitException);
}

TEST(VarStrictFloatTest, LongDoubleFixed) {
  auto reader = make_test_reader("1.2500");
  auto value = reader.read(cplib::var::fexts("v", 0.0L, 2.0L, 4, 4));
  EXPECT_LT(std::abs(value - 1.25L), 1e-18L);
}

TEST(VarStrictFloatTest, LongDoubleRejectsExponent) {
  auto reader = make_test_reader("1e2");
  EXPECT_THROW(reader.read(cplib::var::fexts("v", 0.0L, 200.0L, 0, 10)), TestExitException);
}
