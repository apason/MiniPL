CC=       gcc
STD=      _GNU_SOURCE_
OBJS=     main.o
INCLUDE=  -I "../../../src/"
OTHERS=	   ../../../src/lex.o ../../../src/memory.o
CFLAGS=   -Wall -D$(STD) $(INCLUDE) -c
TARGET=   ../../target/

.c.o:
	$(CC) $(CFLAGS) $<

all:	lex_test

lex_test:	$(OBJS)
	$(CC) $(OBJS) $(OTHERS)  -o $(TARGET)lex_test

clean:
	rm *.o

clobber:	clean
	rm $(TARGET)lex_test
