/**
 * Guessing number in range [1,n].
 *
 * Run test with:
 *
 * ```bash
 * cmake --build build --parallel
 * ctest --test-dir build --output-on-failure --parallel -R '^interactor\.0-guessing-number'
 * ```
 */

#include <variant>

#include "cplib.hpp"

CPLIB_REGISTER_INTERACTOR(intr);

struct Input {
  int n, m;
  static auto read(cplib::var::Reader &in) -> Input {
    auto [n, m] = in(cplib::var::i32("n"), cplib::var::i32("m"));
    return {.n = n, .m = m};
  }
};

struct Query {
  int x;
  static auto read(cplib::var::Reader &in, const Input &input) -> Query {
    auto x = in.read(cplib::var::i32("x", 1, input.n));
    return {.x = x};
  }
};

struct Answer {
  int x;
  static auto read(cplib::var::Reader &in, const Input &input) -> Answer {
    auto x = in.read(cplib::var::i32("x", 1, input.n));
    return {.x = x};
  }
};

struct Operate : std::variant<Query, Answer> {
  static auto read(cplib::var::Reader &in, const Input &input) -> Operate {
    auto op = in.read(cplib::var::String("type", cplib::Pattern("[QA]")));
    if (op == "Q") {
      return {in.read(cplib::var::ExtVar<Query>("Q", input))};
    } else {
      return {in.read(cplib::var::ExtVar<Answer>("A", input))};
    }
  }
};

auto interactor_main() -> void {
  auto input = intr.inf.read(cplib::var::ExtVar<Input>("input"));

  intr.to_user << input.n << '\n';

  int use_cnt = 0;
  while (true) {
    auto op = intr.from_user.read(cplib::var::ExtVar<Operate>("operate", input));
    if (const auto *query = std::get_if<Query>(&op); query != nullptr) {
      if (use_cnt >= 50) {
        intr.quit_wa("Too many queries");
      }
      if (query->x > input.m) {
        intr.to_user << ">\n";
      } else if (query->x == input.m) {
        intr.to_user << "=\n";
      } else {
        intr.to_user << "<\n";
      }
      ++use_cnt;
    } else {
      const auto *answer = std::get_if<Answer>(&op);
      if (answer->x == input.m) {
        intr.quit_ac();
      } else {
        intr.quit_wa(cplib::format("Expected {}, got {}", input.m, answer->x));
      }
    }
  }
}
