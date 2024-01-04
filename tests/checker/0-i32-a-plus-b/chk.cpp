/**
 * Int32 A+B Problem.
 *
 * Run test with:
 *
 * ```bash
 * pnpm gulp test --kind checker --match "^0-i32-a-plus-b$"
 * ```
 */

#include "../../../cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_CHECKER(chk);

void checker_main() {
  auto var_ans = var::i32("ans");
  int ouf_output = chk.ouf.read(var_ans);
  int ans_output = chk.ans.read(var_ans);

  if (ouf_output != ans_output) {
    chk.quit_wa(format("Expected %d, got %d", ans_output, ouf_output));
  }
  chk.quit_ac();
}
