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
#include "c2html.h"
#include "db.h"

AVL_TREE db_ctag = NULL;
node *db_root_node = NULL;
AVL_TREE db_menus = NULL;
int n_files = 0;

tag_menu *lookup_tag_menu(ctag *t);

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

node *new_node(const char *name, node *parent, node_type typ)
{
	node *res;
	int i;
	char buffer[DEFAULT_BUFSIZE];

	if (flags & FLAG_DEBUG_DB) {
		printf(PR("begin: name=\"%s\", parent=\"%s\", typ=%d\n"),
			name, parent ? parent->full_name : "<<null>>", typ);
	} /* if */

	if (parent && avl_tree_get(parent->subnodes, name)) {
		fprintf(stderr,
			PR("trying to create already existent node %s in parent %s\n"),
			name, parent->full_name);
		exit(EXIT_FAILURE);
	} /* if */
	
	assert(res = malloc(sizeof (node)));
	res->name = intern(name);
	res->parent = parent;
	res->type = typ;
	assert(res->subnodes = new_avl_tree(
		(AVL_FCOMP) strcmp,
		NULL,
		NULL,
		(AVL_FPRNT) print_string));
	res->level = parent ? parent->level + 1 : 1;
	res->index_f = NULL;

	/* construct the path to it. */
	assert(res->path = calloc(res->level + 1, sizeof (node *)));
	{	node *p;
		for (	i = res->level-1, p = res;
				i >= 0;
				i--, p = p->parent)
		{
			res->path[i] = p;
		} /* for */
		res->path[res->level] = NULL;
	} /* block */

	/* construct the full name */
	{	static char buffer[DEFAULT_BUFSIZE];
		size_t bs = sizeof buffer;
		int n;
		char *aux = buffer;

		for (i = 0; i < res->level; i++) {
			n = snprintf(aux, bs, "%s%s",
				(i)	? "/"
					: "",
				res->path[i]->name);
			aux += n; bs -= n;
		} /* for */

		res->full_name = intern(buffer);
	} /* block */

	if (flags & FLAG_DEBUG_DB) {
		printf(PR("->\n"
			"  name     : %s\n"
			"  parent   : %s\n"
			"  type     : %d\n"
			"  subnodes : %p\n"
			"  level    : %d\n"
			"  full_name: %s\n"
			"  index_f  : %p\n"),
			res->name,
			res->parent
				? res->parent->full_name
				: "<<NULL>>",
			res->type,
			res->subnodes,
			res->level,
			res->full_name,
			res->index_f);
		printf(PR("end -> %p\n"), res);
	} /* if */

	return res;
} /* new_node */

