/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

#ifndef CPLIB_CMD_ARGS_HPP_
#define CPLIB_CMD_ARGS_HPP_

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace cplib::cmd_args {

/// Parsed command line args.
struct ParsedArgs {
  /// Command line parameters that do not start with "--"will be stored in `ordered` in their
  /// original relative order.
  std::vector<std::string> ordered;

  // `--flag`
  std::vector<std::string> flags;

  // `--var=value` or `--var value`
  std::map<std::string, std::string> vars;

  ParsedArgs() = default;

  /// Parse from raw command line args.
  explicit ParsedArgs(const std::vector<std::string>& args);

  [[nodiscard]] auto has_flag(std::string_view name) const -> bool;
};
}  // namespace cplib::cmd_args

#include "cmd_args.i.hpp"  // IWYU pragma: export

#endif
