%{
/* $Id: lexical.l,v 0.18 2005/02/22 19:23:36 luis Exp $
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

#define YY_DECL	int yylex(char *fnout)

/* types */

/* prototypes */
void pr(FILE *, char *);
void scanfile(char *, char *, char *);
void createmenu(CtagNode *);
char *relfilename(char *fmt, char *fdest, char *fsour);

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
"/*"					{	PUSH(incomment); fprintf(OUTPUT, "<FONT COLOR=\"blue\"><I>/*"); }
<incomment>\n			|
<incomment>.			pr(OUTPUT, yytext);
<incomment>TODO			|
<incomment>FIXME		|
<incomment>XXX			|
<incomment>ARGSUSED		|
<incomment>NEVERREACHED	fprintf(OUTPUT, "<U>%s</u>", yytext);
<incomment>"*/"			{	fprintf(OUTPUT, "*/</i></font>"); POP(); }

"#"{sp}*\n				fprintf(OUTPUT, "<B>%s</b>", yytext); 
"#"{directive}			{	PUSH(indirective);
							fprintf(OUTPUT, "<FONT COLOR=\"maroon\"><B>%s</b><I>", yytext);
						}
<indirective>defined	fprintf(OUTPUT, "<B>%s</b>", yytext);
<indirective>\\\n		fputs("\\\n", OUTPUT);
<indirective>\n			{	fputs("</i></font>\n", OUTPUT); POP(); }

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
while					fprintf(OUTPUT, "<FONT COLOR=\"red\"><B>%s</b></font>", yytext);

0{odig}*{lu}			|
[1-9]{dig}*{lu}			|
0[xX]{xdig}*{lu}		|
{mant}({exp})?{fl}		|
{mant2}{exp}{fl}		fprintf(OUTPUT, "<FONT COLOR=\"red\"><B><I>%s</i></b></font>", yytext);

\"						{	PUSH(instring); fputs("<FONT COLOR=\"green\"><I>\"", OUTPUT); }
<instring>\"			{	fputs("\"</i></font>", OUTPUT); POP(); }
<instring>{esc}			{	fputs("<FONT COLOR=\"blue\"><U>", OUTPUT);pr(OUTPUT, yytext);
							fputs("</u></font>", OUTPUT);
						}
<instring>\n			|
<instring>.				pr(OUTPUT, yytext);

\'						{	PUSH(inchar); fputs("<FONT COLOR=\"red\"><B><I>\'", OUTPUT); }
<inchar>\'				{	fputs("\'</i></b></font>", OUTPUT); POP(); }
<inchar>{esc}			{	fputs("<FONT COLOR=\"blue\"><U>", OUTPUT);pr(OUTPUT, yytext);
							fputs("</u></font>", OUTPUT);
						}
<inchar>\n				|
<inchar>.				fputs(yytext, OUTPUT);

<INITIAL>"(@"[^@]*"@)"	fprintf(OUTPUT, "<%0.*s>", yyleng-4, yytext+2);

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
										"<A HREF=\"%s#%s\">",
										relfilename("%s"EXT2, ce->file, fnout),
										ce->sym);
								} else {
									fprintf(OUTPUT,
										"<!-- FIXME --><A HREF=\"%s\">",
										relfilename(PFX1"%s"EXT2, ce->sym, fnout));
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

char *relfilename(char *fmt, char *fdest, char *fsour)
{
	static char buffer[10240];  /* we return a pointer to this buffer */
	char *outname;
	int not_found = TRUE;

	{	char *f1, *f2, *f3;
		f1 = buffer;
		f2 = f1 + sprintf(buffer, fmt, fdest) + 1;
		f3 = f2 + sprintf(f2, "%s", fsour) + 1;
		fdest = f1; fsour = f2; outname = f3;
	}
#if 0
	fprintf(stderr, "relfilename: \"%s\" -> \"%s\"", fsour, fdest);
#endif
	if (!(flags & FLAG_RELFILENAME)) return fdest;
	while (not_found) {
		char *p1 = strchr (fsour, '/');
		char *p2 = strchr (fdest, '/');

		if (!p1 || !p2) break;
		*p1 = '\0'; *p2 = '\0';
		not_found = (strcmp(fsour, fdest) == 0);
		*p1++ = '/'; *p2++ = '/';
		if (not_found) {
			fsour = p1; fdest = p2;
		}
	}
	/* we have eliminated the common part, now construct the output string */
	{	char *p, *q = outname;
		while (p = strchr(fsour, '/')) {
			*p++ = '\0';
			if (!strcmp(fsour, "..")) {
				fprintf(stderr,
					PROGNAME": "__FILE__"(%d): \"..\" in path, output can be damaged\n",
					__LINE__);
			}
			if (strcmp(fsour, "."))  /* . implies NO OPERATION */
				q += sprintf (q, "../");
			fsour = p;
		}
		q += sprintf (q, "%s", fdest);
	}

#if 0
	fprintf(stderr, " ==> \"%s\";\n", outname);
#endif


	return outname;
} /* relfilename */

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
	yylex(fout);
	fprintf(OUTPUT, "<!---------------------------------------------------->\n");
	fprintf(OUTPUT, "    </pre>\n");
	fclose (INPUT);
	html_close(OUTPUT);
	yy_delete_buffer(bs);
} /* scanfile */

/* $Id: lexical.l,v 0.18 2005/02/22 19:23:36 luis Exp $ */
