#include <testlib.h>

auto main(int argc, char** argv) -> int {
  registerTestlibCmd(argc, argv);
  auto n = inf.readInt(1, 1e7, "n");
  auto a = ouf.readInts(n, 1, 1e9, "a");
  auto b = ans.readInts(n, 1, 1e9, "a");
  if (a != b) quit(_wa, "");
  quit(_ok, "");
}
