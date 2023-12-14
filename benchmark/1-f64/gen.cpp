#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

std::mt19937 rnd;

const int n = 5e6;
const int V = 1e9;

auto main(int argc, char** argv) -> int {
  if (argc != 4)
    std::cerr << "Usage: " << argv[0] << " <input-file> <output-file> <answer-file>\n",
        exit(EXIT_FAILURE);
  rnd.seed(std::random_device()());
  std::ofstream fin(argv[1]), fout(argv[2]), fans(argv[3]);
  fin << n << '\n';
  fout << std::fixed << std::setprecision(9);
  fans << std::fixed << std::setprecision(9);
  for (int i = 1; i <= n; ++i) {
    double x = std::uniform_real_distribution<double>(0, V)(rnd);
    fout << x << '\n';
    fans << x << '\n';
  }
}
