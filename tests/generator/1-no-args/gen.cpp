#include "cplib.hpp"

CPLIB_REGISTER_GENERATOR(gen, args);

auto generator_main() -> void {
  std::cout << 42 << '\n';
  gen.quit_ok();
}
