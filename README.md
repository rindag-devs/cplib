# CPLib

[![header][badge.header]][header] ![std][badge.std] [![license][badge.license]][license] [![docs][badge.docs]][docs]

[badge.header]: https://img.shields.io/badge/single%20header-main-blue.svg
[badge.std]: https://img.shields.io/badge/min%20std-C%2B%2B%2017-blue.svg
[badge.license]: https://img.shields.io/github/license/rindag-devs/cplib
[badge.docs]: https://img.shields.io/github/deployments/rindag-devs/cplib/Production?label=docs
[header]: https://github.com/rindag-devs/cplib/blob/single-header-snapshot/cplib.hpp
[license]: https://github.com/rindag-devs/cplib/blob/main/LICENSE
[docs]: https://cplib.vercel.app/

## Getting Started

Visit the [documentation home page][docs] to learn more.

## Introduction

CPLib is a library written in C++ for processing test data of competitive programming problems. It helps you write clear and efficient checkers, interactors, validators, and generators.

Here is a basic example of a checker using CPLib:

```cpp filename="chk.cpp" copy showLineNumbers
#include "cplib.hpp"

using namespace cplib;

CPLIB_REGISTER_CHECKER(chk);

void checker_main() {
  auto var_ans = var::i32(-2000, 2000, "ans");

  int ouf_output = chk.ouf.read(var_ans);
  int ans_output = chk.ans.read(var_ans);

  if (ouf_output != ans_output) {
    chk.quit_wa(format("Expected %d, got %d", ans_output, ouf_output));
  }

  chk.quit_ac();
}
```

The above example demonstrates a core logic of CPLib: "variable input template". Using the "variable input template", you can save and reuse the logic of reading variables with the same restrictions. At the same time, based on the hierarchy of variable input, CPLib generates concise input stack information in case of errors.

## License

[MPL-2.0][license]

Copyright (c) 2023-present, rindag-devs
