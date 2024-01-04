/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

import gulp from "gulp";
import { deleteAsync } from "del";
import { gulpPlugin } from "gulp-plugin-extras";
import { mkdir, readdir } from "fs/promises";
import { dirname } from "path";
import { execFile } from "child_process";
import { promisify } from "util";
import pupa from "pupa";
import parseArgs from "minimist";
import _ from "lodash";

const argv = parseArgs(process.argv.slice(2));

const Checker = {};

Checker.testAll = async () => {
  let testNames = (await readdir("tests/checker/", { withFileTypes: true }))
    .filter((dirent) => dirent.isDirectory())
    .map((dirent) => dirent.name);

  if (argv.match) {
    const pattern = new RegExp(argv.match);
    testNames = testNames.filter((name) => pattern.test(name));
  }

  const testTasks = testNames.map((testItem) =>
    gulp.series(Checker.compile(testItem), Checker.test(testItem)),
  );
  return gulp.parallel(...testTasks);
};

Checker.compile = (testName) => {
  return compileFile(`tests/checker/${testName}/chk.cpp`, `out/tests/checker/${testName}/chk`);
};

Checker.test = (testName) => {
  const task = (cb) => {
    gulp
      .src(`tests/checker/${testName}/data/*.{in,out,ans,json}`)
      .pipe(
        gulpTest({
          prog: `out/tests/checker/${testName}/chk`,
          progArgs: ["{in}", "{out}", "{ans}", "--report-format=json"],
          expectedStderrExt: "json",
        }),
      )
      .on("error", (err) => console.error(err.toString()));
    cb();
  };
  task.displayName = "test:checker:" + testName;
  return task;
};

const Generator = {};

Generator.testAll = async () => {
  let testNames = (await readdir("tests/generator/", { withFileTypes: true }))
    .filter((dirent) => dirent.isDirectory())
    .map((dirent) => dirent.name);

  if (argv.match) {
    const pattern = new RegExp(argv.match);
    testNames = testNames.filter((name) => pattern.test(name));
  }

  const testTasks = testNames.map((testItem) =>
    gulp.series(Generator.compile(testItem), Generator.test(testItem)),
  );
  return gulp.parallel(...testTasks);
};

Generator.compile = (testName) => {
  return compileFile(`tests/generator/${testName}/gen.cpp`, `out/tests/generator/${testName}/gen`);
};

Generator.test = (testName) => {
  const task = (cb) => {
    gulp
      .src(`tests/generator/${testName}/data/*.{args,in}`)
      .pipe(
        gulpTest({
          prog: `out/tests/generator/${testName}/gen`,
          progArgsExt: "args",
          expectedStdoutExt: "in",
        }),
      )
      .on("error", (err) => console.error(err.toString()));
    cb();
  };
  task.displayName = "test:generator:" + testName;
  return task;
};

const Interactor = {};

Interactor.testAll = async () => {
  let testNames = (await readdir("tests/interactor/", { withFileTypes: true }))
    .filter((dirent) => dirent.isDirectory())
    .map((dirent) => dirent.name);

  if (argv.match) {
    const pattern = new RegExp(argv.match);
    testNames = testNames.filter((name) => pattern.test(name));
  }

  const testTasks = testNames.map((testItem) =>
    gulp.series(Interactor.compile(testItem), Interactor.test(testItem)),
  );
  return gulp.parallel(...testTasks);
};

Interactor.compile = (testName) => {
  return gulp.parallel(
    compileFile(`tests/interactor/${testName}/intr.cpp`, `out/tests/interactor/${testName}/intr`),
    compileFile(`tests/interactor/${testName}/std.cpp`, `out/tests/interactor/${testName}/std`),
  );
};

Interactor.test = (testName) => {
  const task = (cb) => {
    gulp
      .src(`tests/interactor/${testName}/data/*.{in,json}`)
      .pipe(
        gulpTest({
          prog: `out/tests/interactor/${testName}/intr`,
          interactWith: `out/tests/interactor/${testName}/std`,
          progArgs: ["{in}", "--report-format=json"],
          expectedStderrExt: "json",
        }),
      )
      .on("error", (err) => console.error(err.toString()));
    cb();
  };
  task.displayName = "test:interactor:" + testName;
  return task;
};

const Validator = {};

Validator.testAll = async () => {
  let testNames = (await readdir("tests/validator/", { withFileTypes: true }))
    .filter((dirent) => dirent.isDirectory())
    .map((dirent) => dirent.name);

  if (argv.match) {
    const pattern = new RegExp(argv.match);
    testNames = testNames.filter((name) => pattern.test(name));
  }

  const testTasks = testNames.map((testItem) =>
    gulp.series(Validator.compile(testItem), Validator.test(testItem)),
  );
  return gulp.parallel(...testTasks);
};

