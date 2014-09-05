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

#include "intern.h"
#include "ctag.h"

static AVL_TREE db_ctag = NULL;
node *db_root_node = NULL;
AVL_TREE db_menus = NULL;
int n_files = 0;

tag_menu *lookup_tag_menu(ctag *t);

/* static function prototypes */
static int print_ctag_key(FILE *f, const ctag *a);
static int print_string(FILE *f, const char *s);
static int ctag_cmp(const ctag *a, const ctag *b);

ctag *lookup_ctag(const char *id, const char *fi, const char *ss)
{
	ctag *res;
	node *nod;
	tag_menu *men;
	ctag key;

	DEB((PR("begin <%s|%s|%p>\n"), id, fi, ss));

	if (!db_ctag) {
		DEB((PR("initializing db_ctag database\n")));
		assert(db_ctag = new_avl_tree(
			(AVL_FCOMP) ctag_cmp,
			NULL,
			NULL,
			(AVL_FPRNT) print_ctag_key));
	} /* if */

	key.id = id = res->id = intern(id);
	key.fi = fi = res->fi = intern(fi);
	key.ss = ss = res->ss = intern(ss);

	if (avl_tree_get(db_

	assert(res = malloc(sizeof(ctag))); /* allocate memory */

	/* get the node this tag belongs to */
	nod = res->nod = name2node(db_root_node, fi);

	assert(nod->type == FLAG_ISFILE);

	/* insert the ctag in list of tags in the same node.
	 * we reuse the subnodes field of the file nod*/
	res->next_in_file = avl_tree_get(nod->subnodes, id);
	res->tag_no_in_file = res->next_in_file
		? res->next_in_file->tag_no_in_file + 1
		: 1;
	avl_tree_put(nod->subnodes, id, res);

	men = lookup_tag_menu(res);

	/* insert ctag in list corresponding to file. */
	avl_tree_put(men->group_by_file, res->nod->full_name, res);
	men->ntags++;
	men->last_tag = res; /* last registered tag, for one node menus */


	if (flags & FLAG_DEBUG_DB) {
		printf(PR("return:\n"
			"  id            : %s\n"
			"  fi            : %s\n"
			"  ss            : \"%s\"\n"
			"  tag_no_in_file: %d\n"
			"  next_in_file  : %p\n"
			"  nod           : %s\n"),
			res->id,
			res->fi,
			res->ss,
			res->tag_no_in_file,
			res->next_in_file,
			res->nod->full_name);
	} /* if */

	return res;
} /* new_ctag */


tag_menu *lookup_tag_menu(ctag *t)
{
	tag_menu *res;

	if (flags & FLAG_DEBUG_DB) {
		printf(PR("begin: looking tag_menu \"%s\" for tag <%s,%s,%s> in db_menus\n"),
			t->id, t->id, t->fi, t->ss);
	} /* if */

	/* first, get the tag_menu, if existent. */
	res = avl_tree_get(db_menus, t->id);
	if (!res) {
		char buffer[DEFAULT_BUFSIZE];

		if (flags & FLAG_DEBUG_DB) {
			printf(PR("not found tag_menu %s, create\n"),
				t->id);
		} /* if */
		assert(res = malloc(sizeof (tag_menu)));
		res->flags = 0;
		res->name = t->id;
		res->ntags = 0;
		assert(res->group_by_file = new_avl_tree( /* files menu */
			(AVL_FCOMP) strcmp,
			NULL,
			NULL,
			(AVL_FPRNT) print_string));
		snprintf(buffer, sizeof buffer, PFX1"%s.html", t->id);
		res->nod = new_node(buffer, db_root_node, FLAG_ISFILE);
		res->nod->html_file = res->nod; /* for create_html to work */
		res->last_tag = NULL;
		/* put in the menu_tag database */
		avl_tree_put(db_menus, t->id, res);
	} /* if */

	if (flags & FLAG_DEBUG_DB) {
		printf(PR("end\n"));
	} /* if */

	return res;
} /* lookup_tag_menu */

ctag *lookup_ctag(const char *id, const char *fi, const char *ss)
{
	ctag *res; /* result of lookup */
	ctag key; /* search key */

	key.id = id; /* construct the key to search for the entry. */
	key.fi = fi;
	key.ss = ss;

	if (flags & FLAG_DEBUG_DB) {
		printf(PR("locating [%s][%s][%p]\n"),
		fi, id, ss);
	} /* if */

	res = avl_tree_get(db_ctag, &key);
	if (res) { /* tag does exist, signal it */
		fprintf(stderr,
			PR("error: tag[%s][%s][%s] already in "
			"database, ignored\n"),
			fi, id, ss);
	} else { /* create it */
		if (flags & FLAG_DEBUG_DB) {
			printf(PR("adding [%s][%s][%p] to database\n"),
			fi, id, ss);
		} /* if */

		res = new_ctag(id, fi, ss);
		/* put in database */
		avl_tree_put(db_ctag, res, res);
	} /* if */

	return res;
} /* lookup_ctag */

static int print_ctag_key(FILE *f, const ctag *a)
{
	return fprintf(f, "%s|%s|%p", a->fi, a->id, a->ss);
} /* print_ctag_key */

static int print_string(FILE *f, const char *s)
{
	return fputs(s, f);
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