static node *name2node(node *root, const char *p)
{
	char *aux, *name;
	const char *nam;
	node *nod = root;

	assert(root);
	assert(p);

	name = strdup(p);
	nod = root;

	if (flags & FLAG_DEBUG_DB) {
		printf(PR("begin: root=\"%s\", path=\"%s\"\n"),
			root->name, p);
	} /* if */

	/* we cannot have absolute paths */
	if (name[0] == '/') {
		fprintf(stderr,
			PR("error: cannot allow absolute paths (%s)\n"),
			p);
		exit(EXIT_FAILURE);
	} /* if */

	for(nam = name; nam; nam = aux) {
		node *next;

		if (flags & FLAG_DEBUG_DB) {
			printf(PR("step: solving for \"%s\"\n"), nam);
		} /* if */

		aux = strchr(nam, '/'); /* search for a '/' character */
		if (aux) /* if found, nullify it and every one char following it */
			while (*aux == '/')
				*aux++ = '\0';

		/* now, aux points to the next name component or NULL */
		/* nam is the component name of this element of the path */
		/* CHECK FOR SPECIAL "." ENTRY */
		if (!strcmp(nam, ".")) {
			if (flags & FLAG_DEBUG_DB) {
				printf(PR("component is \".\", ignored\n"));
			} /* if */
			continue; /* it it's the . entry. */
		} /* if */

		/* ... AND CHECK ALSO FOR ".." */
		if (!strcmp(nam, "..")) {
			if (nod->parent == NULL) {
				fprintf(stderr,
					PR("error: \"..\" not allowed in %s\n"), p);
				exit(EXIT_FAILURE);
			} /* if */
			if (flags & FLAG_DEBUG_DB) {
				printf(PR("component is \"..\", special\n"));
			} /* if */
			nod = nod->parent;
			continue;
		} /* if */

		/* now we have a valid name */
		nam = intern(nam);

		/* lookup it on the subnodes field */
		if (flags & FLAG_DEBUG_DB) {
			printf(PR("looking for \"%s\" in [%s]->subnodes\n"),
				nam, nod->full_name);
		} /* if */
		next = avl_tree_get(nod->subnodes, nam);
		if (!next) {
			if (flags & FLAG_DEBUG_DB) {
				printf(PR("\"%s\" not found, creating it in [%s]\n"),
					nam, nod->full_name);
			} /* if */
			next = new_node(
				nam, nod,
				aux	? FLAG_ISDIR
					: FLAG_ISFILE);
			switch (next->type) {
			case FLAG_ISDIR:
				if (flags & FLAG_DEBUG_DB) {
					printf(PR("creating \"index.html\" entry in [%s]\n"),
						next->full_name);
				} /* if */
				next->html_file = new_node(
					"index.html",	/* name of the index file */
					next,			/* parent, is a child of next */
					FLAG_ISFILE);	/* for it to be a file */
				break;
			case FLAG_ISFILE: {
				char buffer[DEFAULT_BUFSIZE];

				snprintf(buffer, sizeof buffer,
					"%s.html", next->name);
				if (avl_tree_get(next->parent->subnodes, buffer)) {
					fprintf(stderr,
						PR("error: name clash with file %s\n"),
						buffer);
					exit(EXIT_FAILURE);
				} /* if */
				if (flags & FLAG_DEBUG_DB) {
					printf(PR("creating \"%s\" entry in [%s]\n"),
						buffer, next->full_name);
				} /* if */
				next->html_file = new_node(
					buffer,			/* name of the html file */
					nod,			/* parent is the parent of next (nod) */
					FLAG_ISFILE);	/* for it to be a file */
				n_files++; 
				} break;
			} /* switch */
			/* and put it (just next) in the subnodes database */
			avl_tree_put(nod->subnodes, nam, next);
		} /* if */
		if (flags & FLAG_DEBUG_DB) {
			printf(PR("step[%s]: end%s.\n"),
				next->name,
				aux ? "... next":"");
		} /* if */
		nod = next;
	} /* for */
	free(name);

	if (flags & FLAG_DEBUG_DB) {
		printf(PR("end\n"));
	} /* if */
	return nod;
} /* name2node */

static ctag *new_ctag(
	const char *id,
	const char *fi,
	const char *ss)
{
	ctag *res;
	node *nod;
	tag_menu *men;

	if (flags & FLAG_DEBUG_DB) {
		printf(PR("new_ctag(\"%s\",\"%s\",\"%s\"): begin\n"),
		id, fi, ss);
	} /* if */

	assert(res = malloc(sizeof(ctag))); /* allocate memory */

	id = res->id = intern(id);
	fi = res->fi = intern(fi);
	ss = res->ss = intern(ss);

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

void db_init(const char *o)
{
	if (flags & FLAG_DEBUG_DB) {
		printf(PR("begin: base directory name = \"%s\"\n"), o);
	} /* if */
	assert(db_ctag = new_avl_tree(
		(AVL_FCOMP) ctag_cmp, /* comparison of the three fields, in order */
		NULL, /* the key is the actual ctag entry, externally allocated */
		NULL,
		(AVL_FPRNT) print_ctag)); /* no key printing function */
	db_root_node = new_node(
		o, NULL, FLAG_ISDIR);
	db_root_node->html_file = new_node(
		"index.html", db_root_node, FLAG_ISFILE);
	db_menus = new_avl_tree(
		(AVL_FCOMP) strcmp, /* comparison of ctags based only on name */
		NULL, /* we allocate externally. */
		NULL,
		(AVL_FPRNT) print_string);
	if (flags & FLAG_DEBUG_DB) {
		printf(PR("begin: base directory name = \"%s\"\n"), o);
	} /* if */
} /* module_init */

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

int common_prefix(node *a, node *b)
{
	int i = 0;
	while (	   a->path[i]
			&& b->path[i]
			&& a->path[i] == b->path[i])
		i++;
	return i;
} /* common_prefix */

char *rel_path(node *a, node *b)
{
	int c = common_prefix(a, b);
	int i;
	static char buffer[4096];
	size_t bs = sizeof buffer;
	char *p = buffer;
	int res, n = 0;

	for (i = a->level-1; i > 0 && i > c; i--) {
		res = snprintf(p, bs, "%s..",
			n++ ? "/" : "");
		p += res; bs -= res;
	} /* for */
	/* now i == c or i == 0 */
	while(i < b->level) {
		res = snprintf(p, bs, "%s%s",
			n++ ? "/" : "",
			b->path[i++]->name);
		p += res; bs -= res;
	} /* while */

	return buffer;
} /* rel_path */

/* $Id$ */
