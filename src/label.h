#ifndef SEMANTICS_HEADER
#define SEMANTICS_HEADER

#include "tree.h"

/*
 * This file contains the type declarations related
 * to labels. (The entities in the symbol table)
 */

typedef char label[TOKEN_MAX_LENGTH +1];
typedef enum LABEL_TYPE {UNDEF = 0, INT, STRING, BOOL} label_type;

typedef struct VALUE{
    int            i;
    int            b;
    char          *s;
    int        empty;
    int        error;
    int     constant;
    label_type    lt;
} value;

typedef struct LABEL_LIST{
    label *l;
    value  v;
    struct LABEL_LIST *next;

}label_list;

#endif
