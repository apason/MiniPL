#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "tokens.h"
#include "memory.h"

/*
 * Function declarations for the scanner.
 * These are all static methods only invoked
 * within this translation unit.
 */
static token_list *handleOthers            (token_list *tl, char *buffer, FILE *input);
static token_list *handleSlash             (token_list *tl, char *buffer, FILE *input);
static token_list *handleErrors            (token_list *tl, char *buffer, FILE *input);
static token_list *handleCol               (token_list *tl, char *buffer, FILE *input);
static token_list *handleIntLiterals       (token_list *tl, char *buffer, FILE *input);
static token_list *handleStringLiterals    (token_list *tl, char *buffer, FILE *input);
static token_list *handlePeriod            (token_list *tl, char *buffer, FILE *input);
static int         isLastTokenControlError (token_list *list                         );

/* Functions used to separate keywords from identifiers. */
static int        isTypeKey      (char *word);
static int        isForKey       (char *word);
static int        isVarKey       (char *word);
static int        isEndKey       (char *word);
static int        isInKey        (char *word);
static int        isDoKey        (char *word);
static int        isReadKey      (char *word);
static int        isPrintKey     (char *word);
static int        isAssertKey    (char *word);

static int        equals         (char *a, char *b);

static void       addEOF         (token_list *tl);


/* This external (global) variable is used to keep track
 * of the line number of the input file. It is incremented
 * every time when a new line is read.
 */
static int        line_number;


/*
 * This is the main function of lexical analyzer. it is called
 * from the main function of a program.
 *
 * Variable *input is the FILE pointer which contains the input
 * file. The file should be already correctly opened so the main
 * function should handle possible error situations.
 *
 * Return value is type of token_list *. It contains all text
 * from the file (excluding comments) divided into tokens of
 * 21 type defined in tokens.h. If there is text sequence that
 * does not match with any valid token type, the token is
 * returned with type TOKEN_ERROR.
 */
token_list *lex(FILE *input){
    token_list *tmp, *head, *tl = newTokenList();  // Handling of token list.
    char c, buffer[TOKEN_MAX_LENGTH +1];           // Input handling and buffering.
    line_number = 1;                               // Initialize line counter.

    /* As the tokens are appended to the end of the token list, only the pointer
     * to the last element (tl) is used. The head is saved in the start of the
     * scanner for not losing the start of the token list. After the scanning is
     * done, the head is returned.
     */
    head = tl;

    /*
     * This is the main loop of lexer. All tokening happens here.
     * There is cases for every element group which is handled 
     * similarly. 
     */
    while(fread(&c, sizeof(char), 1, input) == sizeof(char)){

	memset(buffer, '\0', TOKEN_MAX_LENGTH +1); // Empty the buffer
	
	switch (c){
	    
       /*
        * This and the next 4 groups are the only groups of items that can
        * be returned directly after founding. There is no need to read next
        * characters and thus no need to ungetc it.
        */
	case '+': case '-': case '*': case '=': case '<': case '&':           // Binary operators
	    buffer[0] = c;
	    tl = addToken(tl, TOKEN_BIN_OP, buffer, line_number, input);
	    break;

	case '(':                                                             // Opening parenthesis
	    buffer[0] = c;
	    tl = addToken(tl, TOKEN_LPAR, buffer, line_number, input);
	    break;

	case ')':                                                             // Closing parenthesis
	    buffer[0] = c;
	    tl = addToken(tl, TOKEN_RPAR, buffer, line_number, input);
	    break;

	case ';':                                                             // Semicolon
	    buffer[0] = c;
	    tl = addToken(tl, TOKEN_SCOL, buffer, line_number, input);
	    break;

	case '!':                                                             // Unary operator
	    buffer[0] = c;
	    tl = addToken(tl, TOKEN_UN_OP, buffer, line_number, input);
	    break;

        /* Start of the range token (..) */
	case '.':
	    buffer[0] = c;
	    tl = handlePeriod(tl, buffer, input);
	    break;

        /* 
	 * The slash can be a start of three valid tokens:
         * One line comment, multiline comment or a division operator.
         */
	case '/':
	    buffer[0] = c;
	    if((tmp = handleSlash(tl, buffer, input)) != NULL)
	       tl = tmp;
	    break;

	/* 
	 * The colon character can be a start of two valid tokens:
	 * The assignment token or the declaration separator.
	 */
	case ':':
	    buffer[0] = c;
	    tl = handleCol(tl, buffer, input);
	    break;

        /* Here we skip all whitespaces */
	case ' ': case '\n': case '\t':
	    if(c == '\n')
		line_number++;
	    break;

	/* now the token is keyword, literal, identifier or error token */
	default:
	    

	    /* The next token is a key word or an identifier. */
	    if(isalpha(c))
		tl = handleOthers(tl, buffer, input);
	    
	    /* Integer literal */
	    else if(isdigit(c))
		tl = handleIntLiterals(tl, buffer, input);

	    /* String literal */
	    else if(c == '"')
		do{
		    memset(buffer, '\0', TOKEN_MAX_LENGTH +1);
		    tl = handleStringLiterals(tl, buffer, input);
		}while(isLastTokenControlError(head));
		      
	    /* Everything else is error */
	    else{
		ungetc(input);
		tl = handleErrors(tl, buffer, input);
	    }

	    break;
	}
    }
    
    /*
     * We had all along one extra slot for new tokens.
     * Now we can mark it as EOF
     */
    addEOF(head);
    
    return head;
}

