CC=gcc --std=c99 -g
CFLAGS = -I. -Imp3 -Istructures

all: data

data: data.c structures/list.o structures/dynarray.o editMetadata.o
	$(CC) data.c structures/list.o structures/dynarray.o editMetadata.o -o data

editMetadata.o: editMetadata.c editMetadata.h
	$(CC) -c editMetadata.c

structures/list.o: structures/list.c structures/list.h
	$(CC) $(CFLAGS) -c structures/list.c -o structures/list.o

structures/dynarray.o: structures/dynarray.c structures/dynarray.h
	$(CC) $(CFLAGS) -c structures/dynarray.c -o structures/dynarray.o

clean:
	rm -f *.o data editMetadata structures/*.o mp3/*.o
	rm -rf *.dSYM