# ℜ⛤✝ reader

A native desktop Markdown viewer built using C++ and **wxWidgets**. It parses Markdown files locally and renders them using a native WebView component.

## Features

- **Drag & Drop Support**: Drag a `.md` or `.markdown` file from your operating system's file manager (e.g. macOS Finder) and drop it directly onto the window to open it.
- **Lazy WebView Initialization**: The application starts as a native frame and loads the native webview only after a file has been successfully opened and parsed. This prevents the native browser engine from hijacking OS-level drag-and-drop actions.
- **Decoupled Architecture**: The file-dropping listener utilizes a generic callback framework (`std::function`), keeping the drop target completely decoupled from the main frame.

---

## Build and Run Instructions

A `Makefile` is provided for convenience. Run these commands from the root directory:

### 1. Build and Run (Clean Build)
Removes previous builds, compiles all targets, and launches the application:
```bash
make all
```

### 2. Rebuild
Quickly compiles incremental changes without performing a clean step:
```bash
make rebuild
```

### 3. Run
Launches the built executable:
```bash
make run
```

---

## Architecture Overview

- **`MainFrame`** (`Components/MainFrame.h` / `.cpp`): The main window containing the toolbar, status bar, and workspace. It coordinates file loading and Markdown parsing.
- **`WebViewPanel`** (`Components/WebViewPanel/`): A custom panel that wraps the native `wxWebView`. It lazily constructs the webview on demand when a Markdown document is first rendered.
- **`FileDropTarget`** (`Components/FileDropTarget/`): A reusable, generic wrapper around `wxFileDropTarget` that accepts a callback function `std::function<void(const wxString&)>` to decouple UI components.
