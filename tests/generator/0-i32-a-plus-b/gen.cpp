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

CPLIB_REGISTER_GENERATOR(gen, args,                                           //
                         n_min = Var<cplib::var::i32>("n-min", -1000, 1000),  //
                         n_max = Var<cplib::var::i32>("n-max", -1000, 1000),  //
                         same = Flag("same"));

auto generator_main() -> void {
  if (args::n_min > args::n_max) {
    cplib::panic("n_min must be <= n_max");
  }

  int a = gen.rnd.next(args::n_min, args::n_max);
  int b = args::same ? a : gen.rnd.next(args::n_min, args::n_max);

  std::cout << a << ' ' << b << '\n';

  gen.quit_ok();
}