/*
 * Here we have three possibilities. The token may be a
 * division operator '/' or it can be a start of comment of
 * each type //one line comment or / * multiline comment
 */
static token_list *handleSlash(token_list *tl, char *buffer, FILE *input){
    char c;

    /* Check for eof */
    if(fread(&c, sizeof(char), 1, input) != sizeof(char)){
	skipch(input);
	return addToken(tl, TOKEN_BIN_OP, buffer, line_number, input);
    }
    
    /* One line comment detected! */
    else if(c == '/'){
	while((c = fgetc(input)) != '\n' && c != EOF);
	if(c == '\n')
	    line_number++;
    }
    
    /* Start of multiline comment */
    else if(c == '*'){

	while((c = fgetc(input))){
	    if(c == EOF)
		return addToken(tl, TOKEN_ERROR, buffer, line_number, input);

	    else if( c != '*')
		continue;
	    
	    else if(fread(&c, sizeof(char), 1, input) != sizeof(char))
		return addToken(tl, TOKEN_ERROR, buffer, line_number, input);

	    else if(c == '/') break;

	    else
		ungetc(input);
	}
    }
    
    /* It was a division operator */
    else{
	ungetc(input);
	return addToken(tl, TOKEN_BIN_OP, buffer, line_number, input);
    }


    return NULL;
}

/*
 * Handle colon: The possibilities are an assignment operator (:=)
 * and the separator in variable declarations.
 */
static token_list *handleCol(token_list *tl, char *buffer, FILE *input){
    char c;

    /*
     * In case there is no more input, the colon character should
     * represent a separator in decladation.
     */
    if(fread(&c, sizeof(char), 1, input) != sizeof(char)){
	return addToken(tl, TOKEN_COL, buffer, line_number, input);
    }

    /* An assignment operator detected. */
    if(c == '='){
	buffer[1] = c;
	skipch(input);
	return addToken(tl, TOKEN_ASSIGN, buffer, line_number, input);
    }
    /* Separator detected. */
    else{
	ungetc(input);
	return addToken(tl, TOKEN_COL, buffer, line_number, input);
    }
}

/*
 * The period character can only be a start of an range token (..)
 * or a start of an error token.
 */
static token_list *handlePeriod(token_list *tl, char *buffer, FILE *input){
    char c;
    
    if(fread(&c, sizeof(char), 1, input) != sizeof(char)){
	return addToken(tl, TOKEN_ERROR, buffer, line_number, input);
    }

    /* Range token detected. */
    if(c == '.'){
	buffer[1] = c;
	return addToken(tl, TOKEN_RANGE, buffer, line_number, input);
    }
    else {
	ungetc(input);
	return addToken(tl, TOKEN_ERROR, buffer, line_number, input);
    }
}

/*
 * Here we have two possibilities. next token can be an identifier
 * or a language key word. This method handles them both by first
 * collecting all the token data. After that the decision whether
 * the buffer contains an keyword is made and appropriate keyword
 * token or an identifier is returned.
 */
