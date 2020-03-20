#ifndef LEX_HEADER
#define LEX_HEADER

#include <stdio.h>

#include "tokens.h"

/* These function calls are so frequently used that I made them a macros. */
#define ungetc(x) 	 fseek(x, -sizeof(char), SEEK_CUR)
#define skipch(x)        fseek(x,  sizeof(char), SEEK_CUR)

/*
 * The main function of lexical analyzer,
 * aka the scanner interface.
 */
extern token_list *lex   (FILE *input);
    
#endif
