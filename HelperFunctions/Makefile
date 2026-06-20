all: clean build run
dev: rebuild run

clean:
	rm -rf ./build ./pgplus.db

build:
	cmake -B build && cmake --build build

run:
	./build/HelperFunctionsApp

rebuild: 
	cmake --build build 