static token_list *handleOthers(token_list *tl, char *buffer, FILE *input){
    int  i;
    char c;
    
    ungetc(input);

    /*
     * If the token is otherwise valid, but too long identified, the
     * prefix of size TOKEN_MAX_LENGTH is treated as an error and the
     * suffix is returned as a valid identifier token.
     */
    for(i = 0; i <= TOKEN_MAX_LENGTH; i++){

	if(fread(&c, sizeof(char), 1, input) != sizeof(char)){
	    skipch(input);
	    break;
	}

	else if(isalnum(c) || c == '_')
	    buffer[i] = c;
	else
	    break;
    }
    if(i >= TOKEN_MAX_LENGTH){
	strncpy(buffer, "Ignoring too long identifier.", TOKEN_MAX_LENGTH);
	return addToken(tl, TOKEN_ERROR, buffer, line_number, input);
    }
    
    /* Check the buffer against every known language keyword.   */
    else if(isAssertKey(buffer))
	return addToken(tl, TOKEN_ASSERTKEY,  buffer, line_number, input);
    else if(isPrintKey(buffer))
	return addToken(tl, TOKEN_PRINTKEY,   buffer, line_number, input);
    else if(isTypeKey(buffer))
	return addToken(tl, TOKEN_TYPEKEY,    buffer, line_number, input);
    else if(isReadKey(buffer))
	return addToken(tl, TOKEN_READKEY,    buffer, line_number, input);
    else if(isForKey(buffer))
	return addToken(tl, TOKEN_FORKEY,     buffer, line_number, input);
    else if(isVarKey(buffer))
	return addToken(tl, TOKEN_VARKEY,     buffer, line_number, input);
    else if(isEndKey(buffer))
	return addToken(tl, TOKEN_ENDKEY,     buffer, line_number, input);
    else if(isInKey(buffer))
	return addToken(tl, TOKEN_INKEY,      buffer, line_number, input);
    else if(isDoKey(buffer))
	return addToken(tl, TOKEN_DOKEY,      buffer, line_number, input);

    /* No keyword detected. Returning token of type identifier. */
    else
	return addToken(tl, TOKEN_IDENTIFIER, buffer, line_number, input);
}

/*
 * Read the following integer literal.
 */
static token_list *handleIntLiterals(token_list *tl, char *buffer, FILE *input){
    int  i;
    char c;
    
    ungetc(input);
    
    for(i = 0; i <= TOKEN_MAX_LENGTH; i++){

	if(fread(&c, sizeof(char), 1, input) != sizeof(char)){
	    skipch(input);
	    return addToken(tl, TOKEN_INT_LITERAL, buffer, line_number, input);
	}
		
	if(isdigit(c))
	    buffer[i] = c;

	else
	    break;
    }
		
    return addToken(tl, TOKEN_INT_LITERAL, buffer, line_number, input);
}

/*
 * Read the following string literal.
 */
static token_list *handleStringLiterals(token_list *tl, char *buffer, FILE *input){
    char c;
    int last_replaced = 0;
    
    for(int i = 0; i < TOKEN_MAX_LENGTH; i++){

	/* Encountering EOF means there is an unterminated string literal */
	if( fread(&c, sizeof(char), 1, input) != sizeof(char)){
	    skipch(input);
	    sprintf(buffer, "Unterminated string literal.");
	    return addToken(tl, TOKEN_ERROR, buffer, line_number, input);
	}

	/*
	 * The switch statement compares the read character to all possible
	 * escape characters. If the previous character is the escape character
	 * (a backslash), the following character is escaped. The variable named
	 * "last_replaced" acts as an indicator that last character is an escaped
	 * backslash. In that case the replacement is not done. Otherwise the 
	 * lexer would not allow consecutive escaped backslasehs 
	 * (e.g. "\\\\" would produce a single backslash instead of two).
	 */
	switch(c){
	case '"':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\"';
		i--;
	    } else
		return addToken(tl, TOKEN_STRING_LITERAL, buffer, line_number, input);
	    break;
	case '\\':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\\';
		i--;
		last_replaced = 1;
	    /* 
	     * If the next character is not one in this switch structure
	     * the error is occured since the backslashes must be escaped
	     * and they can not occur in a string literals by themselves.
             */
	    } else{
		buffer[i] = '\\'; // Possible error is handled in default case.
		last_replaced = 0;
	    }
	    break;
	case 'n':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\n';
		i--;
	    } else{
		buffer[i] = 'n';
		last_replaced = 0;
	    }
	    break;
	case 't':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\t';
		i--;
	    } else{
		buffer[i] = 't';
		last_replaced = 0;
	    }
	    break;
	case 'a':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\a';
		i--;
	    } else{
		buffer[i] = 'a';
		last_replaced = 0;
	    }
	    break;
	case 'b':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\b';
		i--;
	    } else{
		buffer[i] = 'b';
		last_replaced = 0;
	    }
	    break;
	case 'f':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\f';
		i--;
	    } else{
		buffer[i] = 'f';
		last_replaced = 0;
	    }
	    break;
	case 'r':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\r';
		i--;
	    } else{
		buffer[i] = 'r';
		last_replaced = 0;
	    }
	    break;
	case 'v':
	    if(buffer[i-1] == '\\' && !last_replaced){
		buffer[i-1] = '\v';
		i--;
	    } else{
		buffer[i] = 'v';
		last_replaced = 0;
	    }
	    break;
	default:
	    buffer[i] = c;
	    /* Check for the error situation described in the case '\\' */
	    if(!last_replaced && buffer[i-1] == '\\'){
		sprintf(buffer, "Undefined control sequence \\%c in string literal", c);
		return addToken(tl, TOKEN_ERROR, buffer, line_number, input);
	    }
	    last_replaced = 0; 
	}
	
    }

    /* Too long string literal */
    sprintf(buffer, "String literal is too long.");
    return addToken(tl, TOKEN_ERROR, buffer, line_number, input);
}

