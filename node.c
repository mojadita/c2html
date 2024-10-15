/* $Id: node.c,v 1.1 2014/09/09 20:23:06 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: sáb ago 23 22:34:53 EEST 2014
 * Copyright: (C) 2014-2024 LUIS COLORADO.  All rights reserved.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "debug.h"
#include "intern.h"
#include "node.h"
#include "c2html.h"

#define BUFFER_SIZE             4096
#define NULL_POINTER_STRING     "<<NULL>>"

int n_dir = 0;
int n_file = 0;
int n_html = 0;

/* if you modify the enum node_type_e list,
 * you have also to modify here. */
static char *type2string[] = {
    "TYPE_DIR",
    "TYPE_SOURCE",
    "TYPE_HTML",
    "TYPE_MENU",
};

node *
new_node(
        const char *name,
        node       *parent,
        node_type   typ)
{
    node *res;

    assert(name != NULL);
    name = intern(name); /* intern is an idempotent function */

    DEB(FLAG_DEBUG_NODES,
        "start: name='%s', parent=%p, type='%s'\n",
        name, parent, type2string[typ]);

    if (       parent
            && parent->subnodes
            && (res = avl_tree_get(parent->subnodes, name)))
    {
        /* already existent */
        assert(res->type == typ);
        return res;
    }

    /* not found, create it */
    res = malloc(sizeof (node)); /* get memory */
    if (!res) {
        ERR(EXIT_FAILURE,
            "malloc: %s\n",
            strerror(errno));
    }

    res->name     = name;
    res->parent   = parent;
    res->type     = typ;
    res->flags    = 0;
    res->level    = parent
        ? parent->level + 1
        : 1;
    res->subnodes = NULL;
    res->index_f  = NULL;
    res->menu     = NULL;

    DEB(FLAG_DEBUG_NODES,
        "[%s][%s]: malloc() -> %p\n",
        parent
            ? parent->full_name
            : "",
        name,
        res);

    /* construct the path to it. use + 1
     * to alloc for a NULL pointer at end. */
    {   node *p = res;
        int i;

        res->path = calloc(
            res->level + 1,
            sizeof *res->path);
        DEB(FLAG_DEBUG_NODES,
            "construct res->path = %p\n",
            res->path);
        res->path[res->level] = NULL;
        for (i = res->level - 1; i >= 0; i--) {
            res->path[i] = p;
            DEB(FLAG_DEBUG_NODES,
                "Setting res->path[%d] = '%s'(%p)\n",
                i, p->name, p);
            p = p->parent;
        } /* for */
    } /* block */

    /* construct the full name */
    {   char buffer[BUFFER_SIZE];
        size_t bs = sizeof buffer, n;
        char *aux = buffer;
        char *sep = "";

        for (int i = 0; i < res->level; i++) {
            n = snprintf(aux, bs,
                "%s%s",
                sep,
                res->path[i]->name);
            sep = "/"; aux += n; bs -= n;
        } /* for */

        res->full_name = intern(buffer);
    } /* block */

    /* if directory, create the subnodes avltree */
    switch (res->type) {
    case TYPE_DIR:
        DEB(FLAG_DEBUG_NODES,
            "'%s': create subnodes map\n",
            res->full_name);
        res->subnodes = new_avl_tree(
                (AVL_FCOMP) strcmp,
                NULL, NULL,
                (AVL_FPRNT) fputs);
        /* a directory will have an index.html file inside.
         * This file is not linked to the parent, so it will not be
         * added to the directory tree. */
        res->html_file = new_node(
                "index.html",
                res,
                TYPE_HTML);
        break;

    case TYPE_SOURCE: case TYPE_MENU:
        /* add an '.html' file for process_file to work properly */
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof buffer,
                "%s.html", res->name);
        /* a source will have an source.html file in html_file fiel.
         * This file is not linked to the parent, so it will not be
         * added to the directory tree either. */
        res->html_file = new_node(
                intern(buffer),
                res->parent,
                TYPE_HTML);
        break;
    default: break; /* nothing to do */
    } /* switch */

    /* add to parent directory if feasible */
    if (parent && parent->type == TYPE_DIR) {
        assert(parent->subnodes != NULL);
        avl_tree_put(parent->subnodes, res->name, res);

        DEB(FLAG_DEBUG_NODES,
            "'%s: added to parent '%s'\n",
            res->full_name, res->parent->full_name);
    } /* if */

    DEB(FLAG_DEBUG_NODES,
        "'%s'(%p): end\n",
        res->full_name, res);
    return res;
} /* new_node */

