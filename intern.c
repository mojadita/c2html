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
	AVL_ITERATOR p;
	if (!strings_avl) {
		assert(strings_avl = new_avl_tree(
			(AVL_FCOMP) strcmp,
			(AVL_FCONS) strdup,
			(AVL_FDEST) free,
			(AVL_FPRNT) print_string));
	} /* if */
	p = avl_tree_atkey(strings_avl, s, MT_EQ);
	if (!p) {
		p = avl_tree_put(strings_avl, s, NULL);
		avl_iterator_set_data(p, (void *)avl_iterator_key(p));
	} /* if */
	return avl_iterator_data(p);
} /* intern */
	
/* $Id$ */
