#include <testlib.h>

auto main(int argc, char** argv) -> int {
  registerTestlibCmd(argc, argv);
  inf.readInt(1, 1e7, "n");
  auto a = ouf.readToken("[a-z]*", "a");
  auto b = ans.readToken("[a-z]*", "a");
  if (a != b) quit(_wa, "");
  quit(_ok, "");
}
