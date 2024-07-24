/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
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
#include <vector>

/* cplib_embed_ignore start */
#include "utils.hpp"  // for format
/* cplib_embed_ignore end */

namespace cplib::json {

inline Value::~Value() = default;

inline String::String(std::string inner) : inner(std::move(inner)) {}

inline auto String::to_string() -> std::string {
  std::stringbuf buf(std::ios::out);
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
          buf.sputn(cplib::format("%04hhx", static_cast<unsigned char>(c)).c_str(), 4);
        } else {
          buf.sputc(c);
        }
    }
  }
  buf.sputc('\"');
  return buf.str();
}

inline Int::Int(std::int64_t inner) : inner(inner) {}

inline auto Int::to_string() -> std::string { return std::to_string(inner); }

inline Real::Real(double inner) : inner(inner) {}

inline auto Real::to_string() -> std::string { return cplib::format("%.10g", inner); }

inline Bool::Bool(bool inner) : inner(inner) {}

inline auto Bool::to_string() -> std::string {
  if (inner) {
    return "true";
  } else {
    return "false";
  }
}

inline List::List(std::vector<std::unique_ptr<Value>> inner) : inner(std::move(inner)) {}

inline auto List::to_string() -> std::string {
  std::stringbuf buf(std::ios::out);
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

inline auto Map::to_string() -> std::string {
  std::stringbuf buf(std::ios::out);
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
