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
import { gulpTest } from "../test_plugin.js";

const argv = parseArgs(process.argv.slice(2));

export async function testAll() {
  let testNames = (await readdir("tests/checker/", { withFileTypes: true }))
    .filter((dirent) => dirent.isDirectory())
    .map((dirent) => dirent.name);

  if (argv.match) {
    const pattern = new RegExp(argv.match);
    testNames = testNames.filter((name) => pattern.test(name));
  }

  const testTasks = testNames.map((testItem) => gulp.series(compile(testItem), test(testItem)));
  return gulp.parallel(...testTasks);
}

function compile(testName: string) {
  return gxxCompileFile(`tests/checker/${testName}/chk.cpp`, `out/tests/checker/${testName}/chk`);
}

function test(testName: string) {
  const task = (cb: gulp.TaskFunctionCallback) => {
    gulp
      .src(`tests/checker/${testName}/data/*.{in,out,ans,json}`)
      .pipe(
        gulpTest({
          prog: `out/tests/checker/${testName}/chk`,
          progArgs: ["{in}", "{out}", "{ans}", "--report-format=json"],
          expectedStderrExt: "json",
        }),
      )
      .on("error", (err) => {
        console.error(err.toString());
        process.exitCode = 1;
      });
    cb();
  };
  task.displayName = "test:checker:" + testName;
  return task;
}
