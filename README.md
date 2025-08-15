# CPLib

[![header][badge.header]][header] ![std][badge.std] [![license][badge.license]][license] [![docs][badge.docs]][docs]

[badge.header]: https://img.shields.io/badge/single%20header-main-blue.svg
[badge.std]: https://img.shields.io/badge/min%20std-C%2B%2B%2020-blue.svg
[badge.license]: https://img.shields.io/github/license/rindag-devs/cplib
[badge.docs]: https://img.shields.io/github/deployments/rindag-devs/cplib/Production?label=docs
[header]: https://github.com/rindag-devs/cplib/blob/single-header-snapshot/cplib.hpp
[license]: https://github.com/rindag-devs/cplib/blob/main/COPYING.LESSER
[docs]: https://cplib.aberter0x3f.top/

## Getting Started

Visit the [documentation home page][docs] to learn more.

## Introduction

CPLib is a library written in C++ for processing test data of competitive programming problems. It helps you write clear and efficient checkers, interactors, validators, and generators.

CPLib embraces a more structured and modular approach, centered around `Input` and `Output` structs. This design paradigm enhances clarity, maintainability, and testability of your problem logic.

Each checker defines:

- An `Input` struct with a static `read` method to parse problem input.
- An `Output` struct with a static `read` method to parse participant/jury output and perform basic format validation, and a static `evaluate` method that takes an `evaluate::Evaluator`, participant's output, jury's output, and the original input to compare results and determine correctness.

Here is a basic example of an A+B problem checker using CPLib:

```cpp filename="chk.cpp" copy showLineNumbers
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
```

This example demonstrates how CPLib now leverages structured data types (`Input`, `Output`) and dedicated `read`/`evaluate` methods to clearly separate concerns: input parsing, output parsing, and result evaluation. The `evaluate::Evaluator` provides a powerful and flexible way to compare results, handle floating-point precision, and provide detailed feedback in case of errors.

## License

[LGPL-3.0-or-later][license]

Copyright (c) 2023-present, rindag-devs
