/**
 * Check if two sets of integer sets are equal.
 *
 * sets of integer sets = `std::set<std::set<int32_t>>`.
 *
 * Run test with:
 *
 * ```bash
 * pnpm gulp test --kind checker --match "^3-set-of-sets$"
 * ```
 */

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include "../../../cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_CHECKER(chk);

struct Set : std::vector<int32_t> {
  static Set read(var::Reader& in, int32_t n) {
    auto len = in.read(var::i32(1, n, "len"));
    auto result = in.read(var::i32(1, n, "set") * len);

    std::sort(result.begin(), result.end());
    if (std::unique(result.begin(), result.end()) != result.end())
      in.fail("Elements of set are not unique");

    return {result};
  }
};

struct Output {
  int32_t len;
  std::vector<Set> sets;

  static Output read(var::Reader& in, int32_t n) {
    auto len = in.read(var::i32(0, std::nullopt, "len"));
    auto sets = in.read(var::ExtVar<Set>("sets", n) * len);
    std::sort(sets.begin(), sets.end());
    return {len, sets};
  }

  static void check(const Output& expected, const Output& result) {
    if (expected.len != result.len)
      chk.quit_wa(format("Wrong number of sets, expected %d, got %d", expected.len, result.len));
    if (expected.sets != result.sets) chk.quit_wa("Wrong sets");
  }
};

void checker_main() {
  auto n = chk.inf.read(var::i32("n"));

  auto output = var::ExtVar<Output>("output", n);
  auto ouf_output = chk.ouf.read(output);
  auto ans_output = chk.ans.read(output);

  Output::check(ans_output, ouf_output);
  chk.quit_ac();
}
