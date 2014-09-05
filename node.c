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
#include "node.h"

#ifndef DEBUG
#define DEBUG	1
#endif

#if DEBUG
#define DEB(X) printf X
#else
#define DEB(X)
#endif

#define BUFFER_SIZE				4096
#define NULL_POINTER_STRING		"<<NULL>>"
#define PR(X) __FILE__":%d:%s: "X,__LINE__,__func__
#define INC_LEVEL	1

/* if you modify the enum node_type_e list,
 * you have also to modify here. */
static char *type2string[] = {
	"TYPE_DIR",
	"TYPE_FILE",
	"TYPE_HTML",
};

static int print_string(FILE *f, const char *s)
{
	return fputs(s, f);
} /* print_string */

node *new_node(const char *name, const node *parent, const node_type typ)
{
	node *res;

	assert(name);

	name = intern(name); /* intern is an idempotent function */

	DEB((PR("begin: name=[%s], parent=(%p), type=[%s]\n"),
		name,
		parent,
		type2string[typ]));

	if (parent)
		assert(avl_tree_get(parent->subnodes, name) == NULL);
	
	assert(res = malloc(sizeof (node))); /* get memory */
	DEB((PR("res = %p\n"), res));
	res->name = name;
	res->parent = parent;
	res->type = typ;
	res->flags = 0;
	res->level = parent ? parent->level + 1 : 1;
	assert(res->subnodes = new_avl_tree(
		(AVL_FCOMP) strcmp, NULL, NULL,
		(AVL_FPRNT) print_string));
	res->index_f = NULL;

	/* construct the path to it. use + 1
	 * to alloc for a NULL pointer at end. */
	assert(res->path = calloc(res->level + 1, sizeof (node *)));
	{	const node *p = res;
		int i;

		res->path[res->level] = NULL;
		for (i = res->level-1; i >= 0; i--)
		{
			DEB((PR("Setting res->path[%d] = p(%p/%s)\n"),
				i, p, p->name));
			res->path[i] = p;
			p = p->parent;
		} /* for */
	} /* block */

	/* construct the full name */
	{	char buffer[BUFFER_SIZE];
		size_t bs = sizeof buffer, n;
		char *aux = buffer;
		int i;

		for (i = 0; i < res->level; i++) {
			n = snprintf(aux, bs, "%s%s",
				(i)	? "/"
					: "",
				res->path[i]->name);
			aux += n; bs -= n;
		} /* for */

		res->full_name = intern(buffer);
	} /* block */

	/* add to parent directory */
	if (parent)
		avl_tree_put(parent->subnodes, res->name, res);

	DEB((PR("return name:[%s], parent:[%s], type:[%s], level:[%d], full_path:[%s]\n"),
		res->name,
		res->parent
			? res->parent->full_name
			: NULL_POINTER_STRING,
		type2string[res->type],
		res->level,
		res->full_name));

	/* now, add its html_file, if existent. */
	switch(res->type) {

	case TYPE_DIR:
		res->html_file = new_node("index.html", res, TYPE_HTML);
		avl_tree_put(res->subnodes, res->html_file->name, (void *) res->html_file);
		break;

	case TYPE_FILE: {
		char buffer[BUFFER_SIZE];
		assert(parent);
		snprintf(buffer, sizeof buffer, "%s.html", res->name);
		res->html_file = new_node(buffer, parent, TYPE_HTML);
		avl_tree_put(parent->subnodes, res->html_file->name, (void *) res->html_file);
	} break;

	case TYPE_HTML:
		res->html_file = res;
		break;

	} /* switch */

	return res;
} /* new_node */

