/* $Id: intern.h,v 1.1 2014/09/09 20:22:07 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Fri Aug  8 19:22:29 EEST 2014
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

/* Do not include anything BEFORE the line below, as it would not be
 * protected against double inclusion from other files
 */
#ifndef _INTERN_H
#define _INTERN_H

#include <stdio.h>

/* prototypes */
const char *intern(const char *s);
int print_string(FILE *o, const char *s);

#endif /* _INTERN_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */
/* $Id: intern.h,v 1.1 2014/09/09 20:22:07 luis Exp $ */
