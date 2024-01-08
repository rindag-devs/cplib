/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

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
