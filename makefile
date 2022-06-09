all: main.o
	gcc main.o -o code -lm
main.0: main.c stb_image.h
	 gcc -c main.c 
