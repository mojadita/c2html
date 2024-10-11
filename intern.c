/* $Id: intern.c,v 1.1 2014/09/09 20:22:07 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: sáb ago 23 11:50:37 EEST 2014
 * Disclaimer: (C) 2014-2024 LUIS COLORADO SISTEMAS S.L.U.
 *              All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "configure.h"
#include "debug.h"
#include "intern.h"
#include "c2html.h"

static AVL_TREE intern_strings = NULL;

const char *intern(const char *s)
{
    char *res;

    DEB(FLAG_DEBUG_INTERN, " begin for '%s'(%p)\n", s, s);
    if (!intern_strings) {
        DEB(FLAG_DEBUG_INTERN,
                " initializing intern_strings\n");
        intern_strings = new_avl_tree(
            (AVL_FCOMP) strcmp,
            NULL,
            NULL,
            (AVL_FPRNT) fputs);
        if (!intern_strings) {
            ERR(EXIT_FAILURE,
                "new_avl_tree: initializing intern_strings: %s\n",
                strerror(errno));
        }
    } /* if */

    res = avl_tree_get(intern_strings, s);
    if (!res) {
        DEB(FLAG_DEBUG_INTERN,
                "adding nonexistent '%s' to database\n", s);
        res = strdup(s);
        avl_tree_put(intern_strings, res, res);
    } /* if */

    DEB(FLAG_DEBUG_INTERN, " end s='%s'(%p)\n", res, res);

    return res;
} /* intern */

/* $Id: intern.c,v 1.1 2014/09/09 20:22:07 luis Exp $ */
