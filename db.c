/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 22:34:53 EEST 2014
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
#include "node.h"
#include "menu.h"
#include "db.h"

AVL_TREE db_ctag = NULL;
node *db_root_node = NULL;
int n_files = 0;

tag_menu *lookup_tag_menu(ctag *t);

static int print_ctag_key(FILE *f, const ctag *a);
static int print_string(FILE *f, const char *s);
static int ctag_cmp(const ctag *a, const ctag *b);

ctag *lookup_ctag(
	const char *id,
	const char *fi,
	const char *ss)
{
	ctag *res;
	ctag key;
	node *nod;
	tag_menu *men;

	if (!db_ctag) {
		assert(db_ctag = new_avl_tree(
			(AVL_FCOMP) ctag_cmp,
			NULL, NULL,
			(AVL_FPRNT) print_ctag_key));
	} /* if */

	key.id = id = intern(id);
	key.fi = fi = intern(fi);
	key.ss = ss = intern(ss);

	DEB("begin: id=[%s], fi=[%s], ss=[%p]\n", id, fi, ss);

	if (res = avl_tree_get(db_ctag, &key)) {
		fprintf(stderr,
			PR("trying to add <%s|%s|%p>, already existent in db_ctag\n"),
			id, fi, ss);
		exit(EXIT_FAILURE);
	} /* if */

	assert(res = malloc(sizeof(ctag))); /* allocate memory */

	res->id = id;
	res->fi = fi;
	res->ss = ss;

	/* get the node this tag belongs to */
	assert(db_root_node);

	nod = res->nod = name2node(db_root_node, fi, TYPE_FILE);

	/* insert the ctag in list of tags in the same node.
	 * we reuse the subnodes field of the file nod*/
	res->next_in_file = avl_tree_get(nod->subnodes, id);
	res->tag_no_in_file = res->next_in_file
		? res->next_in_file->tag_no_in_file + 1
		: 1;
	avl_tree_put(nod->subnodes, id, res);

	assert(men = lookup_menu(res->id));

	/* insert ctag in list corresponding to file. */
	avl_tree_put(men->group_by_file, res->nod->full_name, res);
	men->ntags++;
	men->last_tag = res; /* last registered tag, for one node menus */


	DEB("return:\n"
			"  id            : [%s]\n"
			"  fi            : [%s]\n"
			"  ss            : %p\n"
			"  tag_no_in_file: %d\n"
			"  next_in_file  : %p\n"
			"  nod           : %s\n",
			res->id,
			res->fi,
			res->ss,
			res->tag_no_in_file,
			res->next_in_file,
			res->nod->full_name);

	return res;
} /* new_ctag */

static int print_ctag_key(FILE *f, const ctag *a)
{
	return fprintf(f, "%s|%s|%p", a->fi, a->id, a->ss);
} /* print_ctag */

static int print_string(FILE *f, const char *s)
{
	return fprintf(f, "%s", s);
} /* print_string */

/* this function compares the unique index of <id,fi,ss>
 * for the db_ctag database */
static int ctag_cmp(const ctag *a, const ctag *b)
{
	int res;
	res = strcmp(a->fi, b->fi); /* file is the most significative */
	if (res == 0) {
		res = strcmp(a->id, b->id); /* id is the next most significative */
		if (res == 0) {
			/* ss is the least significative, and we don't mind the values,
			 * so we just compare the pointers. As all the strings are
			 * interned, two equal strings implies their
			 * addresses are also equal */
#if 0
			res = strcmp(a->ss, b->ss);
#else
			if (a->ss < b->ss) res = -1;
			else if (a->ss > b->ss) res = +1;
			else res = 0;
#endif
		} /* if */
	} /* if */
	return res;
} /* ctag_cmp */

/* $Id$ */
