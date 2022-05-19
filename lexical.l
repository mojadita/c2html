%{
/* $Id: lexical.l,v 0.22 2014/09/09 20:23:04 luis Exp $
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
#include "debug.h"
#include "db.h"
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
<incomment>.            {   fprintf_html(OUTPUT, "%s", yytext); COL(); }

"//"                    {   PUSH(incomment2); fprintf(OUTPUT, "<span class=\"comment\">//"); COL(); }
<incomment2>TODO        {   fputs("<span class=\"todo\">TODO</span>", OUTPUT); COL(); }
<incomment2>FIXME       {   fputs("<span class=\"fixme\">FIXME</span>", OUTPUT); COL(); }
<incomment2>XXX         {   fputs("<span class=\"xxx\">XXX</span>", OUTPUT); COL(); }
<incomment2>ARGSUSED        {   fputs("<span class=\"argsused\">ARGSUSED</span>", OUTPUT); COL(); }
<incomment2>NOTREACHED  {   fputs("<span class=\"notreached\">NOTREACHED</span>", OUTPUT); COL(); }
<incomment2>\n          {   fputs("</span>\n", OUTPUT); POP(); newline(FALSE); }
<incomment2>.           {   fprintf_html(OUTPUT, "%s", yytext); COL(); }

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
<indirective>\\\n       {   fputs("\\\n", OUTPUT); newline(FALSE); }
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
<indirective>include    |
<indirective>pragma     |
<indirective>defined    {   fprintf(OUTPUT, "<span class=\"resvword\">%s</span>", yytext); COL(); }
<indirective>{alpha}{alnum} { process_identifier(yytext, fin, OUTPUT); COL(); }
<indirective>"<"[^>\n\ \t]*">"  { fputs("<span class=\"standardinclude\">", OUTPUT);
                                    fprintf_html(OUTPUT, "%s", yytext);
                                    fputs("</span>", OUTPUT); COL(); }
<indirective>\"[^\"\n\ \t]*\"   { fputs("<span class=\"nonstandardinclude\">", OUTPUT);
                                    fprintf_html(OUTPUT, "%s", yytext);
                                    fputs("</span>", OUTPUT); COL(); }
<indirective>\n         {   fputs("</span>\n", OUTPUT); newline(FALSE); POP(); }
<indirective>.          {   fprintf_html(OUTPUT, "%s", yytext); COL(); }

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
while                   {   fprintf(OUTPUT, "<span class=\"resvword\">%s</span>", yytext); }

0{odig}*{lu}            {   fprintf(OUTPUT, "<span class=\"number octal\">%s</span>", yytext); }
[1-9]{dig}*{lu}         {   fprintf(OUTPUT, "<span class=\"number decimal\">%s</span>", yytext); }
0[xX]{xdig}*{lu}        {   fprintf(OUTPUT, "<span class=\"number hexadecimal\">%s</span>", yytext); }
{mant}({exp})?{fl}      {   fprintf(OUTPUT, "<span class=\"number double\">%s</span>", yytext); }
{mant2}{exp}{fl}        {   fprintf(OUTPUT, "<span class=\"number double\">%s</span>", yytext); }

\"                      {   PUSH(instring); fprintf(OUTPUT, "<span class=\"string\">\""); }
<instring>\"            {   fprintf(OUTPUT, "\"</span>"); POP(); }
<instring>{esc}         {   fprintf(OUTPUT, "<span class=\"escape char\">%s</span>", yytext); }
<instring>\n            {   fputs("\n", OUTPUT); newline(FALSE); }
<instring>.             {   fprintf_html(OUTPUT, "%s", yytext); }

\'                      {   PUSH(inchar); fprintf(OUTPUT, "<span class=\"charliteral\">\'"); }
<inchar>\'              {   fprintf(OUTPUT, "\'</span>"); POP(); }
<inchar>{esc}           {   fprintf(OUTPUT, "<span class=\"escapechar\">%s</span>", yytext); }
<inchar>\n              {   fprintf_html(OUTPUT, "%s", yytext); newline(FALSE); }
<inchar>.               {   fprintf_html(OUTPUT, "%s", yytext); }

<INITIAL,incomment,instring,inchar>^"(@"    {   fputs("<", OUTPUT); PUSH(inATAG); }
<inATAG>"@)(@"          {   fputs("><", OUTPUT); }
<inATAG>"@)"            {   fputs(">", OUTPUT); POP(); }
<inATAG>.               {   fputs(yytext, OUTPUT); }

{alpha}{alnum}*         {   process_identifier(yytext, fin, OUTPUT); COL(); }
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
                        }

"["                     |
"]"                     |
"{"                     |
"}"                     |
"("                     |
")"                     {   fputs("<span class=\"bracket\">", OUTPUT);
                            fputs(yytext, OUTPUT);
                            fputs("</span>", OUTPUT);
                        }

\n                      {   fputs(yytext, OUTPUT); newline(FALSE); }
.                       {   fprintf_html(OUTPUT, "%s", yytext); }

%%

/* functions */

static void process_identifier(const char *s, const node *fin, FILE *o)
{   /* identifier */
    tag_menu *men;

    DEB(FLAG_DEBUG_PROCESS_IDENT,
            "identifier processing(%s): begin\n", yytext);

    fprintf(o, "<span class=\"identifier\">");

    DEB(FLAG_DEBUG_PROCESS_IDENT,
            "looking for menu_entry \"%s\" in db_menus\n",
        yytext);
    men = avl_tree_get(db_menus, yytext);
    if (men) { /* there is an entry in menus database */
        DEB(FLAG_DEBUG_PROCESS_IDENT,
                "\"%s\" found!!! check if ntags > 1\n",
            men->id);
        if (men->ntags > 1) { /* multiple entry */
            char *rp;

            D(create_menu(men));

            D(rp = rel_path(fin->html_file, men->nod->html_file));
            DEB(FLAG_DEBUG_PROCESS_IDENT,
                    "<a href=\"%s\">%s</a>\n",
                rp, men->id);

            fprintf(o,
                "<a href=\"%s\">%s</a>",
                rp, men->id);

        } else { /* single entry */

            ctag *tag = men->last_tag;
            char *rp;

            DEB(FLAG_DEBUG_PROCESS_IDENT,
                    "menu %s:single menu entry "
                    "ctag=<%s,%s,%s>\n",
                men->id, tag->id,
                tag->fi, tag->ss);

            assert(tag); /* ctag * */
            assert(tag->nod); /* node * */
            assert(tag->nod->html_file); /* node * */

            rp = rel_path(fin->html_file, tag->nod->html_file);
            DEB(FLAG_DEBUG_PROCESS_IDENT,
                    "menu %s:<a href=\"%s#%s-%d\">%s</a>\n",
                tag->id,
                rp, tag->id, tag->tag_no_in_file,
                tag->id);
            fprintf(o, "<a href=\"%s#%s-%d\">%s</a>",
                rp, tag->id, tag->tag_no_in_file, tag->id);
        } /* if */
    } else { /* no entry found, print it plain */
        DEB(FLAG_DEBUG_PROCESS_IDENT,
                "%s not found, print plain.\n", yytext);
        fprintf(o, "%s", yytext);
    } /* if */
    fprintf(o, "</span>");
    DEB(FLAG_DEBUG_PROCESS_IDENT,
            "identifier(%s): end\n", yytext);
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

void create_menu(tag_menu *m)
{
    FILE *outf;
    AVL_ITERATOR i;

    assert(m);

    DEB(FLAG_DEBUG_CREATE_MENU,
            "begin: menu=\"%s\"\n", m->id);

    if (m->flags & TAG_MENU_FLAG_ALREADY_CREATED) {
        DEB(FLAG_DEBUG_CREATE_MENU,
                "already created \"%s\", give up.\n",
            m->id);
        return;
    } /* if */

    m->flags |= TAG_MENU_FLAG_ALREADY_CREATED;

    assert(m->nod);

    outf = html_create(m->nod);
    fprintf(outf, "      <ul>\n");

    DEB(FLAG_DEBUG_CREATE_MENU,
            "MEN: name=\"%s\", ntags=%d\n",
        m->id, m->ntags);

    /* follow the list of files for this id */
    for (   i = avl_tree_first(m->group_by_file);
            i;
            i = avl_iterator_next(i))
    {
        ctag *t1 = avl_iterator_data(i), *t2;

        assert(t1);
        assert(t1->nod);
        assert(t1->nod->full_name);

        DEB(FLAG_DEBUG_CREATE_MENU,
                "FILE: %s\n", (const char *)avl_iterator_key(i));

        DEB(FLAG_DEBUG_CREATE_MENU,
                "step: file = %s\n",
            t1->nod->full_name);

        fprintf(outf,
            "        <li class=\"menu file\">File "
            "<span class=\"file\">%s</span>.\n"
            "          <ul>\n",
            t1->nod->full_name);

        for(t2 = t1; t2; t2 = t2->next_in_file) {
            char *rp = rel_path(m->nod->html_file, t2->nod->html_file);

            DEB(FLAG_DEBUG_CREATE_MENU,
                    "substep: ctag[%p] = <%s,%s,%p>, next=[%p]\n",
                t2, t2->id, t2->nod->full_name, t2->ss, t2->next_in_file);
            DEB(FLAG_DEBUG_CREATE_MENU,
                    "substep: <a href=\"%s#%s-%d\">%s(%d)</a>\n",
                rp,
                t2->id, t2->tag_no_in_file,
                t2->id, t2->tag_no_in_file);
            fprintf(outf,
                "            <li class=\"menu tag\">"
                "<a href=\"%s#%s-%d\">%s(%d)</a></li>\n",
                rp,
                t2->id, t2->tag_no_in_file,
                t2->id, t2->tag_no_in_file);
        } /* for */

        fprintf(outf,
            "          </ul></li><!-- File %s -->\n",
            t1->nod->full_name);
        DEB(FLAG_DEBUG_CREATE_MENU,
                "step: end\n");
    } /* for */

    fprintf(outf, "      </ul>\n");

    html_close(m->nod);

} /* create_menu */


void scanfile (const node *fin)
{
    FILE *INPUT;
    YY_BUFFER_STATE bs;
    char buffer[64];
    const node *fout;

    assert(fout = fin->html_file);

    DEB(FLAG_DEBUG_SCANFILE,
            "Generating [%s-->%s]\n",
            fin->full_name, fout->full_name);

    INPUT = fopen (fin->full_name, "r");
    if (!INPUT) {
        ERR(1, "fopen(READ):%s:%s(errno=%d)\n",
            fin->full_name, strerror(errno), errno);
    } /* if */
    OUTPUT = html_create(fin);
    if (!OUTPUT) {
        ERR(1, "fopen(WRITE):%s:%s(errno=%d)\n",
            fout->full_name, strerror(errno), errno);
        exit(EXIT_FAILURE);
    } /* if */
    bs = yy_create_buffer (INPUT, YY_BUF_SIZE);
    yy_switch_to_buffer (bs);

    fprintf(OUTPUT,
			"    <div id=\"listing\"><div id=\"left\"></div>\n"
			"      <pre id=\"right\">\n");
    fprintf(OUTPUT, "<!---------------------------------------------------->\n");
    pointer=0;
    PUSH(INITIAL);
    newline(TRUE);
    yylex(fin, fout);
    fprintf(OUTPUT, "<!---------------------------------------------------->\n");
    fprintf(OUTPUT,
			"      </pre>\n"
			"    </div>\n");
    fclose (INPUT);
    html_close(fin);
    yy_delete_buffer(bs);
} /* scanfile */
