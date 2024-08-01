import type { BufferFile } from "vinyl";
import { gulpPlugin } from "gulp-plugin-extras";
import { resolve } from "path";
import Vinyl from "vinyl";

const includeRegex = /#include "([^"]+)"/g;
const iwyuPragmaRegex = /\/\/\W*IWYU pragma:.*|\/\*\W*IWYU pragma:.*\*\//gm;
const embedIgnoreStartRegex = /\/\*\W*cplib_embed_ignore start\W*\*\//g;
const embedIgnoreEndRegex = /\/\*\W*cplib_embed_ignore end\W*\*\//g;
const ignoreHeaders = new Set<String>(["regex.h"]);

enum TokenType {
  Include,
  Ignore,
  IgnoreStart,
  IgnoreEnd,
}

interface Token {
  type: TokenType;
  content: RegExpMatchArray;
}

export function gulpCplibEmbed(
  mainPath: string,
  outputFilePath: string,
): NodeJS.ReadableStream & NodeJS.WritableStream {
  const files: BufferFile[] = [];

  const absMainMath = resolve(mainPath);

  return gulpPlugin(
    "gulp-cplib-embed",
    async (file: BufferFile): Promise<any> => {
      files.push(file);
    },
    {
      supportsDirectories: false,
      supportsAnyType: false,
      async *onFinish() {
        const embedContent = makeEmbed(files.find((f) => f.path === absMainMath)!, files);

        yield new Vinyl({
          path: outputFilePath,
          contents: Buffer.from(embedContent, "utf8"),
        });
      },
    },
  ) as NodeJS.ReadableStream & NodeJS.WritableStream;

  function makeEmbed(file: BufferFile, files: BufferFile[]): string {
    const content = file.contents.toString("utf8");

    const includes = Array.from(content.matchAll(includeRegex));
    const iwyuPragmas = Array.from(content.matchAll(iwyuPragmaRegex));
    const ignoreStarts = Array.from(content.matchAll(embedIgnoreStartRegex));
    const ignoreEnds = Array.from(content.matchAll(embedIgnoreEndRegex));

    const tokens: Token[] = includes
      .flatMap((x) => (ignoreHeaders.has(x[1]) ? [] : { type: TokenType.Include, content: x }))
      .concat(iwyuPragmas.map((x) => ({ type: TokenType.Ignore, content: x })))
      .concat(ignoreStarts.map((x) => ({ type: TokenType.IgnoreStart, content: x })))
      .concat(ignoreEnds.map((x) => ({ type: TokenType.IgnoreEnd, content: x })))
      .sort((x, y) => x.content.index! - y.content.index!);

    let depth = 0;

    const rangesToReplace = tokens.flatMap((token) => {
      switch (token.type) {
        case TokenType.Include:
          return [{ replacement: depth === 0, content: token.content }];
        case TokenType.Ignore:
          return [{ replacement: false, content: token.content }];
        case TokenType.IgnoreStart:
          ++depth;
          return [{ replacement: false, content: token.content }];
        case TokenType.IgnoreEnd:
          if (!depth) throw new Error("Unmatched ignore end");
          --depth;
          return [{ replacement: false, content: token.content }];
        default:
          throw new Error("Unexpected token type");
      }
    });

    if (depth !== 0) {
      throw new Error("Unmatched ignore start");
    }

    const result = rangesToReplace.reduceRight(
      (prev, range) =>
        splice(
          prev,
          range.content.index!,
          range.content.index! + range.content[0].length,
          range.replacement
            ? makeEmbed(files.find((f) => f.path === resolve(file.base, range.content[1]))!, files)
            : "",
        ),
      content,
    );

    return result;
  }

  /// Replace specimen[start,end) to replacement
  function splice(specimen: string, start: number, end: number, replacement: string): string {
    const head = specimen.substring(0, start);
    const tail = specimen.substring(end, specimen.length);
    const result = head + replacement + tail;
    return result;
  }
}
