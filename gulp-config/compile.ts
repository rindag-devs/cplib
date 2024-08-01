import { execFile } from "child_process";
import { mkdir, readFile } from "fs/promises";
import type gulp from "gulp";
import parseArgs from "minimist";
import { dirname } from "path";
import { promisify } from "util";

const argv = parseArgs(process.argv.slice(2));

const compileFlags = await readFile("compile_flags.txt", "utf-8").then((content) =>
  content.split("\n").filter((line) => line.length > 0),
);

const cppCompiler = (argv["cpp_compiler"] as string) ?? "g++";

export function gxxCompileFile(filePath: string, output: string): gulp.TaskFunction {
  const extraArgs =
    argv["debug-compile"] || argv["d"] ? ["-g", "-O0", "-fsanitize=address,undefined"] : ["-O2"];

  const task = async (cb: gulp.TaskFunctionCallback): Promise<void> => {
    const outputDir = dirname(output);
    await mkdir(outputDir, { recursive: true });
    await promisify(execFile)(
      cppCompiler,
      [filePath, "-o", output, "-O2"].concat(compileFlags).concat(extraArgs),
    );
    cb();
  };
  task.displayName = "compile:" + filePath;
  return task;
}
