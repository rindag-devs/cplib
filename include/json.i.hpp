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

/* cplib_embed_ignore start */
#include <string_view>
#include <type_traits>
#include <variant>
#if defined(CPLIB_CLANGD) || defined(CPLIB_IWYU)
#pragma once
#include "json.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_JSON_HPP_
#error "Must be included from json.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <ios>
#include <sstream>
#include <string>
#include <utility>

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::json {

namespace detail {
inline auto encode_string(std::string_view inner) -> std::string {
  std::stringbuf buf(std::ios_base::out);
  buf.sputc('\"');
  for (char c : inner) {
    switch (c) {
      case '"':
        buf.sputc('\\');
        buf.sputc('\"');
        break;
      case '\\':
        buf.sputc('\\');
        buf.sputc('\\');
        break;
      case '\b':
        buf.sputc('\\');
        buf.sputc('b');
        break;
      case '\f':
        buf.sputc('\\');
        buf.sputc('f');
        break;
      case '\n':
        buf.sputc('\\');
        buf.sputc('n');
        break;
      case '\r':
        buf.sputc('\\');
        buf.sputc('r');
        break;
      case '\t':
        buf.sputc('\\');
        buf.sputc('t');
        break;
      default:
        if (('\x00' <= c && c <= '\x1f') || c == '\x7f') {
          buf.sputc('\\');
          buf.sputc('u');
          for (int i = 3; i >= 0; --i) {
            unsigned int digit = (static_cast<unsigned int>(c) >> (i * 4)) & 0x0F;
            buf.sputc(static_cast<char>(digit < 10 ? '0' + digit : 'a' + (digit - 10)));
          }
        } else {
          buf.sputc(c);
        }
    }
  }
  buf.sputc('\"');
  return buf.str();
}

inline auto encode_list(const List &inner) -> std::string {
  std::stringbuf buf(std::ios_base::out);
  buf.sputc('[');
  if (!inner.empty()) {
    auto it = inner.begin();
    auto tmp = it->to_string();
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      tmp = it->to_string();
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    }
  }
  buf.sputc(']');
  return buf.str();
}

inline auto encode_map(const Map &inner) -> std::string {
  std::stringbuf buf(std::ios_base::out);
  buf.sputc('{');
  if (!inner.empty()) {
    auto it = inner.begin();
    buf.sputc('\"');
    auto tmp = it->first;
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    buf.sputc('\"');
    buf.sputc(':');
    tmp = it->second.to_string();
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      buf.sputc('\"');
      tmp = it->first;
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
      buf.sputc('\"');
      buf.sputc(':');
      tmp = it->second.to_string();
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    }
  }
  buf.sputc('}');
  return buf.str();
}
}  // namespace detail

[[nodiscard]] inline auto Value::to_string() const -> std::string {
  return std::visit(
      [](const auto &arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, String>) {
          return detail::encode_string(arg);
        } else if constexpr (std::is_same_v<T, Int>) {
          return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, Real>) {
          return cplib::format("{:.10g}", arg);
        } else if constexpr (std::is_same_v<T, Bool>) {
          return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, List>) {
          return detail::encode_list(arg);
        } else if constexpr (std::is_same_v<T, Map>) {
          return detail::encode_map(arg);
        } else {
          panic("JSON value to string failed: unknown type");
        }
      },
      inner);
}

[[nodiscard]] auto Value::is_string() const -> bool {
  return std::holds_alternative<String>(inner);
}
[[nodiscard]] auto Value::is_int() const -> bool { return std::holds_alternative<Int>(inner); }
[[nodiscard]] auto Value::is_real() const -> bool { return std::holds_alternative<Real>(inner); }
[[nodiscard]] auto Value::is_bool() const -> bool { return std::holds_alternative<Bool>(inner); }
[[nodiscard]] auto Value::is_list() const -> bool { return std::holds_alternative<List>(inner); }
[[nodiscard]] auto Value::is_map() const -> bool { return std::holds_alternative<Map>(inner); }

[[nodiscard]] auto Value::as_string() -> String & { return std::get<String>(inner); }
[[nodiscard]] auto Value::as_string() const -> const String & { return std::get<String>(inner); }

[[nodiscard]] auto Value::as_int() -> Int & { return std::get<Int>(inner); }
[[nodiscard]] auto Value::as_int() const -> const Int & { return std::get<Int>(inner); }

[[nodiscard]] auto Value::as_real() -> Real & { return std::get<Real>(inner); }
[[nodiscard]] auto Value::as_real() const -> const Real & { return std::get<Real>(inner); }

[[nodiscard]] auto Value::as_bool() -> Bool & { return std::get<Bool>(inner); }
[[nodiscard]] auto Value::as_bool() const -> const Bool & { return std::get<Bool>(inner); }

[[nodiscard]] auto Value::as_list() -> List & { return std::get<List>(inner); }
[[nodiscard]] auto Value::as_list() const -> const List & { return std::get<List>(inner); }

[[nodiscard]] auto Value::as_map() -> Map & { return std::get<Map>(inner); }
[[nodiscard]] auto Value::as_map() const -> const Map & { return std::get<Map>(inner); }

}  // namespace cplib::json
