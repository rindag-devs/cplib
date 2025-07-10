/**
 * Int32 A+B Problem.
 *
 * Run test with:
 *
 * ```bash
 * pnpm test -- --kind checker --match "^0-i32-a-plus-b$"
 * ```
 */

#include "cplib.hpp"

using namespace cplib;

struct Input {
  static Input read(var::Reader&) { return {}; }
};

struct Output {
  int ans;

  static Output read(var::Reader& in, const Input&) {
    auto ans = in.read(var::i32("ans", -2000, 2000));
    return {ans};
  }

  static evaluate::Result evaluate(evaluate::Evaluator& ev, const Output& pans, const Output& jans,
                                   const Input&) {
    auto res = evaluate::Result::ac();
    res &= ev.eq("ans", pans.ans, jans.ans);
    return res;
  }
};

CPLIB_REGISTER_CHECKER(chk, Input, Output);
