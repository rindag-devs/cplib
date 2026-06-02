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

#ifndef CPLIB_PATTERN_HPP_
#define CPLIB_PATTERN_HPP_

#include <string>
#include <string_view>

#include "regex.h"

namespace cplib {

/**
 * Regex pattern in POSIX-Extended style. Used for matching strings.
 *
 * Format see
 * `https://en.wikibooks.org/wiki/Regular_Expressions/POSIX-Extended_Regular_Expressions`.
 */
struct Pattern {
 public:
  /**
   * Create pattern instance by string.
   *
   * @param src The source string representing the regex pattern.
   */
  explicit Pattern(std::string src);

  Pattern(const Pattern &other);
  auto operator=(const Pattern &other) -> Pattern &;
  Pattern(Pattern &&) noexcept = default;
  auto operator=(Pattern &&) noexcept -> Pattern & = default;

  /**
   * Checks if given string matches the pattern.
   *
   * @param s The input string to be matched against the pattern.
   * @return True if the given string matches the pattern, False otherwise.
   */
  [[nodiscard]] auto match(std::string_view s) const -> bool;

  /**
   * Returns the source string of the pattern.
   *
   * @return The source string representing the regex pattern.
   */
  [[nodiscard]] auto src() const -> std::string_view;

 private:
  struct RegexHandle {
    regex_t regex{};
    bool compiled = false;

    RegexHandle() = default;
    RegexHandle(const RegexHandle &) = delete;
    auto operator=(const RegexHandle &) -> RegexHandle & = delete;
    RegexHandle(RegexHandle &&other) noexcept;
    auto operator=(RegexHandle &&other) noexcept -> RegexHandle &;
    ~RegexHandle();
  };

  static auto compile_regex(std::string_view src) -> RegexHandle;

  std::string src_;
  RegexHandle re_;
};
}  // namespace cplib

#include "pattern.i.hpp"  // IWYU pragma: export

#endif
