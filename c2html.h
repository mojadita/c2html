/* $Id: c2html.h,v 0.18 2014/09/09 20:22:05 luis Exp $
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
#ifndef C2HTML_H
#define C2HTML_H

#include "configure.h"
#include "node.h"

/* constants */
#ifndef VERSION
#error  VERSION is not defined
#endif

#ifndef NELEM
#define NELEM(arr) (sizeof (arr) / sizeof (arr)[0])
#endif

#ifndef EX_PATH
#error  EX_PATH must be defined to the path of the ex(1) command.
#endif

#ifndef FALSE
#define FALSE (1 == 0)
#define TRUE    (!FALSE)
#endif

#define DEFAULT_BUFSIZE 4096
#define EXT1            ".temp"
#define EXT2            ".html"
#define PFX1            "ta-"
#define PFX2            "i-"

#undef TRACEON      /* 1 */

/* variables */

#define DEFAULT_TAG_FILE        "tags"
#define DEFAULT_OUTPUT          "html"
#define DEFAULT_BASE_DIR        NULL
#define DEFAULT_BASE_DIR_STRING "<NULL>"
#define DEFAULT_STYLE_FILE      "style.css"
#define DEFAULT_JS_FILE         "javascript.js"

extern int          flags;
extern const char   *tag_file;
extern const char   *output;
extern const char   *base_dir;
extern const char   *style_file;
extern node         *style_node;
extern const char   *js_file;
extern node         *js_node;

/* functions */

#endif /* C2HTML_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */

/* $Id: c2html.h,v 0.18 2014/09/09 20:22:05 luis Exp $ */
