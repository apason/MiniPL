#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokens.h"
#include "tree.h"
#include "label.h"
#include "memory.h"


/*
 * Helper functions used only in this translation unit.
 */
static int         insert             (token *id, value v        );
static int         update             (token *id, value new_value);
static void        forceUpdate        (token *id, value new_value);
static int         isConstant         (token *id                 );
static label_type  findLabelType      (token *id                 );
static value       findLabelValue     (token *id                 );
static int         getIntValue        (char  *data               );
static label_list *findLabel          (token *id                 );
static void        printValue         (value  v                  );


/*
 * The following function declarations represents the non-terminals
 * in the grammar. Each of them returns 0 if error is encountered.
 * 1 otherwise.
 */
static int   program            (program_node             *pn     );
static int   stmts              (stmts_node               *stmtsn );
static int   statement          (statement_node           *stmtn  );
static int   for_               (for_node                 *forn   );
static int   declaration        (declaration_node         *decn   );
static int   assignment         (assignment_node          *assn   );
static value declarationSuffix  (declaration_suffix_node  *asn    );
static value expression         (expression_node          *expn   );
static value unaryExpression    (unary_expression_node    *uen    );
static value binaryExpression   (binary_expression_node   *ben    );
static value operand            (operand_node             *opn    );
static value operandSuffix      (operand_suffix_node      *osn    );
static value enclosedExpression (enclosed_expression_node *een    );
static int   assert             (assert_node              *assertn);
static int   read               (read_node                *readn  );
static int   print              (print_node               *printn );

/*
 * Definition of type error_type and declaration od printError()
 */
enum error_type {SEMANTIC_ERROR, RUNTIME_ERROR};
static void printError (token *t,  char *message, enum error_type et );

/*
 * To avoid passing head of the label list to every function, 
 * its address is kept in global variable global_list.
 */
static label_list *global_list;

/*
 * The template values used in the functions.
 */
static value default_value = {0, 0, NULL, 0, 0, 0, 0};
static value empty_value   = {0, 0, NULL, 1, 0, 0, 0};
static value error_value   = {0, 0, NULL, 0, 1, 0, 0};

/*
 * Main function of semantic analysis and running the interpreter.
 * Semantics checking includes that there is no use of undefined
 * variables and no variables can be defined multiple times. Also 
 * checks for type correctness.
 *
 * Input parameter *pn is pointer to syntax tree.
 * Returns 1 if there was no errors, 0 otherwise.
 */
int run(program_node *pn){
    int tmp;
    
    global_list = NULL;

    tmp = program(pn);

    freeLabelList(global_list);
    freeSyntaxTree(pn, NULL);

    return tmp;
}

/*
 * There are function for every non-leaf node in the parse
 * tree. If there is a semantic error, 0 is returned,
 * 1 otherwise.
 */

static int program(program_node *pn){
    return stmts(pn->sln);
}

static int stmts(stmts_node *stmtsn){

    if(stmtsn == NULL) return 1;

    return statement(stmtsn->stmtn) && stmts(stmtsn->stmtsn);
}

static int statement(statement_node *stmtn){

    if( declaration(stmtn->decn) &&
	assignment(stmtn->assn)  &&
	for_(stmtn->forn)        &&
	read(stmtn->readn)       &&
	print(stmtn->printn)     &&
	assert(stmtn->assertn))

	return 1;

    return 0;
}

/*
 * The for statement may have the following semantic errors:
 * Either or both of the range expressions may not be integers
 * and for control variable may not be integer.
 */
static int for_(for_node *forn){

    if(forn == NULL) return 1;

    if(findLabelType(forn->id) != INT){
	printError(forn->id, "For variable should be integer", SEMANTIC_ERROR);
	return 0;
    }

    value range_start = expression(forn->expn1);
    value range_end   = expression(forn->expn2);

    if(range_start.lt != INT || range_end.lt != INT){
	printError(forn->id, "For range should be integer", SEMANTIC_ERROR);
	return 0;
    }

    value counter = default_value;

    /* The control variable is integer by definition. */
    counter.lt = INT;
    counter.constant = 1;
    
    for(counter.i = range_start.i; counter.i <= range_end.i; counter.i++){
	forceUpdate(forn->id, counter);
	if(stmts(forn->stmtsn) == 0)
	    return 0;
    }
    
    counter.constant = 0;
    forceUpdate(forn->id, counter);

    return 1;
}

/*
 * The declaration statement must check that the symbol to be
 * declared does not exist in the symbol table already.
 *
 * The type check of the expression and initial variable must
 * also be done in case the variable is explicitly initialized.
 */
