/**
 * Check if two sets of integer sets are equal.
 *
 * sets of integer sets = `std::set<std::set<int32_t>>`.
 *
 * Run test with:
 *
 * ```bash
 * cmake --build build --parallel
 * ctest --test-dir build --output-on-failure --parallel -R '^checker\.3-set-of-sets'
 * ```
 */

#include <algorithm>
#include <cstdint>
#include <optional>
#include <ranges>
#include <vector>

#include "cplib.hpp"

struct Input {
  std::int32_t n;

  static auto read(cplib::var::Reader &in) -> Input {
    std::int32_t n = in.read(cplib::var::i32("n"));
    return {.n = n};
  }
};

struct Set : std::vector<std::int32_t> {
  static auto read(cplib::var::Reader &in, const Input &input) -> Set {
    auto len = in.read(cplib::var::i32("len", 1, input.n));
    auto result = in.read(cplib::var::i32("set", 1, input.n) * len);

    std::ranges::sort(result);
    if (std::ranges::unique(result).begin() != result.end()) {
      in.fail("Elements of set are not unique");
    }

    return {result};
  }
};

struct Output {
  std::int32_t len;
  std::vector<Set> sets;

  static auto read(cplib::var::Reader &in, const Input &input) -> Output {
    auto len = in.read(cplib::var::i32("len", 0, std::nullopt));
    auto sets = in.read(cplib::var::ExtVar<Set>("sets", input) * len);

    std::ranges::sort(sets);
    return {.len = len, .sets = std::move(sets)};
  }

  static auto evaluate(cplib::evaluate::Evaluator &ev, const Output &pans, const Output &jans,
                       const Input &) -> cplib::evaluate::Result {
    auto res = cplib::evaluate::Result::ac();
    res &= ev.eq("len", pans.len, jans.len);
    res &= ev.eq("sets", pans.sets, jans.sets);
    return res;
  }
};

CPLIB_REGISTER_CHECKER(Input, Output);
