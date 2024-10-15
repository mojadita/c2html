/* $Id: menu.c,v 1.1 2014/09/09 20:23:05 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Fri Sep  5 15:59:37 EEST 2014
 * Disclaimer: (C) 2014-2024 LUIS COLORADO. All rights reserved.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <avl.h>

#include "configure.h"
#include "debug.h"
#include "intern.h"
#include "c2html.h"

#include "menu.h"

AVL_TREE db_menus = NULL;
char *default_menu_name = DEFAULT_MENU_BASE;
static node *menus_dir = NULL;

tag_menu *
lookup_menu(
        const char *id,
        node       *root)
{
    tag_menu *res;

    DEB(FLAG_DEBUG_PROCESS_MENU,
        "begin: id='%s'\n", id);

    if (!db_menus) {
        DEB(FLAG_DEBUG_PROCESS_MENU,
            "'%s': initializing db_menus\n",
            id);
        db_menus = new_avl_tree(
            (AVL_FCOMP) strcmp,
            NULL,
            NULL,
            (AVL_FPRNT) fputs);
        if (!db_menus) {
            ERR(EXIT_FAILURE,
                "'%s': Error: cannot allocate mem: %s\n",
                id, strerror(errno));
            /* NOTREACHED */
        }
    } /* if */

    DEB(FLAG_DEBUG_PROCESS_MENU,
        "searching for key='%s' in db_menus\n",
        id);
    res = avl_tree_get(db_menus, id);
    if (!res) {
        char buffer[4096];
        DEB(FLAG_DEBUG_PROCESS_MENU,
            "'%s': not found, creating it\n",
            id);
        res = malloc(sizeof (tag_menu));
        if (!res) {
            ERR(EXIT_FAILURE,
                "'%s': Error: cannot allocate mem: %s\n",
                id, strerror(errno));
        }
        res->id    = id;                    /* id */
        res->flags = 0;                     /* flags */
        res->ntags = 0;                     /* ntags */
        res->group_by_file = new_avl_tree(  /* group_by_file */
                (AVL_FCOMP) strcmp,
                NULL, NULL,
                (AVL_FPRNT) fputs);
        if (!res->group_by_file) {
            ERR(EXIT_FAILURE,
                "'%s': Cannot allocate memory: %s\n",
                id, strerror(errno));
            /* NOTREACHED */
        }
        snprintf(buffer, sizeof buffer,
                "%s/%c/%s",
                default_menu_name,
                res->id[0], res->id);
        res->nod = name2node(root, intern(buffer), TYPE_MENU); /* nod */
        res->nod->menu = res; /* reference to each other */
        DEB(FLAG_DEBUG_PROCESS_MENU,
            "'%s': nod = '%s'(%p)",
            id, res->nod->full_name, res->nod);
        res->last_tag = NULL;
        avl_tree_put(db_menus, id, res);
    } /* if */

    DEB(FLAG_DEBUG_PROCESS_MENU,
        "end '%s' -> '%s'(%p)\n",
        id, res->id, res);

    return res;
} /* lookup_menu */



void fprint_menu(FILE *f, const tag_menu *m)
{
#define P(fld, fmt) fprintf(f, "%20s: " fmt "\n", #fld, m->fld)

    P(id, "%s");
    P(flags, "%02x");
    P(ntags, "%d");
    P(group_by_file, "%p");
    P(nod->type, "%d");
    P(nod->full_name, "%s");
    P(first_tag, "%p");
    P(last_tag, "%p");
} /* fprint_menu */

void print_menus(void)
{
    for (AVL_ITERATOR it = avl_tree_first(db_menus);
            it;
            it = avl_iterator_next(it))
    {
        fprint_menu(stdout, avl_iterator_data(it));
    }
} /* print_menus */

/* $Id: menu.c,v 1.1 2014/09/09 20:23:05 luis Exp $ */
