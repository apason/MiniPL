CC=       gcc
STD=      _GNU_SOURCE_
OBJS=     main.o
INCLUDE=  -I "../../../src/"
OTHERS=   ../../../src/lex.o ../../../src/parser.o ../../../src/memory.o ../../../src/semantics.o
CFLAGS=   -Wall -D$(STD) $(INCLUDE) -c
TARGET=   ../../target/

.c.o:
	$(CC) $(CFLAGS) $<

all:	semantics_test

semantics_test:	$(OBJS)
	$(CC) $(OBJS) $(OTHERS) -o $(TARGET)semantics_test

clean:
	rm *.o

clobber:	clean
	rm $(TARGET)semantics_test
