# AGENTS.md

## Language

This project is written in C++20.

## Build system

Any directory containing a `Makefile` (e.g. the repo root, `Backend/DirectoryScanner`,
`Backend/MarkdownToHtmlAsync`, `Components/FileBrowserTreePanel`, `HelperFunctions`) is an
independently buildable and runnable component. `cd` into that directory and use:

- `make all` — clean, build, and run
- `make rebuild` — incremental build (no clean)
- `make run` — run the already-built binary
- `make dev` — rebuild and run (fast inner loop, no clean)

At the repo root, `make check` runs cppcheck static analysis (excludes wxFormBuilder-generated
`*Wx.h`/`*Wx.cpp` files; exits non-zero on any finding). Run it before committing.

Use these `make` targets only; do not invoke `cmake` directly.
