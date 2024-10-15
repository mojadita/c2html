/* $Id: menu.h,v 1.1 2014/09/09 20:23:06 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Sep  5 15:59:13 EEST 2014
 * Copyright: (C) 2014-2024 LUIS COLORADO.  All rights reserved.
 */
#ifndef _MENU_H
#define _MENU_H

#include <avl.h>

#include "node.h"
#include "ctag.h"

#define TAG_MENU_FLAG_ALREADY_CREATED   0x00000001

typedef struct tag_menu_s {
    const char *id; /* name of tag */
    int         flags; /* flags list, see above. */
    int         ntags; /* number of tags with this name */
    AVL_TREE    group_by_file; /* list of tags grouped by file */
    node       *nod; /* node for this menu/html file for this menu */
    ctag       *first_tag; /* first tag registered for this menu,
                                 * for unique tags. */
    ctag       *last_tag; /* last tag registered for this menu,
                                * for unique tags. */
} tag_menu;

extern AVL_TREE db_menus;
extern char *default_menu_name;

tag_menu *
lookup_menu(
        const char *id,
        node       *rt);

void fprint_menu(FILE *f, const tag_menu *m);

#endif /* _MENU_H */
/* $Id: menu.h,v 1.1 2014/09/09 20:23:06 luis Exp $ */
