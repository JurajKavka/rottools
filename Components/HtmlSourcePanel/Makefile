all: clean build run
dev: rebuild run

clean:
	rm -rf ./build

build:
	cmake -B build && cmake --build build

run:
	./build/HtmlSourcePanelApp

rebuild:
	cmake --build build

format:
	find . -path ./build -prune -or \( \( -name "*.cpp" -or -name "*.h" \) -not -name "*Wx.cpp" -not -name "*Wx.h" \) -exec clang-format -i {} +