static int declaration(declaration_node *decn){

    if(decn == NULL)          return 1;

    label_type expected;
    if(strncmp(decn->typeKey->value,      "int",    TOKEN_MAX_LENGTH) == 0)
	expected = INT;
    else if(strncmp(decn->typeKey->value, "string", TOKEN_MAX_LENGTH) == 0)
	expected = STRING;
    else if(strncmp(decn->typeKey->value, "bool",   TOKEN_MAX_LENGTH) == 0)
	expected = BOOL;

    value v = declarationSuffix(decn->asn);

    if(v.empty){
	v = default_value;
	v.lt = expected;
	if(expected == STRING){
	    v.s = malloc(sizeof(char));
	    v.s[0] = '\0';
	}
    }
    
    if(v.lt != expected){
	printError(decn->id, "Incompatible types in declaration", SEMANTIC_ERROR);
	return 0;
    }

    if(insert(decn->id, v) == 0){
	char msg[TOKEN_MAX_LENGTH + 25];
	sprintf(msg, "Redeclaration of symbol %s", decn->id->value);
	printError(decn->id, msg, SEMANTIC_ERROR);
	return 0;
    }

    return 1;
}

static value declarationSuffix(declaration_suffix_node *asn){
    if(asn == NULL) return empty_value;
    
    return expression(asn->expn);
}

/*
 * The assignment statement must perform the checks for
 * type compatibility and ensure the variable is declared.
 */
static int assignment(assignment_node *assn){

    if(assn == NULL)        return 1;
    
    label_type lt = findLabelType(assn->id);
    if(lt == UNDEF){
	char msg[TOKEN_MAX_LENGTH +20];
	sprintf(msg, "Undefined variable %s", assn->id->value);
	printError(assn->id, msg, SEMANTIC_ERROR);
	return 0;
    }


    value v = expression(assn->expn);

    if(lt != v.lt){
	printError(assn->id, "Incompatible types in assignment", SEMANTIC_ERROR);
	return 0;
    }

    if(v.error != 1 && v.empty != 1)
	return update(assn->id, v);
    
    return 0;
}

static value expression(expression_node *expn){

    if(expn == NULL) return empty_value;

    value uexp = unaryExpression(expn->unaryen);

    if (uexp.empty != 1)
	return uexp;

    value v = binaryExpression(expn->binaryen);

    return v;
}

/*
 * The unary expression must check that the argument is
 * of type bool.
 */
static value unaryExpression(unary_expression_node *uen){

    if(uen == NULL) return empty_value;

    if(uen->unop){
	value v = operand(uen->opern);
	if(v.lt != BOOL){
	    printError(uen->unop, "The argument type of unary expression must be bool", SEMANTIC_ERROR);
	    return error_value;
	}
	v.b ^= 1;
	return v;
    }

    return operand(uen->opern);
}

/*
 * The binary expression has a lot of error checks that must be done.
 * Some of the binary operators can only be used with specific
 * data types. In addition all binary operators can only operate
 * with the same data types.
 */
