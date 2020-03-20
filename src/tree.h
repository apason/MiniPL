#ifndef TREE_HEADER
#define TREE_HEADER

#include "tokens.h"

/*
 * In this file we have definition of each node in the
 * syntax tree. Tokens from the lexical analyzer are leaves
 * of the tree and each non-leaf node is one of the
 * following nonterminals. The root of the tree is always
 * program node which has the node stmts_node and a token EOF.
 */

/*
 * Type definitions of structures.
 */
typedef struct PROGRAM_NODE             program_node            ;
typedef struct STMTS_NODE               stmts_node              ;
typedef struct STATEMENT_NODE           statement_node          ;
typedef struct FOR_NODE                 for_node                ;
typedef struct DECLARATION_NODE         declaration_node        ;
typedef struct DECLARATION_SUFFIX_NODE  declaration_suffix_node ;
typedef struct ASSIGNMENT_NODE          assignment_node         ;
typedef struct EXPRESSION_NODE          expression_node         ;
typedef struct UNARY_EXPRESSION_NODE    unary_expression_node   ;
typedef struct BINARY_EXPRESSION_NODE   binary_expression_node  ;
typedef struct OPERAND_NODE             operand_node            ;
typedef struct ENCLOSED_EXPRESSION_NODE enclosed_expression_node;
typedef struct OPERAND_SUFFIX_NODE      operand_suffix_node     ;
typedef struct ASSERT_NODE              assert_node             ;
typedef struct READ_NODE                read_node               ;
typedef struct PRINT_NODE               print_node              ;

/*
 * Definitons:
 */
struct PROGRAM_NODE{
     stmts_node               *sln        ;
     token                    *eof        ;
    
};

struct STMTS_NODE{
    statement_node            *stmtn      ;
    token                     *sCol       ;
    stmts_node                *stmtsn     ;
};

struct STATEMENT_NODE{
    declaration_node          *decn       ;
    assignment_node           *assn       ;
    for_node                  *forn       ;
    read_node                 *readn      ;
    print_node                *printn     ;
    assert_node               *assertn    ;
    
};

struct FOR_NODE{
    token                     *forKeyStart;
    token                     *id         ;
    token                     *inKey      ;
    expression_node           *expn1      ;
    token                     *range      ;
    expression_node           *expn2      ;
    token                     *doKey      ;
    stmts_node                *stmtsn     ;
    token                     *endKey     ;
    token                     *forKeyEnd  ;
};

struct DECLARATION_NODE{
    token                     *varKey     ;
    token                     *id         ;
    token                     *col        ;
    token                     *typeKey    ;
    declaration_suffix_node   *asn        ;
};

struct DECLARATION_SUFFIX_NODE {
    token                     *ass        ;
    expression_node           *expn       ;
};

struct ASSIGNMENT_NODE{
     token                    *id         ;
     token                    *assOp      ;
     expression_node          *expn       ;
};

struct EXPRESSION_NODE{
    unary_expression_node     *unaryen    ;
    binary_expression_node    *binaryen   ;
};

struct UNARY_EXPRESSION_NODE{
    token                     *unop       ;
    operand_node              *opern      ;
};

struct BINARY_EXPRESSION_NODE{
    operand_node              *opern      ;
    operand_suffix_node       *osn        ;
};

struct OPERAND_NODE{
    token                     *intLit     ;
    token                     *strLit     ;
    token                     *id         ;
    enclosed_expression_node  *expren     ;
};

struct ENCLOSED_EXPRESSION_NODE{
    token                     *lPar       ;
    expression_node           *expn       ;
    token                     *rPar       ;
};

struct OPERAND_SUFFIX_NODE{
    token                     *op         ;
    operand_node              *opn        ;
};

struct ASSERT_NODE{
    token                     *assert     ;
    token                     *lPar       ;
    expression_node           *expn       ;
    token                     *rPar       ;
};

struct READ_NODE{
    token                     *read       ;
    token                     *id         ;
};

struct PRINT_NODE{
    token                     *print      ;
    expression_node           *expn       ;
};

#endif
