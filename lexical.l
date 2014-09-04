%{
/* $Id: lexical.l,v 0.21 2009/01/03 22:23:11 luis Exp $
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
#include <assert.h>
#include <errno.h>

#include <avl.h>

#include "intern.h"
#include "db.h"
#include "c2html.h"
#include "html_output.h"

/* constants */
#define PUSHSIZE		10
#define PUSH(X)						\
	if (pointer < PUSHSIZE) {		\
		state[pointer++] = (X);		\
		BEGIN(X);			\
	}
#define POP()						\
	if (--pointer > 0)				\
		BEGIN(state[pointer-1]);
static int state[PUSHSIZE];
static int pointer = 0;

#define YY_DECL	int yylex(node *fin, node *fnout)

/* types */

/* prototypes */
void pr(FILE *, char *);
void scanfile(node *);
void create_menu(tag_menu *m);
void newline(int do_reset);

/* variables */
static FILE *OUTPUT;

/* gcc compiler allows \n in strings, so we allow them also */
%}
%x incomment
%s indirective
%x ininclude
%x instring
%x inchar
%x inATAG

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

alnum		([a-zA-Z_0-9])
alpha		([a-zA-Z_])

directive	(define|undef|if|ifdef|ifndef|else|elif|endif|error|pragma)
fn			("<"[^">"\n]*">")
fn2			(\"[^"\n]*\")
ident		({alpha}{alnum}*)
%%
"/*"					{	PUSH(incomment); fprintf(OUTPUT, "<span class=\"comment\">/*"); }
<incomment>\n			{	pr(OUTPUT, yytext); newline(FALSE); }
<incomment>.			{	pr(OUTPUT, yytext); }
<incomment>TODO			{	fprintf(OUTPUT, "<span class=\"todo\">%s</span>", yytext); }
<incomment>FIXME		{	fprintf(OUTPUT, "<span class=\"fixme\">%s</span>", yytext); }
<incomment>XXX			{	fprintf(OUTPUT, "<span class=\"xxx\">%s</span>", yytext); }
<incomment>ARGSUSED		{	fprintf(OUTPUT, "<span class=\"argsused\">%s</span>", yytext); }
<incomment>NEVERREACHED	{	fprintf(OUTPUT, "<U>%s</u>", yytext); }
<incomment>"*/"			{	fprintf(OUTPUT, "*/</span>"); POP(); }

"#"{sp}*{directive}		{	PUSH(indirective);
							fprintf(OUTPUT, "<span class=\"directive\">%s", yytext);
						}
"#"{sp}*include{sp}*	{	PUSH(ininclude);
							fprintf(OUTPUT, "<span class=\"directive\">%s", yytext);
						}
<indirective>defined	{	fprintf(OUTPUT, "<span class=\"resvword\">%s</span>", yytext); }
<indirective>\\\n		{	fprintf(OUTPUT, "\\\n"); newline(FALSE); }
<indirective>\n			{	fputs("</span>\n", OUTPUT); newline(FALSE); POP(); }
<ininclude>{fn}			{	fprintf(OUTPUT, "<span class=\"include standard\">");
							pr(OUTPUT, yytext);
							fputs("</span>", OUTPUT);
							POP();
							PUSH(indirective);
						}
<ininclude>{fn2}		{	fprintf(OUTPUT, "<span class=\"include\">");
							pr(OUTPUT, yytext);
							fputs("</span>", OUTPUT);
							POP();
							PUSH(indirective);
						}

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
while					{	fprintf(OUTPUT, "<span class=\"resvword\">%s</span>", yytext); }

0{odig}*{lu}			{	fprintf(OUTPUT, "<span class=\"number octal\">%s</span>", yytext); }
[1-9]{dig}*{lu}			{   fprintf(OUTPUT, "<span class=\"number decimal\">%s</span>", yytext); }
0[xX]{xdig}*{lu}		{	fprintf(OUTPUT, "<span class=\"number hexadecimal\">%s</span>", yytext); }
{mant}({exp})?{fl}		{	fprintf(OUTPUT, "<span class=\"number double\">%s</span>", yytext); }
{mant2}{exp}{fl}		{	fprintf(OUTPUT, "<span class=\"number double\">%s</span>", yytext); }

\"						{	PUSH(instring); fprintf(OUTPUT, "<span class=\"string\">\""); }
<instring>\"			{	fprintf(OUTPUT, "\"</span>"); POP(); }
<instring>{esc}			{	fprintf(OUTPUT, "<span class=\"escapechar\">%s</span>", yytext); }
<instring>\n			{	pr(OUTPUT, yytext); newline(FALSE); }
<instring>.				{	pr(OUTPUT, yytext); }

