/**
 * Int32 A+B Problem.
 *
 * Run test with:
 *
 * ```bash
 * pnpm test -- --kind validator --match "^0-i32-a-plus-b$"
 * ```
 */

#include <cstdint>

#include "cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_VALIDATOR(val);

void validator_main() {
  int32_t n, m;
  val.traits({
      {"n_positive", [&]() { return n > 0; }},
      {"m_positive", [&]() { return m > 0; }},
  });

  std::tie(n, std::ignore, m, std::ignore) =
      val.inf(var::i32(-1000, 1000, "n"), var::space, var::i32(-1000, 1000, "m"), var::eoln);

  val.quit_valid();
}
