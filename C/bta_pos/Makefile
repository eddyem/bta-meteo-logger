CC = gcc
LDFLAGS = -lcrypt

all : bta_pos 

bta_pos : bta_pos.c bta_shdata.c
	$(CC) bta_pos.c bta_shdata.c $(LDFLAGS) -o bta_pos
clean:
	/bin/rm -f *.o *~
