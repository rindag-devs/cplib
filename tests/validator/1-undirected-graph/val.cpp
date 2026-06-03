/**
 * Validator for undirected graphs.
 *
 * Run test with:
 *
 * ```bash
 * cmake --build build --parallel
 * ctest --test-dir build --output-on-failure --parallel -R '^validator\.1-undirected-graph'
 * ```
 */

#include <algorithm>
#include <vector>

#include "cplib.hpp"

struct Edge {
  int u, v;

  static auto read(cplib::var::Reader &in, int n) -> Edge {
    auto [u, _, v] = in(cplib::var::i32("u", 1, n), cplib::var::space, cplib::var::i32("v", 1, n));
    return {.u = u, .v = v};
  }
};

struct Input {
  int n, m;
  std::vector<Edge> edges;

  static auto read(cplib::var::Reader &in) -> Input {
    auto [n, _sp1, m, _sp2] = in(cplib::var::i32("n", 2, 1000), cplib::var::space,
                                 cplib::var::i32("m", 1, 1000), cplib::var::eoln);
    auto edges =
        in.read(cplib::var::Vec(cplib::var::ExtVar<Edge>("edges", n), m, cplib::var::eoln));
    in.read(cplib::var::eoln);
    return {.n = n, .m = m, .edges = std::move(edges)};
  }
};

struct DSU {
  std::vector<int> fa, size;

  explicit DSU(int n) {
    fa.resize(n);
    size.resize(n);
    for (int i = 0; i < n; ++i) {
      fa[i] = i;
      size[i] = 1;
    }
  }

  auto find(int x) -> int {
    while (x != fa[x]) {
      x = (fa[x] = fa[fa[x]]);
    }
    return x;
  }

  auto merge(int x, int y) -> void {
    x = find(x);
    y = find(y);
    if (x == y) {
      return;
    }
    if (size[x] < size[y]) {
      std::swap(x, y);
    }
    size[x] += size[y];
    fa[y] = x;
  }
};

auto is_connected(const Input &input) -> bool {
  DSU dsu(input.n);

  for (auto [u, v] : input.edges) {
    dsu.merge(u - 1, v - 1);
  }

  for (int i = 1; i < input.n; ++i) {
    if (dsu.find(i) != dsu.find(0)) {
      return false;
    }
  }
  return true;
}

auto traits(const Input &input) -> std::vector<cplib::validator::Trait> {
  return {
      {"g_is_connected", [&]() -> bool { return is_connected(input); }},
      {"g_is_tree", [&]() -> bool { return input.n == input.m + 1; }, {{"g_is_connected", true}}},
  };
}

CPLIB_REGISTER_VALIDATOR(Input, traits);
