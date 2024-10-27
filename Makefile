dependencies = $(shell find src/ -type f -name '*.c')
flags = -O2 -Wall -Wextra

all: build-folder
	gcc -c $(dependencies) $(flags)
	mv *.o build

main: all
	gcc -o build/main.out main.c build/*.o -lraylib -lm

run: all main
	./build/main.out

build-folder:
	mkdir -p build

clean:
	rm -rf build
