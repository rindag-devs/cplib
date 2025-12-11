/**
 * Check shortest path from 1 to n.
 *
 * Run test with:
 *
 * ```bash
 * cmake --build build --parallel
 * ctest --test-dir build --output-on-failure --parallel -R '^checker\.2-shortest-path'
 * ```
 */

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "cplib.hpp"

using namespace cplib;

// Define an Edge struct for reading graph edges
struct Edge {
  int u, v, w;

  // Edge::read takes a Reader and the graph size 'n' as context
  static Edge read(var::Reader& in) {
    // Using in() operator for reading multiple variables conveniently
    auto [u, v, w] = in(var::i32("u"), var::i32("v"), var::i32("w"));
    return {u, v, w};
  }
};

// Define the Input struct to hold graph data
struct Input {
  int n, m;
  std::map<std::pair<int, int>, int> graph;  // Adjacency map for quick edge lookup

  static Input read(var::Reader& in) {
    auto [n, m] = in(var::i32("n"), var::i32("m"));
    // Read 'm' edges, passing 'n' as context to Edge::read via var::ExtVar
    auto edges = in.read(var::ExtVar<Edge>("edges") * m);

    std::map<std::pair<int, int>, int> graph;
    for (auto [u, v, w] : edges) {
      graph[{u, v}] = w;
      graph[{v, u}] = w;  // Undirected graph
    }

    return {n, m, std::move(graph)};
  }
};

// Define the Output struct to hold the shortest path sum and the path itself
struct Output {
  int sum, len;
  std::vector<int> plan;

  // Output::read takes Reader and the Input struct as context
  static Output read(var::Reader& in, const Input& input) {
    // Read sum and length, with appropriate ranges
    auto sum = in.read(var::i32("sum", 0, std::nullopt));
    auto len = in.read(var::i32("len", 1, input.n * input.n));  // Max path length for N nodes

    // Read the path plan, ensuring nodes are within [1, input.n]
    auto plan = in.read(var::i32("plan", 1, input.n) * len);

    // Perform validity checks on the path
    if (plan.empty()) in.fail("Path cannot be empty");
    if (plan.front() != 1) in.fail("Path should begin with 1");
    if (plan.back() != input.n) in.fail("Path should end with n");

    int result_sum = 0;
    for (int i = 1; i < (int)plan.size(); ++i) {
      if (!input.graph.count({plan[i - 1], plan[i]}))
        in.fail(format("Edge {} <-> {} does not exist", plan[i - 1], plan[i]));
      result_sum += input.graph.at({plan[i - 1], plan[i]});
    }

    if (result_sum != sum)
      in.fail(format("Calculated path sum ({}) from plan does not match reported sum ({})",
                     result_sum, sum));

    return {sum, len, std::move(plan)};
  }

  // Output::evaluate compares participant's output with jury's output
  static evaluate::Result evaluate(evaluate::Evaluator& ev, const Output& pans, const Output& jans,
                                   const Input&) {
    auto res = evaluate::Result::ac();

    // If participant's sum is strictly less than jury's sum,
    // it implies the jury's answer is not optimal. This is an internal error.
    if (pans.sum < jans.sum) {
      ev.fail(
          format("Participant's path sum ({}) is less than jury's path sum ({})! This indicates a "
                 "judge error.",
                 pans.sum, jans.sum));
    }

    // The problem asks for the shortest path sum.
    // If participant's sum is greater than jury's, it's WA.
    res &= ev.eq("sum", pans.sum, jans.sum);

    // If sums are equal, the path itself needs to be valid (already checked in read)
    // and correctly connects 1 to n (also checked in read).
    // No further comparison of 'plan' is strictly needed if 'sum' is correct,
    // as any valid path yielding the shortest sum is acceptable.
    // However, for completeness or stricter checks, one might compare 'len' or 'plan'
    // if multiple shortest paths exist and specific ones are preferred.
    // For this problem, we only care about the sum.

    return res;
  }
};

// Register the checker
CPLIB_REGISTER_CHECKER(Input, Output);
