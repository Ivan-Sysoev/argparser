# argparser

A small library for parsing command-line arguments, written from scratch.
It supports integers, floats, string arguments (with a fixed max length) and flags,
both in short (`-n`) and long (`--name`) form, with no length limit on names or descriptions.

I built it **test-first**: a GoogleTest suite defines the interface and behaviour the library is
expected to expose, and the implementation grows until everything passes. The tests are the spec.

I also gave myself a hard constraint: **no containers, no strings, no templates, no classes** in
the implementation. So the storage is hand-managed with dynamic allocation, and I spent a while
thinking about how to avoid copy-paste across the different argument types.

**Build & run**
```sh
cmake -B build && cmake --build build
./build/bin/argparser --sum 2 4 6     # example app that adds its arguments
./build/bin/argparser --mult 2 4 6    # ...or multiplies them
```
Library code is in [`lib/`](lib), tests in [`tests/`](tests).
