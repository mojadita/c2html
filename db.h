/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 21:13:42 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO.  All rights reserved.
 */
#ifndef _DB_H
#define _DB_H

#include <stdio.h>
#include <avl.h>

typedef struct ctag_s {
	const char		*id; /* tag identifier */
	const char		*fi; /* file this tag points to. */
	const char		*ss; /* scan string for ex(1) */
	int			tag_no; /* tag number */
	const struct ctag_s	*next; /* next tag with the same id in this tag file */
	//const struct ctag_s	*next_in_nod;
	struct node_s		*nod; /* file node this tag points to. */
}	*ctag_p,
	ctag;

typedef enum node_type_e {
	FLAG_ISDIR,
	FLAG_ISFILE,
}	*node_type_p,
	node_type;

typedef struct node_s {
	const char		*name; /* name of this node. */
	struct node_s		*parent; /* parent node of this. */
	node_type		type; /* type of this node. */
	AVL_TREE		subnodes; /* children of this node */
	int 			level; /* level of this node */
	const char		*full_name; /* full path name */
	FILE			*index_f; /* index.html file descriptor */
}	*node_p,
	node;

extern AVL_TREE db_ctag;
extern node_p db_root_node;
extern int n_files;

void db_init(const char *);
void mk_dir(const node *nod);
ctag_p ctag_lookup(const char *id, const char *fi, const char *ss);
ctag_p ctag_lookup_by_id(const char *id);

#endif /* _DB_H */
/* $Id$ */
