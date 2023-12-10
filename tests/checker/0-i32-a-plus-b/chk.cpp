/**
 * Int32 A+B Problem.
 */

#include <cstdint>
#include <vector>

#include "../../../cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_CHECKER(chk);

auto checker_main() -> void {
  auto var_ans = var::i32("ans");
  int ouf_output = chk.ouf.read(var_ans);
  int ans_output = chk.ans.read(var_ans);

  if (ouf_output != ans_output) {
    chk.quit_wa(format("Expected %d, got %d", ans_output, ouf_output));
  }
  chk.quit_ac();
}