static value binaryExpression(binary_expression_node *ben){

    if(ben == NULL) return empty_value;

    value suffix = operandSuffix(ben->osn);
    value oper   = operand(ben->opern);

    suffix.constant = 0;
    oper.constant = 0;

    if(oper.empty == 1 || suffix.error == 1 || oper.error == 1)
	return error_value;

    if(suffix.empty != 1){
	if(suffix.lt != oper.lt){
	    printError(ben->osn->op, "Mismatched types in expression", SEMANTIC_ERROR);
	    return error_value;
	}
	
	if(strncmp(ben->osn->op->value, "+", 1) == 0){
	    
	    if(suffix.lt == INT){
		suffix.i += oper.i;
		return suffix;
	    } else if(suffix.lt == STRING){
		oper.s = realloc(oper.s, strlen(suffix.s) + strlen(oper.s));
		strcat(oper.s, suffix.s);
	    } else{
		printError(ben->osn->op, "Trying to use addition operator with boolean values", SEMANTIC_ERROR);
		return error_value;
	    }
	} else if(strncmp(ben->osn->op->value, "-", 1) == 0){
	    if(suffix.lt == INT){
		suffix.i = oper.i - suffix.i;
		return suffix;
	    } else{
		printError(ben->osn->op, "Trying to use subtraction operator with non integer values", SEMANTIC_ERROR);
		return error_value;
	    }
	} else if(strncmp(ben->osn->op->value, "*", 1) == 0){
	    if(suffix.lt == INT){
		suffix.i *= oper.i;
		return suffix;
	    } else{
		printError(ben->osn->op, "Trying to use multiplication operator with non integer values", SEMANTIC_ERROR);
		return error_value;
	    }
	} else if(strncmp(ben->osn->op->value, "/", 1) == 0){
	    if(suffix.i == 0){
		printError(ben->osn->op, "Division by zero", RUNTIME_ERROR);
		return error_value;
	    }
	    if(suffix.lt == INT){
		suffix.i = oper.i / suffix.i;
		return suffix;
	    } else{
		printError(ben->osn->op, "trying to use division operator with non integer values", SEMANTIC_ERROR);
		return error_value;
	    }
	} else if(strncmp(ben->osn->op->value, "&", 1) == 0){
	    if(suffix.lt == BOOL){
		suffix.b &= oper.b;
		return suffix;
	    } else{
		printError(ben->osn->op, "Trying to use logical and operator with non boolean values", SEMANTIC_ERROR);
		return error_value;
	    }
	} else if(strncmp(ben->osn->op->value, "<", 1) == 0){
	    if(suffix.lt == UNDEF){
		printError(ben->osn->op, "Trying to use boolean operator < with non boolean values", SEMANTIC_ERROR);
		return error_value;
	    }
	    switch(suffix.lt){
	    case INT:
		if(oper.i < suffix.i)
		    suffix.b = 1;
		else
		    suffix.b = 0;
		break;
	    case STRING:
		if(strcmp(oper.s, suffix.s) < 0)
		    suffix.b = 1;
		else
		    suffix.b = 0;
		break;
	    case BOOL:
		if(oper.b < suffix.b)
		    suffix.b = 1;
		else
		    suffix.b = 0;
		break;
	    }
	    suffix.lt = BOOL;
	    return suffix;
	    
	} else if(strncmp(ben->osn->op->value, "=", 1) == 0){
	    if(suffix.lt == UNDEF){
		printError(ben->osn->op, "Trying to compare types with undefined types", SEMANTIC_ERROR);
		return error_value;
	    }
	    switch(suffix.lt){
	    case INT:
		if(oper.i == suffix.i)
		    suffix.b = 1;
		else
		    suffix.b = 0;
		break;
	    case STRING:
		if(strcmp(suffix.s, oper.s) == 0)
		    suffix.b = 1;
		else
		    suffix.b = 0;
		break;
	    case BOOL:
		if(oper.b == suffix.b)
		    suffix.b = 1;
		else
		    suffix.b = 0;
		break;
	    }
	    suffix.lt = BOOL;
	    return suffix;
	}
    }

    return oper;
}

static value operandSuffix(operand_suffix_node *osn){
    if(osn == NULL) return empty_value;

    return operand(osn->opn);
}

static value operand (operand_node *opn){

    value v = enclosedExpression(opn->expren);

    if(v.empty != 1)
	return v;

    v = default_value;

    if(opn->intLit != NULL){
	v.lt = INT;
	v.i = getIntValue(opn->intLit->value);
    } else if(opn->strLit != NULL){
	v.lt = STRING;
	v.s = (char*)malloc(sizeof(char)*strlen(opn->strLit->value)+1);
	memset(v.s, '\0', strlen(opn->strLit->value) +1);
	strcpy(v.s, opn->strLit->value);
    } else {
	v = findLabelValue(opn->id);

	if(v.empty)
	    v = error_value;
    }

    return v;
}

static value enclosedExpression(enclosed_expression_node *een){
    if(een == NULL) return empty_value;

    return expression(een->expn);
}

static int assert(assert_node *assertn){

    if(assertn == NULL) return 1;
    value v = expression(assertn->expn);

    if(v.b != 1){
	printError(assertn->assert, "Assertion failed", SEMANTIC_ERROR);
	return 0;
    }

    return 1;
}

static int read(read_node *readn){

    if(readn == NULL) return 1;
    value v = default_value;
    v.lt = findLabelType(readn->id);

    char tmp[512];
    
    if(v.lt == UNDEF){
	printError(readn->id, "Undefined label in read statement", SEMANTIC_ERROR);
	return 0;
    }

    switch(v.lt){
    case INT:
	if(scanf("%d", &(v.i)) != 1){
	    printError(readn->id, "Failed to read integer", RUNTIME_ERROR);
	    return 0;
	}
	return update(readn->id, v);

    case STRING:
	if(scanf("%s", tmp) != 1){
	    printError(readn->id, "Failed to read string", RUNTIME_ERROR);
	    return 0;
	}
	v.s = (char*)malloc(sizeof(char)*512);
	strncpy(v.s, tmp, 512);
	return update(readn->id, v);

    default:
	printError(readn->id, "Cannot read boolean value", RUNTIME_ERROR);
	return 0;
    }
}

