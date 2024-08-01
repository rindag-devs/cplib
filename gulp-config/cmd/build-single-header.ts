import gulp from "gulp";
import { gulpCplibEmbed } from "../plugin/cplib-embed.js";

export function buildSingleHeader(cb: gulp.TaskFunctionCallback): void {
  gulp
    .src("include/**/*.hpp")
    .pipe(gulpCplibEmbed("include/cplib.hpp", "cplib.hpp"))
    .pipe(gulp.dest("out/single-header/"));
  cb();
}
