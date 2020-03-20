#include <stdio.h>
#include <stdlib.h>

#include "tokens.h"
#include "tree.h"
#include "parser.h"
#include "memory.h"

/* Definitions for types used in the parser. */
enum consumption_types {CONSUME, NO_CONSUME};
enum discard_option    {AFTER_SEMICOLON, SEMICOLON};

/*
 * The following function declarations represents the non-terminals
 * in the grammar. Each of them returns the parse tree of that
 * particular substitution rule, or error if there is one.
 */
static program_node             *program            (void);
static stmts_node               *stmts              (void);
static statement_node           *statement          (void);
static for_node                 *for_               (void);
static declaration_node         *declaration        (void);
static assignment_node          *assignment         (void);
static declaration_suffix_node  *declarationSuffix  (void);
static expression_node          *expression         (void);
static unary_expression_node    *unaryExpression    (void);
static binary_expression_node   *binaryExpression   (void);
static operand_node             *operand            (void);
static operand_suffix_node      *operandSuffix      (void);
static enclosed_expression_node *enclosedExpression (void);
static assert_node              *assert             (void);
static read_node                *read               (void);
static print_node               *print              (void);


/* These are the declarations of the helper functions used in parser. */
static void                      printError         (token *t                          );
static void                      discardTokens      (enum discard_option o             );
static token                    *match              (token_type tt, consumption_type ct);


static token_list *global_tlist;  // Points to the next unhandled token.

/*
 * Some non-terminals in the grammar can be substituted to epsilon.
 * The functions corresponding those non-terminals returns NULL if 
 * the rule is expanded to epsilon. For this reason the NULL value
 * can not be used as an error indicator.
 *
 * This problem is resolved by using the specific error pointer
 * called "error". The integer "errorv" only serves to allocate
 * memory for that pointer.
 */
static int errorv;
static void *error = &errorv;

static int errors_found = 0;      // Indicator for errorneous program.

/*
 * This is the "interface" of the parser. It is the only non-static
 * function in this translation unit.
 *
 * Input parameter tlist is pointer to token list which represents
 * the input program. In success the pointer to the parse tree is returned.
 * In error, the return value from the program_node is NULL.
 */
		   
program_node *parse(token_list *tlist){
    global_tlist = tlist;
    
    /* This is for passing error pointer value to memory.o */
    freeSyntaxTree(NULL, error);

    return program();
}


/*
 * This parser is so called recursive descent parser. it works
 * producing the parse tree from root to leaves, from left to right.
 *
 * There is function for every nonterminal in the grammar. Every function
 * could call to function match() many times. Each of them represents
 * one element in predict set.
 * 
 * Because of the LL(1):ness of the grammar, the parser could have been
 * written to work in O(n) and it obviously was. There is no backtracking
 * so the program works in linear time.
 *
 * NOTE! There is "epsilon token" needed in first-sets of some nonterminals.
 * Those are actually no tokens. It only indicates that we can continue
 * parsing if the next token is in the follow set of that nonterminal.
 *
 * If EPS(nonterminal), then the corresponding function does not return 
 * error at any situation.
 *
 */

/*
 * The following functions each corresponds to the grammar rule of the
 * same name. (See the specification of the grammar from documentation.)
 * If the grammar rule at hand can not be parsed, the error pointer is
 * returned. In success, the parse tree of that grammar rule is returned
 * (or NULL, if the parse tree corresponds to an epsilon).
 *
 * NOTE! The function stmts() does not necessarily return error
 * although one occurs. See the comments in that function.
 */

static program_node *program(void){
    program_node *pn = newProgramNode();

    if((pn->sln = stmts()) != error)
	if((pn->eof = match(TOKEN_EOF, CONSUME)) != NULL){

	    /*
	     * Errors can also be indicated with the variable errors_found. 
	     * For more information see the comments in stmts() function.
	     */
	    if(errors_found){
		freeProgram(pn);
		return NULL;
	    } else 
		return pn;
	}

    fprintf(stderr, "Syntax  error in line %3d: Unexpected token %s\n", global_tlist->value->line_number, global_tlist->value->value);
    freeProgram(pn);
    return NULL;
}

static stmts_node *stmts(void){
    stmts_node *sln = newStmtsNode();
    token *t;

    if((t = match(TOKEN_VARKEY,     NO_CONSUME))  != NULL ||
       (t = match(TOKEN_IDENTIFIER, NO_CONSUME))  != NULL ||
       (t = match(TOKEN_FORKEY,     NO_CONSUME))  != NULL ||
       (t = match(TOKEN_READKEY,    NO_CONSUME))  != NULL ||
       (t = match(TOKEN_PRINTKEY,   NO_CONSUME))  != NULL ||
       (t = match(TOKEN_ASSERTKEY,  NO_CONSUME))  != NULL
       ){

	/*
	 * If the statement() function returns an error, the parser tries to recover
	 * and keep parsing the input. The strategy is to find the next semicolon
	 * and start parsing from there. At this situation the error indicator
	 * errors_found is set but no errors are returned. The error indicator
	 * is checked before returning from program() function.
	 */
	if((sln->stmtn = statement()) == error){
	    errors_found = 1;
	    printError(t);
	    discardTokens(SEMICOLON);
	}
	
	/*
	 * In case of the statement is correctly parsed but there is no semicolon
	 * right after it, the parsers assumes that the semicolon may just be forgotten.
	 * The error indicator is set and the parsing is continued.
	 */
	if((sln->sCol = match(TOKEN_SCOL, CONSUME)) == NULL){
	    errors_found = 1;
	    fprintf(stderr, "Syntax  error in line %3d: Expected semicolon.\n", t->line_number);
	    discardTokens(AFTER_SEMICOLON);
	}

	if((sln->stmtsn = stmts()) != error)
	    return sln;
	    
	freeStmts(sln);
	return error;
	    
    }
    
    freeStmts(sln);
    return NULL;
}

