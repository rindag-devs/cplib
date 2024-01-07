/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of
 * the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/
 *
 * See https://github.com/rindag-devs/cplib/ to get latest version or bug tracker.
 */

import { gulpPlugin } from "gulp-plugin-extras";
import type { BufferFile } from "vinyl";
import ts from "typescript";
import log from "fancy-log";

export function gulpTypescriptLint(): NodeJS.ReadableStream & NodeJS.WritableStream {
  const files: BufferFile[] = [];

  return gulpPlugin(
    "gulp-typescript-lint",
    async (file: BufferFile) => {
      files.push(file);
      return file;
    },
    {
      supportsDirectories: false,
      supportsAnyType: false,
      async *onFinish() {
        const configPath = ts.findConfigFile("./", ts.sys.fileExists, "tsconfig.json");
        if (!configPath) {
          throw new Error("Could not find a valid 'tsconfig.json'");
        }

        const configFile = ts.readConfigFile(configPath, ts.sys.readFile);
        const compilerOptions = ts.parseJsonConfigFileContent(configFile.config, ts.sys, "./");
        compilerOptions.options.noEmit = true;

        const program = ts.createProgram(
          files.map((f) => f.path),
          compilerOptions.options,
        );

        const emitResult = program.emit();
        const allDiagnostics = ts.getPreEmitDiagnostics(program).concat(emitResult.diagnostics);

        allDiagnostics.forEach((diagnostic) => {
          const message = formatDiagnosticMessage(diagnostic);
          if (diagnostic.category === ts.DiagnosticCategory.Error) {
            const error = new Error(`TypesScript error: ${message}`) as Error & {
              isPresentable?: boolean;
            };
            error.isPresentable = true;
            throw error;
          }
          log.warn(`TypesScript warning: ${message}`);
        });
      },
    },
  ) as NodeJS.ReadableStream & NodeJS.WritableStream;

  function formatDiagnosticMessage(diagnostic: ts.Diagnostic): string {
    if (!diagnostic.file) {
      return ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
    }
    const { line, character } = ts.getLineAndCharacterOfPosition(
      diagnostic.file,
      diagnostic.start!,
    );
    const message = ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
    return `${diagnostic.file.fileName} (${line + 1},${character + 1}): ${message}`;
  }
}
