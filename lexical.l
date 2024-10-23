%{
/* $Id: lexical.l,v 0.22 2014/09/09 20:23:04 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Sat Jun  5 21:01:12 MEST 1999
 * Copyright: (c) 1999-2024 Luis Colorado.  All rights reserved.
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

#include "configure.h"
#include "intern.h"
#include "debug.h"
#include "ctag.h"
#include "c2html.h"
#include "html_output.h"
#include "lexical.h"

/* constants */
#define PUSHSIZE        32
#define PUSH(X)                     \
    if (pointer < PUSHSIZE) {       \
        state[pointer++] = (X);     \
        BEGIN(X);           \
    }
#define POP()                       \
    if (--pointer > 0)              \
        BEGIN(state[pointer-1]);

#define COL()

static int state[PUSHSIZE];
static int pointer = 0;

static void process_identifier(const char *s, const node *fin, FILE *o);
#define YY_DECL int yylex(const node *fin, const node *fnout)

/* types */

/* prototypes */

/* variables */
static FILE *OUTPUT;

/* gcc compiler allows \n in strings, so we allow them also */
%}

%x incomment
%x incomment2
%s indirective
%x ininclude
%x instring
%x inchar
%x inATAG

%option noyywrap
%option pointer

odig        ([0-7])
dig         ([0-9])
xdig        ([0-9a-fA-F])
lu          (([uUlL]|[uU][lL]|[lL][uU])?)

mant        ({dig}+\.{dig}*|{dig}*\.{dig}+)
mant2       ({dig}+)
exp         ([eE][+-]?{dig}+)
fl          (([flFL]|[fF][lL]|[lL][fF])?)

esc         (\\(.|\n|{odig}{odig}?{odig}?|[xX]{xdig}+))
sp          ([\ \t])
fmtesc      (

alnum       ([a-zA-Z_0-9])
alpha       ([a-zA-Z_])

dbeg        {sp}*"#"{sp}*
directive   (define|undef|if|ifdef|ifndef|else|elif|endif|error|pragma)
fn          ("<"[^">"\n]*">")
fn2         (\"[^"\n]*\")
ident       ({alpha}{alnum}*)

%%

"/*"                    {   PUSH(incomment); fputs("<span class=\"comment\">/*", OUTPUT); COL(); }
<incomment>\n           {   fputs("\n", OUTPUT); newline(FALSE); }
<incomment>TODO         {   fputs("<span class=\"todo\">TODO</span>", OUTPUT); COL(); }
<incomment>FIXME        {   fputs("<span class=\"fixme\">FIXME</span>", OUTPUT); COL(); }
<incomment>XXX          {   fputs("<span class=\"xxx\">XXX</span>", OUTPUT); COL(); }
<incomment>ARGSUSED     {   fputs("<span class=\"argsused\">ARGSUSED</span>", OUTPUT); COL(); }
<incomment>NOTREACHED   {   fputs("<span class=\"notreached\">NOTREACHED</span>", OUTPUT); COL(); }
<incomment>"*/"         {   fputs("*/</span>", OUTPUT); POP(); COL(); }

"//"                    {   PUSH(incomment2); fprintf(OUTPUT, "<span class=\"comment\">//"); COL(); }
<incomment2>TODO        {   fputs("<span class=\"todo\">TODO</span>", OUTPUT); COL(); }
<incomment2>FIXME       {   fputs("<span class=\"fixme\">FIXME</span>", OUTPUT); COL(); }
<incomment2>XXX         {   fputs("<span class=\"xxx\">XXX</span>", OUTPUT); COL(); }
<incomment2>ARGSUSED        {   fputs("<span class=\"argsused\">ARGSUSED</span>", OUTPUT); COL(); }
<incomment2>NOTREACHED  {   fputs("<span class=\"notreached\">NOTREACHED</span>", OUTPUT); COL(); }
<incomment2>\n          {   fputs("</span>\n", OUTPUT); POP(); newline(FALSE); }

"#"                     {   PUSH(indirective);
                            fputs("<span class=\"directive\">"
                                "<span class=\"hash\">#</span>",
                                OUTPUT);
                            COL();
                        }
<indirective>"#"{sp}*{ident} {
                            fprintf(OUTPUT,
                                "<span class=\"stringmacroparameter\">"
                                "%s</span>",
                                yytext);
                            COL();
                        }
<indirective>"##"       {   fputs("<span class=\"doublehash\">##</span>",
                                OUTPUT);
                            COL();
                        }
<indirective>####*      {   fputs(yytext, OUTPUT); COL(); }
<indirective>\\\n       {   fputs("\\\n", OUTPUT); newline(FALSE); COL(); }
<indirective>define     |
<indirective>undef      |
<indirective>ifdef      |
<indirective>ifndef     |
<indirective>if         |
<indirective>endif      |
<indirective>else       |
<indirective>elif       |
<indirective>line       |
<indirective>error      |
<indirective>warning    |
<indirective>pragma     |
<indirective>defined    {   fprintf(OUTPUT, "<span class=\"resvword\">%s</span>", yytext); COL(); }
<indirective>{alpha}{alnum} { process_identifier(yytext, fin, OUTPUT); COL(); }
<indirective>\n         {   fputs("</span>\n", OUTPUT); newline(FALSE); POP(); COL(); }
<indirective>include    {   fprintf(OUTPUT, "<span class=\"resvword\">%s</span>", yytext); COL(); BEGIN ininclude; }
<ininclude>"<"(\\.|[^>\n\ \t])*">"  { fputs("<span class=\"standardinclude\">", OUTPUT);
                                    fprintf_html(OUTPUT, "%s", yytext);
                                    fputs("</span>", OUTPUT); COL(); BEGIN indirective;
                              }
<ininclude>\"(\\.|[^\"\n\ \t])*\"   { fputs("<span class=\"nonstandardinclude\">", OUTPUT);
                                    fprintf_html(OUTPUT, "%s", yytext);
                                    fputs("</span>", OUTPUT); COL(); BEGIN indirective;
                              }

