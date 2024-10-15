/* $Id: test_ctag.c,v 1.1 2014/09/09 20:23:07 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Sat Sep  6 13:59:16 EEST 2014
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
#define DEBUG   1

#define IN_TEST_CTAG_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "debug.h"
#include "node.h"
#include "menu.h"
#include "ctag.h"

/* variables */
static char TEST_CTAG_C_RCSId[]="\n$Id: test_ctag.c,v 1.1 2014/09/09 20:23:07 luis Exp $\n";

node *root;

int flags = FLAG_DEBUG_ALWAYS;

/* functions */
int do_dir_pre(const node *dir, const char *s)
{
    DEB(0, "%s: begin: dir=[%s]\n", s, dir->full_name);
    DEB(0, "%s: end: dir=[%s]\n", s, dir->full_name);
    return 0;
} /* do_file */

int do_dir_post(const node *dir, const char *s)
{
    DEB(0, "%s: begin: dir=[%s]\n", s, dir->full_name);
    DEB(0, "%s: end: dir=[%s]\n", s, dir->full_name);
    return 0;
} /* do_file */

int do_file(const node *fil, const char *s)
{
    DEB(0, "%s: begin: file=[%s]\n", s, fil->full_name);
    DEB(0, "%s: end: file=[%s]\n", s, fil->full_name);
    return 0;
} /* do_file */

/* main program */
int main (int argc, char **argv)
{
    node *style, *javascript;
    D(root = new_node("root", NULL, TYPE_DIR));
    D(style = new_node("style.css", root, TYPE_HTML));
    D(javascript = new_node("javascript.js", root, TYPE_HTML));
    D(lookup_ctag("pepe", "a/b/cd/pepe", "134", root));
    D(lookup_ctag("pepe", "a/b/cd/pepe", "246", root));
    D(lookup_ctag("pepe", "a/b/cd/juan", "246", root));
    D(lookup_ctag("juan", "a/b/cd/juan", "246", root));
    do_recur(root, do_dir_pre, do_file, do_dir_post);
} /* main */

/* $Id: test_ctag.c,v 1.1 2014/09/09 20:23:07 luis Exp $ */
