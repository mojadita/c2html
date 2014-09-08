/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Sep  5 15:37:17 EEST 2014
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

#define IN_TEST_MENU_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <avl.h>
#include "debug.h"
#include "menu.h"
#include "node.h"

/* constants */

/* types */

/* prototypes */

/* variables */
static char TEST_MENU_C_RCSId[]="\n$Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $\n";

/* functions */
int print_menu(tag_menu *men)
{
	printf(PR("MENU %p, id=%s, flags=0x%08x, ntags=%d, nod=[%p], last_tag=[%p]\n"),
		men, men->id, men->flags, men->ntags, men->nod, men->last_tag);
} /* print_menu */

/* main program */
int main (int argc, char **argv)
{
	tag_menu *res;
	AVL_ITERATOR i;

	node *root = new_node("test", NULL, TYPE_DIR);

	res = lookup_menu("TEST_MENU", root);
	res = lookup_menu("TEST_MENU2", root);
	res = lookup_menu("ANOTHER_TEST_MENU", root);
	for (	i = avl_tree_first(db_menus);
			i;
			i = avl_iterator_next(i))
	{
		print_menu(avl_iterator_data(i));
	} /* for */
} /* main */

/* $Id: main.c.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
