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
#include "cmd_args.hpp"  // IWYU pragma: associated
#else
#ifndef CPLIB_CMD_ARGS_HPP_
#error "Must be included from cmd_args.hpp"
#endif
#endif
/* cplib_embed_ignore end */

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cplib::cmd_args {

namespace detail {
inline auto split_var(std::string_view var) -> std::pair<std::string_view, std::string_view> {
  auto eq_pos = var.find('=');
  return {var.substr(2, eq_pos - 2), var.substr(eq_pos + 1)};
}
}  // namespace detail

inline ParsedArgs::ParsedArgs(const std::vector<std::string>& args) {
  std::optional<std::string> last_flag;

  for (const auto& arg : args) {
    if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-') {
      if (arg.find('=') != std::string::npos) {
        // `--var=value`
        auto [key, value] = detail::split_var(arg);
        vars.emplace(std::string(key), std::string(value));
        if (last_flag.has_value()) {
          flags.emplace_back(*last_flag);
        }
        last_flag = std::nullopt;
      } else {
        // `--flag` or `--var value`
        if (last_flag.has_value()) {
          flags.emplace_back(*last_flag);
        }
        last_flag = arg.substr(2);
      }
    } else if (last_flag.has_value()) {
      // `value` of `--var value`
      vars.emplace(std::string(*last_flag), std::string(arg));
      last_flag = std::nullopt;
    } else {
      // Parameter without "--"
      ordered.emplace_back(arg);
      last_flag = std::nullopt;
    }
  }

  if (last_flag.has_value()) {
    flags.emplace_back(*last_flag);
  }

  // Sort for binary search
  std::sort(flags.begin(), flags.end());
}

inline auto ParsedArgs::has_flag(std::string_view name) const -> bool {
  return std::binary_search(flags.begin(), flags.end(), name);
}

}  // namespace cplib::cmd_args
