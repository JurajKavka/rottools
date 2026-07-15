.DEFAULT_GOAL := help

.PHONY: help all dev build run run-fg rebuild dmg \
        run-filetree run-htmlsource run-mdsource run-dirscan run-md2html run-helpers \
        build-webview build-filedrop \
        format check clean _demos

##@ General
help: ## Show this help (default target)
	@awk 'BEGIN {FS = ":.*##"} \
	  /^##@/ {printf "\n\033[1m%s\033[0m\n", substr($$0, 5)} \
	  /^[a-zA-Z0-9_-]+:.*##/ {printf "  \033[36m%-16s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
	@echo ""

##@ App — rotreader (the whole tree)
all: clean build run-fg   ## Clean, build, and run rotreader (foreground)
dev: rebuild run-fg       ## Rebuild and run in foreground (fast inner loop)

build:                 ## Configure + build the whole tree (system wx)
	cmake -S . -B build -G Ninja && cmake --build build

run:                   ## Launch rotreader (.app bundle)
	open ./build/apps/rotreader/rotreader.app

run-fg:                ## Launch rotreader in foreground (see printLog output)
	./build/apps/rotreader/rotreader.app/Contents/MacOS/rotreader

rebuild:               ## Incremental build of the whole tree
	cmake --build build

dmg: build             ## Package rotreader as a macOS .dmg (CPack)
	cd build && cpack -G DragNDrop

##@ Components — build & run a shared library in isolation
run-filetree: _demos   ## FileBrowserTreePanel   (rottools::ui_filetree)
	cmake --build build --target rottools_ui_filetree_app
	./build/libs/ui/FileBrowserTreePanel/rottools_ui_filetree_app

run-htmlsource: _demos ## HtmlSourcePanel        (rottools::ui_htmlsource)
	cmake --build build --target rottools_ui_htmlsource_app
	./build/libs/ui/HtmlSourcePanel/rottools_ui_htmlsource_app

run-markdownpreview: _demos ## MarkdownPreviewPanel        (rottools::ui_markdownpreview)
	cmake --build build --target rottools_ui_markdownpreview_app
	./build/libs/ui/MarkdownPreviewPanel/rottools_ui_markdownpreview_app

run-mdsource: _demos   ## MarkdownSourcePanel    (rottools::ui_mdsource)
	cmake --build build --target rottools_ui_mdsource_app
	./build/libs/ui/MarkdownSourcePanel/rottools_ui_mdsource_app

run-dirscan: _demos    ## DirectoryScanner       (rottools::dirscan)
	cmake --build build --target rottools_dirscan_app
	./build/libs/backend/DirectoryScanner/rottools_dirscan_app

run-md2html: _demos    ## MarkdownToHtmlAsync    (rottools::md2html)
	cmake --build build --target rottools_md2html_app
	./build/libs/backend/MarkdownToHtmlAsync/rottools_md2html_app

run-helpers: _demos    ## HelperFunctions        (rottools::helpers)
	cmake --build build --target rottools_helpers_app
	./build/libs/HelperFunctions/rottools_helpers_app

##@ Components — library-only (no demo app; compile-check the lib)
build-webview:         ## WebViewPanel           (rottools::ui_webview)
	cmake -S . -B build -G Ninja && cmake --build build --target rottools_ui_webview

build-filedrop:        ## FileDropTarget         (rottools::ui_filedrop)
	cmake -S . -B build -G Ninja && cmake --build build --target rottools_ui_filedrop

##@ Quality
# clang-format all hand-written sources (wxFormBuilder *Wx.* files are excluded).
format:                ## Format all non-generated sources
	find . -path ./build -prune -or \( \( -name "*.cpp" -or -name "*.h" \) -not -name "*Wx.cpp" -not -name "*Wx.h" \) -exec clang-format -i {} +

# Static analysis. wxFormBuilder-generated *Wx.h/*Wx.cpp are excluded.
# constParameterCallback is suppressed globally: wxWidgets' Bind() requires event
# handlers to take a non-const reference.
check:                 ## Run cppcheck static analysis over libs + apps
	cppcheck --std=c++20 --enable=warning,style,performance,portability \
		--suppress=missingIncludeSystem \
		--suppress=constParameterCallback \
		--suppress="*:*Wx.h" --suppress="*:*Wx.cpp" \
		--inline-suppr --error-exitcode=1 -i build --quiet \
		libs apps

clean:                 ## Remove build/ and dist/
	rm -rf ./build ./dist

# internal: (re)configure the tree with the per-library demo apps enabled
_demos:
	cmake -S . -B build -G Ninja -DROTTOOLS_BUILD_LIB_APPS=ON
