#ifndef TOKENS_HEADER
#define TOKENS_HEADER

#include <stdio.h>

/* 
 * Definitions of token types. If the language is
 * expanded, this list of definitions can be appended
 * with new token types.
 */
#define TOKEN_BIN_OP         0
#define TOKEN_LPAR           1
#define TOKEN_RPAR           2
#define TOKEN_SCOL           3
#define TOKEN_COL            4
#define TOKEN_ASSIGN         5
#define TOKEN_IDENTIFIER     6
#define TOKEN_INT_LITERAL    7
#define TOKEN_STRING_LITERAL 8
#define TOKEN_TYPEKEY        9
#define TOKEN_FORKEY        10
#define TOKEN_VARKEY        11
#define TOKEN_ENDKEY        12
#define TOKEN_INKEY         13
#define TOKEN_DOKEY         14
#define TOKEN_READKEY       15
#define TOKEN_PRINTKEY      16
#define TOKEN_ASSERTKEY     17
#define TOKEN_ERROR         18
#define TOKEN_UN_OP         19
#define TOKEN_EOF           20
#define TOKEN_RANGE         21

/* 
 * This is the size of the input buffer. Any token,
 * including string literals must fit in the buffer
 * of this size.
 */
#define TOKEN_MAX_LENGTH 1024


/* Token related type definitions. */
typedef unsigned int token_type;
typedef char token_value[TOKEN_MAX_LENGTH +1];

/*
 * Structure for token. Every token has type of integer value
 * and string containing the actual token. The type is unsigned
 * integer and defines how the token value is used. Additionally
 * the line number is associated to the token in order to improve
 * the produced error messages.
 */
typedef struct{
    token_type  type;
    token_value value;
    int         line_number;
} token;

/*
 * This is data structure type which is returned from lexical
 * analyzer. The list just contains all tokens from the input.
 */
typedef struct TOKEN_LIST{
    token             *value;
    struct TOKEN_LIST *next;
} token_list;

/*
 * This is mainly for debugging and testing. The user interface
 * will not print the token list.
 */
extern void printTokenList   (token_list *list);

/*
 * This function is used in the main program, to clean up the
 * error tokens before the parser is invoked.
 */
extern token_list *correctTokenList (token_list *list);

#endif
