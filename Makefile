run: build
	./main
build: main.c bmp.c
	cc -O2 main.c bmp.c -o main

debug: build_debug
	gdb ./debug_main

build_debug: main.c bmp.c
	cc -O0 -g main.c bmp.c -o debug_main

bmp: build_bmp
	./bmp

build_bmp: bmp.c
	cc -std=c99 -O2 bmp.c -o bmp

bmp_debug: build_bmp_debug
	gdb ./debug_bmp

build_bmp_debug: bmp.c
	cc -O0 -g bmp.c -o debug_bmp
