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
    "TYPE_FILE",
    "TYPE_HTML",
};

node *new_node(const char *name, const node *parent, const node_type typ)
{
    node *res;

    assert(name);
    name = intern(name); /* intern is an idempotent function */

    DEB(FLAG_DEBUG_NODES,
        "begin: name='%s', parent=%p, type='%s'\n",
        name, parent, type2string[typ]);

    if (parent)
        assert(avl_tree_get(parent->subnodes, name) == NULL);

    assert(res = malloc(sizeof (node))); /* get memory */
    DEB(FLAG_DEBUG_NODES, "malloc() -> %p\n", res);
    res->name = name;
    res->parent = parent;
    res->type = typ;
    res->flags = 0;
    res->level = parent ? parent->level + 1 : 1;
    assert(res->subnodes = new_avl_tree(
        (AVL_FCOMP) strcmp, NULL, NULL,
        (AVL_FPRNT) fputs));
    DEB(FLAG_DEBUG_NODES,
        "res->subnodes(AVL_TREE) = %p\n", res->subnodes);
    res->index_f = NULL;

    /* construct the path to it. use + 1
     * to alloc for a NULL pointer at end. */
    assert(res->path = calloc(res->level + 1, sizeof (node *)));
    DEB(FLAG_DEBUG_NODES, "construct res->path = %p\n", res->path);
    {   const node *p = res;
        int i;

        res->path[res->level] = NULL;
        for (i = res->level-1; i >= 0; i--) {
            res->path[i] = p;
            DEB(FLAG_DEBUG_NODES,
                "Setting res->path[%d] = %p('%s')\n",
                i, p, p->name);
            p = p->parent;
        } /* for */
    } /* block */

    /* construct the full name */
    {   char buffer[BUFFER_SIZE];
        size_t bs = sizeof buffer, n;
        char *aux = buffer;
        char *sep = "";

        for (int i = 0; i < res->level; i++) {
            n = snprintf(aux, bs, "%s%s",
                sep,
                res->path[i]->name);
            sep = "/"; aux += n; bs -= n;
        } /* for */

        res->full_name = intern(buffer);
    } /* block */
    DEB(FLAG_DEBUG_NODES,
            "res->full_name = '%s'(%p)\n",
            res->full_name, res->full_name);

    /* add to parent directory */
    if (parent) {
        avl_tree_put(parent->subnodes, res->name, res);
        DEB(FLAG_DEBUG_NODES,
            "added to parent '%s'\n",
            res->parent->full_name);
    } /* if */

    /* now, add its html_file, if existent. */
    switch(res->type) {

    case TYPE_DIR: {
            char *name = "index.html";
            DEB(FLAG_DEBUG_NODES,
                    "create subnode [%s] for TYPE_DIR\n", name);
            res->html_file = new_node(name, res, TYPE_HTML);
            avl_tree_put(res->subnodes, name, res->html_file);
            res->flags = NODE_FLAG_NONE;
            n_dir++;
        } break;

    case TYPE_FILE: {
            char name[BUFFER_SIZE];
            assert(parent);
            snprintf(name, sizeof name, "%s.html", res->name);
            DEB(FLAG_DEBUG_NODES,
                    "create subnode [%s] in parent [%s] for TYPE_FILE\n",
                name, parent->full_name);
            res->html_file = new_node(name, parent, TYPE_HTML);
            avl_tree_put(parent->subnodes, res->html_file->name, (void *) res->html_file);
            res->flags = NODE_FLAG_DONT_RECUR_PREORDER | NODE_FLAG_DONT_RECUR_POSTORDER;
            n_file++;
        } break;

    case TYPE_HTML:
        res->flags = NODE_FLAG_ALL; /* don't pass through this file in do_recur() */
        res->html_file = res;
        n_html++;
        break;

    } /* switch */

    DEB(FLAG_DEBUG_NODES, "end [res=%p]\n", res);
    return res;
} /* new_node */

