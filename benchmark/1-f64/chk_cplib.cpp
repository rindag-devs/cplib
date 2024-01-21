#include "cplib.hpp"

CPLIB_REGISTER_CHECKER(chk);

using namespace cplib;

auto checker_main() -> void {
  auto n = chk.inf.read(var::i32("n", 1, 1e7));
  auto a = chk.ouf.read(var::f64("a", 1, 1e9) * n);
  auto b = chk.ans.read(var::f64("a", 1, 1e9) * n);
  if (a != b) chk.quit_wa("");
  chk.quit_ac();
}
