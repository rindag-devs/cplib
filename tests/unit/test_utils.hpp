#pragma once

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "cplib.hpp"

namespace cplib_test {

// Exception thrown by the test reader to intercept std::exit
struct TestExitException : std::runtime_error {
  explicit TestExitException(const std::string& msg) : std::runtime_error(msg) {}
};

// Creates a Reader that throws TestExitException instead of calling std::exit
inline auto make_test_reader(std::string content, bool strict = false) -> cplib::var::Reader {
  auto buf = std::make_unique<std::stringbuf>(std::move(content));
  auto stream = std::make_unique<cplib::io::InStream>(std::move(buf), "test_stream", strict);

  return cplib::var::Reader(std::move(stream), cplib::trace::Level::NONE,
                            [](const cplib::var::Reader&, std::string_view msg) {
                              throw TestExitException(std::string(msg));
                            });
}

}  // namespace cplib_test
