/**
 * Int32 A+B Problem.
 *
 * Run test with:
 *
 * ```bash
 * pnpm test -- --kind checker --match "^0-i32-a-plus-b$"
 * ```
 */

#include "cplib.hpp"

using namespace cplib;

// Define the Input struct to read problem-specific inputs (A and B for A+B problem)
struct Input {
  int a, b;

  static Input read(var::Reader& in) {
    // Read integer 'a' within range [-1000, 1000]
    auto a = in.read(var::i32("a", -1000, 1000));
    // Read integer 'b' within range [-1000, 1000]
    auto b = in.read(var::i32("b", -1000, 1000));
    return {a, b};
  }
};

// Define the Output struct to read the answer and evaluate it
struct Output {
  int ans;

  // Static read method to parse participant/jury output
  // It receives a var::Reader and the Input struct for context if needed.
  static Output read(var::Reader& in, const Input&) {
    // Read integer 'ans' within range [-2000, 2000]
    auto ans = in.read(var::i32("ans", -2000, 2000));
    return {ans};
  }

  // Static evaluate method to compare participant's output with jury's output
  // It receives an evaluate::Evaluator, participant's output (pans),
  // jury's output (jans), and the original Input.
  static evaluate::Result evaluate(evaluate::Evaluator& ev, const Output& pans, const Output& jans,
                                   const Input&) {
    // Use ev.eq to compare the 'ans' field.
    // If pans.ans == jans.ans, it contributes to AC. Otherwise, it marks WA.
    auto res = evaluate::Result::ac();
    res &= ev.eq("ans", pans.ans, jans.ans);
    return res;
  }
};

// Register the checker with CPLib, specifying the Input and Output structs.
// This macro sets up the main checker logic behind the scenes.
CPLIB_REGISTER_CHECKER(Input, Output);
