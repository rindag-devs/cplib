import gulp from "gulp";
import { gulpTypescriptLint } from "../plugin/typescript-lint.js";
import gulpESLintNew from "gulp-eslint-new";

export const lintTs = gulp.parallel(typescriptLint, eslint);

function typescriptLint(cb: gulp.TaskFunctionCallback): void {
  gulp
    .src(["gulpfile.ts", "gulp-config/**/*.ts"])
    .pipe(gulpTypescriptLint())
    .on("error", (err) => {
      console.error(err.toString());
      process.exitCode = 1;
    });
  cb();
}

function eslint(cb: gulp.TaskFunctionCallback): void {
  gulp
    .src(["gulpfile.ts", "gulp-config/**/*.ts"])
    .pipe(gulpESLintNew())
    .pipe(gulpESLintNew.format())
    .pipe(gulpESLintNew.failAfterError());
  cb();
}
