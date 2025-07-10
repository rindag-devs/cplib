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
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "cplib.hpp"

using namespace cplib;

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

    if (plan.empty()) {
      in.fail("Plan cannot be empty");
    }
    if (plan.front() != 1) {
      in.fail("Plan should begin with 1");
    }
    if (plan.back() != input.n) {
      in.fail("Plan should end with n");
    }
    if ((int)plan.size() != len) {
      in.fail(format("Plan length ({}) does not match reported len ({})", plan.size(), len));
    }

    int result_sum = 0;
    for (int i = 1; i < (int)plan.size(); ++i) {
      if (!input.graph.count({plan[i - 1], plan[i]})) {
        in.fail(format("Edge {} <-> {} does not exist", plan[i - 1], plan[i]));
      }
      result_sum += input.graph.at({plan[i - 1], plan[i]});
    }

    if (result_sum != sum) {
      in.fail(format("Calculated path sum ({}) from plan does not match reported sum ({})",
                     result_sum, sum));
    }

    return {sum, len, plan};
  }

  static evaluate::Result evaluate(evaluate::Evaluator& ev, const Output& pans, const Output& jans,
                                   const Input&) {
    auto res = evaluate::Result::ac();

    if (pans.sum < jans.sum) {
      ev.fail(
          format("Participant's path sum ({}) is less than judge's path sum ({})! This indicates a "
                 "judge error.",
                 pans.sum, jans.sum));
    }

    res &= ev.eq("sum", pans.sum, jans.sum);

    return res;
  }
};

CPLIB_REGISTER_CHECKER(chk, Input, Output);