\'						{	PUSH(inchar); fprintf(OUTPUT, "<span class=\"charliteral\">\'"); }
<inchar>\'				{	fprintf(OUTPUT, "\'</span>"); POP(); }
<inchar>{esc}			{	fprintf(OUTPUT, "<span class=\"escapechar\">%s</span>", yytext); }
<inchar>\n				{	pr(OUTPUT, yytext); newline(FALSE); }
<inchar>.				{	pr(OUTPUT, yytext); }

<INITIAL>^"(@"			{	fputs("<", OUTPUT); BEGIN(inATAG); }
<inATAG>"@)(@"			{	fputs("><", OUTPUT); }
<inATAG>"@)"			{	fputs(">", OUTPUT); BEGIN(INITIAL); }
<inATAG>.				{	fputs(yytext, OUTPUT); }

{alpha}{alnum}*			{{	/* identifier */
							tag_menu *men;

							if (flags & FLAG_DEBUG_LEX) {
								printf(PR("identifier processing(%s): begin\n"),
									yytext);
							} /* if */

							fprintf(OUTPUT, "<span class=\"identifier\">");
							if (flags & FLAG_DEBUG_LEX) {
								printf(PR("looking for menu_entry \"%s\" in db_menus\n"),
									yytext);
							} /* if */
							men = avl_tree_get(db_menus, yytext);
							if (men) { /* there is an entry in menus database */
								if (flags & FLAG_DEBUG_LEX) {
									printf(PR("\"%s\" found!!! check if ntags > 1\n"),
										men->name);
								} /* if */
								if (men->ntags > 1) { /* multiple entry */
									char *rp;
									if (flags & FLAG_DEBUG_LEX) {
										printf(PR("menu \"%s\":multiple menu entry(%d), "
											"calling create_menu\n"),
											men->name, men->ntags);
									} /* if */

									create_menu(men);

									rp = rel_path(fin->html_file, men->nod->html_file);
									if (flags & FLAG_DEBUG_LEX) {
										printf(PR("<a href=\"%s\">%s</a>\n"),
											rp, men->name);
									} /* if */
									fprintf(OUTPUT, "<a href=\"%s\">%s</a>",
										rp, men->name);
								} else { /* single entry */
									ctag *t_aux = men->last_tag;
									char *rp;

									if (flags & FLAG_DEBUG_LEX) {
										printf(PR("menu %s:single menu entry ctag=<%s,%s,%s>\n"),
											men->name, t_aux->id, t_aux->fi, t_aux->ss);
									} /* if */

									assert(t_aux); /* ctag * */
									assert(t_aux->nod); /* node * */
									assert(t_aux->nod->html_file); /* node * */
									rp = rel_path(fin->html_file, t_aux->nod->html_file);
									if (flags & FLAG_DEBUG_LEX) {
										printf(PR("menu %s:<a href=\"%s#%s-%d\">%s</a>\n"),
											t_aux->id,
											rp, t_aux->id, t_aux->tag_no_in_file,
											t_aux->id);
									} /* if */
									fprintf(OUTPUT, "<a href=\"%s#%s-%d\">%s</a>",
										rp, t_aux->id, t_aux->tag_no_in_file, t_aux->id);
								} /* if */
							} else { /* no entry found, print it plain */
								if (flags & FLAG_DEBUG_LEX) {
									printf(PR("%s not found, print plain.\n"), yytext);
								} /* if */
								fprintf(OUTPUT, "%s", yytext);
							} /* if */
							fprintf(OUTPUT, "</span>");
							if (flags & FLAG_DEBUG_LEX) {
								printf(PR("identifier(%s): end\n"), yytext);
							} /* if */
						}}
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
":"						{	fputs("<span class=\"operator\">", OUTPUT);
							pr(OUTPUT, yytext);fputs("</span>", OUTPUT);
						}

"["						|
"]"						|
"{"						|
"}"						|
"("						|
")"						{	fputs("<span class=\"bracket\">", OUTPUT);
							pr(OUTPUT, yytext);fputs("</span>", OUTPUT);
						}

\n						{	fputs(yytext, OUTPUT); newline(FALSE); }
.						{	fputs(yytext, OUTPUT); }

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

void newline(int do_reset)
{
	static int linenum = 0;

	if (do_reset) linenum = 0;
	if (flags & FLAG_LINENUMBERS) {
		fprintf(OUTPUT, "<span class=\"linenum\">%06d</span> ",
			++linenum);
	} /* if */
} /* newline */

