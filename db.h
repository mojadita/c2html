/* $Id$
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: sáb ago 23 21:13:42 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO.  All rights reserved.
 */
#ifndef _DB_H
#define _DB_H

#include <avl.h>

typedef struct ctag_s {
	const char		*id; /* tag identifier */
	const char		*fi; /* file this tag points to. */
	const char		*ss; /* scan string for ex(1) */
	int				tag_no; /* tag number */
	struct ctag_s	*next; /* next tag with the same id in this tag file */
} ctag, *ctag_p;

ctag_p ctag_lookup(const char *id, const char *fi, const char *ss);
ctag_p ctag_lookup_by_id(const char *id);
ctag_p ctag_lookup_by_fi_id(const char *fi, const char *id);

#endif /* _DB_H */
/* $Id$ */
