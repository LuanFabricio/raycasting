dependencies = $(shell find src/ -type f -name '*.c')
flags = -O2 -Wall -Wextra

all: build-folder
	gcc -o build/core.so $(dependencies) $(flags) -lm -fPIC -shared

main: all
	gcc -o build/main.out main.c build/core.so -lraylib -lm

run: all main
	./build/main.out

build-folder:
	mkdir -p build

clean:
	rm -rf build
