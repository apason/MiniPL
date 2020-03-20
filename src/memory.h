#ifndef MEMORY_HEADER
#define MEMORY_HEADER

#include "tree.h"
#include "tokens.h"
#include "label.h"
/*
 * here is all declarations of memory related functions.
 */

// LEXICAL ANALYSIS ---------------------------------------------

token_list *addToken      (token_list *tl,  token_type type, char *buffer,   int line, FILE *input);
token      *newToken      (token_type type, char *value,     int line_number                      );
token_list *newTokenList  (void                                                                   );
void        freeTokenList (token_list *tl                                                         );

// SYNTAX ANALYSIS -----------------------------------------------

/* Functions to allocate memory. */
program_node             *newProgramNode            (void);
stmts_node               *newStmtsNode              (void);
statement_node           *newStatementNode          (void);
for_node                 *newForNode                (void);
declaration_node         *newDeclarationNode        (void);
declaration_suffix_node  *newDeclarationSuffixNode  (void);
assignment_node          *newAssignmentNode         (void);
expression_node          *newExpressionNode         (void);
unary_expression_node    *newUnaryExpressionNode    (void);
binary_expression_node   *newBinaryExpressionNode   (void);
operand_node             *newOperandNode            (void);
enclosed_expression_node *newEnclosedExpressionNode (void);
operand_suffix_node      *newOperandSuffixNode      (void);
assert_node              *newAssertNode             (void);
read_node                *newReadNode               (void);
print_node               *newPrintNode              (void);

/* Functions to deallocate memory. */
void freeSyntaxTree         (program_node             *pn, void *err);
void freeProgram            (program_node             *pn           );
void freeStmts              (stmts_node               *stmtln       );
void freeStatement          (statement_node           *stmtn        );
void freeFor                (for_node                 *forn         );
void freeDeclaration        (declaration_node         *decn         );
void freeDeclarationSuffix  (declaration_suffix_node  *asn          );
void freeAssignment         (assignment_node          *assn         );
void freeExpression         (expression_node          *expn         );
void freeUnaryExpression    (unary_expression_node    *uen          );
void freeBinaryExpression   (binary_expression_node   *ben          );
void freeOperand            (operand_node             *opn          );
void freeEnclosedExpression (enclosed_expression_node *expen        );
void freeOperandSuffix      (operand_suffix_node      *osn          );
void freeAssert             (assert_node              *asn          );
void freeRead               (read_node                *rn           );
void freePrint              (print_node               *pr           );


// SEMANTIC ANALYSIS ---------------------------------------------
label_list *newLabelListNode(label_list **list, label *l, value v);
void        freeLabelList   (label_list  *ll                     );

#endif
