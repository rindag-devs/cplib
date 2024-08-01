import type gulp from "gulp";
import { deleteAsync } from "del";

export async function clean(cb: gulp.TaskFunctionCallback): Promise<void> {
  await deleteAsync(["out/**"]);
  cb();
}
