/* $Id: header.h.m4,v 1.7 2005-11-07 19:39:53 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Wed Sep  3 22:09:54 EEST 2014
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
#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

#define PR(X)	"%s:%d:%s: " X, __FILE__, __LINE__, __func__

static char LOG_H_RCSId[] = "\n$Id: header.h.m4,v 1.7 2005-11-07 19:39:53 luis Exp $\n";

/* constants */

/* prototypes */

/* produces a log message with the format of the FMT macro */
size_t info(const char *fmt, const char *fi, const int li, const char *fn, ...);
size_t vinfo(const char *fmt, va_list p);

/* produces a warning message with the format of the FMT macro */
size_t warning(const char *fmt, const char *fi, const int li, const char *fn, ...);
size_t vwarning(const char *fmt, va_list p);

/* produces an error message with the format of the FMT macro
 * these functions don't return, as they exit with EXIT_FAILURE code */
void error(const char *fmt, const char *fi, const int li, const char *fn, ...);
void verror(const char *fmt, va_list p);

#endif /* LOG_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */

/* $Id: header.h.m4,v 1.7 2005-11-07 19:39:53 luis Exp $ */
