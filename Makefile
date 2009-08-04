#
# Makefile for check_carp
#

CC = cc
CFLAGS  = -Wall
PROGRAM = check_carp
DESTDIR = /usr/local/libexec/nagios/

all:	$(PROGRAM)

$(PROGRAM):  
	$(CC) $(CFLAGS) -o $(PROGRAM) $(PROGRAM).c

install:
	install -m 755 -o root -g wheel $(PROGRAM) $(DESTDIR)/$(PROGRAM)

uninstall:
	rm -f $(DESTDIR)/$(PROGRAM)

clean:
	rm -f $(PROGRAM)
