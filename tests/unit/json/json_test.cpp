#include <optional>
#include <string>

#include "cplib.hpp"
#include "gtest/gtest.h"

TEST(JsonSerializationTest, Primitives) {
  auto null_val = cplib::json::Value(cplib::json::Null{std::nullopt});
  EXPECT_EQ(null_val.to_string(), "null");

  auto int_val = cplib::json::Value(cplib::json::Int{12345});
  EXPECT_EQ(int_val.to_string(), "12345");

  auto real_val = cplib::json::Value(cplib::json::Real{123.456});
  EXPECT_EQ(real_val.to_string(), "123.456");

  auto bool_true = cplib::json::Value(cplib::json::Bool{true});
  EXPECT_EQ(bool_true.to_string(), "true");

  auto bool_false = cplib::json::Value(cplib::json::Bool{false});
  EXPECT_EQ(bool_false.to_string(), "false");
}

TEST(JsonSerializationTest, StringAndEscaping) {
  auto simple_str = cplib::json::Value(cplib::json::String{"hello world"});
  EXPECT_EQ(simple_str.to_string(), "\"hello world\"");

  // Test special characters that need escaping
  auto escaped_str = cplib::json::Value(cplib::json::String{"\" \\ \b \f \n \r \t"});
  EXPECT_EQ(escaped_str.to_string(), "\"\\\" \\\\ \\b \\f \\n \\r \\t\"");

  // Test control characters (e.g., ASCII 1)
  auto control_char_str = cplib::json::Value(cplib::json::String{std::string(1, '\x01')});
  EXPECT_EQ(control_char_str.to_string(), "\"\\u0001\"");
}

TEST(JsonSerializationTest, List) {
  auto list = cplib::json::List{};
  list.emplace_back(cplib::json::Int{1});
  list.emplace_back(cplib::json::String{"two"});
  list.emplace_back(cplib::json::Bool{true});

  auto list_val = cplib::json::Value(list);
  EXPECT_EQ(list_val.to_string(), "[1,\"two\",true]");

  auto empty_list = cplib::json::Value(cplib::json::List{});
  EXPECT_EQ(empty_list.to_string(), "[]");
}

TEST(JsonSerializationTest, Map) {
  auto map = cplib::json::Map{};
  map.emplace("key1", cplib::json::Value(cplib::json::String{"value1"}));
  map.emplace("key2", cplib::json::Value(cplib::json::Int{100}));

  auto map_val = cplib::json::Value(map);
  // FlatMap is sorted, so keys will be in order
  EXPECT_EQ(map_val.to_string(), "{\"key1\":\"value1\",\"key2\":100}");

  auto empty_map = cplib::json::Value(cplib::json::Map{});
  EXPECT_EQ(empty_map.to_string(), "{}");
}

TEST(JsonSerializationTest, NestedStructure) {
  auto sub_map = cplib::json::Map{};
  sub_map.emplace("sub_key", cplib::json::Value(cplib::json::String{"sub_value"}));

  auto list = cplib::json::List{};
  list.emplace_back(cplib::json::Int{1});
  list.emplace_back(cplib::json::Value{sub_map});

  auto main_map = cplib::json::Map{};
  main_map.emplace("data", cplib::json::Value{list});
  main_map.emplace("status", cplib::json::Value(cplib::json::String{"ok"}));

  auto nested_val = cplib::json::Value(main_map);
  EXPECT_EQ(nested_val.to_string(), "{\"data\":[1,{\"sub_key\":\"sub_value\"}],\"status\":\"ok\"}");
}

TEST(JsonSerializationTest, RawValue) {
  // Raw JSON should be inserted as-is, without extra escaping
  std::string raw_json_str = R"({"raw": true, "value": 123})";
  auto raw_val = cplib::json::Value(cplib::json::Raw{raw_json_str});
  EXPECT_EQ(raw_val.to_string(), raw_json_str);

  // When nested inside another structure
  auto map = cplib::json::Map{};
  map.emplace("payload", raw_val);
  auto container_val = cplib::json::Value(map);
  EXPECT_EQ(container_val.to_string(), "{\"payload\":" + raw_json_str + "}");
}
