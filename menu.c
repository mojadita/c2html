/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Sep  5 15:59:37 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO. All rights reserved.
 */
#define DEBUG	1

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "debug.h"
#include "intern.h"
#include "menu.h"

AVL_TREE db_menus = NULL;

/* forward prototype definition */

tag_menu *lookup_menu(const char *id)
{
	tag_menu *res;

	DEB((PR("begin: id=[%s]\n"), id));
	
	if (!db_menus) {
		DEB((PR("initializing db_menus\n")));
		assert(db_menus = new_avl_tree(
			(AVL_FCOMP) strcmp,
			NULL,
			NULL,
			(AVL_FPRNT) print_string));
	} /* if */

	id = intern(id);

	DEB((PR("searching for [%s] in db_menus\n"), id));
	res = avl_tree_get(db_menus, id);
	if (!res) {
		assert(res = malloc(sizeof (tag_menu)));
		res->id = id;
		res->flags = 0;
		res->ntags = 0;
		assert(res->group_by_file = new_avl_tree(
				(AVL_FCOMP) strcmp,
				NULL, NULL,
				(AVL_FPRNT) print_string));
		res->nod = NULL;
		res->last_tag = NULL;
		avl_tree_put(db_menus, id, res);
	} /* if */
	DEB((PR("end [%p]\n"), res));

	return res;
} /* lookup_menu */

/* $Id$ */
