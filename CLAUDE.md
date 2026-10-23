# CLAUDE.md

The full project instructions live in AGENTS.md — read it.

@AGENTS.md

## Hard rules (repeated here so they are always in context)

- **Never run `git commit` or `git push`** unless the user asks for it in that same message.
  Make the edits, then stop and report. A green pipeline is not approval.
- **Never push to `main`** — not directly, not via `HEAD:main`, not force. "Commit and push"
  means the current branch. `main` changes only through a pull request.
- **Never configure, build, run, or package without explicit approval for the exact command.**
  Use the app-specific Make targets documented in `AGENTS.md`; do not infer direct CMake commands.
- **Never modify `*.fbp` files or the generated `*Wx.h` / `*Wx.cpp` files.**
- Event/callback handlers are named `Handle*`, never `On*`.
