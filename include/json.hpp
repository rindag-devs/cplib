/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_JSON_HPP_
#define CPLIB_JSON_HPP_

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace cplib::json {

struct Value {
  virtual ~Value() = 0;

  [[nodiscard]] virtual auto clone() const -> std::unique_ptr<Value> = 0;

  virtual auto to_string() -> std::string = 0;
};

struct String : Value {
  std::string inner;

  explicit String(std::string inner);

  [[nodiscard]] auto clone() const -> std::unique_ptr<Value> override;

  auto to_string() -> std::string override;
};

struct Int : Value {
  std::int64_t inner;

  explicit Int(std::int64_t inner);

  [[nodiscard]] auto clone() const -> std::unique_ptr<Value> override;

  auto to_string() -> std::string override;
};

struct Real : Value {
  double inner;

  explicit Real(double inner);

  [[nodiscard]] auto clone() const -> std::unique_ptr<Value> override;

  auto to_string() -> std::string override;
};

struct Bool : Value {
  bool inner;

  explicit Bool(bool inner);

  [[nodiscard]] auto clone() const -> std::unique_ptr<Value> override;

  auto to_string() -> std::string override;
};

struct List : Value {
  std::vector<std::unique_ptr<Value>> inner;

  explicit List(std::vector<std::unique_ptr<Value>> inner);

  [[nodiscard]] auto clone() const -> std::unique_ptr<Value> override;

  auto to_string() -> std::string override;
};

struct Map : Value {
  std::map<std::string, std::unique_ptr<Value>> inner;

  explicit Map(std::map<std::string, std::unique_ptr<Value>> inner);

  [[nodiscard]] auto clone() const -> std::unique_ptr<Value> override;

  auto to_string() -> std::string override;
};

}  // namespace cplib::json

#include "json.i.hpp"  // IWYU pragma: export

#endif
