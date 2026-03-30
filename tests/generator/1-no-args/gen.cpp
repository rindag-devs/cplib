#include "cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_GENERATOR(gen, args);

void generator_main() {
  std::cout << 42 << '\n';
  gen.quit_ok();
}
