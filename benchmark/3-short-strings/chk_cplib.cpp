#include "cplib.hpp"

CPLIB_REGISTER_CHECKER(chk);

using namespace cplib;

auto checker_main() -> void {
  auto n = chk.inf.read(var::i32("n", 1, 1e7));
  auto a = chk.ouf.read(var::String("a", Pattern("[a-z]*")) * n);
  auto b = chk.ans.read(var::String("a", Pattern("[a-z]*")) * n);
  if (a != b) chk.quit_wa("");
  chk.quit_ac();
}
