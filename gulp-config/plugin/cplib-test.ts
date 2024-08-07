import type { BufferFile } from "vinyl";
import { execFile } from "child_process";
import { gulpPlugin } from "gulp-plugin-extras";
import _ from "lodash";
import pupa from "pupa";
import { promisify } from "util";
import { stripExtension } from "../util.js";

export interface Options {
  prog: string;
  interactWith?: string;
  progArgs?: string[];
  progArgsExt?: string;
  stdinExt?: string;
  expectedStdoutExt?: string;
  expectedStderrExt?: string;
}

export function gulpCplibTest(options: Options): NodeJS.ReadableStream & NodeJS.WritableStream {
  const {
    prog,
    interactWith,
    progArgs,
    progArgsExt,
    stdinExt,
    expectedStdoutExt,
    expectedStderrExt,
  } = options;
  const tests: Record<string, Record<string, BufferFile>> = {};

  return gulpPlugin(
    "gulp-cplib-test",
    async (file: BufferFile): Promise<any> => {
      const testId = stripExtension(file.basename);
      tests[testId] = {
        ...tests[testId],
        [file.extname.substring(1)]: file,
      };
    },
    {
      supportsDirectories: false,
      supportsAnyType: false,
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
                : progArgs!
            ).map((arg) => pupa(arg, filePaths));

            const result = await (interactWith
              ? executeInteractive(prog, interactWith, args)
              : execute(prog, stdinExt ? files[stdinExt]?.contents : undefined, args));

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
      },
    },
  ) as NodeJS.ReadableStream & NodeJS.WritableStream;

  function compareResult(id: string, expected: Buffer, result: Buffer, streamName: string): void {
    if (expected.equals(result)) return;
    const error = new Error(
      `Failed on ${id}: ${streamName} did not match.\nExpected:\n${expected}\nGot:\n${result}`,
    ) as Error & {
      isPresentable?: boolean;
    };
    error.isPresentable = true;
    throw error;
  }

  async function execute(
    prog: string,
    stdinBuf: Buffer | undefined,
    args: readonly string[],
  ): Promise<{ stdout: Buffer; stderr: Buffer }> {
    const promise = promisify(execFile)(prog, args, { encoding: "buffer" });
    if (stdinBuf) promise.child.stdin?.end(stdinBuf);
    const { stdout, stderr } = await promise.catch((e: { stdout: Buffer; stderr: Buffer }) => e);
    return { stdout, stderr };
  }

  async function executeInteractive(
    prog: string,
    interactWith: string,
    args: readonly string[],
  ): Promise<{ stdout: Buffer; stderr: Buffer }> {
    const execFilePromisified = promisify(execFile);

    const intrPromise = execFilePromisified(prog, args, { encoding: "buffer" });
    const stdPromise = execFilePromisified(interactWith, [], { encoding: "buffer" });

    intrPromise.child.stdout?.pipe(stdPromise.child.stdin as NodeJS.WritableStream);
    stdPromise.child.stdout?.pipe(intrPromise.child.stdin as NodeJS.WritableStream);

    const { stdout, stderr } = (
      await Promise.all([
        intrPromise.catch((e: { stdout: Buffer; stderr: Buffer }) => e),
        stdPromise.catch((e: { stdout: Buffer; stderr: Buffer }) => e),
      ])
    )[0];

    return { stdout, stderr };
  }
}
