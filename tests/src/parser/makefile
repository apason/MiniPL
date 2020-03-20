CC=       gcc
STD=      _GNU_SOURCE_
OBJS=     main.o
INCLUDE=  -I "../../../src/"
OTHERS=   ../../../src/lex.o ../../../src/parser.o ../../../src/memory.o
CFLAGS=   -Wall -Wno-parentheses -D$(STD) $(INCLUDE) -c
TARGET=   ../../target/

.c.o:
	$(CC) $(CFLAGS) $<

all:	parser_test

parser_test:	$(OBJS)
	$(CC) $(OBJS) $(OTHERS) -o $(TARGET)parser_test

clean:
	rm *.o

clobber:	clean
	rm $(TARGET)parser_test
