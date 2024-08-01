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

#ifndef CPLIB_CMD_ARGS_HPP_
#define CPLIB_CMD_ARGS_HPP_

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace cplib::cmd_args {

/// Parsed command-line args.
struct ParsedArgs {
  /// Command-line parameters that do not start with "--"will be stored in `ordered` in their
  /// original relative order.
  std::vector<std::string> ordered;

  // `--flag`
  std::vector<std::string> flags;

  // `--var=value` or `--var value`
  std::map<std::string, std::string> vars;

  ParsedArgs() = default;

  /// Parse from raw command-line args.
  explicit ParsedArgs(const std::vector<std::string>& args);

  [[nodiscard]] auto has_flag(std::string_view name) const -> bool;
};
}  // namespace cplib::cmd_args

#include "cmd_args.i.hpp"  // IWYU pragma: export

#endif
