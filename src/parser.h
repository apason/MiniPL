#ifndef parser_header
#define parser_header

/*
 * This is needed for type token_list.
 */
#include "tokens.h"

/*
 * tree.h contains definition of program_node
 */
#include "tree.h"

/*
 * consumption_type can be either CONSUME or
 * NO_CONSUME. That value is used in function 
 * match() to determine whether the token is
 * consumed after it was checked.
 */
typedef unsigned int consumption_type;

/*
 * Main function of syntax analysis.
 * This acts as an interface of the parser.
 */
extern program_node *parse(token_list *tlist);

#endif
