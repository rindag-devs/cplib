/**
 * Int32 A+B Problem.
 *
 * Run test with:
 *
 * ```bash
 * cmake --build build --parallel
 * ctest --test-dir build --output-on-failure --parallel -R '^generator\.0-i32-a-plus-b'
 * ```
 */

#include "cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_GENERATOR(gen, args,                                    //
                         n_min = Var<var::i32>("n-min", -1000, 1000),  //
                         n_max = Var<var::i32>("n-max", -1000, 1000),  //
                         same = Flag("same"));

void generator_main() {
  using namespace args;

  if (n_min > n_max) panic("n_min must be <= n_max");

  int a = gen.rnd.next(n_min, n_max);
  int b = same ? a : gen.rnd.next(n_min, n_max);

  std::cout << a << ' ' << b << '\n';

  gen.quit_ok();
}
