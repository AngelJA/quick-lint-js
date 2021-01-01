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

"use strict";

let assert = require("assert");
let vscode = require("vscode");
let { createParserAsync } = require("./quick-lint-js.js");

let clientID = "quick-lint-js";

let toDispose = [];

// @@@ recovers poorly if wasm crashes (e.g. OOM).

class DocumentLinter {
  constructor(document, diagnosticsCollection) {
    this._document = document;
    this._diagnosticsCollection = diagnosticsCollection;
    this._parserPromise = createParserAsync();
    this._needInitialSync = true;
  }

  _initialSync(parser) {
    assert.strictEqual(this._needInitialSync, true);
    parser.replaceText(
      {
        start: { line: 0, character: 0 },
        end: { line: 0, character: 0 },
      },
      this._document.getText()
    );
    this._needInitialSync = false;
  }

  _getParserAsync() {
    return this._parserPromise;
  }

  dispose() {
    this._parserPromise.then((parser) => {
      parser.dispose();
    });
  }

  async editorChangedVisibilityAsync() {
    let parser = await this._getParserAsync();
    if (this._needInitialSync) {
      this._initialSync(parser);
      this._lint(parser);
    } else {
      // No changes could have been made with the editor closed. Ignore.
    }
  }

  async textChangedAsync(changes) {
    let parser = await this._getParserAsync();
    if (this._needInitialSync) {
      this._initialSync(parser);
    } else {
      for (let change of changes) {
        parser.replaceText(change.range, change.text);
      }
    }
    this._lint(parser);
  }

  _lint(parser) {
    let diagnostics = parser.lint();
    this._updateDocumentDiagnostics(diagnostics);
  }

  _updateDocumentDiagnostics(qljsDiagnostics) {
    // @@@ severity
    let diagnostics = qljsDiagnostics.map((diag) => {
      let vsCodeDiag = new vscode.Diagnostic(
        new vscode.Range(
          new vscode.Position(diag.startLine, diag.startCharacter),
          new vscode.Position(diag.endLine, diag.endCharacter)
        ),
        diag.message,
        vscode.DiagnosticSeverity.Error
      );
      vsCodeDiag.source = "quick-lint-js";
      return vsCodeDiag;
    });
    this._diagnosticsCollection.set(this._document.uri, diagnostics);
  }
}

class DocumentLinterCollection {
  constructor(diagnosticsCollection) {
    this._diagnosticsCollection = diagnosticsCollection;

    // Mapping from URI string to DocumentLinter.
    this._linters = new Map();
  }

  getLinter(document) {
    let documentURIString = document.uri.toString();
    let linter = this._linters.get(documentURIString);
    if (typeof linter === "undefined") {
      linter = new DocumentLinter(document, this._diagnosticsCollection);
      this._linters.set(documentURIString, linter);
    }
    return linter;
  }

  disposeLinter(document) {
    let documentURIString = document.uri.toString();
    let linter = this._linters.get(documentURIString);
    if (typeof linter !== "undefined") {
      linter.dispose();
      this._linters.delete(documentURIString);
    }
  }

  dispose() {
    for (let [_uri, linter] of this._linters) {
      linter.dispose();
    }
    this._linters.clear();
  }
}

async function activateAsync(context) {
  let diagnostics = vscode.languages.createDiagnosticCollection();
  toDispose.push(diagnostics);

  let linters = new DocumentLinterCollection(diagnostics);
  toDispose.push(linters);

  toDispose.push(
    vscode.workspace.onDidChangeTextDocument((event) => {
      logAsyncErrors(
        (async () => {
          await linters
            .getLinter(event.document)
            .textChangedAsync(event.contentChanges);
        })()
      );
    })
  );

  /*
  for (let k of Object.keys(vscode.workspace)) {
    //console.log("@@@ register workspace." + k);
    if (/^on/.test(k) && !["onDidChangeTunnels"].includes(k)) {
      toDispose.push(vscode.workspace[k]((document) => {
        console.log("@@@ workspace." + k);
      }));
    }
  }
  for (let k of Object.keys(vscode.window)) {
    //console.log("@@@ register window." + k);
    if (/^on/.test(k) && !["onDidChangeTerminalDimensions", "onDidWriteTerminalData"].includes(k)) {
      toDispose.push(vscode.window[k]((document) => {
        console.log("@@@ window." + k);
      }));
    }
  }
  */

  toDispose.push(
    vscode.window.onDidChangeVisibleTextEditors((editors) => {
      logAsyncErrors(async () => {
        // @@@ Promise.all
        for (let editor of vscode.window.visibleTextEditors) {
          await linters
            .getLinter(editor.document)
            .editorChangedVisibilityAsync();
        }
      });
    })
  );

  toDispose.push(
    vscode.workspace.onDidCloseTextDocument((document) => {
      linters.disposeLinter(document);
    })
  );

  for (let editor of vscode.window.visibleTextEditors) {
    await linters.getLinter(editor.document).editorChangedVisibilityAsync(); // @@@ Promise.all
  }
}
exports.activate = activateAsync;

async function deactivateAsync() {
  for (let disposable of toDispose) {
    await disposable.dispose();
  }
}
exports.deactivate = deactivateAsync;

// @@@ delete if unused
function logAsyncErrors(promise) {
  if (typeof promise === "function") {
    promise = promise();
  }
  return promise.catch((error) => {
    debugger;
    logError(error);
    //@@@ return Promise.reject(error);
  });
}

function logError(error) {
  if (error && error.stack) {
    console.error("quick-lint-js error:", error.stack);
  } else {
    console.error("quick-lint-js error:", error);
  }
}
