#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "cplib.hpp"
#include "gtest/gtest.h"
#include "io.hpp"

using cplib::io::InStream;
using cplib::var::Reader;

// Helper to create a Reader from a string for testing
auto create_reader_from_string(const std::string& content) -> Reader {
  auto buf = std::make_unique<std::stringbuf>(content);
  return Reader(std::make_unique<InStream>(std::move(buf), "test_stream", false),
                cplib::trace::Level::NONE, [](const Reader&, std::string_view msg) -> void {
                  FAIL() << "Reader failed with message: " << msg;
                });
}

TEST(VarReaderTest, ReadInt) {
  auto reader = create_reader_from_string("123 -456");
  auto [a, b] = reader(cplib::var::i32("a"), cplib::var::i32("b"));
  EXPECT_EQ(a, 123);
  EXPECT_EQ(b, -456);
}

TEST(VarReaderTest, ReadFloat) {
  auto reader = create_reader_from_string("3.14 -1.5e2");
  auto [a, b] = reader(cplib::var::f64("a"), cplib::var::f64("b"));
  EXPECT_DOUBLE_EQ(a, 3.14);
  EXPECT_DOUBLE_EQ(b, -150.0);
}

TEST(VarReaderTest, ReadVector) {
  auto reader = create_reader_from_string("3 10 20 30");
  auto len = reader.read(cplib::var::i32("len"));
  auto vec = reader.read(cplib::var::i32("vec") * len);

  ASSERT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 10);
  EXPECT_EQ(vec[1], 20);
  EXPECT_EQ(vec[2], 30);
}
