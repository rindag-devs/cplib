/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
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
