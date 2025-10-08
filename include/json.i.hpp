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
#include <streambuf>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::json {

inline Raw::Raw(std::string inner) : inner(std::move(inner)) {}

inline auto Value::encode_string(std::streambuf& buf, std::string_view inner) -> void {
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
}

inline auto Value::encode_list(std::streambuf& buf, const List& inner) -> void {
  buf.sputc('[');
  if (!inner.empty()) {
    auto it = inner.begin();
    it->write_string(buf);
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      it->write_string(buf);
    }
  }
  buf.sputc(']');
}

inline auto Value::encode_map(std::streambuf& buf, const Map& inner) -> void {
  buf.sputc('{');
  if (!inner.empty()) {
    auto it = inner.begin();
    encode_string(buf, it->first);
    buf.sputc(':');
    it->second.write_string(buf);
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      encode_string(buf, it->first);
      buf.sputc(':');
      it->second.write_string(buf);
    }
  }
  buf.sputc('}');
}

inline auto Value::write_string(std::streambuf& buf) const -> void {
  std::visit(
      [&buf](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, Null>) {
          constexpr std::string_view NULL_STR = "null";
          buf.sputn(NULL_STR.data(), NULL_STR.size());
        } else if constexpr (std::is_same_v<T, String>) {
          encode_string(buf, arg);
        } else if constexpr (std::is_same_v<T, Int>) {
          auto str = std::to_string(arg);
          buf.sputn(str.c_str(), str.length());
        } else if constexpr (std::is_same_v<T, Real>) {
          auto str = cplib::format("{:.10g}", arg);
          buf.sputn(str.c_str(), str.length());
        } else if constexpr (std::is_same_v<T, Bool>) {
          if (arg) {
            constexpr std::string_view TRUE_STR = "true";
            buf.sputn(TRUE_STR.data(), TRUE_STR.size());
          } else {
            constexpr std::string_view FALSE_STR = "false";
            buf.sputn(FALSE_STR.data(), FALSE_STR.size());
          }
        } else if constexpr (std::is_same_v<T, List>) {
          encode_list(buf, arg);
        } else if constexpr (std::is_same_v<T, Map>) {
          encode_map(buf, arg);
        } else if constexpr (std::is_same_v<T, Raw>) {
          buf.sputn(arg.inner.data(), arg.inner.size());
        } else {
          panic("JSON value to string failed: unknown type");
        }
      },
      inner);
}

[[nodiscard]] inline auto Value::to_string() const -> std::string {
  std::stringbuf buf(std::ios_base::out);
  write_string(buf);
  return buf.str();
}

[[nodiscard]] auto Value::is_null() const -> bool { return std::holds_alternative<Null>(inner); }
[[nodiscard]] auto Value::is_string() const -> bool {
  return std::holds_alternative<String>(inner);
}
[[nodiscard]] auto Value::is_int() const -> bool { return std::holds_alternative<Int>(inner); }
[[nodiscard]] auto Value::is_real() const -> bool { return std::holds_alternative<Real>(inner); }
[[nodiscard]] auto Value::is_bool() const -> bool { return std::holds_alternative<Bool>(inner); }
[[nodiscard]] auto Value::is_list() const -> bool { return std::holds_alternative<List>(inner); }
[[nodiscard]] auto Value::is_map() const -> bool { return std::holds_alternative<Map>(inner); }

[[nodiscard]] auto Value::as_string() -> String& { return std::get<String>(inner); }
[[nodiscard]] auto Value::as_string() const -> const String& { return std::get<String>(inner); }

[[nodiscard]] auto Value::as_int() -> Int& { return std::get<Int>(inner); }
[[nodiscard]] auto Value::as_int() const -> const Int& { return std::get<Int>(inner); }

[[nodiscard]] auto Value::as_real() -> Real& { return std::get<Real>(inner); }
[[nodiscard]] auto Value::as_real() const -> const Real& { return std::get<Real>(inner); }

[[nodiscard]] auto Value::as_bool() -> Bool& { return std::get<Bool>(inner); }
[[nodiscard]] auto Value::as_bool() const -> const Bool& { return std::get<Bool>(inner); }

[[nodiscard]] auto Value::as_list() -> List& { return std::get<List>(inner); }
[[nodiscard]] auto Value::as_list() const -> const List& { return std::get<List>(inner); }

[[nodiscard]] auto Value::as_map() -> Map& { return std::get<Map>(inner); }
[[nodiscard]] auto Value::as_map() const -> const Map& { return std::get<Map>(inner); }

}  // namespace cplib::json
