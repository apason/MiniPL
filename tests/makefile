.PHONY: lex parser semantics
all:	lex parser semantics

lex:
	$(MAKE) -C src/lex
	bash lex/test.sh

parser:
	$(MAKE) -C src/parser
	bash parser/test.sh

semantics:
	$(MAKE) -C src/semantics
	bash semantics/test.sh

clean:
	$(MAKE) -C src/parser clean
	$(MAKE) -C src/lex clean
	$(MAKE) -C src/semantics clean

clobber:
	$(MAKE) -C src/parser clobber
	$(MAKE) -C src/lex clobber
	$(MAKE) -C src/semantics clobber
