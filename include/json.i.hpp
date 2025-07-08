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

#include <cstdint>
#include <ios>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib::json {

inline Value::~Value() = default;

inline String::String(std::string inner) : inner(std::move(inner)) {}

[[nodiscard]] inline auto String::clone() const -> std::unique_ptr<Value> {
  return std::make_unique<String>(*this);
}

inline auto String::to_string() -> std::string {
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
          buf.sputn(cplib::format("{:04x}", static_cast<unsigned char>(c)).c_str(), 4);
        } else {
          buf.sputc(c);
        }
    }
  }
  buf.sputc('\"');
  return buf.str();
}

inline Int::Int(std::int64_t inner) : inner(inner) {}

[[nodiscard]] inline auto Int::clone() const -> std::unique_ptr<Value> {
  return std::make_unique<Int>(*this);
}

inline auto Int::to_string() -> std::string { return std::to_string(inner); }

inline Real::Real(double inner) : inner(inner) {}

[[nodiscard]] inline auto Real::clone() const -> std::unique_ptr<Value> {
  return std::make_unique<Real>(*this);
}

inline auto Real::to_string() -> std::string { return cplib::format("{:.10g}", inner); }

inline Bool::Bool(bool inner) : inner(inner) {}

[[nodiscard]] inline auto Bool::clone() const -> std::unique_ptr<Value> {
  return std::make_unique<Bool>(*this);
}

inline auto Bool::to_string() -> std::string {
  if (inner) {
    return "true";
  } else {
    return "false";
  }
}

inline List::List(std::vector<std::unique_ptr<Value>> inner) : inner(std::move(inner)) {}

[[nodiscard]] inline auto List::clone() const -> std::unique_ptr<Value> {
  std::vector<std::unique_ptr<Value>> list;
  list.reserve(inner.size());

  for (const auto& value : inner) {
    list.push_back(value->clone());
  }

  return std::make_unique<List>(std::move(list));
}

inline auto List::to_string() -> std::string {
  std::stringbuf buf(std::ios_base::out);
  buf.sputc('[');
  if (!inner.empty()) {
    auto it = inner.begin();
    auto tmp = (*it)->to_string();
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      tmp = (*it)->to_string();
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    }
  }
  buf.sputc(']');
  return buf.str();
}

inline Map::Map(std::map<std::string, std::unique_ptr<Value>> inner) : inner(std::move(inner)) {}

[[nodiscard]] inline auto Map::clone() const -> std::unique_ptr<Value> {
  std::map<std::string, std::unique_ptr<Value>> map;

  for (const auto& [key, value] : inner) {
    map.emplace(key, value->clone());
  }

  return std::make_unique<Map>(std::move(map));
}

inline auto Map::to_string() -> std::string {
  std::stringbuf buf(std::ios_base::out);
  buf.sputc('{');
  if (!inner.empty()) {
    auto it = inner.begin();
    buf.sputc('\"');
    auto tmp = it->first;
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    buf.sputc('\"');
    buf.sputc(':');
    tmp = it->second->to_string();
    buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    ++it;
    for (; it != inner.end(); ++it) {
      buf.sputc(',');
      buf.sputc('\"');
      tmp = it->first;
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
      buf.sputc('\"');
      buf.sputc(':');
      tmp = it->second->to_string();
      buf.sputn(tmp.c_str(), static_cast<std::streamsize>(tmp.size()));
    }
  }
  buf.sputc('}');
  return buf.str();
}

}  // namespace cplib::json
