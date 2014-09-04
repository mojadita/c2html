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
	int				tag_no_in_file; /* tag number in file. */
	struct ctag_s	*next_in_file; /* next tag with the same id in this tag file */
	//struct ctag_s 	*next_in_menu; /* next tag of this name in <tag,file> database */
	//const struct ctag_s	*next_in_nod;
	struct node_s		*nod; /* file node this tag points to. */
} ctag;

typedef enum node_type_e {
	FLAG_ISDIR,
	FLAG_ISFILE,
	FLAG_TYPEMASK		=(1<<4)-1,
	FLAG_DONTPROCESS 	=(1<<4),
} node_type;

typedef struct node_s {
	const char		*name; /* name of this node. */
	struct node_s	*parent; /* parent node of this. */
	node_type		type; /* type of this node. */
	AVL_TREE		subnodes; /* children of this node */
	int 			level; /* level of this node, root node is at level 1 */
	FILE			*index_f; /* index.html file descriptor */
	struct node_s	**path; /* path from the root */
	const char		*full_name; /* full path name */
	struct node_s	*html_file;
} node;

#define TAG_MENU_FLAG_ALREADY_CREATED	(1 << 0)

typedef struct tag_menu_s {
	int				flags; /* flags list, see above. */
	const char		*name; /* name of tag */
	int				ntags; /* number of tags with this name */
	AVL_TREE		group_by_file; /* list of tags grouped by file */
	struct node_s	*nod; /* node for this file */
	struct ctag_s	*last_tag;
} tag_menu;

extern AVL_TREE db_ctag;
extern node *db_root_node;
extern int n_files;
extern AVL_TREE db_menus;

void db_init(const char *);
void mk_dir(const node *nod);
node *new_node(const char *name, node *parent, node_type typ);
ctag *lookup_ctag(const char *id, const char *fi, const char *ss);
ctag *ctag_lookup_by_id(const char *id);
int common_prefix(node *a, node *b);
char *rel_path(node *a, node *b);

#endif /* _DB_H */
/* $Id$ */
