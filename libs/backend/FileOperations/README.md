# File operations architecture

This library has one dependency direction:

    FileClipboard -> FileOperationTask -> FileOperations -> Native/*
     request only       thread only       policy/engine     OS primitives

## FileOperations

FileOperations::Execute() is synchronous and is the only public execution
path. It validates one immutable request and returns one result containing the
normalized request, outcome, error, and any entry left behind for review.

- Existing destinations are never overwritten.
- Regular-file data is copied by the platform-native backend.
- Directory copy is recursive. Symbolic links are copied as links and are not
  followed.
- A directory is copied into a unique temporary sibling and published with an
  atomic no-overwrite move only after every entry succeeds.
- A failed or cancelled partial directory is sent to system Trash. If that
  cleanup fails, its exact temporary path is returned as residualPath.
- Same-volume moves use an atomic native rename.
- Cross-volume regular-file moves copy first, verify source identity, and then
  privately remove that verified source.
- Cross-volume directory moves are unsupported until recursive source identity
  can be proven safely.
- Trash never falls back to permanent deletion.

## FileOperationTask

This class adds one worker thread around FileOperations::Execute(). It owns no
filesystem policy. Progress and completion callbacks run on its worker thread;
wxWidgets code must queue snapshots to the UI thread.

Cancellation is immediate while native regular-file copying is producing
chunks. Atomic rename, file creation, and some platform Trash calls can only
observe cancellation before their native operation begins.

## FileClipboard

The clipboard stores one path plus Copy or Move intent. It performs no
filesystem calls. Paste creates a FileOperationRequest, and a successful Move
result clears the stored selection.

## System Trash backends

- macOS: NSWorkspace recycleURLs
- Windows: IFileOperation with FOFX_RECYCLEONDELETE
- Linux: GIO g_file_trash

The permanent unlink/DeleteFile helpers under NativeFileMove are private and
are used only for an identity-verified cross-volume regular-file move or an
operation-owned partial regular file.
