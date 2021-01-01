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

let { createParserAsync } = require("./quick-lint-js.js");
let fs = require("fs");

async function main() {
  let fileName = process.argv[2];
  let fileContent = fs.readFileSync(fileName, "utf-8");

  let parser = await createParserAsync();
  try {
    parser.replaceText(
      {
        start: { line: 0, character: 0 },
        end: { line: 0, character: 0 },
      },
      fileContent
    );
    let diagnostics = parser.lint();

    for (let diag of diagnostics) {
      console.log(
        `${diag.startLine}:${diag.startCharacter}-${diag.endLine}:${diag.endCharacter}: error: ${diag.message}`
      );
    }
  } finally {
    parser.dispose();
  }
}

main().catch((error) => {
  console.error(error.stack);
  process.exit(1);
});
