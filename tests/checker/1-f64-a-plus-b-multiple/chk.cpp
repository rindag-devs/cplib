/**
 * Float64 A+B Problem, with multiple testcases.
 *
 * If the minimum value of relative error and absolute error does not exceed 10^(-6),
 * it is considered correct.
 *
 * Run test with:
 *
 * ```bash
 * cmake --build build --parallel
 * ctest --test-dir build --output-on-failure --parallel -R '^checker\.1-f64-a-plus-b-multiple'
 * ```
 */

#include <string>
#include <vector>

#include "cplib.hpp"

constexpr double MAX_ERR = 1e-6;

struct Input {
  int n;

  static auto read(cplib::var::Reader &in) -> Input {
    int n = in.read(cplib::var::i32("n"));
    return {.n = n};
  }
};

struct Output {
  std::vector<double> ans;

  static auto read(cplib::var::Reader &in, Input inp) -> Output {
    auto ans = in.read(cplib::var::f64("ans") * inp.n);
    return {.ans = ans};
  }

  static auto evaluate(cplib::evaluate::Evaluator &ev, const Output &pans, const Output &jans,
                       Input inp) -> cplib::evaluate::Result {
    auto res = cplib::evaluate::Result::ac();
    for (int i = 0; i < inp.n; ++i) {
      res &= ev.approx(std::to_string(i), pans.ans[i], jans.ans[i], MAX_ERR);
    }
    return res;
  }
};

CPLIB_REGISTER_CHECKER(Input, Output);
