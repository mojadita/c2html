/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 11:50:37 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO SISTEMAS S.L.U.  All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "intern.h"
#include <avl.h>

AVL_TREE strings_avl = NULL;

static int print_string(FILE *o, const char *s)
{
	return fprintf(o, "%s", s);
} /* print_string */

const char *intern(const char *s)
{
	char *res;

#if DEBUG
	printf("intern(%s);\n", s);
#endif
	if (!strings_avl) {
		assert(strings_avl = new_avl_tree(
			(AVL_FCOMP) strcmp,
			NULL,
			NULL,
			(AVL_FPRNT) print_string));
	} /* if */
	res = avl_tree_get(strings_avl, s);
	if (!res) {
#if DEBUG
		printf("Interning [%s]\n", s);
#endif
		res = strdup(s);
		avl_tree_put(strings_avl, res, res);
	} /* if */

	return res;
} /* intern */
	
/* $Id$ */