auto                    |
break                   |
case                    |
char                    |
const                   |
continue                |
default                 |
do                      |
double                  |
else                    |
enum                    |
extern                  |
float                   |
for                     |
goto                    |
if                      |
int                     |
long                    |
register                |
return                  |
short                   |
signed                  |
sizeof                  |
static                  |
struct                  |
switch                  |
typedef                 |
union                   |
unsigned                |
void                    |
volatile                |
while                   {   fprintf(OUTPUT, "<span class=\"resvword\">%s</span>", yytext); COL(); }

0{odig}*{lu}            {   fprintf(OUTPUT, "<span class=\"number octal\">%s</span>", yytext); COL(); }
[1-9]{dig}*{lu}         {   fprintf(OUTPUT, "<span class=\"number decimal\">%s</span>", yytext); COL(); }
0[xX]{xdig}*{lu}        {   fprintf(OUTPUT, "<span class=\"number hexadecimal\">%s</span>", yytext); COL(); }
{mant}({exp})?{fl}      {   fprintf(OUTPUT, "<span class=\"number double\">%s</span>", yytext); COL(); }
{mant2}{exp}{fl}        {   fprintf(OUTPUT, "<span class=\"number double\">%s</span>", yytext); COL(); }

\"                      {   PUSH(instring); fprintf(OUTPUT, "<span class=\"string\">\""); COL(); }
<instring>\"            {   fprintf(OUTPUT, "\"</span>"); POP(); }
<instring>{esc}         {   fprintf(OUTPUT, "<span class=\"escape char\">%s</span>", yytext); COL(); }
<instring>\n            {   fputs("\n", OUTPUT); newline(FALSE); }

\'                      {   PUSH(inchar); fprintf(OUTPUT, "<span class=\"charliteral\">\'"); COL(); }
<inchar>\'              {   fprintf(OUTPUT, "\'</span>"); POP(); }
<inchar>{esc}           {   fprintf(OUTPUT, "<span class=\"escapechar\">%s</span>", yytext); COL(); }
<inchar>\n              {   fprintf_html(OUTPUT, "%s", yytext); newline(FALSE); COL(); }

<INITIAL,incomment,instring,inchar>^"(@"    {   fputs("<", OUTPUT); PUSH(inATAG); COL(); }
<inATAG>"@)(@"          {   fputs("><", OUTPUT); }
<inATAG>"@)"            {   fputs(">", OUTPUT); POP(); }

