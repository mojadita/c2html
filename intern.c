/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 11:50:37 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO SISTEMAS S.L.U.  All rights reserved.
 */

#define DEBUG 1

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "debug.h"
#include "intern.h"

AVL_TREE intern_strings = NULL;

int print_string(FILE *o, const char *s)
{
	return fprintf(o, "%s", s);
} /* print_string */

const char *intern(const char *s)
{
	char *res;

	DEB((PR("begin\n")));
	if (!intern_strings) {
		DEB((PR("initializing <intern_strings>\n")));
		assert(intern_strings = new_avl_tree(
			(AVL_FCOMP) strcmp,
			NULL,
			NULL,
			(AVL_FPRNT) print_string));
	} /* if */
	DEB((PR("looking for [%s]\n"), s));
	res = avl_tree_get(intern_strings, s);
	if (!res) {
		DEB((PR("adding [%s] to database\n"), s));
		res = strdup(s);
		avl_tree_put(intern_strings, res, res);
	} /* if */

	DEB((PR("end\n")));

	return res;
} /* intern */
	
/* $Id$ */
