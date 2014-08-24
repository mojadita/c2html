/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 22:34:53 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "intern.h"
#include "db.h"

AVL_TREE db_ctag = NULL;
AVL_TREE db_ctag_ix_id = NULL;
node db_root_node;

static int print_ctag(FILE *f, const ctag *a)
{
	return fprintf(f, "%s|%s|%p", a->fi, a->id, a->ss);
}

static int print_string(FILE *f, const char *s)
{
	return fprintf(f, "%s", s);
}

/* this function compares the unique index of <id,fi,ss>
 * for the db_ctag database */
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
} /* ctag_cmp */

static void module_init()
{
	assert(db_ctag = new_avl_tree(
		(AVL_FCOMP) ctag_cmp, /* comparison of the three fields, in order */
		NULL, /* the key is the actual ctag entry, externally allocated */
		NULL,
		(AVL_FPRNT) print_ctag)); /* no key printing function */
	assert(db_ctag_ix_id = new_avl_tree(
		(AVL_FCOMP) strcmp, /* comparison of the tg and id fields */
		NULL, /* the key is the id string, internalized */
		NULL,
		(AVL_FPRNT) print_string)); /* no key printing function */
	db_root_node.name = "c2html";
	db_root_node.parent = NULL;
	db_root_node.flags = FLAG_ISDIR;
	db_root_node.level = 0;
	assert(db_root_node.subnodes = new_avl_tree(
		(AVL_FCOMP) strcmp,
		NULL, /* the key is the id string, internalized */
		NULL,
		(AVL_FPRNT) print_string)); /* no key printing function */
} /* module_init */

static const node *name2node(const char *p)
{
	char *aux, *name = strdup(p);
	const char *nam;
	node_p nod = &db_root_node;

	/* we cannot have absolute paths */
	if (name[0] == '/') {
		fprintf(stderr,
			"ERROR: cannot allow absolute paths (%s)\n", p);
		exit(EXIT_FAILURE);
	} /* if */

	for(nam = name; nam; nam = aux) {
		node_p next;

		aux = strchr(nam, '/'); /* search for a '/' character */
		if (aux) /* if found, nullify it and every one following it */
			while (*aux == '/') *aux++ = '\0';
		/* now, aux points to the next name component or NULL */
		/* nam is the component name of this element of the path */
		if (!strcmp(nam, ".")) {
			continue; /* it it's the . entry. */
		} /* if */
		if (!strcmp(nam, "..")) {
			if (nod->parent == NULL) {
				fprintf("ERROR: cannot allow ../ at the beginning of the file name (%s)\n",
					p);
				exit(EXIT_FAILURE);
			} /* if */
			nod = nod->parent;
			continue;
		} /* if */

		nam = intern(nam);
		next = avl_tree_get(nod->subnodes, nam);
		if (!next) {
			assert(next = malloc(sizeof(node)));
			next->name = nam;
			next->parent = nod;
			next->flags = aux ? FLAG_ISDIR : FLAG_ISFILE;
			assert(next->subnodes = new_avl_tree(
				(AVL_FCOMP) strcmp,
				NULL, NULL, NULL));
			next->level = nod->level + 1;
			avl_tree_put(nod->subnodes, nam, next);
#if 0
			printf("NODE: %p\n"
				"  next->name = %s\n"
				"  next->parent = %p(%s)\n"
				"  next->flags = 0x%02x\n"
				"  next->level = %d\n",
				next,
				next->name,
				next->parent, next->parent->name,
				next->flags,
				next->level);
#endif
		} /* if */
		nod = next;
	} /* for */
	free(name);
	return nod;
} /* name2nod */

ctag_p ctag_lookup(const char *id, const char *fi, const char *ss)
{
	ctag_p res; /* result of lookup */
	ctag key; /* search key */

	if (!db_ctag) /* module not initialized */
		module_init();

	key.id = id; /* construct the key to search for the entry. */
	key.fi = fi;
	key.ss = ss;

#if 0
	printf("locating [%s][%s][%p]\n", fi, id, ss);
#endif
	res = avl_tree_get(db_ctag, &key);
	if (res) { /* tag doesn't exist, create it */
		fprintf(stderr,
			"ERROR: TAG[%s][%s][%p] ALREADY IN DATABASE, IGNORED\n",
			fi, id, ss);
	} else {
		ctag_p prev;

		assert(res = malloc(sizeof(ctag))); /* allocate memory */
		res->id = id;
		res->fi = fi;
		res->ss = ss;
		res->next = ctag_lookup_by_id(id);
		res->tag_no = res->next
				? res->next->tag_no + 1
				: 1;
		res->nod = name2node(res->fi);
		avl_tree_put(db_ctag, res, res);
		avl_tree_put(db_ctag_ix_id, id, res);
		{	node_p nod;  /* allocate the path components in the array. */

			assert(res->path = calloc(res->nod->level+2, sizeof(char *)));
			res->path[res->nod->level+1] = NULL;
			for (nod = res->nod; nod; nod = nod->parent)
				res->path[nod->level] = nod->name;
		} /* block */
	} /* if */

	return res;
} /* ctag_lookup */

ctag_p ctag_lookup_by_id(const char *id)
{
	if (!db_ctag) /* module not initialized */
		module_init();

	return avl_tree_get(db_ctag_ix_id, id);
} /* ctag_lookup_by_id */

/* $Id$ */
