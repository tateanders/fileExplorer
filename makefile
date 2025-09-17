CC=gcc --std=c99 -g
CFLAGS = -I. -IID3v2dot3 -IID3v2dot4 -Istructures

all: data

data: data.c structures/list.o structures/dynarray.o ID3v2dot3/readID3v2dot3.o ID3v2dot3/editID3v2dot3.o ID3v2dot4/readID3v2dot4.o ID3v2dot4/editID3v2dot4.o metadata.o
	$(CC) $(CFLAGS) data.c structures/list.o structures/dynarray.o ID3v2dot3/readID3v2dot3.o ID3v2dot3/editID3v2dot3.o ID3v2dot4/readID3v2dot4.o ID3v2dot4/editID3v2dot4.o metadata.o -o data

metadata.o: metadata.c metadata.h
	$(CC) $(CFLAGS) -c metadata.c

structures/list.o: structures/list.c structures/list.h
	$(CC) $(CFLAGS) -c structures/list.c -o structures/list.o

structures/dynarray.o: structures/dynarray.c structures/dynarray.h
	$(CC) $(CFLAGS) -c structures/dynarray.c -o structures/dynarray.o

ID3v2dot3/readID3v2dot3.o: ID3v2dot3/readID3v2dot3.c ID3v2dot3/readID3v2dot3.h
	$(CC) $(CFLAGS) -c ID3v2dot3/readID3v2dot3.c -o ID3v2dot3/readID3v2dot3.o

ID3v2dot3/editID3v2dot3.o: ID3v2dot3/editID3v2dot3.c ID3v2dot3/editID3v2dot3.h
	$(CC) $(CFLAGS) -c ID3v2dot3/editID3v2dot3.c -o ID3v2dot3/editID3v2dot3.o

ID3v2dot4/readID3v2dot4.o: ID3v2dot4/readID3v2dot4.c ID3v2dot4/readID3v2dot4.h
	$(CC) $(CFLAGS) -c ID3v2dot4/readID3v2dot4.c -o ID3v2dot4/readID3v2dot4.o

ID3v2dot4/editID3v2dot4.o: ID3v2dot4/editID3v2dot4.c ID3v2dot4/editID3v2dot4.h
	$(CC) $(CFLAGS) -c ID3v2dot4/editID3v2dot4.c -o ID3v2dot4/editID3v2dot4.o

clean:
	rm -f *.o data metadata structures/*.o ID3v2dot3/*.o ID3v2dot4/*.o
	rm -rf *.dSYM
	clear
	clear