node *name2node(node *root, const char *p, const node_type typ)
{
    char *aux, *name;
    const char *nam;
    node *nod;

    assert(root); /* we need a root file to begin search from */
    assert(p); /* we need also a path */

    name = strdup(p); /* alloc a copy, to be returned at end */
    nod = root;

    DEB(FLAG_DEBUG_NODES,
        "begin: root=\"%s\", path=\"%s\"\n",
            root->name, p);

    /* we cannot have absolute paths */
    assert(name[0] != '/');

    for(nam = name; nam; nam = aux) {
        node *next;

        DEB(FLAG_DEBUG_NODES,
            "step: parsing [%s][%s]\n", nod->full_name, nam);

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
            if (nod->parent == NULL) {
                fprintf(stderr,
                    PR("error: \"..\" not allowed in %s\n"), p);
                exit(EXIT_FAILURE);
            } /* if */
            DEB(FLAG_DEBUG_NODES,
                "component is \"..\", special\n");
            nod = (node *) nod->parent;
            continue;
        } /* if */

        /* now we have a valid name */
        nam = intern(nam);

        /* lookup it on the subnodes field */
        if (nod->type != TYPE_DIR) {
            fprintf(stderr,
                PR("%s is not a directory, cannot search/create node [%s] on it\n"),
                nod->full_name, nam);
            return NULL;
        } /* if */

        DEB(FLAG_DEBUG_NODES,
            "looking for [%s] in [%s]->subnodes\n",
            nam, nod->full_name);
        next = avl_tree_get(nod->subnodes, nam);
        if (!next) {
            DEB(FLAG_DEBUG_NODES,
                "[%s] not found, creating it in [%s]\n",
                nam, nod->full_name);

            /* all but the last in the hierarchy is a directory */
            next = new_node(
                nam, nod,
                aux ? TYPE_DIR
                    : typ);
        } /* if */

        DEB(FLAG_DEBUG_NODES,
            "step[%s]: end%s.\n",
            next->name,
            aux
                ? "... next"
                : "");
        nod = next;
    } /* for */

    /* free the temporary copy of the name */
    free(name);

    DEB(FLAG_DEBUG_NODES,
        "end '%s' -> %p\n",
        nod->full_name, nod);
    return nod;
} /* name2node */

/* returns the length of the common
 * prefix of two nodes, a and b. */
int common_prefix(const node *a, const node *b)
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
char *rel_path(const node *a, const node *b)
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

int do_recur(
        const node *nod,
        node_callback pre,
        node_callback fil,
        node_callback pos,
        void *clos)
{
    AVL_ITERATOR i;
    int res = 0;

    DEB(FLAG_DEBUG_NODES,
        "%*sENTER: %s: %s\n",
        (nod->level<<2)-1, "",
        type2string[nod->type],
        nod->full_name);

    switch(nod->type) {
    case TYPE_DIR: case TYPE_FILE:
        if (!(nod->flags & NODE_FLAG_DONT_RECUR_PREORDER) && pre) {
            DEB(FLAG_DEBUG_NODES,
                    "%*sBEFORE: %s: %s\n",
                    (nod->level<<2)-1, "",
                    type2string[nod->type],
                    nod->full_name);
            if ((res = pre(nod, clos)) != 0)
                return res;
        }
    /* else nothing */
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
    case TYPE_FILE:
        if (!(nod->flags & NODE_FLAG_DONT_RECUR_INFILE) && fil) {
            DEB(FLAG_DEBUG_NODES,
                    "%*sINFILE: %s: %s\n",
                    (nod->level<<2)-1, "",
                    type2string[nod->type],
                    nod->full_name);
            if ((res = fil(nod, clos)) != 0) return res;
        }
    /* else nothing */
    } /* switch */
    switch(nod->type) {
    case TYPE_DIR: case TYPE_FILE:
        if (!(nod->flags & NODE_FLAG_DONT_RECUR_POSTORDER) && pos) {
            DEB(FLAG_DEBUG_NODES,
                    "%*sAFTER: %s: %s\n",
                    (nod->level<<2)-1, "",
                    type2string[nod->type],
                    nod->full_name);
            if ((res = pos(nod, clos)) != 0) return res;
        }
        break;
    /* else nothing */
    } /* switch */

    DEB(FLAG_DEBUG_NODES,
        "%*sLEAVE: %s: %s\n",
        (nod->level<<2)-1, "",
        type2string[nod->type],
        nod->full_name);
    return res;
} /* do_recur */

/* $Id: node.c,v 1.1 2014/09/09 20:23:06 luis Exp $ */
