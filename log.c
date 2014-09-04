/* $Id: main.c.m4,v 1.7 2005-11-07 19:39:53 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Wed Sep  3 22:10:16 EEST 2014
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

#define IN_LOG_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "log.h"

/* variables */
static char LOG_C_RCSId[]="\n$Id: main.c.m4,v 1.7 2005-11-07 19:39:53 luis Exp $\n";

/* functions */
size_t log(const char *fmt, ...)
{
	va_list p;
	size_t res;

	va_start(p, fmt);
	res = vlog(fmt, p);
	va_end(p);

	return res;
} /* log */

size_t vlog(const char *fmt, va_list p)
{
	return vprintf(fmt, p);
} /* vlog */
	
size_t warning(const char *fmt, ...)
{
	size_t res;
	va_list p;
	va_start(p, fmt);
	res = vwarning(fmt, p);
	va_end(p);
	return res;
} /* warning */

size_t vwarning(const char *fmt, va_list p)
{
	return vfprintf(stderr, fmt, p);
} /* vwarning */

/* produces an error message with the format of the FMT macro
 * these functions don't return, as they exit with EXIT_FAILURE code */
void error(const char *fmt, ...)
{
	va_list p;

	va_start(p, fmt);
	vfprintf(stderr, fmt, p);
	va_end(p);
	exit(EXIT_FAILURE);
} /* error */

void verror(const char *fmt, va_list p)
{
	vfprintf(stderr, fmt, p);
	exit(EXIT_FAILURE);
} /* error */

/* $Id: main.c.m4,v 1.7 2005-11-07 19:39:53 luis Exp $ */
