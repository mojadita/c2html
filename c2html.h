/* $Id: c2html.h,v 0.17 2009/01/03 22:23:11 luis Exp $
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
#ifndef C2HTML_H
#define C2HTML_H

#include "multifree.h"
#include "hashTable.h"

/* constants */
#ifndef VERSION
#error VERSION is not defined
#endif
#define DEFAULT_TAGS	"tags"
#ifndef EX_PATH
#error EX_PATH must be defined to the path of the ex(1) command.
#endif
#define EXCMD_BUFSIZE	1024
#define EXT1			".temp"
#define EXT2			".html"
#define PFX1			"ta-"
#define PFX2			"i-"

#define FLAG_MENU_CREATED	1

#ifndef FALSE
#define FALSE (1 == 0)
#define TRUE	(!FALSE)
#endif

#undef TRACEON		/* 1 */

#define FLAG_TWOLEVEL	(1 << 0)
#define FLAG_VERBOSE	(1 << 1)
#define FLAG_RELFILENAME	(1 << 2)
#define FLAG_LINENUMBERS	(1 << 3)
#define FLAG_PROGRESS		(1 << 4)

/* types */
typedef struct ctag_node {
	unsigned int flags;
	char *sym;  /* symbol */
	char *file; /* file */
	char *ctfile; /* ctag file */
	int tag_num; /* tag number */
	struct ctag_node *ctags_next;
	struct ctag_node *next;
} CtagNode;

typedef struct file_node {
	char *name;
	struct file_node *files_next;
	struct ctag_node *ctags_first;
	struct ctag_node *ctags_last;
} FileNode;

/* prototypes */
FILE *html_create(const char *name, const node *p);
void html_close(FILE *f);

/* variables */
extern HashTable syms_table, files_table;
extern FileNode *files_first, *files_last;

extern int flags;
extern const char *tag_file;
extern const char *output;
extern const char *base_dir;
extern const char *style_file;
extern const node *style_node;

/* functions */

#endif /* C2HTML_H */
/* Do not include anything AFTER the line above, as it would not be
 * protected against double inclusion from other files.
 */

/* $Id: c2html.h,v 0.17 2009/01/03 22:23:11 luis Exp $ */
