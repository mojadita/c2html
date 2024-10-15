/* $Id: node.h,v 1.1 2014/09/09 20:23:06 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: sáb ago 23 21:13:42 EEST 2014
 * Copyright: (C) 2014-2024 LUIS COLORADO.  All rights reserved.
 */
#ifndef _NODE_H
#define _NODE_H

#include <stdio.h> /* for FILE */
#include <avl.h>

/* if modify something to this type, do it also in
 * node.c, definition of type2string string table. */
typedef enum node_type_e {
    TYPE_DIR,       /* used for directories */
    TYPE_SOURCE,    /* used for normal files */
    TYPE_HTML,      /* used for the HTML files associated */
    TYPE_MENU,      /* used for menu files */
} node_type;

#define NODE_FLAG_NONE                  0x00000000
#define NODE_FLAG_DONT_RECUR_PREORDER   0x00000001
#define NODE_FLAG_DONT_RECUR_POSTORDER  0x00000002
#define NODE_FLAG_DONT_RECUR_INFILE     0x00000004
#define NODE_FLAG_ALL                   0x00000007

extern int n_dir;
extern int n_file;
extern int n_html;

typedef struct node_s node;

struct node_s {
    node_type    type;       /* type of this node. */
    const char  *name;       /* name of this node. */
    node        *parent;     /* parent node of this. */
    int          flags;      /* flags, see above. */
    int          level;      /* level of this node, root node is at level 1 */
    node       **path;       /* path from the root */
    const char  *full_name;  /* full path name */
    const char  *orig_name;  /* original filename */
    node        *html_file;  /* html assoc. file. valid for files and
                              * directories */
    AVL_TREE     subnodes;   /* children of this node, if any. type of nodes
                              * will depend on the type of this node. */
    FILE        *index_f;    /* FILE descriptor for associated file (e.g.
                              * for completing th associated html file) */
};

node *
new_node(
        const char *name,
        node       *parent,
        node_type   typ);

node *
name2node(
        node       *root,
        const char *path,
        node_type   typ);

int
common_prefix(
        const node *a,
        const node *b);

char *
rel_path(
        const node *a,
        const node *b);

typedef int (*node_callback)(node *node, void *closure);

int
do_recur(node *nod,
    node_callback dir_pre,
    node_callback file_in,
    node_callback dir_pos,
    void *closure);

void fprint_node(FILE *f, const node *n);

#endif /* _NODE_H */
/* $Id: node.h,v 1.1 2014/09/09 20:23:06 luis Exp $ */
