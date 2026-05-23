# AGENTS

Project type: Linux library.

## Building docs

requires: [doxygen]

steps:

- `cd docs`
- `doxygen` (generates `docs/html`)
- optional: open `docs/html/index.html`

## Build library

requires: `cmake`, build tools

steps:

- `cmake -B build -S . [...cmake options]`
- `cmake --build build`
- `sudo cmake --install build` (optional)

### Build environment & dependency ordering

- Dependency ordering (use git HEAD of `master` unless specified):
  1. [RF24] (build & install)
  2. [RF24Network] (build & install)
  3. [RF24Mesh] (build & install)
  4. `RF24Gateway` (build)
- Example commands to install dependencies from source (repeat for each
  dependency):
  - `git clone <repo> && cd <repo>`
  - `cmake -B build -S .`
  - `cmake --build build`
  - `sudo cmake --install build`

## Examples

requires: `cmake`, build tools

steps:

- `cmake -B examples/build examples [...cmake options]`
- `cmake --build examples/build`

## Format sources

requires: `clang-format` v14.x

steps:

- `clang-format-14 -i --style=file <file>`
- On Windows (powershell does not expand globs):
  `clang-format -i --style=file <file>`

## Clang-tidy notes

Note: clang-tidy can only be used when targeting Linux builds. Prefer WSL when
building on a Windows host machine.

- Generate compilation database before running `clang-tidy`:
  - `cmake -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
- `clang-tidy-14 -p build --style=file <file>`

[RF24]: https://github.com/nRF24/RF24
[RF24Network]: https://github.com/nRF24/RF24Network
[RF24Mesh]: https://github.com/nRF24/RF24Mesh
[doxygen]: https://github.com/doxygen/doxygen
