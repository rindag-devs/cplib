/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

import type gulp from "gulp";
import { deleteAsync } from "del";

export async function clean(cb: gulp.TaskFunctionCallback) {
  await deleteAsync(["out/**"]);
  cb();
}
