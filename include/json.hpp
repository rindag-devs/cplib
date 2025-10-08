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
#include <optional>
#include <streambuf>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::json {

struct Value;

using Null = std::nullopt_t;
using String = std::string;
using Int = std::int64_t;
using Real = double;
using Bool = bool;
using List = std::vector<Value>;
using Map = FlatMap<std::string, Value>;

struct Raw {
  std::string inner;
  explicit Raw(std::string inner);
};

struct Value {
  std::variant<Null, String, Int, Real, Bool, List, Map, Raw> inner;

  auto write_string(std::streambuf &buf) const -> void;
  [[nodiscard]] auto to_string() const -> std::string;

  [[nodiscard]] auto is_null() const -> bool;
  [[nodiscard]] auto is_string() const -> bool;
  [[nodiscard]] auto is_int() const -> bool;
  [[nodiscard]] auto is_real() const -> bool;
  [[nodiscard]] auto is_bool() const -> bool;
  [[nodiscard]] auto is_list() const -> bool;
  [[nodiscard]] auto is_map() const -> bool;

  [[nodiscard]] auto as_string() -> String &;
  [[nodiscard]] auto as_string() const -> const String &;

  [[nodiscard]] auto as_int() -> Int &;
  [[nodiscard]] auto as_int() const -> const Int &;

  [[nodiscard]] auto as_real() -> Real &;
  [[nodiscard]] auto as_real() const -> const Real &;

  [[nodiscard]] auto as_bool() -> Bool &;
  [[nodiscard]] auto as_bool() const -> const Bool &;

  [[nodiscard]] auto as_list() -> List &;
  [[nodiscard]] auto as_list() const -> const List &;

  [[nodiscard]] auto as_map() -> Map &;
  [[nodiscard]] auto as_map() const -> const Map &;

  static auto encode_list(std::streambuf &buf, const List &inner) -> void;
  static auto encode_map(std::streambuf &buf, const Map &inner) -> void;
  static auto encode_string(std::streambuf &buf, std::string_view inner) -> void;
};

}  // namespace cplib::json

#include "json.i.hpp"  // IWYU pragma: export

#endif
