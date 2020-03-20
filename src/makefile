CC=	gcc
STD=	_GNU_SOURCE_
OBJS=	main.o lex.o memory.o parser.o semantics.o
CFLAGS=	-Wall  -Wno-parentheses -Wno-switch -D$(STD) -c -Werror -g
TARGET= ../target/

.c.o:
	$(CC) $(CFLAGS) $<

all:	project

project:	$(OBJS)
		$(CC) $(OBJS) -o $(TARGET)minipl

clean:
	rm -f *.o

clobber:	clean
	rm $(TARGET)minipl
