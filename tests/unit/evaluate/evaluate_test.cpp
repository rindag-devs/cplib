#include <compare>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "cplib.hpp"
#include "gtest/gtest.h"

// Test fixture for Result tests
class EvaluateResultTest : public ::testing::Test {
 protected:
  cplib::evaluate::Result res_ac_ = cplib::evaluate::Result::ac("Accepted");
  cplib::evaluate::Result res_wa_ = cplib::evaluate::Result::wa("Wrong");
  cplib::evaluate::Result res_pc_ = cplib::evaluate::Result::pc(0.5, "Partially");
  cplib::evaluate::Result res_zero_ = cplib::evaluate::Result::zero();
};

TEST_F(EvaluateResultTest, OperatorPlus) {
  auto sum1 = res_pc_ + res_wa_;
  EXPECT_EQ(sum1.status, cplib::evaluate::Result::Status::WRONG_ANSWER);  // Worst status
  EXPECT_DOUBLE_EQ(sum1.score, 0.5);                                      // 0.5 + 0.0
  EXPECT_EQ(sum1.message, "Partially\nWrong");

  auto sum2 = res_ac_ + res_pc_;
  EXPECT_EQ(sum2.status, cplib::evaluate::Result::Status::PARTIALLY_CORRECT);  // Worst status
  EXPECT_DOUBLE_EQ(sum2.score, 1.5);                                           // 1.0 + 0.5
  EXPECT_EQ(sum2.message, "Accepted\nPartially");
}

TEST_F(EvaluateResultTest, OperatorAnd) {
  auto conj1 = res_ac_ & res_pc_;
  EXPECT_EQ(conj1.status, cplib::evaluate::Result::Status::PARTIALLY_CORRECT);  // Worst status
  EXPECT_DOUBLE_EQ(conj1.score, 0.5);                                           // min(1.0, 0.5)
  EXPECT_EQ(conj1.message, "Accepted\nPartially");
}

TEST_F(EvaluateResultTest, OperatorMultiply) {
  auto scaled = res_pc_ * 0.5;
  EXPECT_EQ(scaled.status, cplib::evaluate::Result::Status::PARTIALLY_CORRECT);
  EXPECT_DOUBLE_EQ(scaled.score, 0.25);
  EXPECT_EQ(scaled.message, "Partially");
}

TEST_F(EvaluateResultTest, Comparison) {
  EXPECT_TRUE(res_wa_ < res_pc_);
  EXPECT_TRUE(res_pc_ < res_ac_);
  EXPECT_TRUE(cplib::evaluate::Result::pc(0.4, "") < cplib::evaluate::Result::pc(0.5, ""));
  EXPECT_EQ(res_ac_ <=> res_ac_, std::strong_ordering::equal);
}

TEST_F(EvaluateResultTest, MessageMergingWithNamedFlag) {
  auto parent_res = cplib::evaluate::Result::pc(0.8, "Parent");

  auto unnamed_child = cplib::evaluate::Result::wa("Child");
  unnamed_child.named = false;

  auto named_child = cplib::evaluate::Result::wa("Child");
  named_child.named = true;

  auto combined_unnamed = parent_res + unnamed_child;
  EXPECT_EQ(combined_unnamed.message, "Parent\nChild");

  auto combined_named = parent_res + named_child;
  EXPECT_EQ(combined_named.message, "Parent");  // Message from named child is NOT merged
}

// Test fixture for Evaluator tests
class EvaluatorTest : public ::testing::Test {
 protected:
  bool fail_hook_called_ = false;
  bool eval_hook_called_ = false;
  std::unique_ptr<cplib::evaluate::Evaluator> ev_;

  void SetUp() override {
    fail_hook_called_ = false;
    eval_hook_called_ = false;

    auto fail_func = [this](const cplib::evaluate::Evaluator&, std::string_view) -> void {
      fail_hook_called_ = true;
      // Throw to prevent std::exit in a test environment
      throw std::runtime_error("fail_func called");
    };

    auto eval_hook = [this](const cplib::evaluate::Evaluator&,
                            const cplib::evaluate::Result&) -> void { eval_hook_called_ = true; };

    ev_ = std::make_unique<cplib::evaluate::Evaluator>(cplib::trace::Level::NONE,
                                                       std::move(fail_func), std::move(eval_hook));
  }
};

TEST_F(EvaluatorTest, Eq) {
  auto res_ok = ev_->eq("val", 10, 10);
  EXPECT_EQ(res_ok.status, cplib::evaluate::Result::Status::ACCEPTED);
  EXPECT_FALSE(eval_hook_called_);

  auto res_fail = ev_->eq("val", 10, 11);
  EXPECT_EQ(res_fail.status, cplib::evaluate::Result::Status::WRONG_ANSWER);
  EXPECT_TRUE(eval_hook_called_);
  EXPECT_TRUE(res_fail.message.find("not equal") != std::string::npos);
}

TEST_F(EvaluatorTest, Approx) {
  auto res_ok = ev_->approx("val", 1.0, 1.0000001, 1e-6);
  EXPECT_EQ(res_ok.status, cplib::evaluate::Result::Status::ACCEPTED);
  EXPECT_FALSE(eval_hook_called_);

  auto res_fail = ev_->approx("val", 1.0, 1.0001, 1e-6);
  EXPECT_EQ(res_fail.status, cplib::evaluate::Result::Status::WRONG_ANSWER);
  EXPECT_TRUE(eval_hook_called_);
  EXPECT_TRUE(res_fail.message.find("not approximately equal") != std::string::npos);
}

// Custom struct to test the Evaluatable concept
struct Point {
  int x, y;
  auto operator<=>(const Point&) const = default;
};

struct PointSolution {
  Point p;
  static auto evaluate(cplib::evaluate::Evaluator& ev, const PointSolution& pans,
                       const PointSolution& jans) -> cplib::evaluate::Result {
    auto res = cplib::evaluate::Result::ac();
    res &= ev.eq("x", pans.p.x, jans.p.x);
    res &= ev.eq("y", pans.p.y, jans.p.y);
    return res;
  }
};

TEST_F(EvaluatorTest, CustomEvaluatable) {
  PointSolution pans{{.x = 10, .y = 20}};
  PointSolution jans{{.x = 10, .y = 20}};

  auto res_ok = (*ev_)("point_check", pans, jans);
  EXPECT_EQ(res_ok.status, cplib::evaluate::Result::Status::ACCEPTED);
  // The inner ev.eq calls are not final, so the outer hook shouldn't fire for them
  // unless the final result is non-AC.
  EXPECT_FALSE(eval_hook_called_);

  PointSolution wrong_pans{{.x = 10, .y = 21}};
  auto res_fail = (*ev_)("point_check", wrong_pans, jans);
  EXPECT_EQ(res_fail.status, cplib::evaluate::Result::Status::WRONG_ANSWER);
  EXPECT_TRUE(eval_hook_called_);
}

TEST_F(EvaluatorTest, FailHook) {
  EXPECT_THROW(ev_->fail("test fail"), std::runtime_error);
  EXPECT_TRUE(fail_hook_called_);
}
