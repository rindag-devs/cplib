#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

std::mt19937 rnd;

const int n = 1e7;
const int m = 10;

auto main(int argc, char** argv) -> int {
  if (argc != 4)
    std::cerr << "Usage: " << argv[0] << " <input-file> <output-file> <answer-file>\n",
        exit(EXIT_FAILURE);
  rnd.seed(std::random_device()());
  std::ofstream fin(argv[1]), fout(argv[2]), fans(argv[3]);
  fin << n << '\n';
  for (int i = 1; i <= n; ++i) {
    for (int j = 1; j <= m; ++j) {
      char c = std::uniform_int_distribution<int>('a', 'z')(rnd);
      fout << c;
      fans << c;
    }
    fout << '\n', fans << '\n';
  }
}
