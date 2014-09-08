/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Sep  5 15:59:13 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO.  All rights reserved.
 */
#ifndef _MENU_H
#define _MENU_H

#include <stdio.h>
#include <avl.h>

#include "node.h"

#define TAG_MENU_FLAG_ALREADY_CREATED	0x00000001

typedef struct tag_menu_s {
	const char		*id; /* name of tag */
	int				flags; /* flags list, see above. */
	int				ntags; /* number of tags with this name */
	AVL_TREE		group_by_file; /* list of tags grouped by file */
	struct node_s	*nod; /* node for this menu/html file for this menu */
	struct ctag_s	*last_tag; /* last tag registered for this menu, for unique tags. */
} tag_menu;

extern AVL_TREE db_menus;
extern char *default_menu_name;

tag_menu *lookup_menu(const char *id, node *rt);

#endif /* _MENU_H */
/* $Id$ */