node *
name2node(
        node           *root,
        const char     *path,
        const node_type typ)
{
    /* we need a root file to begin search from */
    assert(root != NULL && path != NULL);

    char *name    = strdup(path); /* alloc a copy, to be freed at end */
    node *actual  = root;

    DEB(FLAG_DEBUG_NODES,
        "begin: root=\"%s\", path=\"%s\"\n",
        root->name, path);

    /* we cannot have absolute paths */
    assert(name[0] != '/');

    char *aux = NULL;
    for(const char *nam = name; nam; nam = aux) {
        node *next;

        DEB(FLAG_DEBUG_NODES,
            "step: parsing [%s>|<%s]\n",
            actual->full_name, nam);

        aux = strchr(nam, '/'); /* search for a '/' character */
        if (aux) /* if found, nullify it and every one char following it */
            while (*aux == '/')
                *aux++ = '\0';

        /* now, aux points to the next name component or NULL */
        /* nam is the component name of this element of the path */
        /* CHECK FOR SPECIAL "." ENTRY */
        if (!strcmp(nam, ".")) {
            DEB(FLAG_DEBUG_NODES,
                "component is \".\", ignored\n");
            continue; /* it it's the . entry. */
        } /* if */

        /* ... AND CHECK ALSO FOR ".." */
        if (!strcmp(nam, "..")) {
            if (actual->parent == NULL) {
                fprintf(stderr,
                    PR("error: \"..\" not allowed in %s\n"), path);
                exit(EXIT_FAILURE);
            } /* if */
            DEB(FLAG_DEBUG_NODES,
                "component is \"..\", special\n");
            actual = (node *) actual->parent;
            continue;
        } /* if */

        /* now we have a valid name */
        nam = intern(nam);

        /* lookup it on the subnodes field */
        if (actual->type != TYPE_DIR) {
            WRN("%s is not a directory, cannot "
                "search/create node [%s] on it\n",
                actual->full_name, nam);
            return NULL;
        } /* if */

        DEB(FLAG_DEBUG_NODES,
            "looking for [%s] in [%s]->subnodes\n",
            nam, actual->full_name);
        next = avl_tree_get(actual->subnodes, nam);
        if (!next) {
            DEB(FLAG_DEBUG_NODES,
                "[%s] not found, creating it in [%s]\n",
                nam, actual->full_name);

            /* all but the last in the hierarchy is a directory */
            next = new_node(
                nam, actual,
                aux ? TYPE_DIR
                    : typ);
        } /* if */

        DEB(FLAG_DEBUG_NODES,
            "step[%s]: end%s.\n",
            next->name,
            aux ? "... continue"
                : "");
        actual = next;
    } /* for */

    /* free the temporary copy of the name */
    free(name);

    DEB(FLAG_DEBUG_NODES,
        "end '%s'(%p)\n",
        actual->full_name, actual);
    return actual;
} /* name2node */

/* returns the length of the common
 * prefix of two nodes, a and b. */
int
common_prefix(
        const node *a,
        const node *b)
{
    int i = 0;
    while (    a->path[i]
            && b->path[i]
            && a->path[i] == b->path[i])
        i++;
    return i;
} /* common_prefix */

/* computes the relative path from a
 * to b. */
