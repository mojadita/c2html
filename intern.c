/* $Id: intern.c,v 1.1 2014/09/09 20:22:07 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 11:50:37 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO SISTEMAS S.L.U.  All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "debug.h"
#include "intern.h"

AVL_TREE intern_strings = NULL;

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
			(AVL_FPRNT) fputs));
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
	
/* $Id: intern.c,v 1.1 2014/09/09 20:22:07 luis Exp $ */
