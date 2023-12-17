/**
 * Validator for undirected graphs.
 */

#include <cstdint>
#include <numeric>
#include <vector>

#include "../../../cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_VALIDATOR(val);

struct Edge {
  int u, v;

  static Edge read(var::Reader& in, int n) {
    auto [u, _, v] = in(var::i32(1, n, "u"), var::space, var::i32(1, n, "v"));
    return {u, v};
  }
};

struct Input {
  int n, m;
  std::vector<Edge> edges;

  static Input read(var::Reader& in) {
    auto [n, _, m, __] = in(var::i32(2, 1000, "n"), var::space, var::i32(1, 1000, "m"), var::eoln);
    auto edges = in.read(var::Vec(var::ExtVar<Edge>("edges", n), m, var::eoln));
    in.read(var::eoln);
    return {n, m, edges};
  }
};

struct DSU {
  std::vector<int> fa, size;

  DSU(int n) {
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

void validator_main() {
  Input input;

  val.traits({
      {"g_is_connected", [&]() { return is_connected(input); }},
      {"g_is_tree", [&]() { return input.n == input.m + 1; }, {"g_is_connected"}},
  });

  input = val.inf.read(var::ExtVar<Input>("input"));

  val.quit_valid();
}