static statement_node *statement(void){
    statement_node *stmtn = newStatementNode();
    
    if(match(TOKEN_VARKEY,          NO_CONSUME) != NULL){
	if((stmtn->decn = declaration()) != error)
	    return stmtn;
    }
    else if(match(TOKEN_IDENTIFIER, NO_CONSUME) != NULL){
	if((stmtn->assn = assignment())  != error)
	    return stmtn;
    }
    
    else if(match(TOKEN_FORKEY,     NO_CONSUME) != NULL){
	if((stmtn->forn = for_())        != error)
	    return stmtn;
    }

    else if(match(TOKEN_READKEY,    NO_CONSUME) != NULL){
	if((stmtn->readn = read())       != error)
	    return stmtn;
    }

    else if(match(TOKEN_PRINTKEY,   NO_CONSUME) != NULL){
	if((stmtn->printn = print())     != error)
	    return stmtn;
    }

    else if(match(TOKEN_ASSERTKEY,  NO_CONSUME) != NULL){
	if((stmtn->assertn = assert())   != error)
	    return stmtn;
    }
    
    freeStatement(stmtn);
    return error;
}

static for_node *for_(void){
    for_node *forn = newForNode();

    if((forn->forKeyStart                                   = match     (TOKEN_FORKEY,     CONSUME)) != NULL )
	if((forn->id                                        = match     (TOKEN_IDENTIFIER, CONSUME)) != NULL )
	    if((forn->inKey                                 = match     (TOKEN_INKEY,      CONSUME)) != NULL )
		if((forn->expn1                             = expression()                         ) != error)
		    if((forn->range                         = match     (TOKEN_RANGE,      CONSUME)) != NULL )
			if((forn->expn2                     = expression()                         ) != error)
			    if((forn->doKey                 = match     (TOKEN_DOKEY,      CONSUME)) != NULL )
				if((forn->stmtsn            = stmts     ()                         ) != error)
				    if((forn->endKey        = match     (TOKEN_ENDKEY,     CONSUME)) != NULL )
					if((forn->forKeyEnd = match     (TOKEN_FORKEY,     CONSUME)) != NULL )
					    return forn;
    freeFor(forn);
    return error;
}

static declaration_node *declaration(void){
    declaration_node *decn = newDeclarationNode();

    if((decn->varKey                                        = match(TOKEN_VARKEY,          CONSUME)) != NULL ){
	if((decn->id                                        = match(TOKEN_IDENTIFIER,      CONSUME)) != NULL )
	    if((decn->col                                   = match(TOKEN_COL,             CONSUME)) != NULL )
		if((decn->typeKey                           = match(TOKEN_TYPEKEY,         CONSUME)) != NULL )
		    if((decn->asn                           = declarationSuffix()                  ) != error)
			return decn;
    }
    	
    freeDeclaration(decn);
    return error;
}


static assignment_node *assignment(void){
    assignment_node *assn = newAssignmentNode();

    if((assn->id                                            = match(TOKEN_IDENTIFIER,      CONSUME)) != NULL ){
	if((assn->assOp                                     = match(TOKEN_ASSIGN,          CONSUME)) != NULL )
	    if((assn->expn                                  = expression()                         ) != error)
		return assn;
    }

    freeAssignment(assn);
    return error;
}


static declaration_suffix_node *declarationSuffix(void){
    declaration_suffix_node *asn = newDeclarationSuffixNode();
    
    if((asn->ass                                            = match(TOKEN_ASSIGN,          CONSUME)) != NULL ){
	if((asn->expn                                       = expression()                         ) != error)
	    return asn;
	else{
	    freeDeclarationSuffix(asn);
	    return error;
	}
    }
    freeDeclarationSuffix(asn);
    return NULL;
}

static expression_node *expression(void){
    expression_node *expn = newExpressionNode();
    
    if((                                                      match(TOKEN_UN_OP,        NO_CONSUME)) != NULL ){
	if((expn->unaryen                                   = unaryExpression()                    ) != error)
	    return expn;
    }
    else if ((expn->binaryen                                = binaryExpression()                   ) != error)
	return expn;

    freeExpression(expn);
    return error;
}