/*
 * This function is invoked if everything else fails.
 * The error token is considered to be the character sequence
 * starting from an error point and ending to the next 
 * whitespace character.
 */

static token_list *handleErrors(token_list *tl, char *buffer, FILE *input){
    int  i, tmp = line_number;
    char c;

    for(i = 0; i <= TOKEN_MAX_LENGTH; i++){

  	if(fread(&c, sizeof(char), 1, input) != sizeof(char))
	    break;

	if(c == ' ' || c == '\n' || c == '\t'){
	    if(c == '\n')
		line_number++;
	    break;
	}

	buffer[i] = c;
    }
    
    token_value buffer2;
    sprintf(buffer2, "Unidentified token: %s", buffer);
    return addToken(tl, TOKEN_ERROR, buffer2, tmp, input);
}

/*
 * The following functions are used to check if the buffer
 * contains one of the reserved key words in the language.
 *
 * The input parameter word is pointer to buffer in the switch statement.
 * Returns 1 if it is reserved key we are looking for, 0 otherwise.
 */

static int equals(char *a, char *b){
    if(strncmp(a, b, TOKEN_MAX_LENGTH)           == 0)
	return 1;
    return 0;
}

static int isTypeKey(char *word){

    if(strncmp(word, "int",    TOKEN_MAX_LENGTH) == 0 ||
       strncmp(word, "string", TOKEN_MAX_LENGTH) == 0 ||
       strncmp(word, "bool",   TOKEN_MAX_LENGTH) == 0 )
	return 1;
    
    return 0;
}

static int isForKey    (char *word){
    return equals(word, "for"   );
}

static int isVarKey    (char *word){
    return equals(word, "var"   );
}

static int isEndKey    (char *word){
    return equals(word, "end"   );
}

static int isInKey     (char *word){
    return equals(word, "in"    );
}

static int isDoKey     (char *word){
    return equals(word, "do"    );
}

static int isReadKey   (char *word){
    return equals(word, "read"  );
}

static int isPrintKey  (char *word){
    return equals(word, "print" );
}

static int isAssertKey (char *word){
    return equals(word, "assert");
}

    

/*
 * This function adds special EOF token
 * to the end of the token list.
 */
static void addEOF(token_list *tl){
    for(; tl->next; tl=tl->next);

    tl->value = newToken(TOKEN_EOF, "EOF", line_number);
}

/*
 * This function is mainly for debugging and tests.
 * It takes token_list * parameter and prints the types
 * of all its nodes. E.g. it prints the token sequence.
 * For more information see the documentation.
 *
 * No return value.
 */
void printTokenList(token_list *list){
    for(; list; list = list->next)
	printf("%d\n", list->value->type);
}

/*
 * This function finds and prints the lexical errors
 * added to the token list. Additionally it removes the
 * found error tokens from the list and returns the first
 * non error token.
 * 
 * Note that this function is supposed to be used only after
 * the final token (EOF) is added to token list.
 * Otherwise it will use a null pointer dereference
 * and cause undefined behaviour.
 */
token_list *correctTokenList(token_list *list){
    token_list *head = NULL;
    token_list *del = NULL;
    
    for(token_list *prev = list; list != NULL;){
	if(list->value->type == TOKEN_ERROR){
	    fprintf(stderr, "Lexical error in line %3d: %s\n", list->value->line_number, list->value->value);
	    if(head != NULL)
		prev->next = list->next;
	    del = list;
	    list = list->next;
	    del->next = NULL;
	    freeTokenList(del);
	    continue;
	} else if (head == NULL)
	    head = list;
	
	prev = list;
	list = list->next;
    }

    return head;
}

/*
 * Checks if the last (valid) token on the token list
 * is an control character error token. Used to determine 
 * whether the next backslash should be used as an escape 
 * character.
 *
 * Note that this function is supposed to be used only
 * before the final (EOF) token is added to the list.
 * Otherwise this function returns the error state
 * of the second last token.
 */
static int isLastTokenControlError(token_list *list){
    int isControlError = 0;

    for(; list->next != NULL; list = list->next)
	if(list->value->type == TOKEN_ERROR &&
	   strncmp(list->value->value, "String", 6) == 0 ||
	   strncmp(list->value->value, "Undefined", 9) == 0)
	    isControlError = 1;
	else
	    isControlError = 0;

    return isControlError;
}
