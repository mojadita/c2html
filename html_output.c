/* $Id: html_output.c,v 1.7 2014/09/09 20:22:07 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Mon Jun 14 23:55:37 MEST 1999
 * Copyright: (c) 1999-2024 Luis Colorado.  All rights reserved.
 *
 *  C2HTML -- A program to convert C source code into cross referenced HTML.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#define IN_HTML_OUTPUT_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>

#include "configure.h"
#include "debug.h"
#include "node.h"
#include "c2html.h"

#include "html_output.h"

/* constants */
static const char FILE_TYPE_UNKNOWN[] = "&lt;&lt;FILE TYPE UNKNOWN&gt;&gt;";

/* types */

/* prototypes */

/* variables */

/* functions */
void fprintf_html(FILE *f, const char *fmt, ...)
{
    char buffer[4096];
    va_list p;
    char *s = buffer;

    va_start(p, fmt);
    vsnprintf(buffer, sizeof buffer, fmt, p);
    va_end(p);

    while (*s) {
        switch (*s) {
        case '<': fprintf(f, "&lt;"); break;
        case '>': fprintf(f, "&gt;"); break;
        case '&': fprintf(f, "&amp;"); break;
        default: fputc(*s, f); break;
        }
        s++;
    } /* while */
} /* fprintf_html */


int path_print(FILE *f, const node *p)
{
    int i;
    int res = 0;

    assert(p);
    DEB(FLAG_DEBUG_PROCESS_FILE,
        " start '%s'\n",
        p->full_name);
    if (!p->html_file) {
        ERR(EXIT_FAILURE,
            "f is NULL!!! (in node '%s')\n",
            p->full_name);
    }

    res += fprintf(f, "<span class=\"path\">");
    const char *sep = "";
    for (i = 0; i < p->level-1; i++) {
        res += fprintf(f,
            "%s<a href=\"%s\">%s</a>",
            sep,
            rel_path(p->html_file, p->path[i]->html_file),
            p->path[i]->name);
        sep = "/";
    } /* for */
    res += fprintf(f,
        "%s%s", sep, p->name);
    res += fprintf(f, "</span>");
    return res;
} /* path_print */

int
html_generate_ref(
        FILE *o,            /* where to write the reference in */
        const char *ident,  /* the reference name to write. */
        const node *fin)    /* file in which the reference is found */
{
    int res = 0;

    tag_menu *men = avl_tree_get(db_menus, ident);
    if (men) { /* there is an entry in menus database */
        DEB(FLAG_DEBUG_PROCESS_IDENT,
            "\"%s\" found!!! check if ntags > 1\n",
            men->id);
        if (men->ntags > 1) { /* multiple entry in database */

            char *rp;
            rp = rel_path(fin->html_file, men->nod->html_file);

            DEB(FLAG_DEBUG_PROCESS_IDENT,
                "<a href=\"%s\">%s</a>\n",
                rp, men->id);

            fprintf(o,
                "<a href=\"%s\">%s</a>",
                rp, men->id);

        } else { /* single entry */

            char *rp;
            ctag *tag = men->last_tag;

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
                "menu %s: writing <a href=\"%s#%s-%d\">%s</a>\n",
                tag->id,
                rp, tag->id, tag->tag_no_in_file, tag->id);
            fprintf(o, "<a href=\"%s#%s-%d\">%s</a>",
                rp, tag->id, tag->tag_no_in_file, tag->id);
        } /* if */
    } else { /* no entry found, print it plain */
        DEB(FLAG_DEBUG_PROCESS_IDENT,
            "%s not found, print plain.\n", ident);
        fprintf(o, "%s", ident);
    } /* if */
    return res;
} /* html_generate_ref */

