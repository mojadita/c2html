%{
/* $Id: lexical.l,v 0.14 1999/06/14 23:04:02 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Sat Jun  5 21:01:12 MEST 1999
 * 
 *     C2HTML -- A program to convert C source code to cross-referenced HTML.
 *     Copyright (C) 1999  <Luis.Colorado@SLUG.HispaLinux.ES>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 */

#define IN_LEXICAL_L

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <multifree.h>
#include <hashTable.h>
#include "c2html.h"

/* constants */
#define PUSHSIZE		10
#define PUSH(X)						\
	if (pointer < PUSHSIZE) {		\
		state[pointer++] = (X);		\
		BEGIN (trace(X));			\
	}
#define POP()						\
	if (--pointer > 0)				\
		BEGIN trace(state[pointer-1]);
static int state[PUSHSIZE];
static int pointer = 0;

/* types */

/* prototypes */
void pr(FILE *, char *);
void scanfile(char *, char *, char *);
void createmenu(CtagNode *);

/* variables */
static FILE *OUTPUT;

/* gcc compiler allows \n in strings, so we allow them also */
%}
%x incomment
%s indirective
%x instring
%x inchar

%option noyywrap
%option pointer

odig		([0-7])
dig			([0-9])
xdig		([0-9a-fA-F])
lu			(([uUlL]|[uU][lL]|[lL][uU])?)

mant		({dig}+\.{dig}*|{dig}*\.{dig}+)
mant2		({dig}+)
exp			([eE][+-]?{dig}+)
fl			(([flFL]|[fF][lL]|[lL][fF])?)

esc			(\\(.|\n|{odig}{odig}?{odig}?|[xX]{xdig}+))
sp			([\ \t])

alnum		([a-zA-Z\$_0-9])
alpha		([a-zA-Z\$_])

directive	(define|undef|include|if|ifdef|ifndef|else|elif|endif|error|pragma)
%%
"/*"					{	PUSH(incomment); fprintf(OUTPUT, "<I>/*"); }
<incomment>\n			|
<incomment>.			pr(OUTPUT, yytext);
<incomment>TODO			|
<incomment>FIXME		|
<incomment>XXX			|
<incomment>ARGSUSED		|
<incomment>NEVERREACHED	fprintf(OUTPUT, "<U>%s</u>", yytext);
<incomment>"*/"			{	fprintf(OUTPUT, "*/</i>"); POP(); }

"#"{sp}*\n				fprintf(OUTPUT, "<B>%s</b>", yytext); 
"#"{directive}			{	PUSH(indirective);
							fprintf(OUTPUT, "<B>%s</b><I>", yytext);
						}
<indirective>defined	fprintf(OUTPUT, "<B>%s</b>", yytext);
<indirective>\\\n		fputs("\\\n", OUTPUT);
<indirective>\n			{	fputs("</i>\n", OUTPUT); POP(); }

auto					|
break					|
case					|
char					|
const					|
continue				|
default					|
do						|
double					|
else					|
enum					|
extern					|
float					|
for						|
goto					|
if						|
int						|
long					|
register				|
return					|
short					|
signed					|
sizeof					|
static					|
struct					|
switch					|
typedef					|
union					|
unsigned				|
void					|
volatile				|
while					fprintf(OUTPUT, "<B>%s</b>", yytext);

0{odig}*{lu}			|
[1-9]{dig}*{lu}			|
0[xX]{xdig}*{lu}		|
{mant}({exp})?{fl}		|
{mant2}{exp}{fl}		fprintf(OUTPUT, "<B><I>%s</i></b>", yytext);

\"						{	PUSH(instring); fputs("<B><I>\"", OUTPUT); }
<instring>\"			{	fputs("\"</i></b>", OUTPUT); POP(); }
<instring>{esc}			{	fputs("<U>", OUTPUT);pr(OUTPUT, yytext);
							fputs("</u>", OUTPUT);
						}
<instring>\n			|
<instring>.				pr(OUTPUT, yytext);

\'						{	PUSH(inchar); fputs("<B><I>\'", OUTPUT); }
<inchar>\'				{	fputs("\'</i></b>", OUTPUT); POP(); }
<inchar>{esc}			{	fputs("<U>", OUTPUT);pr(OUTPUT, yytext);
							fputs("</u>", OUTPUT);
						}
<inchar>\n				|
<inchar>.				fputs(yytext, OUTPUT);

