/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

import gulp from "gulp";
import { gulpCplibEmbed } from "../plugin/cplib-embed.js";

export function buildSingleHeader(cb: gulp.TaskFunctionCallback): void {
  gulp
    .src("include/**/*.hpp")
    .pipe(gulpCplibEmbed("include/cplib.hpp", "cplib.hpp"))
    .pipe(gulp.dest("out/single-header/"));
  cb();
}
