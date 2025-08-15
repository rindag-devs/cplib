/**
 * Validator for undirected graphs.
 *
 * Run test with:
 *
 * ```bash
 * pnpm test -- --kind validator --match "^1-undirected-graph$"
 * ```
 */

#include <algorithm>
#include <vector>

#include "cplib.hpp"

using namespace cplib;

struct Edge {
  int u, v;

  static Edge read(var::Reader& in, int n) {
    auto [u, _, v] = in(var::i32("u", 1, n), var::space, var::i32("v", 1, n));
    return {u, v};
  }
};

struct Input {
  int n, m;
  std::vector<Edge> edges;

  static Input read(var::Reader& in) {
    auto [n, _sp1, m, _sp2] =
        in(var::i32("n", 2, 1000), var::space, var::i32("m", 1, 1000), var::eoln);
    auto edges = in.read(var::Vec(var::ExtVar<Edge>("edges", n), m, var::eoln));
    in.read(var::eoln);
    return {n, m, edges};
  }
};

struct DSU {
  std::vector<int> fa, size;

  explicit DSU(int n) {
    fa.resize(n);
    size.resize(n);
    for (int i = 0; i < n; ++i) fa[i] = i, size[i] = 1;
  }

  int find(int x) {
    while (x != fa[x]) x = (fa[x] = fa[fa[x]]);
    return x;
  }

  void merge(int x, int y) {
    x = find(x), y = find(y);
    if (x == y) return;
    if (size[x] < size[y]) std::swap(x, y);
    size[x] += size[y];
    fa[y] = x;
  }
};

bool is_connected(const Input& input) {
  DSU dsu(input.n);

  for (auto [u, v] : input.edges) {
    dsu.merge(u - 1, v - 1);
  }

  for (int i = 1; i < input.n; ++i) {
    if (dsu.find(i) != dsu.find(0)) return false;
  }
  return true;
}

std::vector<validator::Trait> traits(const Input& input) {
  return {
      {"g_is_connected", [&]() { return is_connected(input); }},
      {"g_is_tree", [&]() { return input.n == input.m + 1; }, {{"g_is_connected", true}}},
  };
}

CPLIB_REGISTER_VALIDATOR(Input, traits);
