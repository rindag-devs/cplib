/**
 * Float64 A+B Problem, with multiple testcases.
 *
 * If the minimum value of relative error and absolute error does not exceed 10^(-6),
 * it is considered correct.
 *
 * Run test with:
 *
 * ```bash
 * pnpm gulp test --kind checker --match "^1-f64-a-plus-b-multiple$"
 * ```
 */

#include <cstdint>
#include <vector>

#include "../../../cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_CHECKER(chk);

constexpr double MAX_ERR = 1e-6;

struct Output {
  std::vector<double> ans;

  static Output read(var::Reader& in, int32_t n) {
    auto ans = in.read(var::f64("ans") * n);
    return {ans};
  }

  static void check(const Output& expected, const Output& result, int32_t n) {
    for (int32_t i = 0; i < n; ++i) {
      if (!cplib::float_equals(expected.ans[i], result.ans[i], MAX_ERR)) {
        auto delta = cplib::float_delta(expected.ans[i], result.ans[i]);
        chk.quit_wa(format("ans[%d] error, expected %f, got %f, delta %f", i, expected.ans[i],
                           result.ans[i], delta));
      }
    }
  }
};

void checker_main() {
  auto n = chk.inf.read(var::i32("n"));

  auto output = var::ExtVar<Output>("output", n);
  auto ouf_output = chk.ouf.read(output);
  auto ans_output = chk.ans.read(output);

  Output::check(ans_output, ouf_output, n);
  chk.quit_ac();
}
