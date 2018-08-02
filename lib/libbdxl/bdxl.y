%{
#include "y.tab.h"
%}


%union {
        int intval;
        char *strval;
};

%token  <intval>        INTEGER
%token  <strval>        STRING
%token  <strval>        ID
%token                  LBRACE
%token                  RBRACE
%token                  COLON
%token                  COMMA
%token                  EQUAL
%token                  STRUCT
%token                  SEQ
%token                  SET
%token                  OF

%%

%%
