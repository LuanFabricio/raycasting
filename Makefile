dependencies = $(shell find src/ -type f -name '*.c') raycasting.c
flags = -O2 -Wall -Wextra

all: build-folder
	# TODO: Add a recompile only for modified files
	gcc -o build/core.so $(dependencies) $(flags) -lm -fPIC -shared

main: all
	gcc -g -o build/main.out main.c -lraylib

run: all main
	./build/main.out

build-folder:
	mkdir -p build

clean:
	rm -rf build
