/**
 * Int32 A+B Problem.
 *
 * Run test with:
 *
 * ```bash
 * pnpm gulp test --kind generator --match "^0-i32-a-plus-b$"
 * ```
 */

#include <cstdint>
#include <vector>

#include "../../../cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_GENERATOR(gen, Args, {
  Var<var::i32> n_min{var::i32("n-min")};
  Var<var::i32> n_max{var::i32("n-max")};
});

void generator_main(const Args& args) {
  if (args.n_min.value > args.n_max.value) panic("n_min must be less than n_max");

  auto a = gen.rnd.next(args.n_min.value, args.n_max.value);
  auto b = gen.rnd.next(args.n_min.value, args.n_max.value);

  std::cout << a << ' ' << b << '\n';

  gen.quit_ok();
}
