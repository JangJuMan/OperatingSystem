all: smalloc.h smalloc.c test1.c test2.c test3.c
	gcc -o test1 test1.c smalloc.c
	gcc -o test2 test2.c smalloc.c
	gcc -o test3 test3.c smalloc.c 
	gcc -o test4 test4.c smalloc.c
	gcc -o test4_v1.0 test4.c smalloc_v1.0.c

clean:
	rm -rf test1 test2 test3 test4 test4_v1.0
