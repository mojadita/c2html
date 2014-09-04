/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 21:13:42 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO.  All rights reserved.
 */
#ifndef _NODE_H
#define _NODE_H

#include <stdio.h>
#include <avl.h>

typedef enum node_type_e {
	TYPE_DIR,		/* used for directories */
	TYPE_FILE,		/* used for normal files */
	TYPE_HTML,		/* used for the HTML files associated */
} node_type;

typedef struct node_s {
	const char		*name; /* name of this node. */
	struct node_s	*parent; /* parent node of this. */
	node_type		type; /* type of this node. */
	int 			level; /* level of this node, root node is at level 1 */
	struct node_s	**path; /* path from the root */
	const char		*full_name; /* full path name */
	struct node_s	*html_file; /* html assoc. file. valid for files and directories */
	AVL_TREE		subnodes; /* children of this node */
	FILE			*index_f; /* index.html file descriptor (for html files) */
} node;

extern node *db_root_node;
extern int n_files;

node *new_node(const char *name, node *parent, node_type typ);

int common_prefix(node *a, node *b);
char *rel_path(node *a, node *b);

#endif /* _NODE_H */
/* $Id$ */
