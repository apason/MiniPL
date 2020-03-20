SRC= ./src
TESTS= ./tests

all:	project

tests:	project
	$(MAKE) -C $(TESTS) all

project:
	$(MAKE) -C $(SRC) all

clean:
	$(MAKE) -C $(SRC) clean
	$(MAKE) -C $(TESTS) clean

clobber:
	$(MAKE) -C $(SRC) clobber
	$(MAKE) -C $(TESTS) clobber
