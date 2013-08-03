all: clean install
install: main
main: main.o mini_cache.o mini_allocator.o 
	gcc main.o mini_cache.o mini_allocator.o -o main -g
mini_cache.o: mini_cache.c mini_cache.h 
	gcc -c -g mini_cache.c
mini_allocator.o: mini_allocator.c mini_allocator.h 
	gcc -c -g mini_allocator.c 
clean:
	rm *.o main
