#include "../../cplib.hpp"

CPLIB_REGISTER_CHECKER(chk);

using namespace cplib;

auto checker_main() -> void {
  chk.inf.read(var::i32(1, 1e7, "n"));
  auto a = chk.ouf.read(var::String(Pattern("[a-z]*"), "a"));
  auto b = chk.ans.read(var::String(Pattern("[a-z]*"), "a"));
  if (a != b) chk.quit_wa("");
  chk.quit_ac();
}
