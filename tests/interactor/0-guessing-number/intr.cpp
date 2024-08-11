/**
 * Guessing number in range [1,n].
 *
 * Run test with:
 *
 * ```bash
 * pnpm test -- --kind interactor --match "^0-guessing-number$"
 * ```
 */

#include <variant>

#include "cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_INTERACTOR(intr);

struct Input {
  int n, m;
  static Input read(var::Reader& in) {
    auto [n, m] = in(var::i32("n"), var::i32("A"));
    return {n, m};
  }
};

struct Query {
  int x;
  static Query read(var::Reader& in, const Input& input) {
    auto x = in.read(var::i32("x", 1, input.n));
    return {x};
  }
};

struct Answer {
  int x;
  static Answer read(var::Reader& in, const Input& input) {
    auto x = in.read(var::i32("x", 1, input.n));
    return {x};
  }
};

struct Operate : std::variant<Query, Answer> {
  static Operate read(var::Reader& in, const Input& input) {
    auto op = in.read(var::String("type", Pattern("[QA]")));
    if (op == "Q") {
      return {in.read(var::ExtVar<Query>("Q", input))};
    } else {
      return {in.read(var::ExtVar<Answer>("A", input))};
    }
  }
};

void interactor_main() {
  auto input = intr.inf.read(var::ExtVar<Input>("input"));

  intr.to_user << input.n << '\n';

  int use_cnt = 0;
  while (true) {
    auto op = intr.from_user.read(var::ExtVar<Operate>("operate", input));
    if (op.index() == 0) {
      const auto& Q = std::get<0>(op);
      if (use_cnt >= 50) intr.quit_wa("Too many queries");
      if (Q.x > input.m)
        intr.to_user << ">\n";
      else if (Q.x == input.m)
        intr.to_user << "=\n";
      else
        intr.to_user << "<\n";
      ++use_cnt;
    } else {
      const auto& A = std::get<1>(op);
      if (A.x == input.m)
        intr.quit_ac();
      else
        intr.quit_wa(format("Expected %d, got %d", input.m, A.x));
    }
  }
}
