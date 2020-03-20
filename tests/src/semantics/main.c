#include <stdio.h>

#include "lex.h"
#include "parser.h"

extern int run(program_node *pn);

int main(int argc, char *argv[]){
    FILE *input = fopen(argv[1], "r");
    if(input == NULL) return -1;
    return run(parse(lex(input))) > 0 ? 1 : 0;
}
