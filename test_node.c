/* $Id: test_node.c,v 1.1 2014/09/09 20:23:07 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Thu Sep  4 20:14:34 EEST 2014
 * Copyright: (c) 1999-2024 Luis Colorado.  All rights reserved.
 *
 * Disclaimer:
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define IN_TEST_NODE_C

#define DEBUG   1

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "debug.h"
#include "node.h"


/* constants */
#define MAX     50

/* types */
int flags = 0;

/* prototypes */

/* variables */
static char TEST_NODE_C_RCSId[]="\n$Id: test_node.c,v 1.1 2014/09/09 20:23:07 luis Exp $\n";

node **nodes;
int nodes_n = 0;

/* functions */
int f(const node *n)
{
    printf(PR("Nodo %s\n"), n->full_name);
    return 0;
} /* f */

/* main program */
int main (int argc, char **argv)
{
    int i;
    node *res;

    assert(nodes = calloc(MAX, sizeof (node *)));
    for (i = 0; i < MAX; i++) {
        char buffer[16];
        snprintf(buffer, sizeof buffer, "#%06d", i);
        nodes[i] = new_node(
            buffer,
            i   ? nodes[rand() % i]
                : NULL,
            TYPE_DIR);
    } /* for */

    name2node(nodes[0], "style.css", TYPE_SOURCE)->flags |= NODE_FLAG_DONT_RECUR_INFILE;
    name2node(nodes[0], "javascript.js", TYPE_SOURCE)->flags |= NODE_FLAG_DONT_RECUR_INFILE;
    name2node(nodes[0], "prueba.c", TYPE_SOURCE);
    name2node(nodes[0], "home/eluscoo/profile", TYPE_HTML);
    name2node(nodes[0], "home/eluscoo/prueba.c", TYPE_SOURCE);
    name2node(nodes[0], "home/eluscoo/prueba.h", TYPE_SOURCE);
    name2node(nodes[0], "home/eluscoo/prueba.h", TYPE_SOURCE);
    name2node(nodes[0], "home/eluscoo/a/b", TYPE_DIR);
    name2node(nodes[0], "home/eluscoo/a/b/h", TYPE_DIR);
    name2node(nodes[0], "home/eluscoo/profile/pepe", TYPE_SOURCE);
    res = name2node(nodes[0], "home/eluscoo/pepe/../fich_1.html", TYPE_HTML);
    do_recur(nodes[0], f, f, f);
    printf(PR("rel_path(%s, %s) -> %s\n"), res->full_name, res->full_name,
        rel_path(res, res));


#if 0
    for(i = 0; i < MAX-1; i++) {
        char *s = rel_path(nodes[i]->html_file, nodes[i+1]->html_file);
        printf(PR("rel_path(%s, %s) -> %s\n"),
            nodes[i]->html_file->full_name, nodes[i+1]->html_file->full_name,
            s);
    } /* for */
#endif
} /* main */

/* $Id: test_node.c,v 1.1 2014/09/09 20:23:07 luis Exp $ */