void create_menu(tag_menu *m)
{
    assert(m != NULL && m->nod != NULL);

    DEB(FLAG_DEBUG_CREATE_MENU,
        "begin: menu=\"%s\"\n", m->id);

    FILE *outf = html_create(m->nod);
    fprintf(outf, "      <ul>\n");

    DEB(FLAG_DEBUG_CREATE_MENU,
        "MEN: name=\"%s\", ntags=%d\n",
        m->id, m->ntags);

    /* follow the list of files for this id */
    for (AVL_ITERATOR i = avl_tree_first(m->group_by_file);
            i;
            i = avl_iterator_next(i))
    {
        ctag *t1 = avl_iterator_data(i);

        assert(t1 != NULL && t1->nod != NULL && t1->nod->full_name != NULL);

        DEB(FLAG_DEBUG_CREATE_MENU,
            "MENU: %s\n", (const char *)avl_iterator_key(i));

        DEB(FLAG_DEBUG_CREATE_MENU,
            "step: file = %s\n",
            t1->nod->full_name);

        fprintf(outf,
            "        <li class=\"menu file\">File "
            "<span class=\"file\">%s</span>.\n"
            "          <ul>\n",
            t1->nod->full_name);

        for(ctag *t2 = t1; t2; t2 = t2->next_in_file) {
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

FILE *
html_create(node *n)
{
    assert(n != NULL);

    FILE *f = n->index_f = fopen(n->html_file->full_name, "w");
    if (!f) {
        ERR(EXIT_FAILURE,
            "fopen:%s:%s(errno=%d)\n",
            n->html_file->full_name,
            strerror(errno), errno);
    } /* if */

    const char *typ;

    switch(n->type) {
    case TYPE_DIR:    typ = "Directory";       break;
    case TYPE_SOURCE: typ = "File";            break;
    case TYPE_MENU:   typ = "MENU";            break;
    case TYPE_HTML:   typ = "HTML";            break;
    default:          typ = FILE_TYPE_UNKNOWN; break;
    } /* switch */

    fprintf(f,
"<!-- WARNING: This file was generated automatically with "PROGNAME", "
        "do not edit.\n"
"   - "PROGNAME" "VERSION"\n"
"   -   A program to convert C source code into cross referenced HTML.\n"
"   - Copyright (C) 1999-2024 "AUTHOR_NAME".  All rights reserved.\n"
"   -\n"
"   - This program is free software; you can redistribute it and/or modify\n"
"   - it under the terms of the GNU General Public License as published by\n"
"   - the Free Software Foundation; either version 2 of the License, or\n"
"   - (at your option) any later version.\n"
"   -\n"
"   - This program is distributed in the hope that it will be useful,\n"
"   - but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"   - MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"   - GNU General Public License for more details.\n"
"   -\n"
"   - You should have received a copy of the GNU General Public License\n"
"   - along with this program; if not, write to the Free Software\n"
"   - Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n"
"  -->\n"
"<html>\n"
"  <head>\n"
"    <title>%s %s</title>\n"
"    <link rel=\"stylesheet\" type=\"text/css\" href=\"%s\">\n",
        typ, n->full_name,
        rel_path(n->html_file, style_node));

    fprintf(f,
"    <script src=\"%s\"></script>\n"
"  </head>\n"
"  <body>\n"
"    <div class=\"title\">%s ",
        rel_path(n->html_file, js_node),
        typ);

    path_print(f, n);

    fprintf(f, "</div>\n"
"    <hr/>\n"
        );
    return f;
} /* create_html */

void html_close(node *n)
{
    const char *typ;

    assert(n);

    node *hn = n->html_file;
    FILE *f = n->index_f;

    switch(n->type) {
    case TYPE_DIR:    typ = "Directory";       break;
    case TYPE_SOURCE: typ = "File";            break;
    case TYPE_MENU:   typ = "TAG";             break;
    case TYPE_HTML:   typ = "HTML";            break;
    default:          typ = FILE_TYPE_UNKNOWN; break;
    } /* switch */


    fprintf(f,
"    <hr/>\n"
"    <div class=\"title\">%s ",
        typ);

    path_print(f, n);

    fprintf(f, "</div>\n"
"    <p><a href=\""PACKAGE_URL "\">"
        PROGNAME" "VERSION "</a>: Copyright (C) 1999-2014 "
        "<a href=\"mailto:" AUTHOR_EMAIL "?subject=c2html\">"
        AUTHOR_NAME " &lt;" AUTHOR_EMAIL "&gt;</a>\n"
"  </body>\n"
"</html>\n"
"<!-- %s -->\n",
        hn->full_name);

    fclose(f);

    n->index_f = NULL;
} /* close_html */

/* $Id: html_output.c,v 1.7 2014/09/09 20:22:07 luis Exp $ */
