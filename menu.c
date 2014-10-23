/* $Id: menu.c,v 1.1 2014/09/09 20:23:05 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Sep  5 15:59:37 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO. All rights reserved.
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

#include "menu.h"

AVL_TREE db_menus = NULL;
char *default_menu_name = "00-menus-directory";

/* forward prototype definition */

tag_menu *lookup_menu(const char *id, node *root)
{
	tag_menu *res;
	static node *menus_dir;

	DEB((PR("begin: id=[%s]\n"), id));
	
	if (!db_menus) {
		DEB((PR("initializing db_menus\n")));
		assert(db_menus = new_avl_tree(
			(AVL_FCOMP) strcmp,
			NULL,
			NULL,
			(AVL_FPRNT) fputs));
		//menus_dir = new_node(default_menu_name, root, TYPE_DIR);
	} /* if */

	id = intern(id);

	DEB((PR("searching for [%s] in db_menus\n"), id));
	D(res = avl_tree_get(db_menus, id));
	if (!res) {
		char buffer[4096];
		assert(res = malloc(sizeof (tag_menu)));
		res->id = id;
		res->flags = 0;
		res->ntags = 0;
		assert(res->group_by_file = new_avl_tree(
				(AVL_FCOMP) strcmp,
				NULL, NULL,
				(AVL_FPRNT) fputs));
		D(snprintf(buffer, sizeof buffer,
			"%s/%c/%s.html",
			default_menu_name,
			res->id[0], res->id));
		D(res->nod = name2node(root, buffer, TYPE_HTML));
		DEB((PR("res->nod=%p res->nod->full_name=[%s], "
			"res->nod->html_file->full_name=[%s]\n"),
			res->nod,
			res->nod->full_name,
			res->nod->html_file->full_name));
		res->last_tag = NULL;
		D(avl_tree_put(db_menus, id, res));
	} /* if */
	DEB((PR("end [%p]\n"), res));

	return res;
} /* lookup_menu */

/* $Id: menu.c,v 1.1 2014/09/09 20:23:05 luis Exp $ */
