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
#include "log.h"

node *db_root_node = NULL;
int n_files = 0;

/* if you modify the enum node_type_e list,
 * you have also to modify here. */
static char *type2string[] = {
	"TYPE_DIR",
	"TYPE_FILE",
	"TYPE_HTML",
};

tag_menu *lookup_tag_menu(ctag *t);

static int print_string(FILE *f, const char *s)
{
	return puts(s, f);
} /* print_string */

node *new_node(const char *name, node *parent, node_type typ)
{
	node *res;
	int i;
	char buffer[BUFFER_SIZE];

	assert(name);

	intern(name); /* intern is an idempotent function */

	log(PR("begin: name=\"%s\", parent=\"%s\", typ=%d\n"),
		name,
		parent
			? parent->full_name
			: NULL_POINTER_STRING,
		typ);

	assert(avl_tree_get(parent->subnodes, name));
	
	assert(res = malloc(sizeof (node))); /* get memory */
	res->name =  name;
	res->parent = parent;
	res->type = typ;
	res->level = parent ? parent->level + 1 : 1;
	assert(res->subnodes = new_avl_tree(
		(AVL_FCOMP) strcmp, NULL, NULL,
		(AVL_FPRNT) print_string));
	res->index_f = NULL;

	/* construct the path to it. use + 1
	 * to alloc for a NULL pointer at end. */
	assert(res->path = calloc(res->level + 1, sizeof (node *)));
	{	node *p;
		res->path[res->level] = NULL;
		for (	i = res->level-1, p = res;
				i >= 0 && p;
				i--, p = p->parent)
		{
			res->path[i] = p;
		} /* for */
		assert((i == -1) && (p == NULL));
	} /* block */

	/* construct the full name */
	{	size_t bs = sizeof buffer, n;
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

	switch(res->type) {
	case TYPE_DIR:
		res->html_file = new_node("index.html", res, TYPE_HTML);
		avl_tree_put(res->subnodes, res->html_file->name, res->html_file);
		break;
	case TYPE_FILE:
		assert(parent);
		snprintf(buffer, sizeof buffer,
			"%s.html", res->name);
		res->html_file = new_node(buffer, parent, TYPE_HTML);
		avl_tree_put(parent->subnodes, res->html_file->name, res->html_file);
	case TYPE_HTML:
		res->html_file = res;
	} /* switch */

	log(PR("return {name:[%s], parent:[%s], type:[%s], level:[%s]}\n"),
		res->name,
		res->parent
			? res->parent->full_name
			: NULL_POINTER_STRING,
		type2string[res->type],
		res->level);
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

	log(PR("begin: root=\"%s\", path=\"%s\"\n"),
			root->name, p);
	} /* if */

	/* we cannot have absolute paths */
	assert(name[0] != '/');

	for(nam = name; nam; nam = aux) {
		node *next;

		log(PR("step: [%s][%s]\n"), nod->full_name, nam);

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
			log(PR("component is \"..\", special\n"));
			nod = nod->parent;
			continue;
		} /* if */

		/* now we have a valid name */
		nam = intern(nam);

		/* lookup it on the subnodes field */
		log(PR("looking for [%s] in [%s]->subnodes\n"),
			nam, nod->full_name);
		next = avl_tree_get(nod->subnodes, nam);
		if (!next) {
			log(PR("[%s] not found, creating it in [%s]\n"),
				nam, nod->full_name);

			/* the last in the hierarchy is a directory */
			next = new_node(
				nam, nod,
				aux	? FLAG_ISDIR
					: FLAG_ISFILE);
		} /* if */

		log(PR("step[%s]: end%s.\n"),
			next->name,
			aux
				? "... next"
				: "");
		} /* if */
		nod = next;
	} /* for */

	/* free the temporary copy of the name */
	free(name);

	log(PR("end\n"));
	return nod;
} /* name2node */

/* returns the length of the common
 * prefix of two nodes, a and b. */
int common_prefix(node *a, node *b)
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
char *rel_path(node *a, node *b)
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

/* $Id$ */
