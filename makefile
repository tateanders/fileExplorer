CC=gcc --std=c99 -g

all: data

data: data.c list.o dynarray.o editMetadata.o
	$(CC) data.c list.o dynarray.o editMetadata.o -o data

list.o: list.c list.h
	$(CC) -c list.c

dynarray.o: dynarray.c dynarray.h
	$(CC) -c dynarray.c

editMetadata.o: editMetadata.c editMetadata.h
	$(CC) -c editMetadata.c

clean:
	rm -f *.o data list dynarray editMetadata
	rm -rf *.dSYM