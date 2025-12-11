#include "cplib.hpp"
#include "gtest/gtest.h"

using cplib::evaluate::Result;

class EvaluateResultTest : public ::testing::Test {
 protected:
  Result res_ac = Result::ac();
  Result res_wa = Result::wa("Wrong");
  Result res_pc = Result::pc(0.5, "Partially");
};

TEST_F(EvaluateResultTest, OperatorPlus) {
  Result sum = res_pc + res_wa;
  EXPECT_EQ(sum.status, Result::Status::WRONG_ANSWER);
  EXPECT_DOUBLE_EQ(sum.score, 0.5);
  EXPECT_EQ(sum.message, "Partially\nWrong");
}

TEST_F(EvaluateResultTest, OperatorAnd) {
  Result conjunction = res_ac & res_pc;
  EXPECT_EQ(conjunction.status, Result::Status::PARTIALLY_CORRECT);
  EXPECT_DOUBLE_EQ(conjunction.score, 0.5);
  EXPECT_EQ(conjunction.message, "Partially");
}

TEST_F(EvaluateResultTest, OperatorMultiply) {
  Result scaled = res_pc * 0.5;
  EXPECT_EQ(scaled.status, Result::Status::PARTIALLY_CORRECT);
  EXPECT_DOUBLE_EQ(scaled.score, 0.25);
}

TEST_F(EvaluateResultTest, Comparison) {
  EXPECT_TRUE(res_wa < res_pc);
  EXPECT_TRUE(res_pc < res_ac);
  EXPECT_TRUE(Result::pc(0.4, "") < Result::pc(0.5, ""));
}
