GCC=gcc

all:
	$(GCC) exe_1.c -o ej1 -lpthread
	$(GCC) exe_2.c -o ej2 -lpthread
	$(GCC) exe_3.c -o ej3 -lpthread