static int print(print_node *printn){

    if(printn == NULL) return 1;
    value v = expression(printn->expn);

    if(v.error == 1 || v.empty == 1 || (v.lt != STRING && v.lt != INT)){
	printError(printn->print, "Invalid value in printable expression", RUNTIME_ERROR);
	return 0;
    }

    /*
     * The error message is casted by the function findLabel()
     */
    if (v.empty == 1)
	return 0;

    
    printValue(v);
    return 1;
}

/*
 * Updates the value of the symbol if it is permitted.
 */
static int update(token *id, value new_value){
    if(isConstant(id)){
	printError(id, "Cannot modify the loop control variable", SEMANTIC_ERROR);
	return 0;
    }

    forceUpdate(id, new_value);

    return 1;
}

/*
 * Updates the value of the symbol even when its constant indicator is set.
 */
static void forceUpdate(token *id, value new_value){
    for(label_list *tmp = global_list; tmp != NULL; tmp = tmp->next)
	if(strncmp((char*)tmp->l, id->value, TOKEN_MAX_LENGTH +1) == 0){
	    tmp->v = new_value;
	    return;
	}
}

/*
 * Checks the constant indicator associated to the symbol in token *id.
 * If the constant indicator is set return 1, otherwise return 0.
 */
static int isConstant(token *id){
    value v = findLabelValue(id);
    if(v.constant == 0)
	return 0;
    return 1;
}

/*
 * Inserts a new symbol to the symbol list. 
 * Checks for redeclaration.
 *
 * Note that the possible error is printed from the function that
 * called insert(). No error messages is generated here.
 */
static int insert(token *id, value v){
    label_list *tmp = newLabelListNode(&global_list, (label*)id->value, v);
    label_list *prev;

    global_list = tmp;

    
    /* Check for duplicates */
    for(tmp  = global_list->next, prev = global_list;
	tmp != NULL;
	tmp  = tmp->next,         prev = prev->next)
	
	if(strncmp((char*)tmp->l, id->value, TOKEN_MAX_LENGTH +1) == 0){
	    prev->next = tmp->next;
	    free(tmp);
	    return 0;
	}
    
    return 1;
}

/*
 * Search value for label in *id from the variable list.
 * Return value is its type. UNDEF, INT, STRING or BOOL.
 */
static label_type findLabelType(token *id){
    label_list *l = findLabel(id);

    if(l == NULL)
	return UNDEF;

    return l->v.lt;
}

/*
 * Search value for label *id and return its value.
 */
static value findLabelValue(token *id){
    label_list *l = findLabel(id);
    
    if(l == NULL)
	return empty_value;

    return l->v;
}

/*
 * Returns a label corresponding to the given token.
 *
 * If the label is not found, prints the error message
 * and returns NULL.
 */
static label_list *findLabel(token *id){

    for(label_list *tmp = global_list; tmp != NULL; tmp = tmp->next){
	if(strncmp(id->value, (char*)tmp->l, TOKEN_MAX_LENGTH +1) == 0)
	    return tmp;
    }

    char msg[TOKEN_MAX_LENGTH +31];
    sprintf(msg, "Reference to unknown variable %s", id->value);
    printError(id, msg, SEMANTIC_ERROR);
    return NULL;
}

/*
 * Reads the integer value from the standard input.
 */
static int getIntValue (char *data){
    int tmp;
    sscanf(data, "%d", &tmp);
    return tmp;
}

/*
 * Prints the value of v.
 */
static void printValue(value v){
    switch(v.lt){
    case INT:
	printf("%d", v.i);
	break;
    case STRING:
	printf("%s", v.s);
	break;
    case BOOL:
	printf("BOOL: %s\n", v.b == 1 ? "True" : "False");
	break;
    default:
	printf("Value is invalid\n");
    }
}

/*
 * This function prints the error information associated to the token.
 */
static void printError(token *t, char *message, enum error_type et){

    if(et == SEMANTIC_ERROR)
	fprintf(stderr, "Semantic error in line %3d: %s.\n", t->line_number, message);
    else
	fprintf(stderr, "Runtime error  in line %3d: %s.\n", t->line_number, message);
    
 }

