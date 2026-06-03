/**
 * Int32 A+B Problem.
 *
 * Run test with:
 *
 * ```bash
 * cmake --build build --parallel
 * ctest --test-dir build --output-on-failure --parallel -R '^validator\.0-i32-a-plus-b'
 * ```
 */

#include <cstdint>
#include <tuple>
#include <vector>

#include "cplib.hpp"

struct Input {
  std::int32_t n, m;

  static auto read(cplib::var::Reader &in) -> Input {
    std::int32_t n, m;
    std::tie(n, std::ignore, m, std::ignore) =
        in(cplib::var::i32("n", -1000, 1000), cplib::var::space, cplib::var::i32("m", -1000, 1000),
           cplib::var::eoln);
    return {.n = n, .m = m};
  }
};

auto traits(const Input &input) -> std::vector<cplib::validator::Trait> {
  return {
      {"n_positive", [&]() -> bool { return input.n > 0; }},
      {"m_positive", [&]() -> bool { return input.m > 0; }},
  };
}

CPLIB_REGISTER_VALIDATOR(Input, traits);
