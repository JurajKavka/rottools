all: clean build run
dev: rebuild run

clean:
	rm -rf ./build

build:
	cmake -B build && cmake --build build

run:
	./build/MarkdownToHtmlAsyncApp

rebuild: 
	cmake --build build 
