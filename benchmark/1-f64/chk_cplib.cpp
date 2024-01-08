#include "cplib.hpp"

CPLIB_REGISTER_CHECKER(chk);

using namespace cplib;

auto checker_main() -> void {
  auto n = chk.inf.read(var::i32(1, 1e7, "n"));
  auto a = chk.ouf.read(var::f64(1, 1e9, "a") * n);
  auto b = chk.ans.read(var::f64(1, 1e9, "a") * n);
  if (a != b) chk.quit_wa("");
  chk.quit_ac();
}