{alpha}{alnum}*         {   process_identifier(yytext, fin, OUTPUT); COL(); COL(); }
"..."                   |
"&&"                    |
"-="                    |
"~"                     |
"+"                     |
";"                     |
"<<="                   |
"&="                    |
"->"                    |
">>"                    |
"%"                     |
","                     |
"<"                     |
"^"                     |
">>="                   |
"*="                    |
"/="                    |
"^="                    |
"&"                     |
"-"                     |
"="                     |
"!="                    |
"++"                    |
"<<"                    |
"|="                    |
"."                     |
">"                     |
"|"                     |
"%="                    |
"+="                    |
"<="                    |
"||"                    |
"/"                     |
"?"                     |
"--"                    |
"=="                    |
"!"                     |
"*"                     |
":"                     {   fputs("<span class=\"operator\">", OUTPUT);
                            fprintf_html(OUTPUT, "%s", yytext);
                            fputs("</span>", OUTPUT);
                            COL();
                        }

"["                     |
"]"                     |
"{"                     |
"}"                     |
"("                     |
")"                     {   fputs("<span class=\"bracket\">", OUTPUT);
                            fputs(yytext, OUTPUT);
                            fputs("</span>", OUTPUT);
                            COL();
                        }

\n                      {   fputs(yytext, OUTPUT); newline(FALSE); COL(); }
<incomment>.            {   fprintf_html(OUTPUT, "%s", yytext); COL(); }
<incomment2>.           {   fprintf_html(OUTPUT, "%s", yytext); COL(); }
<indirective>.          {   fprintf_html(OUTPUT, "%s", yytext); COL(); }
<instring>.             {   fprintf_html(OUTPUT, "%s", yytext); COL(); }
<inchar>.               {   fprintf_html(OUTPUT, "%s", yytext); COL(); }
<inATAG>.               {   fprintf_html(OUTPUT, "%s", yytext); COL(); }
<ininclude>.            {   fprintf_html(OUTPUT, "%s", yytext); COL(); }
.                       {   fprintf_html(OUTPUT, "%s", yytext); COL(); }

%%

/* functions */

static void process_identifier(const char *s, const node *fin, FILE *o)
{
    DEB(FLAG_DEBUG_PROCESS_IDENT,
        "identifier processing(%s): begin\n", s);

    fprintf(o, "<span class=\"identifier\">");

    DEB(FLAG_DEBUG_PROCESS_IDENT,
        "looking for menu_entry \"%s\" in db_menus\n",
        yytext);
    html_generate_ref(o, s, fin);
    fprintf(o, "</span>");
    DEB(FLAG_DEBUG_PROCESS_IDENT,
        "ident='%s': end\n", s);
}

void newline(int do_reset)
{
    static int linenum = 0;

    if (do_reset) linenum = 0;
    if (flags & FLAG_LINENUMBERS) {
        fprintf(OUTPUT, "<span class=\"linenum\">%06d</span> ",
            ++linenum);
    } /* if */
} /* newline */


void scanfile (node *fin)
{
    FILE *INPUT;
    YY_BUFFER_STATE bs;
    const node *fout;

    fout = fin->html_file;

    DEB(FLAG_DEBUG_SCANFILE,
        "Generating [%s-->%s]\n",
        fin->full_name, fout->full_name);

    INPUT = fopen (fin->full_name, "r");
    if (!INPUT) {
        ERR(EXIT_FAILURE,
            "fopen(READ):%s:%s(errno=%d)\n",
            fin->full_name, strerror(errno), errno);
    } /* if */
    OUTPUT = html_create(fin);
    if (!OUTPUT) {
        ERR(EXIT_FAILURE,
            "fopen(WRITE):%s:%s(errno=%d)\n",
            fout->full_name, strerror(errno), errno);
        exit(EXIT_FAILURE);
    } /* if */
    bs = yy_create_buffer (INPUT, YY_BUF_SIZE);
    yy_switch_to_buffer (bs);

    fprintf(OUTPUT,
			"           <pre id=\"listing\">\n"
            "             <!---------------------------------------------------->\n");
    pointer=0;
    PUSH(INITIAL);
    newline(TRUE);
    yylex(fin, fout);
    fprintf(OUTPUT,
            "\n"
            "             <!---------------------------------------------------->\n"
			"           </pre>\n");
    fclose (INPUT);
    html_close(fin);
    yy_delete_buffer(bs);
    if (!(flags & FLAG_DONT_DELETE_TEMPORARIES)) {
        unlink(fin->full_name);
    }
} /* scanfile */
