CC = gcc
LDFLAGS = -lcrypt -lm
all : daemon tempmon tempmon_cli
daemon : daemon.c bta_shdata.c
	$(CC) $(LDFLAGS) daemon.c bta_shdata.c $(CFLAGS) -o daemon
tempmon : client.c
	$(CC) client.c $(CFLAGS) -o tempmon
tempmon_cli : client_cli.c
	$(CC) client_cli.c $(CFLAGS) -o tempmon_cli

clean:
	/bin/rm -f *.o *~

### <DEPENDENCIES ON .h FILES GO HERE> 
# name1.o : header1.h header2.h ...
