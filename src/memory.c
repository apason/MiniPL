#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokens.h"
#include "tree.h"
#include "lex.h"
#include "label.h"
#include "memory.h"

/*
 * This pointer is used to indicate error value.
 * See the comment in parser.c
 */
static void *error;

/*
 * LEXICAL ANALYSIS ------------------------------------
 * The following functions are invoked from the scanner.
 */

/*
 * Return new token initialized by parameters.
 * This function expects that the type and the value are correct.
 */
token *newToken(token_type type, char *value, int line_number){
    token *t = (token *)malloc(sizeof(token));

    t->type = type;
    strncpy(t->value, value, TOKEN_MAX_LENGTH +1);
    t->line_number = line_number;

    return t;
}

/*
 * Returns new token_list node with all values initialized to NULL.
 */
token_list *newTokenList(void){
    token_list *tl = (token_list *)malloc(sizeof(token_list));

    tl->value = NULL;
    tl->next  = NULL;
    
    return tl;
}

/*
 * Deletes the token list structure. Note that actual tokens
 * are not deleted but they are not lost either. In parse()
 * funciton tokens are linked to the syntax tree and they are 
 * deleted after the semantic analysis.
 */
void freeTokenList(token_list *tl){
    token_list *tmp;
    
    while(tl != NULL){
	tmp = tl->next;
	free(tl);
	tl = tmp;
    }
}

/*
 * This function creates and adds new token to the end of
 * the token list. 
 * 
 * parameters:
 *   tl is a pointer to the end of the list.
 *   type is a token type, one of the macros defined in tokens.h.
 *   buffer is a pointer to the token value of the new token.
 *   input is pointer to the input stream.
 *
 * A pointer to the new node is returned.
 */
token_list *addToken(token_list *tl, token_type type, char *buffer, int line, FILE *input){

    tl->next  = (token_list *)malloc(sizeof(token_list));
    tl->value = newToken(type, buffer, line);
    tl        = tl->next;
    tl->next  = NULL;
    tl->value = NULL;

    /*
     * Here we have all tokens that can be added 
     * without any backtracking. That means we do
     * not need to ungetc the input stream.
     */
    if(type == TOKEN_BIN_OP                 ||
       type == TOKEN_ERROR                  ||
       type == TOKEN_LPAR                   ||
       type == TOKEN_RPAR                   ||
       type == TOKEN_SCOL                   ||
       type == TOKEN_UN_OP                  ||
       type == TOKEN_STRING_LITERAL         ||
       type == TOKEN_RANGE                  ||
       type == TOKEN_COL);
       
    else
	ungetc(input);
    
    return tl;
}


/*
 * SYNTACTIC ANALYSIS ---------------------------------
 * The following functions are invoked from the parser.
 */

/*
 * Functions to allocate memory.
 * The following functions just creates and initializes
 * nodes of the parse tree. All nonterminals have own
 * function. Note that memory area can not be set with
 * memset() because NULL is not guaranteed to be 0.
 */
program_node *newProgramNode(void){
    program_node *r =
	(program_node*)malloc(sizeof(program_node));

    r->sln = NULL;
    r->eof = NULL;

    return r;
}

stmts_node *newStmtsNode(void){
    stmts_node *r =
	(stmts_node*)malloc(sizeof(stmts_node));

    r->stmtn  = NULL;
    r->sCol   = NULL;
    r->stmtsn = NULL;

    return r;
}

statement_node *newStatementNode(void){
    statement_node *r =
	(statement_node*)malloc(sizeof(statement_node));

    r->decn    = NULL;
    r->assn    = NULL;
    r->forn    = NULL;
    r->readn   = NULL;
    r->printn  = NULL;
    r->assertn = NULL;

    return r;
}
    
for_node *newForNode(void){
    for_node *r =
	(for_node*)malloc(sizeof(for_node));

    r->forKeyStart = NULL;
    r->id          = NULL;
    r->inKey       = NULL;
    r->expn1       = NULL;
    r->range       = NULL;
    r->expn2       = NULL;
    r->doKey       = NULL;
    r->stmtsn      = NULL;
    r->endKey      = NULL;
    r->forKeyEnd   = NULL;

    return r;
}

declaration_node *newDeclarationNode(void){
    declaration_node *r =
	(declaration_node*)malloc(sizeof(declaration_node));

    r->varKey  = NULL;
    r->id      = NULL;
    r->col     = NULL;
    r->typeKey = NULL;
    r->asn     = NULL;

    return r;
}

declaration_suffix_node *newDeclarationSuffixNode(void){
    declaration_suffix_node *r =
	(declaration_suffix_node*)malloc(sizeof(declaration_suffix_node));

    r->ass  = NULL;
    r->expn = NULL;

    return r;
}

assignment_node *newAssignmentNode(void){
    assignment_node *r =
	(assignment_node*)malloc(sizeof(assignment_node));

    r->id    = NULL;
    r->assOp = NULL;
    r->expn  = NULL;

    return r;
}

expression_node *newExpressionNode(void){
    expression_node *r =
	(expression_node*)malloc(sizeof(expression_node));

    r->unaryen = NULL;
    r->binaryen = NULL;

    return r;
}

unary_expression_node *newUnaryExpressionNode(void){
    unary_expression_node *r =
	(unary_expression_node*)malloc(sizeof(unary_expression_node));

    r->unop = NULL;
    r->opern = NULL;

    return r;
}

