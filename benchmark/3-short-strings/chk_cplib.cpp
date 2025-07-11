#include <string>
#include <vector>

#include "cplib.hpp"

using namespace cplib;

struct Input {
  int n;
  static Input read(var::Reader& in) {
    int n = in.read(var::i32("n", 1, 1e7));
    return {n};
  }
};

struct Output {
  std::vector<std::string> a;

  static Output read(var::Reader& in, const Input& inp) {
    auto a = in.read(var::String("a", Pattern("[a-z]*")) * inp.n);
    return {a};
  }

  static evaluate::Result evaluate(evaluate::Evaluator& ev, const Output& pans, const Output& jans,
                                   const Input&) {
    auto res = evaluate::Result::ac();
    res &= ev.eq("a", pans.a, jans.a);
    return res;
  }
};

CPLIB_REGISTER_CHECKER(chk, Input, Output);
