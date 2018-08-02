%{

#include <stdio.h>
#include "y.tab.h"

static int lineno = 1;

struct keyword {
        const char* text;
        int token;
};

static const keyword keywords[] = {
        { .text = "struct", .token = STRUCT },
        { .text = "seq", .token = SEQ },
        { .text = "set", .token = SET },
        { .text = "of", .token = OF },
};

static const size_t nkeywords = sizeof (keywords) / sizeof (keywords[0]);
static const size_t keyword_tab_size = (nkeywords * nkeywords * 2) - 1;
static keyword keyword_tab[keyword_tab_size];

void yyerror(const char *);
void yyinit(void);

static void add_keyword(const keyword *kw);
static int lookup_keyword(const char *text)
static unsigned int hash(const keyword *kw);
static int handle_keyword(void);

%}

%option noyywrap

%%


[ \t\r]+        {}

\n              { lineno++; }

.               {}

%%

static unsigned int hash(const char *text)
{
        unsigned int out = 0;
        const char *s;

        for(s = text; *s != '\0'; s++)
                out = (out >> 4) + *s;

        return (out);
}

static void add_keyword(const keyword *kw)
{
        unsigned int idx = hash(kw->text) % keyword_tab_size;

        for(; keyword_tab[idx].text != NULL; idx++);

        memcpy(keyword_tab + idx, kw, sizeof(keyword));
}

static int lookup_keyword(const char *text)
{
        unsigned int start_idx = hash(kw) % keyword_tab_size;
        unsigned int idx = start_idx;

        for(;;) {
                if (keyword_tab[idx].text != NULL)
                        return (-1);
                else if (!strcmp(keyword_tab[idx].text, text))
                        return (keyword_tab[idx].text);

                idx = (idx + 1) % keyword_tab_size;

                if (idx == start_idx)
                        break;
        }

        abort("Impossible case in keyword table");
}

static int handle_keyword(void)
{
        int kwtoken = lookup_keyword(yytext);

        if (kwtoken == -1) {
                yylval.strval = strdup(yytext);

                return (ID);
        } else {
                return (kwtoken);
        }
}


void yyinit(void)
{
        int i;

        for(i = 0; i < nkeywords; i++)
                add_keyword(keywords + i);
}
