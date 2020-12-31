// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew Glazar
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// TODO(strager): Make this configurable.
// For build instructions, see plugin/vscode/README.md.
let VSCODE_JS_MODULE_PATH = "./dist/quick-lint-js-vscode.js";

async function createParserAsync() {
  return new Parser(await apiPromise);
}
exports.createParserAsync = createParserAsync;

class Parser {
  constructor(api) {
    this._api = api;
    this._parser = this._api._createParser();
  }

  replaceText(range, replacementText) {
    let textUTF8Size = this._api._module.lengthBytesUTF8(replacementText);
    let textUTF8Pointer = this._api._module._malloc(textUTF8Size + 1);
    try {
      this._api._module.stringToUTF8(
        replacementText,
        textUTF8Pointer,
        textUTF8Size + 1
      );
      this._api._replaceText(
        this._parser,
        0,
        0,
        0,
        0,
        textUTF8Pointer,
        textUTF8Size
      );
    } finally {
      this._api._module._free(textUTF8Pointer);
    }
  }

  lint() {
    let diagnosticsPointer = this._api._lint(this._parser);

    let heap = this._api._module.HEAPU8.buffer;
    let rawDiagnosticsU32 = new Uint32Array(heap, diagnosticsPointer);
    let rawDiagnosticsPtr = new Uint32Array(heap, diagnosticsPointer);
    // struct qljs_vscode_diagnostic {
    //   const char* message;
    //   int start_line;
    //   int start_character;
    //   int end_line;
    //   int end_character;
    // };
    let ERROR = {
      message: 0,
      start_line: 1,
      start_character: 2,
      end_line: 3,
      end_character: 4,

      _ptr_size: 5,
      _u32_size: 5,
    };
    let diagnostics = [];
    for (let i = 0; ; ++i) {
      let messagePtr = rawDiagnosticsPtr[i * ERROR._ptr_size + ERROR.message];
      if (messagePtr === 0) {
        break;
      }
      diagnostics.push({
        message: this._api._module.UTF8ToString(messagePtr),
        startLine: rawDiagnosticsU32[i * ERROR._u32_size + ERROR.start_line],
        startCharacter:
          rawDiagnosticsU32[i * ERROR._u32_size + ERROR.start_character],
        endLine: rawDiagnosticsU32[i * ERROR._u32_size + ERROR.end_line],
        endCharacter:
          rawDiagnosticsU32[i * ERROR._u32_size + ERROR.end_character],
      });
    }
    return diagnostics;
  }

  dispose() {
    this._api._destroyParser(this._parser);
    this._parser = null;
  }
}

class API {
  constructor(module) {
    this._module = module;
    this._createParser = module.cwrap(
      "qljs_vscode_create_parser",
      "number",
      []
    );
    this._destroyParser = module.cwrap("qljs_vscode_destroy_parser", "void", [
      "number",
    ]);
    this._replaceText = module.cwrap("qljs_vscode_replace_text", "void", [
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
      "number",
    ]);
    this._lint = module.cwrap("qljs_vscode_lint", "number", ["number"]);
  }
}

let apiPromise = require(VSCODE_JS_MODULE_PATH)().then(
  (module) => new API(module)
);
