/* $Id: ctag.c,v 1.1 2014/09/09 20:22:06 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: sáb ago 23 22:34:53 EEST 2014
 * Disclaimer: (C) 2014-2024 LUIS COLORADO. All rights reserved.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "configure.h"
#include "debug.h"
#include "intern.h"
#include "node.h"
#include "menu.h"
#include "ctag.h"
#include "c2html.h"

static AVL_TREE db_ctag = NULL;

/* static forward function prototypes */
static int print_ctag_key(FILE *f, const ctag *a);
static int ctag_cmp(const ctag *a, const ctag *b);

ctag *
lookup_ctag(
        const char *id, /* tag id */
        const char *fi, /* original filename */
        const char *ss,
        node *root)
{
    ctag *res;
    tag_menu *men;

    DEB(FLAG_DEBUG_CTAGS,
        " start: tag='%s', file='%s', ss='%p'\n",
        id, fi, ss);

    if (!db_ctag) {
        DEB(FLAG_DEBUG_CTAGS,
            " initializing db_ctag database\n");
        db_ctag = new_avl_tree(
                (AVL_FCOMP) ctag_cmp,
                NULL, NULL,
                (AVL_FPRNT) print_ctag_key);
        if (!db_ctag) {
            ERR(EXIT_FAILURE,
                " db_ctag: %s\n", strerror(errno));
        }
    } /* if */

    ctag key = { .id = id, .fi = fi, .ss = ss };

    DEB(FLAG_DEBUG_CTAGS,
        " looking for tag <id=%s|file=%s|ss='%s'>\n",
        id, fi, ss);
    res = avl_tree_get(db_ctag, &key);
    if (!res) {

        DEB(FLAG_DEBUG_CTAGS,
            " ctag <id=%s|file=%s|ss='%s'> "
            "not found, creating it\n",
            id, fi, ss);
        res = malloc(sizeof(ctag)); /* allocate memory */
        if (!res) {
            ERR(EXIT_FAILURE,
                " malloc: %s\n",
                strerror(errno));
            /* NOTREACHED */
        }

        /* init ctags node */
        *res = key;

        /* get the node this tag belongs to */
        node *nod = res->nod
                  = name2node(root, fi, TYPE_SOURCE);

        assert(nod && (nod->type == TYPE_SOURCE));

        /* insert the ctag in list of tags in the same node.
         * We use the subnodes field of the file node */
        if (!nod->subnodes) {
            DEB(FLAG_DEBUG_CTAGS,
                "file '%s': creating ctags list for tag '%s'\n",
                nod->full_name, id);
            nod->subnodes = new_avl_tree(
                 (AVL_FCOMP) strcmp,
                 NULL, NULL,

                 (AVL_FPRNT) fputs);
        }
        /* it's a linked list of tags. */
        res->next_in_file = avl_tree_get(nod->subnodes, id);
        res->tag_no_in_file = res->next_in_file
            ? res->next_in_file->tag_no_in_file + 1
            : 0;
        avl_tree_put(nod->subnodes, id, res);

        men = lookup_menu(res->id, root);

        /* insert ctag in list corresponding to file. */
        avl_tree_put(men->group_by_file, nod->full_name, res);
        men->ntags++;
        men->last_tag = res; /* last registered tag, for one node menus */
    } /* if */

    DEB(FLAG_DEBUG_CTAGS,
        " end id='%s', fi='%s', ss=%p('%s')\n",
        id, fi, ss, ss);
    return res;
} /* lookup_ctag */

static int print_ctag_key(FILE *f, const ctag *a)
{
    return fprintf(f, "<%s|%s|%p>", a->fi, a->id, a->ss);
} /* print_ctag_key */

/* this function compares the unique index of <id,fi,ss>
 * for the db_ctag database */
static int
ctag_cmp(const ctag *a, const ctag *b)
{
    int res = strcmp(a->fi, b->fi); /* file is the most significative */
    if (res != 0) return res;
    res = strcmp(a->id, b->id); /* id is the next most significative */
    if (res != 0) return res;
    return strcmp(a->ss, b->ss);
} /* ctag_cmp */

void fprint_ctag(FILE *f, const ctag *t)
{
#define P(fld, _fmt) fprintf(stderr, "%20s: " _fmt "\n", #fld, t->fld)
    P(id, "%s");
    P(fi, "%s");
    P(ss, "%s");
    P(tag_no_in_file, "%d");
    P(next_in_file, "%p");
    P(nod, "%p");
#undef P
} /* fprint_ctag */

/* $Id: ctag.c,v 1.1 2014/09/09 20:22:06 luis Exp $ */
