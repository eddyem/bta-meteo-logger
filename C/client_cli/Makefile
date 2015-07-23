CXX.SRCS = tempmon_cli
CC = gcc
LDFLAGS = 
CPPFLAGS = 
all : tempmon_cli clean
tempmon_cli : client_cli.o
	$(CC) client_cli.c $(CPPFLAGS) -o tempmon_cli
clean:
	/bin/rm -f *.o *~

### <DEPENDENCIES ON .h FILES GO HERE> 
# name1.o : header1.h header2.h ...