node *name2node(const node *root, const char *p, const node_type typ)
{
	char *aux, *name;
	const char *nam;
	node *nod;

	assert(root);
	assert(p);

	name = strdup(p);
	nod = (node *) root;

	DEB((PR("begin: root=\"%s\", path=\"%s\"\n"),
			root->name, p));

	/* we cannot have absolute paths */
	assert(name[0] != '/');

	for(nam = name; nam; nam = aux) {
		node *next;

		DEB((PR("step: [%s][%s]\n"), nod->full_name, nam));

		aux = strchr(nam, '/'); /* search for a '/' character */
		if (aux) /* if found, nullify it and every one char following it */
			while (*aux == '/')
				*aux++ = '\0';

		/* now, aux points to the next name component or NULL */
		/* nam is the component name of this element of the path */
		/* CHECK FOR SPECIAL "." ENTRY */
		if (!strcmp(nam, ".")) {
			DEB((PR("component is \".\", ignored\n")));
			continue; /* it it's the . entry. */
		} /* if */

		/* ... AND CHECK ALSO FOR ".." */
		if (!strcmp(nam, "..")) {
			if (nod->parent == NULL) {
				fprintf(stderr,
					PR("error: \"..\" not allowed in %s\n"), p);
				exit(EXIT_FAILURE);
			} /* if */
			DEB((PR("component is \"..\", special\n")));
			nod = (node *) nod->parent;
			continue;
		} /* if */

		/* now we have a valid name */
		nam = intern(nam);

		/* lookup it on the subnodes field */
		DEB((PR("looking for [%s] in [%s]->subnodes\n"),
			nam, nod->full_name));
		next = avl_tree_get(nod->subnodes, nam);
		if (!next) {
			DEB((PR("[%s] not found, creating it in [%s]\n"),
				nam, nod->full_name));

			if (nod->type != TYPE_DIR) {
				fprintf(stderr,
					PR("%s is not a directory, cannot create node [%s] on it\n"),
					nod->full_name, nam);
				return NULL;
			} /* if */

			/* all but the last in the hierarchy is a directory */
			next = new_node(
				nam, nod,
				aux	? TYPE_DIR
					: typ);
		} /* if */

		DEB((PR("step[%s]: end%s.\n"),
			next->name,
			aux
				? "... next"
				: ""));
		nod = next;
	} /* for */

	/* free the temporary copy of the name */
	free(name);

	DEB((PR("end\n")));
	return nod;
} /* name2node */

/* returns the length of the common
 * prefix of two nodes, a and b. */
int common_prefix(const node *a, const node *b)
{
	int i = 0;
	while (	   a->path[i]
			&& b->path[i]
			&& a->path[i] == b->path[i])
		i++;
	return i;
} /* common_prefix */

/* computes the relative path from a
 * to b. */
char *rel_path(const node *a, const node *b)
{
	int c = common_prefix(a, b);
	int i;
	static char buffer[BUFFER_SIZE];
	size_t bs = sizeof buffer;
	char *p = buffer;
	int res, n = 0;

	/* first the chain up */
	for (i = a->level-1; i > 0 && i > c; i--) {
		res = snprintf(p, bs, "%s..",
			n++ ? "/" : "");
		p += res; bs -= res;
	} /* for */
	/* now i == c or i == 0 */

	/* then follow it down to the target */
	while(i < b->level) {
		res = snprintf(p, bs, "%s%s",
			n++ ? "/" : "",
			b->path[i++]->name);
		p += res; bs -= res;
	} /* while */

	return buffer;
} /* rel_path */

int
do_recur(const node *nod,
	node_callback pre,
	void *val_pre,
	node_callback fil,
	void *val_fil,
	node_callback pos,
	void *val_pos)
{
	AVL_ITERATOR i;
	int res = 0;

	DEB((PR("%*sENTER: %s[%s]\n"),
		nod->level-1, "",
		nod->full_name,
		type2string[nod->type]));

	switch(nod->type) {
	case TYPE_DIR:
		if (!(nod->flags & NODE_FLAG_DONT_RECUR_PREORDER) && pre)
			if (res = pre(nod, val_pre)) return res;
		for (	i = avl_tree_first(nod->subnodes);
				i;
				i = avl_iterator_next(i))
		{
			if (res = do_recur(avl_iterator_data(i),
				pre, val_pre,
				fil, val_fil,
				pos, val_pos)) return res;
		} /* for */
		if (!(nod->flags & NODE_FLAG_DONT_RECUR_POSTORDER) && pos)
			if (res = pos(nod, val_pos)) return res;
		break;
	case TYPE_FILE:
		if (!(nod->flags & NODE_FLAG_DONT_RECUR_INFILE) && fil)
			if (res = fil(nod, val_fil)) return res;
		break;
	/* on TYPE_HTML we don't do anything */
	case TYPE_HTML: break;
	} /* switch */

	DEB((PR("%*sLEAVE: %s[%s]\n"),
		nod->level-1, "",
		nod->full_name,
		type2string[nod->type]));
	return res;
} /* do_recur */

/* $Id$ */
