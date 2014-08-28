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
#include "db.h"

AVL_TREE db_ctag = NULL;
node_p db_root_node = NULL;
int n_files = 0;

static int print_ctag(FILE *f, const ctag *a)
{
	return fprintf(f, "%s|%s|%p", a->fi, a->id, a->ss);
} /* print_ctag */

static int print_string(FILE *f, const char *s)
{
	return fprintf(f, "%s", s);
} /* print_string */

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

static char *get_name_rec(const node *p, char **buffer, size_t *bs)
{
	char *res = *buffer; /* save the original pointer */
	int n;

	if (p->parent) {
		int n;
		get_name_rec(p->parent, buffer, bs);
	} /* if */
	n = snprintf(*buffer, *bs,
		"%s%s",
		p->parent
			? "/"
			: "",
		p->name);
	*buffer += n; *bs -= n;

	return res;
} /* get_name_rec */

static char *get_name(const node *p)
{
	static char buffer[4096]; /* one page */
	size_t bs = sizeof buffer;
	char *aux = buffer;

	return get_name_rec(p, &aux, &bs);
} /* get_name */

node_p new_node(
	const char *name,
	node_p parent,
	node_type typ)
{
	node_p res;

#if DEBUG
	printf("new_node(%s, %p, %d)\n", name, parent, typ);
#endif
	assert(res = malloc(sizeof (node)));
	res->name = intern(name);
	res->parent = parent;
	res->type = typ;
	assert(res->subnodes = new_avl_tree(
		(AVL_FCOMP) strcmp,
		NULL, NULL, (AVL_FPRNT) print_string));
	res->level = res->parent ? parent->level + 1 : 0;
	res->full_name = intern(get_name(res));
	res->index_f = NULL;

#if DEBUG
	printf( "new_node() ->\n"
		"  name     : %s(%p)\n"
		"  parent   : [0x%p](%s)\n"
		"  type     : %d\n"
		"  subnodes : %p\n"
		"  level    : %d\n"
		"  tag_list : %p\n"
		"  full_name: %s\n"
		"  index_f  : %p\n",
		res->name, res->name,
		res->parent, res->parent ? res->parent->name : "NULL",
		res->type,
		res->subnodes,
		res->level,
		res->tag_list,
		res->full_name,
		res->index_f);
#endif

	return res;
} /* new_node */

static node *name2node(node_p root, const char *p)
{
	char *aux, *name = strdup(p); /* we return it at the end */
	const char *nam;
	node_p nod = root;

#if DEBUG
	printf("name2node(%p(%s), %s);\n", root, root->name, p);
#endif
	/* we cannot have absolute paths */
	if (name[0] == '/') {
		fprintf(stderr,
			"ERROR: cannot allow absolute paths (%s)\n", p);
		exit(EXIT_FAILURE);
	} /* if */

#if DEBUG
	printf("solving name->node: ");
#endif
	for(nam = name; nam; nam = aux) {
		node_p next;

		aux = strchr(nam, '/'); /* search for a '/' character */
		if (aux) /* if found, nullify it and every one char following it */
			while (*aux == '/') *aux++ = '\0';
		/* now, aux points to the next name component or NULL */
		/* nam is the component name of this element of the path */
#if DEBUG
		printf("Solving for [%s] (aux == %p)\n", nam, aux);
#endif

		/* CHECK FOR SPECIAL "." ENTRY */
		if (!strcmp(nam, ".")) {
			continue; /* it it's the . entry. */
		} /* if */

		/* ... AND CHECK ALSO FOR ".." */
		if (!strcmp(nam, "..")) {
			if (nod->parent == NULL) {
				fprintf(stderr,
					"ERROR: ../ not allowed in %s\n",
					p);
				exit(EXIT_FAILURE);
			} /* if */
			nod = nod->parent;
			continue;
		} /* if */

		/* now we have a valid name */
		nam = intern(nam);
		next = avl_tree_get(nod->subnodes, nam);
		if (!next) {
			next = new_node(
				nam, nod,
				aux	? FLAG_ISDIR
					: FLAG_ISFILE);
			if (!aux) n_files++;
			avl_tree_put(nod->subnodes, nam, next);
		} /* if */
			
#if 0
			printf("NODE: %p\n"
				"  next->name = %s\n"
				"  next->parent = %p(%s)\n"
				"  next->type = 0x%02x\n"
				"  next->level = %d\n",
				next,
				next->name,
				next->parent, next->parent->name,
				next->type,
				next->level);
#endif
		nod = next;
	} /* for */
	free(name);
	return nod;
} /* name2node */

static ctag_p new_ctag(
	const char *id,
	const char *fi,
	const char *ss)
{
	ctag_p res;

#if DEBUG
	printf("new_ctag(\"%s\",\"%s\",\"%s\");\n", id, fi, ss);
#endif

	assert(res = malloc(sizeof(ctag))); /* allocate memory */

	res->id = id = intern(id);
	res->fi = fi = intern(fi);
	res->ss = ss = intern(ss);

	/* get the node this tag belongs to */
	res->nod = name2node(db_root_node, res->fi);

	/* insert the ctag in list of tags in the same node. */
	res->next = avl_tree_get(
		res->nod->subnodes,
		id);
	res->tag_no = res->next
		? res->next->tag_no + 1
		: 1;
	avl_tree_put(
		res->nod->subnodes,
		id, res);

	return res;
} /* new_ctag */


void db_init(const char *o)
{
#if DEBUG
	printf("db_init();\n");
#endif
	assert(db_ctag = new_avl_tree(
		(AVL_FCOMP) ctag_cmp, /* comparison of the three fields, in order */
		NULL, /* the key is the actual ctag entry, externally allocated */
		NULL,
		(AVL_FPRNT) print_ctag)); /* no key printing function */
	db_root_node = new_node(
		o, NULL, FLAG_ISDIR);
} /* module_init */

ctag_p ctag_lookup(const char *id, const char *fi, const char *ss)
{
	ctag_p res; /* result of lookup */
	ctag key; /* search key */

	key.id = id; /* construct the key to search for the entry. */
	key.fi = fi;
	key.ss = ss;

#if DEBUG
	printf("locating [%s][%s][%p]\n", fi, id, ss);
#endif
	res = avl_tree_get(db_ctag, &key);
	if (res) { /* tag does exist, signal it */
		fprintf(stderr,
			"ERROR: TAG[%s][%s][%s] ALREADY IN DATABASE, "
			"IGNORED\n",
			fi, id, ss);
	} else { /* create it */
		res = new_ctag(id, fi, ss);
		/* put in database */
		avl_tree_put(db_ctag, res, res);
	} /* if */

	return res;
} /* ctag_lookup */

/* $Id$ */
