/* $Id: html_output.h,v 1.1 2014/09/09 20:22:07 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Sat Jun  5 22:45:02 MEST 1999
 * Disclaimer: (c) 1999 Luis Colorado <luis.colorado@SLUG.CTV.ES>
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

#include "db.h"
#include "c2html.h"

/* prototypes */
FILE *html_create(const node *n);
void html_close(const node *n);

void fprintf_html(FILE *f, const char *fmt, ...);

#endif /* _HTML_OUTPUT_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */
/* $Id: html_output.h,v 1.1 2014/09/09 20:22:07 luis Exp $ */
