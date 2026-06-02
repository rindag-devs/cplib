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
#include "pattern.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_PATTERN_HPP_
#error "Must be included from pattern.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "regex.h"

/* cplib_embed_ignore start */
#include "utils.hpp"
/* cplib_embed_ignore end */

namespace cplib {
inline Pattern::RegexHandle::RegexHandle(RegexHandle &&other) noexcept
    : regex(other.regex), compiled(std::exchange(other.compiled, false)) {}

inline auto Pattern::RegexHandle::operator=(RegexHandle &&other) noexcept -> RegexHandle & {
  if (this == &other) {
    return *this;
  }
  if (compiled) {
    regfree(&regex);
  }
  regex = other.regex;
  compiled = std::exchange(other.compiled, false);
  return *this;
}

inline Pattern::RegexHandle::~RegexHandle() {
  if (compiled) {
    regfree(&regex);
  }
}

namespace detail {

inline auto get_regex_err_msg(int err_code, const regex_t *re) -> std::string {
  std::size_t len = regerror(err_code, re, nullptr, 0);
  std::string buf(len, 0);
  regerror(err_code, re, buf.data(), len);
  return buf;
}
}  // namespace detail

inline auto Pattern::compile_regex(std::string_view src) -> RegexHandle {
  Pattern::RegexHandle re;
  if (int err =
          regcomp(&re.regex, ("^" + std::string(src) + "$").c_str(), REG_EXTENDED | REG_NOSUB);
      err) {
    auto err_msg = detail::get_regex_err_msg(err, &re.regex);
    panic("Pattern constructor failed: " + err_msg);
  }
  re.compiled = true;
  return re;
}

inline Pattern::Pattern(std::string src) : src_(std::move(src)), re_(compile_regex(src_)) {}

inline Pattern::Pattern(const Pattern &other) : src_(other.src_), re_(compile_regex(src_)) {}

inline auto Pattern::operator=(const Pattern &other) -> Pattern & {
  if (this == &other) {
    return *this;
  }
  src_ = other.src_;
  re_ = compile_regex(src_);
  return *this;
}

inline auto Pattern::match(std::string_view s) const -> bool {
#ifdef REG_STARTEND
  regmatch_t match_range{};
  match_range.rm_so = 0;
  match_range.rm_eo = static_cast<regoff_t>(s.size());
  const char *input = s.empty() ? "" : std::addressof(s.front());
  int result = regexec(&re_.regex, input, 1, &match_range, REG_STARTEND);
#else
  const std::string buffer(s);
  int result = regexec(&re_.regex, buffer.c_str(), 0, nullptr, 0);
#endif

  if (!result) return true;

  if (result == REG_NOMATCH) return false;

  auto err_msg = detail::get_regex_err_msg(result, &re_.regex);
  panic("Pattern match failed: " + err_msg);
  return false;
}

inline auto Pattern::src() const -> std::string_view { return src_; }
}  // namespace cplib
