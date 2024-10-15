/* $Id: html_output.h,v 1.1 2014/09/09 20:22:07 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Sat Jun  5 22:45:02 MEST 1999
 * Disclaimer: (c) 1999-2024 Luis Colorado.  All rights reserved.
 *
 *     C2HTML -- A program to conver C source code into cross referenced HTML.
 *     Copyright (C) 1999 <Luis.Colorado@SLUG.HispaLinux.ES>
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
/* Do not include anything BEFORE the line below, as it would not be
 * protected against double inclusion from other files
 */
#ifndef _HTML_OUTPUT_H
#define _HTML_OUTPUT_H

#include <stdio.h>

#include "node.h"

/* prototypes */
FILE *html_create(node *n);
void html_close(node *n);

void fprintf_html(FILE *f, const char *fmt, ...);
void create_menu(tag_menu *m);

/* this function generates a reference from an identifier to its source,
 * depending on the number of tags that the identifier has.  In case of
 * only one reference, a direct reference to the definition is made.
 * In case of several, a reference to a menu file is made. */
int html_generate_ref(
        FILE *o,            /* where to write the reference in */
        const char *ident,  /* the reference name to write. */
        const node *fin);         /* file in which the reference is found */

#endif /* _HTML_OUTPUT_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */
/* $Id: html_output.h,v 1.1 2014/09/09 20:22:07 luis Exp $ */