Validator.compile = (testName) => {
  return compileFile(`tests/validator/${testName}/val.cpp`, `out/tests/validator/${testName}/val`);
};

Validator.test = (testName) => {
  const task = (cb) => {
    gulp
      .src(`tests/validator/${testName}/data/*.{in,json}`)
      .pipe(
        gulpTest({
          prog: `out/tests/validator/${testName}/val`,
          progArgs: ["--report-format=json"],
          stdinExt: "in",
          expectedStderrExt: "json",
        }),
      )
      .on("error", (err) => console.error(err.toString()));
    cb();
  };
  task.displayName = "test:validator:" + testName;
  return task;
};

function compileFile(filePath, output) {
  const extraArgs =
    argv["debug-compile"] || argv["d"] ? ["-g", "-O0", "-fsanitize=address,undefined"] : ["-O2"];

  const task = async (cb) => {
    const outputDir = dirname(output);
    await mkdir(outputDir, { recursive: true });
    await promisify(execFile)("g++", [filePath, "-o", output, "-O2"].concat(extraArgs));
    cb();
  };
  task.displayName = "compile:" + filePath;
  return task;
}

function gulpTest(options) {
  const {
    prog,
    interactWith,
    progArgs,
    progArgsExt,
    stdinExt,
    expectedStdoutExt,
    expectedStderrExt,
  } = options;
  const tests = {};

  return gulpPlugin(
    "gulp-test",
    async (file) => {
      const testId = stripExtension(file.basename);
      tests[testId] = {
        ...tests[testId],
        [file.extname.substring(1)]: file,
      };
    },
    {
      async *onFinish() {
        await Promise.all(
          Object.entries(tests).map(async ([id, files]) => {
            const filePaths = _.mapValues(files, (file) => file.path);
            const args = (
              progArgsExt
                ? files[progArgsExt].contents
                    .toString()
                    .split("\n")
                    .filter((x) => x)
                : progArgs
            ).map((arg) => pupa(arg, filePaths));

            const result = await (interactWith
              ? executeInteractive(prog, interactWith, args)
              : execute(prog, files[stdinExt]?.contents, args));

            if (expectedStdoutExt) {
              const expected = files[expectedStdoutExt].contents;
              compareResult(id, expected, result.stdout, "stdout");
            }

            if (expectedStderrExt) {
              const expected = files[expectedStderrExt].contents;
              compareResult(id, expected, result.stderr, "stderr");
            }
          }),
        );
        yield;
      },
    },
  );

  function compareResult(id, expected, result, streamName) {
    if (expected.equals(result)) return;
    const error = new Error(
      `Failed on ${id}: ${streamName} did not match.\nExpected:\n${expected}\nGot:\n${result}`,
    );
    error.isPresentable = true;
    throw error;
  }

  async function execute(prog, stdinBuf, args) {
    const promise = promisify(execFile)(prog, args, { encoding: "buffer" });
    if (stdinBuf) promise.child.stdin.end(stdinBuf);
    const { stdout, stderr } = await promise.catch((e) => e);
    return { stdout, stderr };
  }

  async function executeInteractive(prog, interactWith, args) {
    const execFilePromisified = promisify(execFile);

    const intrPromise = execFilePromisified(prog, args, { encoding: "buffer" });
    const stdPromise = execFilePromisified(interactWith, [], { encoding: "buffer" });

    intrPromise.child.stdout.pipe(stdPromise.child.stdin);
    stdPromise.child.stdout.pipe(intrPromise.child.stdin);

    const { stdout, stderr } = (
      await Promise.all([intrPromise.catch((e) => e), stdPromise.catch((e) => e)])
    )[0];

    return { stdout, stderr };
  }
}

function stripExtension(str) {
  return str.substring(0, str.lastIndexOf("."));
}

export async function clean(cb) {
  await deleteAsync(["out/**"]);
  cb();
}

export const test = await (async () => {
  const tasks = [];

  if (!argv.kind || argv.kind === "checker") {
    tasks.push(await Checker.testAll(argv));
  }

  if (!argv.kind || argv.kind === "generator") {
    tasks.push(await Generator.testAll(argv));
  }

  if (!argv.kind || argv.kind === "interactor") {
    tasks.push(await Interactor.testAll(argv));
  }

  if (!argv.kind || argv.kind === "validator") {
    tasks.push(await Validator.testAll(argv));
  }

  return gulp.series(...tasks);
})();