binary_expression_node *newBinaryExpressionNode(void){
    binary_expression_node *r =
	(binary_expression_node*)malloc(sizeof(binary_expression_node));

    r->opern = NULL;
    r->osn = NULL;

    return r;
}

operand_node *newOperandNode(void){
    operand_node *r =
	(operand_node*)malloc(sizeof(operand_node));

    r->intLit = NULL;
    r->strLit = NULL;
    r->id     = NULL;
    r->expren = NULL;

    return r;
}

enclosed_expression_node *newEnclosedExpressionNode(void){
    enclosed_expression_node *r =
	(enclosed_expression_node*)malloc(sizeof(enclosed_expression_node));

    r->lPar = NULL;
    r->expn = NULL;
    r->rPar = NULL;

    return r;
}

operand_suffix_node *newOperandSuffixNode(void){
    operand_suffix_node *r =
	(operand_suffix_node*)malloc(sizeof(operand_suffix_node));

    r->op = NULL;
    r->opn = NULL;

    return r;
}

assert_node *newAssertNode(void){
    assert_node *r =
	(assert_node*)malloc(sizeof(assert_node));

    r->assert = NULL;
    r->lPar = NULL;
    r->expn = NULL;
    r->rPar = NULL;

    return r;
}

read_node *newReadNode(void){
    read_node *r =
	(read_node*)malloc(sizeof(read_node));

    r->read = NULL;
    r->id = NULL;

    return r;
}

print_node *newPrintNode(void){
    print_node *r =
	(print_node*)malloc(sizeof(print_node));

    r->print = NULL;
    r->expn = NULL;

    return r;
}

/*
 * Functions to free memory.
 * The following functions just deallocates the nodes of
 * the parse tree. This occurs recursively from root to
 * leaves. All nonterminals have own function.
 */
void freeSyntaxTree(program_node *pn, void *err){
    if(err != NULL)
	error = err;
    if(pn == NULL) return;
    freeProgram(pn);
}

void freeProgram(program_node *pn){
    if(pn == NULL || pn == error) return;

    freeStmts(pn->sln);
    
    free(pn);
}
void freeStmts(stmts_node *stmtln){
    if(stmtln == NULL || stmtln == error) return;

    freeStatement(stmtln->stmtn);
    freeStmts(stmtln->stmtsn);
    
    free(stmtln);
}
void freeStatement(statement_node *stmtn){
    if(stmtn == NULL || stmtn == error) return;

    freeDeclaration(stmtn->decn);
    freeAssignment(stmtn->assn);
    freeFor(stmtn->forn);
    freeRead(stmtn->readn);
    freePrint(stmtn->printn);
    freeAssert(stmtn->assertn);

    free(stmtn);
}

void freeFor(for_node *forn){
    if(forn == NULL || forn == error) return;

    freeExpression(forn->expn1);
    freeExpression(forn->expn2);
    freeStmts(forn->stmtsn);

    free(forn);
}
    
void freeDeclaration(declaration_node *decn){
    if(decn == NULL || decn == error) return;

    freeDeclarationSuffix(decn->asn);

    free(decn);
}

void freeDeclarationSuffix(declaration_suffix_node *asn){
    if(asn == NULL || asn == error) return;

    freeExpression(asn->expn);

    free(asn);
}

void freeAssignment(assignment_node *assn){
    if(assn == NULL || assn == error) return;

    freeExpression(assn->expn);

    free(assn);
}

void freeExpression(expression_node *expn){
    if(expn == NULL || expn == error) return;

    freeUnaryExpression(expn->unaryen);
    freeBinaryExpression(expn->binaryen);
    
    free(expn);
}

void freeUnaryExpression(unary_expression_node *uen){
    if(uen == NULL || uen == error) return ;

    freeOperand(uen->opern);

    free(uen);
}

void freeBinaryExpression(binary_expression_node *ben){
    if(ben == NULL || ben == error) return;

    freeOperand(ben->opern);
    freeOperandSuffix(ben->osn);

    free(ben);
}

void freeOperand(operand_node *opn){
    if(opn == NULL || opn == error) return;

    freeEnclosedExpression(opn->expren);

    free(opn);
}

void freeEnclosedExpression(enclosed_expression_node *expen){
    if(expen == NULL || expen == error) return;

    freeExpression(expen->expn);

    free(expen);
}

void freeOperandSuffix(operand_suffix_node *osn){
    if(osn == NULL || osn == error) return;

    freeOperand(osn->opn);

    free(osn);
}

void freeAssert(assert_node *asn){
    if(asn == NULL || asn == error) return;

    freeExpression(asn->expn);

    free(asn);
}

void freeRead(read_node *rn){
    if(rn == NULL || rn == error) return;

    free(rn);
}

void freePrint(print_node *pr){
    if(pr == NULL || pr == error) return;

    freeExpression(pr->expn);

    free(pr);
}


/*
 * SEMANTIC ANALYSIS -------------------------------------------------
 * The following functions are invoked only from the semantic analyzer.
 *
 * The label list related functions are only used when interpreting.
 */

label_list *newLabelListNode(label_list **list, label *l, value v){
    label_list *new = (label_list*)malloc(sizeof(label_list));

    new->v    = v;
    new->l    = l;
    new->next = *list;

    return new;
}

void freeLabelList(label_list *ll){
    label_list *tmp;

    while(ll != NULL){
	tmp = ll->next;
	free(ll);
	ll = tmp;
    }
}

