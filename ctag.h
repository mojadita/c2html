/* ctag.h -- ctag entry
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: sáb ago 23 21:13:42 EEST 2014
 * Disclaimer: (C) 2014-2024 LUIS COLORADO.  All rights reserved.
 */
#ifndef _CTAG_H
#define _CTAG_H

typedef struct ctag_s ctag;

#include "node.h"


struct ctag_s {
    const char     *id;             /* tag identifier */
    const char     *fi;             /* file this tag points to. */
    const char     *ss;             /* scan string for ex(1) */
    int             tag_no_in_file; /* tag number in file. */
    ctag           *next_in_file;   /* next tag with the same id in
                                     * this tag file */
    node           *nod;            /* file node this tag points to. */
};

ctag *
lookup_ctag(
        const char *id,
        const char *fi,
        const char *ss,
        node       *root);

void fprint_ctag(FILE *f, const ctag *t);

#endif /* _CTAG_H */
