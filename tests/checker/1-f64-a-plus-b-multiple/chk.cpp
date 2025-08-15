/**
 * Float64 A+B Problem, with multiple testcases.
 *
 * If the minimum value of relative error and absolute error does not exceed 10^(-6),
 * it is considered correct.
 *
 * Run test with:
 *
 * ```bash
 * pnpm test -- --kind checker --match "^1-f64-a-plus-b-multiple$"
 * ```
 */

#include <string>
#include <vector>

#include "cplib.hpp"

using namespace cplib;

constexpr double MAX_ERR = 1e-6;

struct Input {
  int n;

  static Input read(var::Reader& in) {
    int n = in.read(var::i32("n"));
    return {n};
  }
};

struct Output {
  std::vector<double> ans;

  static Output read(var::Reader& in, Input inp) {
    auto ans = in.read(var::f64("ans") * inp.n);
    return {ans};
  }

  static evaluate::Result evaluate(evaluate::Evaluator& ev, const Output& pans, const Output& jans,
                                   Input inp) {
    auto res = evaluate::Result::ac();
    for (int i = 0; i < inp.n; ++i) {
      res &= ev.approx(std::to_string(i), pans.ans[i], jans.ans[i], MAX_ERR);
    }
    return res;
  }
};

CPLIB_REGISTER_CHECKER(Input, Output);
