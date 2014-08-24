/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 22:34:53 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "db.h"

static AVL_TREE ctag_db = NULL;
static AVL_TREE ctag_ix_id = NULL;

/* this function compares the unique index of <id,fi,ss>
 * for the ctag_db database */
static int ctag_cmp(const ctag_p a, const ctag_p b)
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
			if (a->ss < b->ss) res = -1;
			else if (a->ss > b->ss) res = +1;
			else res = 0;
		} /* if */
	} /* if */
	return res;
} /* ctag_cmp1 */

static void module_init()
{
	assert(ctag_db = new_avl_tree(
		(AVL_FCOMP) ctag_cmp, /* comparison of the three fields, in order */
		NULL, /* the key is the actual ctag entry, externally allocated */
		NULL,
		NULL)); /* no key printing function */
	assert(ctag_ix_id = new_avl_tree(
		(AVL_FCOMP) strcmp, /* comparison of the tg and id fields */
		NULL, /* the key is the id string, internalized */
		NULL,
		NULL)); /* no key printing function */
} /* module_init */

ctag_p ctag_lookup(const char *id, const char *fi, const char *ss)
{
	ctag_p res; /* result of lookup */
	ctag key; /* search key */

	if (!ctag_db) /* module not initialized */
		module_init();

	key.id = id; /* construct the key to search for the entry. */
	key.fi = fi;
	key.ss = ss;

	res = avl_tree_get(ctag_db, &key);
	if (!res) { /* doesn't exist, create it */
		ctag_p prev;
		assert(res = malloc(sizeof(ctag))); /* allocate memory */
		res->id = id;
		res->fi = fi;
		res->ss = ss;
		res->next = ctag_lookup_by_id(id);
		res->tag_no = res->next
				? res->next->tag_no + 1
				: 1;
		avl_tree_put(ctag_db, res, res);
		avl_tree_put(ctag_ix_id, id, res);
	} /* if */

	return res;
} /* ctag_lookup */

ctag_p ctag_lookup_by_id(const char *id)
{
	if (!ctag_db) /* module not initialized */
		module_init();

	return avl_tree_get(ctag_ix_id, id);
} /* ctag_lookup_by_id */

/* $Id$ */