char *
rel_path(
        const node *a,
        const node *b)
{
    int c = common_prefix(a, b);
    int i;
    static char buffer[BUFFER_SIZE];
    size_t bs = sizeof buffer;
    char *p = buffer;
    int res, n = 0;

    DEB(FLAG_DEBUG_NODES,
        "begin: a=[%s], b=[%s]\n",
        a->full_name, b->full_name);
    DEB(FLAG_DEBUG_NODES,
        "common_prefix() -> %d\n", c);
    /* first the chain up */
    for (i = a->level-1; i > 0 && i > c; i--) {
        res = snprintf(p, bs, "%s..",
            n++ ? "/" : "");
        p += res; bs -= res;
    } /* for */
    /* now i == c or i == 0 */

    /* then follow it down to the target */
    while(i < b->level) {
        res = snprintf(p, bs, "%s%s",
            n++ ? "/" : "",
            b->path[i++]->name);
        p += res; bs -= res;
    } /* while */

    return buffer;
} /* rel_path */

#define INDENT ((nod->level << 2) - 1)

int
do_recur(
        node   *nod,
        node_callback pre,
        node_callback fil,
        node_callback pos,
        void         *clos)
{
    AVL_ITERATOR i;
    int res = 0;

    DEB(FLAG_DEBUG_NODES,
        "%*sENTER: %s: %s\n",
        INDENT, "",
        type2string[nod->type],
        nod->full_name);

    switch(nod->type) {
    case TYPE_DIR:
        if (!(nod->flags & NODE_FLAG_DONT_RECUR_PREORDER)
                && pre)
        {
            DEB(FLAG_DEBUG_NODES,
                "%*sBEFORE: %s: %s\n",
                INDENT, "",
                type2string[nod->type],
                nod->full_name);
            if ((res = pre(nod, clos)) != 0)
                return res;
        }
    default: break; /* nothing to do */
    } /* switch */
    switch(nod->type) {
    case TYPE_DIR:
        if (!(nod->flags & NODE_FLAG_DONT_RECUR_INFILE)) {
            for (   i = avl_tree_first(nod->subnodes);
                    i;
                    i = avl_iterator_next(i))
            {
                if ((res = do_recur(
                        avl_iterator_data(i),
                        pre, fil, pos, clos)) != 0)
                    return res;
            } /* for */
        } /* if */
        break;
    case TYPE_SOURCE: case TYPE_MENU:
        if (!(nod->flags & NODE_FLAG_DONT_RECUR_INFILE)
                && fil)
        {
            DEB(FLAG_DEBUG_NODES,
                "%*sINFILE: %s: %s\n",
                INDENT, "",
                type2string[nod->type],
                nod->full_name);
            if ((res = fil(nod, clos)) != 0) {
                return res;
            }
        }
    default: break; /* nothing to do */
    } /* switch */
    switch(nod->type) {
    case TYPE_DIR:
        if (!(nod->flags & NODE_FLAG_DONT_RECUR_POSTORDER) && pos) {
            DEB(FLAG_DEBUG_NODES,
                "%*sAFTER: %s: %s\n",
                INDENT, "",
                type2string[nod->type],
                nod->full_name);
            if ((res = pos(nod, clos)) != 0) return res;
        }
        break;
    default: break; /* nothing to do */
    } /* switch */

    DEB(FLAG_DEBUG_NODES,
        "%*sLEAVE: %s: %s\n",
        INDENT, "",
        type2string[nod->type],
        nod->full_name);
    return res;
} /* do_recur */

void fprint_node(FILE *f, const node *n)
{
#define P(fld, _fmt) fprintf(stderr, "%20s: "_fmt"\n", #fld, n->fld)
    P(type, "%d");
    P(name, "%s");
    P(parent, "%p");
    P(flags, "%02x");
    P(level, "%d");
    for (int i = 0; i < n->level; i++)
        P(path[i], "%p");
    P(full_name, "%s");
    P(html_file, "%p");
    P(subnodes, "%p");
    P(index_f, "%p");
    P(menu, "%p");
#undef P
}

/* $Id: node.c,v 1.1 2014/09/09 20:23:06 luis Exp $ */
