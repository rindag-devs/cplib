import { readdir } from "fs/promises";
import gulp from "gulp";
import parseArgs from "minimist";
import { gxxCompileFile } from "../compile.js";
import { gulpCplibTest } from "../plugin/cplib-test.js";

const argv = parseArgs(process.argv.slice(2));

export async function testAll(): Promise<gulp.TaskFunction> {
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

function compile(testName: string): gulp.TaskFunction {
  return gxxCompileFile(`tests/checker/${testName}/chk.cpp`, `out/tests/checker/${testName}/chk`);
}

function test(testName: string): gulp.TaskFunction {
  const task = (cb: gulp.TaskFunctionCallback): void => {
    gulp
      .src(`tests/checker/${testName}/data/*.{in,out,ans,json}`)
      .pipe(
        gulpCplibTest({
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
