#include <testlib.h>

auto main(int argc, char** argv) -> int {
  registerTestlibCmd(argc, argv);
  auto n = inf.readInt(1, 1e7, "n");
  auto a = ouf.readTokens(n, "[a-z]*", "a");
  auto b = ans.readTokens(n, "[a-z]*", "a");
  if (a != b) quit(_wa, "");
  quit(_ok, "");
}
