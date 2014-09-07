/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 21:13:42 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO.  All rights reserved.
 */
#ifndef _DB_H
#define _DB_H

#include <stdio.h>
#include <avl.h>
#include "node.h"

typedef struct ctag_s {
	const char		*id; /* tag identifier */
	const char		*fi; /* file this tag points to. */
	const char		*ss; /* scan string for ex(1) */
	int				tag_no_in_file; /* tag number in file. */
	struct ctag_s	*next_in_file; /* next tag with the same id in this tag file */
	node			*nod; /* file node this tag points to. */
} ctag;

ctag *lookup_ctag(const char *id, const char *fi, const char *ss, node *root);

#endif /* _DB_H */
/* $Id$ */
