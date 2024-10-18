dependencies = $(shell find src/ -type f -name '*.c')

all: build-folder
	gcc -c $(dependencies)
	mv *.o build

main: all
	gcc -o build/main.out main.c build/*.o -lraylib

build-folder:
	mkdir -p build

clean:
	rm -rf build
