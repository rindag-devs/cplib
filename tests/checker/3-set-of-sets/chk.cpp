/**
 * Check if two sets of integer sets are equal.
 *
 * sets of integer sets = `std::set<std::set<int32_t>>`.
 *
 * Run test with:
 *
 * ```bash
 * pnpm test -- --kind checker --match "^3-set-of-sets$"
 * ```
 */

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

#include "cplib.hpp"

using namespace cplib;

struct Input {
  int32_t n;

  static Input read(var::Reader& in) {
    int32_t n = in.read(var::i32("n"));
    return {n};
  }
};

struct Set : std::vector<int32_t> {
  static Set read(var::Reader& in, const Input& input) {
    auto len = in.read(var::i32("len", 1, input.n));
    auto result = in.read(var::i32("set", 1, input.n) * len);

    std::sort(result.begin(), result.end());
    if (std::unique(result.begin(), result.end()) != result.end()) {
      in.fail("Elements of set are not unique");
    }

    return {result};
  }
};

struct Output {
  int32_t len;
  std::vector<Set> sets;

  static Output read(var::Reader& in, const Input& input) {
    auto len = in.read(var::i32("len", 0, std::nullopt));
    auto sets = in.read(var::ExtVar<Set>("sets", input) * len);

    std::sort(sets.begin(), sets.end());
    return {len, sets};
  }

  static evaluate::Result evaluate(evaluate::Evaluator& ev, const Output& pans, const Output& jans,
                                   const Input&) {
    auto res = evaluate::Result::ac();
    res &= ev.eq("len", pans.len, jans.len);
    res &= ev.eq("sets", pans.sets, jans.sets);
    return res;
  }
};

CPLIB_REGISTER_CHECKER(chk, Input, Output);
