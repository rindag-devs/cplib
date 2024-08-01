/*
 * This file is part of CPLib.
 *
 * CPLib is free software: you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CPLib is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with CPLib. If
 * not, see <https://www.gnu.org/licenses/>.
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