static unary_expression_node *unaryExpression(void){
    unary_expression_node *uexpn = newUnaryExpressionNode();

    if((uexpn->unop                                         = match(TOKEN_UN_OP,           CONSUME)) != NULL )
	if((uexpn->opern                                    = operand()                            ) != error)
	    return uexpn;

    freeUnaryExpression(uexpn);
    return error;
}

static binary_expression_node *binaryExpression(void){
    binary_expression_node *bexpn = newBinaryExpressionNode();

    if((bexpn->opern                                        = operand()                            ) != error)
	if((bexpn->osn                                      = operandSuffix()                      ) != error)
	    return bexpn;

    freeBinaryExpression(bexpn);
    return error;
}

static operand_node *operand(void){
    operand_node *operandn = newOperandNode();

    if((operandn->intLit                                    = match(TOKEN_INT_LITERAL,     CONSUME)) != NULL )
	return operandn;
    if((operandn->strLit                                    = match(TOKEN_STRING_LITERAL,  CONSUME)) != NULL )
	return operandn;
    if((operandn->id                                        = match(TOKEN_IDENTIFIER,      CONSUME)) != NULL )
	return operandn;
    if((operandn->expren                                    = enclosedExpression()                 ) != error)
	return operandn;

    freeOperand(operandn);
    return error;
}

static operand_suffix_node *operandSuffix(void){
    operand_suffix_node *osn = newOperandSuffixNode();

    if((osn->op                                             = match(TOKEN_BIN_OP,          CONSUME)) != NULL ){
	if((osn->opn                                        = operand()                            ) != error)
	    return osn;

	return error;
    }
    
    freeOperandSuffix(osn);
    return NULL;
}

static enclosed_expression_node *enclosedExpression(void){
    enclosed_expression_node *expen = newEnclosedExpressionNode();

    if((expen->lPar                                         = match(TOKEN_LPAR,            CONSUME)) != NULL )
	if((expen->expn                                     = expression()                         ) != error)
	    if((expen->rPar                                 = match(TOKEN_RPAR,            CONSUME)) != NULL )
		return expen;

    freeEnclosedExpression(expen);
    return error;
}

static assert_node *assert(void){
    assert_node *assertn = newAssertNode();

    if((assertn->assert                                    = match(TOKEN_ASSERTKEY,        CONSUME)) != NULL )
	if((assertn->lPar                                  = match(TOKEN_LPAR,             CONSUME)) != NULL )
	    if((assertn->expn                              = expression()                          ) != error)
		if((assertn->rPar                          = match(TOKEN_RPAR,             CONSUME)) != NULL )
		    return assertn;

    freeAssert(assertn);
    return error;
}

static read_node *read(void){
    read_node *readn = newReadNode();

    if((readn->read                                        = match(TOKEN_READKEY,          CONSUME)) != NULL)
	if((readn->id                                      = match(TOKEN_IDENTIFIER,       CONSUME)) != NULL)
	    return readn;

    freeRead(readn);
    return error;
}

static print_node *print(void){
    print_node *printn = newPrintNode();

    if((printn->print                                      = match(TOKEN_PRINTKEY,         CONSUME)) != NULL )
	if((printn->expn                                   = expression()                          ) != error)
	    return printn;

    freePrint(printn);
    return error;
}

/*
 * This function checks whether the next token in the
 * stream is that the caller is looking for. Parameter
 * tt is the token and ct tells whether it is "consumed"
 * or not. Consuming means that we move to next token
 * in the list.
 */
static token *match(token_type tt, consumption_type ct){
    token *t;
    
    if(global_tlist == NULL || global_tlist->value == NULL)
	return NULL;
    
    if(global_tlist->value->type == tt){
	t = global_tlist->value;

	if(ct == CONSUME)
	    global_tlist = global_tlist->next;
	
	return t;
    } 
    return NULL;
}

/*
 * This function is used to discard all following tokens
 * to the next semicolon. This way the syntax analyzing 
 * can be kept going and multiple errors can be found
 * at a single run.
 */
static void discardTokens(enum discard_option o){
    
    for(;global_tlist->value->type != TOKEN_SCOL &&
	 global_tlist->value->type != TOKEN_EOF  ;
         global_tlist = global_tlist->next)      ;

    if(o == AFTER_SEMICOLON)
	for(;global_tlist->value->type == TOKEN_SCOL;
	     global_tlist = global_tlist->next)     ;

}

/*
 * This function prints the error information associated to the token.
 */
static void printError(token *t){

    fprintf(stderr, "Syntax  error in line %3d: Invalid ", t->line_number);
    
    switch(t->type){
    case TOKEN_VARKEY:
	fprintf(stderr, "declaration");
	break;
    case TOKEN_IDENTIFIER:
	fprintf(stderr, "assignment");
	break;
    case TOKEN_FORKEY:
	fprintf(stderr, "for");
	break;
    case TOKEN_READKEY:
	fprintf(stderr, "read");
	break;
    case TOKEN_PRINTKEY:
	fprintf(stderr, "print");
	break;
    case TOKEN_ASSERTKEY:
	fprintf(stderr, "assert");
	break;
    default:
	fprintf(stderr, "type %d", (int)t->type);
    }

    fprintf(stderr, " statement.\n");
 }
