import { readdir } from "fs/promises";
import gulp from "gulp";
import parseArgs from "minimist";
import { gxxCompileFile } from "../compile.js";
import { gulpCplibTest } from "../plugin/cplib-test.js";

const argv = parseArgs(process.argv.slice(2));

export async function testAll(): Promise<gulp.TaskFunction> {
  let testNames = (await readdir("tests/generator/", { withFileTypes: true }))
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
    `tests/generator/${testName}/gen.cpp`,
    `out/tests/generator/${testName}/gen`,
  );
}

function test(testName: string): gulp.TaskFunction {
  const task = (cb: gulp.TaskFunctionCallback): void => {
    gulp
      .src(`tests/generator/${testName}/data/*.{args,in}`)
      .pipe(
        gulpCplibTest({
          prog: `out/tests/generator/${testName}/gen`,
          progArgsExt: "args",
          expectedStdoutExt: "in",
        }),
      )
      .on("error", (err) => {
        console.error(err.toString());
        process.exitCode = 1;
      });
    cb();
  };
  task.displayName = "test:generator:" + testName;
  return task;
}
