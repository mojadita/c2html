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

#include "debug.h"
#include "intern.h"
#include "node.h"
#include "menu.h"
#include "ctag.h"
#include "c2html.h"

static AVL_TREE db_ctag = NULL;

/* static function prototypes */
static int print_ctag_key(FILE *f, const ctag *a);
static int ctag_cmp(const ctag *a, const ctag *b);

ctag *lookup_ctag(const char *id, const char *fi, const char *ss, node *root)
{
    ctag *res;
    node *nod;
    tag_menu *men;

    DEB(FLAG_DEBUG_CTAGS,
            "ctags: begin <%s|%s|%p>\n", id, fi, ss);

    if (!db_ctag) {
        DEB(FLAG_DEBUG_CTAGS,
                "ctags: initializing db_ctag database\n");
        D(db_ctag = new_avl_tree(
				(AVL_FCOMP) ctag_cmp, NULL, NULL,
				(AVL_FPRNT) print_ctag_key));
        if (!db_ctag) {
            ERR(EXIT_FAILURE,
					"ctags: Cannot allocate AVL_TREE\n");
			/* NOTREACHED */
        }
    } /* if */

    ctag key;

    key.id = id = intern(id);
    key.fi = fi = intern(fi);
    key.ss = ss = intern(ss);

    DEB(FLAG_DEBUG_CTAGS,
            "ctags: looking for tag <%s|%s|%p>\n",
			id, fi, ss);
    res = avl_tree_get(db_ctag, &key);
    if (!res) {
        DEB(FLAG_DEBUG_CTAGS,
                "ctags: key not found, creating it\n");
        res = malloc(sizeof(ctag)); /* allocate memory */
        if (!res) {
            ERR(EXIT_FAILURE,
                "ctags: cannot allocate mem: %s\n",
                strerror(errno));
            /* NOTREACHED */
        }

		/* init ctags node */
        res->id = id;
        res->fi = fi;
        res->ss = ss;

        /* get the node this tag belongs to */
        nod = res->nod = name2node(root, fi, TYPE_FILE);

        assert(nod && (nod->type == TYPE_FILE));

        /* insert the ctag in list of tags in the same node.
         * we reuse the subnodes field of the file nod*/
        res->next_in_file = avl_tree_get(nod->subnodes, id);
        res->tag_no_in_file = res->next_in_file
            ? res->next_in_file->tag_no_in_file + 1
            : 1;
        avl_tree_put(nod->subnodes, id, res);

        men = lookup_menu(res->id, root);
        DEB(FLAG_DEBUG_CTAGS,
                "ctags: lookup_menu(%s)\n", men->id);

        /* insert ctag in list corresponding to file. */
        avl_tree_put(men->group_by_file, nod->full_name, res);
        men->ntags++;
        men->last_tag = res; /* last registered tag, for one node menus */
    } /* if */

    DEB(FLAG_DEBUG_CTAGS,
            "ctags: end\n");
    return res;
} /* lookup_ctag */

static int print_ctag_key(FILE *f, const ctag *a)
{
    return fprintf(f, "%s|%s|%p", a->fi, a->id, a->ss);
} /* print_ctag_key */

/* this function compares the unique index of <id,fi,ss>
 * for the db_ctag database */
static int ctag_cmp(const ctag *a, const ctag *b)
{
    int res;
    res = strcmp(a->fi, b->fi); /* file is the most significative */
    if (res == 0) {
        res = strcmp(a->id, b->id); /* id is the next most significative */
        if (res == 0) {
            /* ss is the least significative, and we don't mind the values,
             * so we just compare the pointers. As all the strings are
             * interned, two equal strings implies their
             * addresses are also equal */
			res = a->ss - b->ss;
        } /* if */
    } /* if */
    return res;
} /* ctag_cmp */

/* $Id: ctag.c,v 1.1 2014/09/09 20:22:06 luis Exp $ */
