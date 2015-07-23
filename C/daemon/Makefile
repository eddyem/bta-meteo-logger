CC = gcc
LDFLAGS = -lcrypt
CPPFLAGS = 
all : daemon tempmon tempmon_cli clean
daemon : daemon.o
	$(CC) $(LDFLAGS) daemon.c $(CPPFLAGS) -o daemon
tempmon : client.o
	$(CC) client.c $(CPPFLAGS) -o tempmon
tempmon_cli : client_cli.o
	$(CC) client_cli.c $(CPPFLAGS) -o tempmon_cli
clean:
	/bin/rm -f *.o *~

### <DEPENDENCIES ON .h FILES GO HERE> 
# name1.o : header1.h header2.h ...
