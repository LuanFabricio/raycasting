dependencies = $(shell find src/ -type f -name '*.c') raycasting.c
flags = -O2 -Wall -Wextra -Wno-strict-aliasing

OBJS = $(dependencies:.c=.o)
TARGET = core.so

all: build-folder $(TARGET)

$(TARGET): $(OBJS)
	gcc $(flags) -o build/$@ $^ -lm -shared

%.o: %.c
	gcc $(flags) -fPIC -c $< -o $@

main: all
	gcc $(flags) -o build/main.out main.c -lraylib

run: all main
	./build/main.out

build-folder:
	mkdir -p build

clean:
	rm -rf build
	rm $(shell find . -type f -name '*.o')
