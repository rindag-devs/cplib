import gulp from "gulp";
import parseArgs from "minimist";
import { testAll as checkerTestAll } from "../test/checker.js";
import { testAll as generatorTestAll } from "../test/generator.js";
import { testAll as interactorTestAll } from "../test/interactor.js";
import { testAll as validatorTestAll } from "../test/validator.js";

const argv = parseArgs(process.argv.slice(2));

export const test = await (async (): Promise<gulp.TaskFunction> => {
  const tasks: gulp.TaskFunction[] = [];

  if (!argv.kind || argv.kind === "checker") {
    tasks.push(await checkerTestAll());
  }

  if (!argv.kind || argv.kind === "generator") {
    tasks.push(await generatorTestAll());
  }

  if (!argv.kind || argv.kind === "interactor") {
    tasks.push(await interactorTestAll());
  }

  if (!argv.kind || argv.kind === "validator") {
    tasks.push(await validatorTestAll());
  }

  return gulp.series(...tasks);
})();
