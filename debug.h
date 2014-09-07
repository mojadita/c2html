/* $Id: header.h.m4,v 1.7 2005/11/07 19:39:53 luis Exp $
 * Author: Luis Colorado <lc@luiscoloradosistemas.com>
 * Date: Fri Sep  5 15:47:29 EEST 2014
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
#ifndef DEBUG_H
#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static char DEBUG_H_RCSId[] = "\n$Id: header.h.m4,v 1.7 2005/11/07 19:39:53 luis Exp $\n";

/* constants */
#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define DEB(X) do { printf X; fflush(stdout); } while (0)
#define D(X) DEB((PR("%s;\n"), #X)); X
#else
#define DEB(X)
#define D(X) X
#endif

#define PR(X) "%s:%d:%s: " X, __FILE__,__LINE__,__func__


/* types */

/* prototypes */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* DEBUG_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */

/* $Id: header.h.m4,v 1.7 2005/11/07 19:39:53 luis Exp $ */