<*>^"(@"[^@]*"@)\n"		fprintf(OUTPUT, "<%0.*s>", yyleng-5, yytext+2);

[a-zA-Z_\$]{alnum}*		{	/* identifier */
							HashEntry *he;
							he = hashTableLookup(&syms_table, yytext);
							if (!he) {
								fprintf (stderr, PROGNAME": "__FILE__"(%d): "
									"hashTableLookup: %s\n",
									__LINE__, sys_errlist[errno]);
								exit(EXIT_FAILURE);
							}
							if (he->data) {
								CtagNode *ce = he->data;
								if (!(ce->next)) {
									fprintf(OUTPUT,
										"<A HREF=\"%s"EXT2"#%s\">",
										ce->file, ce->sym);
								} else {
									fprintf(OUTPUT,
										"<A HREF=\""PFX1"%s"EXT2"\">", ce->sym);
									if (!(ce->flags & FLAG_MENU_CREATED)) {
										createmenu(ce);
									}
								}
							}
							fprintf(OUTPUT, "%s", yytext);
							if (he->data) {
								fprintf(OUTPUT, "</a>");
							}
						}
"..."					|
"&&"					|
"-="					|
"~"						|
"+"						|
";"						|
"<<="					|
"&="					|
"->"					|
">>"					|
"%"						|
","						|
"<"						|
"^"						|
">>="					|
"*="					|
"/="					|
"^="					|
"&"						|
"-"						|
"="						|
"!="					|
"++"					|
"<<"					|
"|="					|
"."						|
">"						|
"|"						|
"%="					|
"+="					|
"<="					|
"||"					|
"/"						|
"?"						|
"--"					|
"=="					|
"!"						|
"*"						|
":"						pr(OUTPUT, yytext);

"["						|
"]"						|
"{"						|
"}"						|
"("						|
")"						fputs(yytext, OUTPUT);

\n						|
.						fputs(yytext, OUTPUT);

%%
/* functions */

void pr(FILE *f, char *s)
{
	while (*s) {
		switch (*s) {
		case '<': fprintf(f, "&lt;"); break;
		case '>': fprintf(f, "&gt;"); break;
		case '&': fprintf(f, "&amp;"); break;
		default: fputc(*s, f); break;
		}
		s++;
	} /* while */
} /* pr */

int trace(int s)
{
#if TRACEON
	switch(s) {
	case INITIAL:		fputs("<!-- INITIAL -->",		OUTPUT); break;
	case incomment:		fputs("<!-- incomment -->",		OUTPUT); break;
	case indirective:	fputs("<!-- indirective -->",	OUTPUT); break;
	case instring:		fputs("<!-- instring -->",		OUTPUT); break;
	case inchar:		fputs("<!-- inchar -->",		OUTPUT); break;
	}
#endif
	return s;
} /* trace */

void createmenu(CtagNode *ce)
{
	FILE *outf;
	char buffer[1024];

	sprintf (buffer, PFX1"%s"EXT2, ce->sym);
#if 0
	fprintf (stderr, PROGNAME": creating menu %s\n", buffer);
#endif
	outf = html_create(buffer, ce->sym);
	fprintf(outf, "    <UL>\n");
	for (;ce; ce = ce->next) {
		fprintf(outf, "      <LI><A HREF=\"%s"EXT2"#%s\">%s</a>\n",
			ce->file, ce->sym, ce->file);
		ce->flags |= FLAG_MENU_CREATED;
	}
	fprintf(outf, "	   </ul>\n");
	html_close(outf);
} /* createmenu */
		

void scanfile (char *fn, char *fin, char *fout)
{
	FILE *INPUT;
	YY_BUFFER_STATE bs;

	INPUT = fopen (fin, "r");
	OUTPUT = html_create(fout, fn);
	bs = yy_create_buffer (INPUT, YY_BUF_SIZE);
	yy_switch_to_buffer (bs);

	fprintf(OUTPUT, "    <PRE>\n");
	fprintf(OUTPUT, "<!---------------------------------------------------->\n");
	pointer=0;
	PUSH(INITIAL);
	yylex();
	fprintf(OUTPUT, "<!---------------------------------------------------->\n");
	fprintf(OUTPUT, "    </pre>\n");
	fclose (INPUT);
	html_close(OUTPUT);
	yy_delete_buffer(bs);
} /* scanfile */

/* $Id: lexical.l,v 0.14 1999/06/14 23:04:02 luis Exp $ */
