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

## Naming conventions

Event/callback handler functions are named `Handle*` (e.g. `HandleMarkdownError`), never
`On*`. The `On` prefix is reserved for the assignment points: wxWidgets framework virtual
overrides (`OnInit`, `OnExit`, `OnDropFiles` — keep their framework names) and members
holding callbacks (`m_onFileOpened`). Any function bound with `Bind()`, passed as a C
callback, or stored in a `std::function` must start with `Handle`.

## wxFormBuilder files — hands off

Never modify `*.fbp` files or the generated `*Wx.h`/`*Wx.cpp` files. The `.fbp` projects are
edited exclusively by the user in wxFormBuilder, and the `*Wx.*` sources are generated from
them. If a change is needed there (renaming a class, adding a widget or a menu item), ask the
user to make it in wxFormBuilder; put all hand-written code in the non-`Wx` subclasses
(e.g. `WebViewPanel` subclasses `WebViewPanelWx`).
