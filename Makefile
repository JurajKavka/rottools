.PHONY: format clean build run

all: clean build run
dev: rebuild run

format:
	find . -path ./build -prune -or \( \( -name "*.cpp" -or -name "*.h" \) -not -name "*Wx.cpp" -not -name "*Wx.h" \) -exec clang-format -i {} +

clean: 
	rm -rf ./build

build:
	cmake -B build && cmake --build build 

run:
	./build/mdreader

rebuild:
	cmake --build build
