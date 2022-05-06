/* $Id: debug.h,v 1.1 2014/09/09 20:22:06 luis Exp $
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

extern int flags;

/* constants */
#ifndef INCLUDE_DEBUG_CODE
#define INCLUDE_DEBUG_CODE 1
#endif

#define PR(_fmt) "%s:%d:%s:"_fmt,__FILE__,__LINE__,__func__

#if INCLUDE_DEBUG_CODE

#   define DEB(_flgs, _fmt, ...) DEB_TAIL(_flgs, PR(_fmt), ##__VA_ARGS__)
#   define DEB_TAIL(_flgs, _fmt, ...) do {  \
        if (flags & _flgs || flags & FLAG_DEBUG_ALL) \
            printf(_fmt, ##__VA_ARGS__);    \
        fflush(stdout);                     \
    } while (0)
#   define D(X) do {                                \
        DEB(flags & FLAG_DEBUG_DB, "%s;\n", #X);    \
        X;                                          \
    } while (0)

#else /* INCLUDE_DEBUG_CODE */

#   define DEB(_fmt, ...)
#   define DEB_TAIL(_fmt, ...)
#   define D(X) do {                \
        X;                          \
    } while (0)

#endif /* INCLUDE_DEBUG_CODE */

#define ERR(_code, _fmt, ...) ERR_TAIL(_code, PR(_fmt), ##__VA_ARGS__)
#define ERR_TAIL(_code, _fmt, ...) do {         \
        fprintf(stderr, _fmt, ##__VA_ARGS__);   \
        if (_code) exit(_code);                 \
    } while (0)

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* DEBUG_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.  */
/* $Id: debug.h,v 1.1 2014/09/09 20:22:06 luis Exp $ */
