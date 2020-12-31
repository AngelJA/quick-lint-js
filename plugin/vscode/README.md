# quick-lint-js Visual Studio Code plugin

This directory contains a plugin for the [Visual Studio Code
editor][VisualStudioCode].

Important: The plugin does not actually exist yet. The plugin is a work in
progress.

## Building

To build the extension, install the [emscripten SDK][emscripten-sdk], [Ninja][],
and [Node.js][], then run the following commands:

    $ cd plugin/vscode/  # Navigate to this directory.
    $ emcmake cmake -S ../.. -B build-emscripten -G Ninja -DCMAKE_BUILD_TYPE=Release
    $ emmake ninja -C build-emscripten quick-lint-js-vscode
    $ emmake cmake --install build-emscripten --component vscode --prefix .

## Testing

Run `cli.js` to manually test the extension:

    $ node plugin/vscode/cli.js PATH_TO_YOUR_JS_FILE.js

[Ninja]: https://ninja-build.org/
[Node.js]: https://nodejs.org/
[VisualStudioCode]: https://code.visualstudio.com/
[emscripten-sdk]: https://emscripten.org/docs/getting_started/downloads.html
