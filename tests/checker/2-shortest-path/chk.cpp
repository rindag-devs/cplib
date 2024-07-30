/**
 * Check shortest path from 1 to n.
 *
 * Run test with:
 *
 * ```bash
 * pnpm test -- --kind checker --match "^2-shortest-path$"
 * ```
 */

#include <map>
#include <utility>
#include <vector>

#include "cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_CHECKER(chk);

struct Edge {
  int u, v, w;

  static Edge read(var::Reader& in) {
    auto [u, v, w] = in(var::i32("u"), var::i32("v"), var::i32("w"));
    return {u, v, w};
  }
};

struct Input {
  int n, m;
  std::map<std::pair<int, int>, int> graph;

  static Input read(var::Reader& in) {
    auto [n, m] = in(var::i32("n"), var::i32("m"));
    auto edges = in.read(var::ExtVar<Edge>("edges") * m);

    std::map<std::pair<int, int>, int> graph;
    for (auto [u, v, w] : edges) {
      graph[{u, v}] = w;
      graph[{v, u}] = w;
    }

    return {n, m, graph};
  }
};

struct Output {
  int sum, len;
  std::vector<int> plan;

  static Output read(var::Reader& in, const Input& input) {
    auto [sum, len] = in(var::i32("sum", 0, std::nullopt), var::i32("len", 1, std::nullopt));
    auto plan = in.read(var::i32("plan", 1, input.n) * len);

    if (plan.front() != 1) in.fail("Plan should begin with 1");
    if (plan.back() != input.n) in.fail("Plan should end with n");
    int result_sum = 0;
    for (int i = 1; i < (int)plan.size(); ++i) {
      if (!input.graph.count({plan[i - 1], plan[i]}))
        in.fail(format("Edge %d <-> %d does not exist", plan[i - 1], plan[i]));
      result_sum += input.graph.at({plan[i - 1], plan[i]});
    }
    if (result_sum != sum) in.fail("Plan and shortest path length do not match");

    return {sum, len, plan};
  }

  static void check(const Output& expected, const Output& result) {
    if (result.sum > expected.sum)
      chk.quit_wa(format("Wrong sum, expected %d, got %d", expected.sum, result.sum));

    if (result.sum < expected.sum)
      panic(format("Contestant answers are better than standard answers, expected %d, got %d",
                   expected.sum, result.sum));
  }
};

void checker_main() {
  auto input = chk.inf.read(var::ExtVar<Input>("input"));

  auto output = var::ExtVar<Output>("output", input);
  auto ouf_output = chk.ouf.read(output);
  auto ans_output = chk.ans.read(output);

  Output::check(ans_output, ouf_output);
  chk.quit_ac();
}
