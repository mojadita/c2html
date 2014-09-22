/* $Id: node.h,v 1.1 2014/09/09 20:23:06 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 21:13:42 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO.  All rights reserved.
 */
#ifndef _NODE_H
#define _NODE_H

#include <stdio.h> /* for FILE */
#include <avl.h>

/* if modify something to this type, do it also in
 * node.c, definition of type2string string table. */
typedef enum node_type_e {
	TYPE_DIR,		/* used for directories */
	TYPE_FILE,		/* used for normal files */
	TYPE_HTML,		/* used for the HTML files associated */
} node_type;

#define NODE_FLAG_NONE					0x00000000
#define NODE_FLAG_DONT_RECUR_PREORDER	0x00000001
#define NODE_FLAG_DONT_RECUR_POSTORDER	0x00000002
#define NODE_FLAG_DONT_RECUR_INFILE		0x00000004
#define NODE_FLAG_ALL					0x00000007

extern int n_dir;
extern int n_file;
extern int n_html;

typedef struct node_s {
	const char				*name; /* name of this node. */
	const struct node_s		*parent; /* parent node of this. */
	node_type				type; /* type of this node. */
	int						flags; /* flags, see above. */
	int 					level; /* level of this node, root node is at level 1 */
	const struct node_s		**path; /* path from the root */
	const char				*full_name; /* full path name */
	struct node_s			*html_file; /* html assoc. file. valid for files and directories */
	AVL_TREE				subnodes; /* children of this node */
	FILE					*index_f; /* index.html file descriptor (for html files) */
} node;

node *new_node(const char *name, const node *parent, const node_type typ);
node *name2node(node *root, const char *path, const node_type typ);
int common_prefix(const node *a, const node *b);
char *rel_path(const node *a, const node *b);

typedef int(*node_callback)(const node *dir, void *val);

int do_recur(const node *nod,
	node_callback dir_pre,
	void *val_pre,
	node_callback file_in,
	void *val_in,
	node_callback dir_pos,
	void *val_pos);

#endif /* _NODE_H */
/* $Id: node.h,v 1.1 2014/09/09 20:23:06 luis Exp $ */
