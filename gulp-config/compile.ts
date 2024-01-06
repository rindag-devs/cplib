/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

import { execFile } from "child_process";
import { mkdir } from "fs/promises";
import type gulp from "gulp";
import parseArgs from "minimist";
import { dirname } from "path";
import { promisify } from "util";

const argv = parseArgs(process.argv.slice(2));

export function gxxCompileFile(filePath: string, output: string) {
  const extraArgs =
    argv["debug-compile"] || argv["d"] ? ["-g", "-O0", "-fsanitize=address,undefined"] : ["-O2"];

  const task = async (cb: gulp.TaskFunctionCallback) => {
    const outputDir = dirname(output);
    await mkdir(outputDir, { recursive: true });
    await promisify(execFile)("g++", [filePath, "-o", output, "-O2"].concat(extraArgs));
    cb();
  };
  task.displayName = "compile:" + filePath;
  return task;
}
