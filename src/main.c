#include <stdio.h>
#include <string.h>

#include "lex.h"
#include "parser.h"
#include "memory.h"

/* 
 * Interface function for the semantic analyzer
 * and executing the input program.
 */
extern int run(program_node *pn);

int main(int argc, char *argv[]){
    FILE *input = fopen(argv[1], "r");

    if(input == NULL) return -1;
    token_list *tl = lex(input);
    fclose(input);

    tl = correctTokenList(tl);

    program_node *pn = parse(tl);
    freeTokenList(tl);

    /* 
     * In case of the lexical and / or syntax error
     * the interpretr is not launched and the semantic
     * analysis is not done.
     */
    if(pn != NULL)
	return run(pn);

    return 0;
}
