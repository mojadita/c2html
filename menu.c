/* $Id: menu.c,v 1.1 2014/09/09 20:23:05 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Sep  5 15:59:37 EEST 2014
 * Disclaimer: (C) 2014 LUIS COLORADO. All rights reserved.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "debug.h"
#include "intern.h"
#include "c2html.h"

#include "menu.h"

AVL_TREE db_menus = NULL;
char *default_menu_name = "00-Index";

tag_menu *lookup_menu(const char *id, node *root)
{
    tag_menu *res;
    static node *menus_dir;

    DEB(FLAG_DEBUG_PROCESS_MENU,
            "begin: id=[%s]\n", id);

    if (!db_menus) {
        DEB(FLAG_DEBUG_PROCESS_MENU,
                "initializing db_menus\n");
        db_menus = new_avl_tree(
            (AVL_FCOMP) strcmp,
            NULL,
            NULL,
            (AVL_FPRNT) fputs);
        if (!db_menus) {
            ERR(EXIT_FAILURE,
                    "Error: cannot allocate mem: %s\n",
                    strerror(errno));
            /* NOTREACHED */
        }
    } /* if */

    id = intern(id);

    DEB(FLAG_DEBUG_PROCESS_MENU,
            "searching for [%s] in db_menus\n", id);
    D(res = avl_tree_get(db_menus, id));
    if (!res) {
        char buffer[4096];
        res = malloc(sizeof (tag_menu));
        if (!res) {
            ERR(EXIT_FAILURE,
                "Error: cannot allocate mem: %s\n",
                strerror(errno));
            /* NOTREACHED */
        }
        res->id = id;
        res->flags = 0;
        res->ntags = 0;
        res->group_by_file = new_avl_tree(
                (AVL_FCOMP) strcmp,
                NULL, NULL,
                (AVL_FPRNT) fputs);
        if (!res->group_by_file) {
            ERR(EXIT_FAILURE,
                "Cannot allocate memory: %s\n",
                strerror(errno));
            /* NOTREACHED */
        }
        D(snprintf(buffer, sizeof buffer,
            "%s/%c/%s.html",
            default_menu_name,
            res->id[0], res->id));
        D(res->nod = name2node(root, buffer, TYPE_HTML));
        DEB(FLAG_DEBUG_PROCESS_MENU,
                "res->nod=%p res->nod->full_name=[%s], "
            "res->nod->html_file->full_name=[%s]\n",
            res->nod,
            res->nod->full_name,
            res->nod->html_file->full_name);
        res->last_tag = NULL;
        D(avl_tree_put(db_menus, id, res));
    } /* if */
    DEB(FLAG_DEBUG_PROCESS_MENU,
            "end [%p]\n", res);

    return res;
} /* lookup_menu */

/* $Id: menu.c,v 1.1 2014/09/09 20:23:05 luis Exp $ */
