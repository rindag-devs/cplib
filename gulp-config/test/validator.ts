/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

import { readdir } from "fs/promises";
import gulp from "gulp";
import parseArgs from "minimist";
import { gxxCompileFile } from "../compile.js";
import { gulpCplibTest } from "../plugin/cplib-test.js";

const argv = parseArgs(process.argv.slice(2));

export async function testAll(): Promise<gulp.TaskFunction> {
  let testNames = (await readdir("tests/validator/", { withFileTypes: true }))
    .filter((dirent) => dirent.isDirectory())
    .map((dirent) => dirent.name);

  if (argv.match) {
    const pattern = new RegExp(argv.match);
    testNames = testNames.filter((name) => pattern.test(name));
  }

  const testTasks = testNames.map((testItem) => gulp.series(compile(testItem), test(testItem)));
  return gulp.parallel(...testTasks);
}

function compile(testName: string): gulp.TaskFunction {
  return gxxCompileFile(
    `tests/validator/${testName}/val.cpp`,
    `out/tests/validator/${testName}/val`,
  );
}

function test(testName: string): gulp.TaskFunction {
  const task = (cb: gulp.TaskFunctionCallback): void => {
    gulp
      .src(`tests/validator/${testName}/data/*.{in,json}`)
      .pipe(
        gulpCplibTest({
          prog: `out/tests/validator/${testName}/val`,
          progArgs: ["--report-format=json"],
          stdinExt: "in",
          expectedStderrExt: "json",
        }),
      )
      .on("error", (err) => {
        console.error(err.toString());
        process.exitCode = 1;
      });
    cb();
  };
  task.displayName = "test:validator:" + testName;
  return task;
}
