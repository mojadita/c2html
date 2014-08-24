/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 21:13:42 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO.  All rights reserved.
 */
#ifndef _DB_H
#define _DB_H

#include <avl.h>

typedef struct ctag_s {
	const char		*id; /* tag identifier */
	const char		*fi; /* file this tag points to. */
	const char		*ss; /* scan string for ex(1) */
	int				tag_no; /* tag number */
	struct ctag_s	*next; /* next tag with the same id in this tag file */
	struct node_s	*nod;
	const char		**path; /* path component strings, NULL terminated */
} ctag, *ctag_p;

#define FLAG_ISDIR		(1 << 0)
#define FLAG_ISFILE		(1 << 1)
typedef struct node_s {
	const char		*name; /* name of this node. */
	struct node_s	*parent; /* parent node of this node. */
	int				flags; /* flags of this node. */
	AVL_TREE		subnodes; /* children of this node */
	int 			level;
} node, *node_p;

extern AVL_TREE db_ctag;
extern AVL_TREE db_ctag_ix_id;
extern node db_root_node;

ctag_p ctag_lookup(const char *id, const char *fi, const char *ss);
ctag_p ctag_lookup_by_id(const char *id);

#endif /* _DB_H */
/* $Id$ */
