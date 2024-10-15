/* $Id: debug.h,v 1.1 2014/09/09 20:22:06 luis Exp $
 * Author: Luis Colorado <luiscoloradourcola@gmail.com>
 * Date: Fri Sep  5 15:47:29 EEST 2014
 * Copyright: (C) 1999-2024 Luis Colorado.  All rights reserved.
 * License: BSD
 */

/* Do not include anything BEFORE the line below, as it would not be
 * protected against double inclusion from other files
 */
#ifndef _DEBUG_H
#define _DEBUG_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FLAG_LINENUMBERS            (1 <<  0)
#define FLAG_PROGRESS               (1 <<  1)
#define FLAG_DEBUG_PROCESS1         (1 <<  2)
#define FLAG_DEBUG_DB               (1 <<  3)
#define FLAG_DEBUG_LEX              (1 <<  4)
#define FLAG_DEBUG_EX               (1 <<  5)
#define FLAG_DEBUG_CREATE_MENU      (1 <<  6)
#define FLAG_DEBUG_PROCESS2         (1 <<  7)
#define FLAG_DEBUG_PROCESS_DIR      (1 <<  8)
#define FLAG_DEBUG_PROCESS_FILE     (1 <<  9)
#define FLAG_DEBUG_PROCESS_IDENT    (1 << 10)
#define FLAG_DEBUG_PROCESS_MENU     (1 << 11)
#define FLAG_DEBUG_SCANFILE         (1 << 12)
#define FLAG_DEBUG_INTERN           (1 << 13)
#define FLAG_DEBUG_CTAGS            (1 << 14)
#define FLAG_DEBUG_NODES            (1 << 15)
#define FLAG_DEBUG_ALWAYS           (1 << 16)
#define FLAG_DEBUG_ALL              (1 << 17)

extern int flags;

/* constants */
#ifndef INCLUDE_DEBUG_CODE
#define INCLUDE_DEBUG_CODE 1
#endif

#define PR(_fmt) "%s:%d:%s:"_fmt,__FILE__,__LINE__,__func__

#if INCLUDE_DEBUG_CODE
#   define DEB(_flgs, _fmt, ...) \
        DEB_TAIL(_flgs, PR("DEB:"_fmt), ##__VA_ARGS__)
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
#   define D(X) X
#endif /* INCLUDE_DEBUG_CODE */

#define ERR(_code, _fmt, ...) ERR_TAIL(_code, PR("ERR:"_fmt), ##__VA_ARGS__)
#define ERR_TAIL(_code, _fmt, ...) do {         \
        fprintf(stderr, _fmt, ##__VA_ARGS__);   \
        if (_code) exit(_code);                 \
        fflush(stderr);                         \
    } while (0)

#define WRN(_fmt, ...) WRN_TAIL(_code, PR("WRN:"_fmt), ##__VA_ARGS__)
#define WRN_TAIL(_code, _fmt, ...) do {         \
        fprintf(stderr, _fmt, ##__VA_ARGS__);   \
        fflush(stderr);                         \
    } while (0)

#define INF(_fmt, ...) INF_TAIL(_code, PR("INF:"_fmt), ##__VA_ARGS__)
#define INF_TAIL(_fmt, ...) do {         \
        fprintf(stderr, _fmt, ##__VA_ARGS__);   \
        fflush(stderr);                         \
    } while (0)

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* _DEBUG_H */
