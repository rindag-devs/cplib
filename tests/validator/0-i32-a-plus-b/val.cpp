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
#include <tuple>
#include <vector>

#include "cplib.hpp"

using namespace cplib;

struct Input {
  int32_t n, m;

  static Input read(var::Reader& in) {
    int32_t n, m;
    std::tie(n, std::ignore, m, std::ignore) =
        in(var::i32("n", -1000, 1000), var::space, var::i32("m", -1000, 1000), var::eoln);
    return {n, m};
  }
};

std::vector<validator::Trait> traits(const Input& input) {
  return {
      {"n_positive", [&]() { return input.n > 0; }},
      {"m_positive", [&]() { return input.m > 0; }},
  };
}

CPLIB_REGISTER_VALIDATOR(Input, traits);