void create_menu(tag_menu *m)
{
	FILE *outf;
	AVL_ITERATOR i;

	assert(m);

	if (flags & FLAG_DEBUG_CREATE_MENU) {
		printf(PR("begin: menu=\"%s\"\n"), m->name);
	} /* if */

	if (m->flags & TAG_MENU_FLAG_ALREADY_CREATED) {
		if (flags & FLAG_DEBUG_CREATE_MENU) {
			printf(PR("already created \"%s\", give up.\n"),
				m->name);
		} /* if */
		return;
	} /* if */
	m->flags |= TAG_MENU_FLAG_ALREADY_CREATED;

	assert(m->nod);

	outf = html_create(m->nod);
	fprintf(outf, "      <ul>\n");

	printf(PR("MEN: name=\"%s\", ntags=%d\n"),
		m->name, m->ntags);

	/* follow the list of files for this id */
	for (	i = avl_tree_first(m->group_by_file);
			i;
			i = avl_iterator_next(i))
	{
		ctag *t = avl_iterator_data(i);

#if 0
		assert(t);
		assert(t->nod);
		assert(t->nod->full_name);
#endif

		printf(PR("FILE: %s\n"), avl_iterator_key(i));

		if (flags & FLAG_DEBUG_CREATE_MENU) {
			printf(PR("step: file = %s\n"),
				t->nod->full_name);
		} /* if */

		fprintf(outf,
			"        <li class=\"menu file\">File "
			"<span class=\"file\">%s</span>.\n"
			"          <ul>\n",
			t->nod->full_name);

		if (0)
		for (t = avl_iterator_data(i); t; t = t->next_in_file)
			printf(PR("TAG[%p] id=[%s], fi=[%s], ss=<%s>, tag_no_in_file=%d, next_in_file=[%p]\n"),
				t, t->id, t->fi, t->ss, t->tag_no_in_file, t->next_in_file);

#if 0
		do {
			if (flags & FLAG_DEBUG_CREATE_MENU) {
				printf(PR("substep: ctag = <%s,%s,%s>\n"),
					t->id, t->nod->full_name, t->ss);
			} /* if */
			char *rp = rel_path(m->nod->html_file, t->nod->html_file);
			if (flags & FLAG_DEBUG_CREATE_MENU) {
				printf(PR("substep: <a href=\"%s#%s-%d\">%s(%d)</a>\n"),
					rp, t->id, t->tag_no_in_file, t->id, t->tag_no_in_file);
			} /* if */
			fprintf(outf,
				"            <li class=\"menu tag\">"
				"<a href=\"%s#%s-%d\">%s(%d)</a></li>\n",
				rp, t->id, t->tag_no_in_file, t->id, t->tag_no_in_file);
			t = t->next_in_file;
		} while(t);
#endif

		fprintf(outf,
			"          </ul></li><!-- File %s -->\n",
			t->nod->full_name); 
		if (flags & FLAG_DEBUG_CREATE_MENU) {
			printf(PR("step: end\n"));
		} /* if */
	} /* for */

	fprintf(outf, "      </ul>\n");
	html_close(m->nod);
		
} /* create_menu */
		

void scanfile (node *fin)
{
	FILE *INPUT;
	YY_BUFFER_STATE bs;
	char buffer[64];
	node *fout;

	assert(fout = fin->html_file);

#if DEBUG
	printf("Generating [%s-->%s]\n", fin->full_name, fout->full_name);
#endif

	INPUT = fopen (fin->full_name, "r");
	if (!INPUT) {
		fprintf(stderr,
			PR("fopen(READ):%s:%s(errno=%d)\n"),
			fin->full_name, strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */
	OUTPUT = html_create(fin);
	if (!OUTPUT) {
		fprintf(stderr,
			PR("fopen(WRITE):%s:%s(errno=%d)\n"),
			fout->full_name, strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */
	bs = yy_create_buffer (INPUT, YY_BUF_SIZE);
	yy_switch_to_buffer (bs);

	fprintf(OUTPUT, "    <pre class=\"listing\">\n");
	fprintf(OUTPUT, "<!---------------------------------------------------->\n");
	pointer=0;
	PUSH(INITIAL);
	newline(TRUE);
	yylex(fin, fout);
	fprintf(OUTPUT, "<!---------------------------------------------------->\n");
	fprintf(OUTPUT, "    </pre>\n");
	fclose (INPUT);
	html_close(fin);
	yy_delete_buffer(bs);
} /* scanfile */

/* $Id: lexical.l,v 0.21 2009/01/03 22:23:11 luis Exp $ */
