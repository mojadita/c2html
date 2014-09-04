/* $Id: c2html.c,v 0.24 2009/01/03 22:23:11 luis Exp $
 * Author: Luis Colorado <Luis.Colorado@SLUG.CTV.ES>
 * Date: Thu Jun  3 19:30:16 MEST 1999
 * Disclaimer:
 *
 *     C2HTML -- A program to convert C source code to cross referenced HTML.
 *     Copyright (c) 1999 Luis Colorado
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

#define IN_C2HTML_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>

#include <avl.h>

#include "db.h"
#include "c2html.h"
#include "intern.h"
#include "html_output.h"

/* constants */

#ifndef DEBUG
#define DEBUG 0
#endif

#define MAXLINELENGTH	4096

char *rcsId = "\n$Id: c2html.c,v 0.24 2009/01/03 22:23:11 luis Exp $\n";

/* types */

/* prototypes */

/* variables */

int flags = 0;
const char *tag_file = DEFAULT_TAG_FILE;
const char *output = DEFAULT_OUTPUT;
const char *base_dir = DEFAULT_BASE_DIR;
const char *style_file = DEFAULT_STYLE_FILE;
node *style_node = NULL;
const char *js_file = DEFAULT_JS_FILE;
node *js_node = NULL;

/* functions */

/* process file of name fn */
void process1(const char *fn)
{
	FILE *tagfile;
	char line [MAXLINELENGTH];
	unsigned long long line_num = 0;
	
	tagfile = fopen (fn, "r");
	if (!tagfile) {
		fprintf (stderr,
			PR("Error opening %s: %s\n"),
			fn, strerror(errno));
		exit (EXIT_FAILURE);
	} /* if */

	if (flags & FLAG_DEBUG_PROCESS1) {
		printf(
			PR("Processing %s...\n"),
			fn);
	} /* if */

	/* file open, process lines */
	while (fgets(line, sizeof line, tagfile)) {
		const char *id, *fi, *st;
		const ctag *tag;
		
		if (flags & FLAG_DEBUG_PROCESS1) {
			printf(PR("step: %s\n"), line);
		} /* if */

		line_num++;

		id = strtok (line, "\t\n"); if (!id) {
			fprintf(stderr,
				PR("%s:%lld:warning: "
				"bad syntax, unrecognized id.\n"),
				fn, line_num);
			continue;
		} /* if */
		fi = strtok (NULL, "\t\n"); if (!fi) {
			fprintf(stderr,
				PR("%s:%lld:warning: "
				"bad syntax, unrecognized file name.\n"),
				fn, line_num);
			continue;
		} /* if */
		st = strtok (NULL, "\n"); if (!st) {
			fprintf(stderr,
				PR("%s:%lld:warning: "
				"bad syntax, unrecognized search string.\n"),
				fn, line_num);
			continue;
		} /* if */

		/* Ignore VIM ctags(1) private symbols */
		if (id[0] == '!') {
			fprintf(stderr,
				PR("%s:%lld:warning: "
				"ignoring \"%s\" vim private identifier\n"),
				fn, line_num, id);
			continue;
		} /* if */

		/* first, find the ctag entry */
		if (flags & FLAG_DEBUG_PROCESS1) {
			printf(
				PR("calling lookup ctag(%s, %s, %s);...\n"),
				id, fi, st);
		} /* if */
		tag = lookup_ctag(id, fi, st);

	} /* while ... */

	if (flags & FLAG_DEBUG_PROCESS1) {
		printf(
			PR("%s:closing tagfile\n"),
			fn);
	} /* if */
	fclose(tagfile);
} /* process1 */

int send_ex(FILE *ex, const char *fmt, ...)
{
	va_list p;

	if (flags & FLAG_DEBUG_EX) {
		printf(PR("EX:"));
		va_start(p, fmt);
		vprintf(fmt, p);
		va_end(p);
	} /* if */
	va_start(p, fmt);
	vfprintf(ex, fmt, p);
	va_end(p);
} /* send_ex */

void process2(node *n)
{
	assert(n);

	if (flags & FLAG_DEBUG_PROCESS2) {
		printf(PR("process2: entering \"%s\"\n"), n->full_name);
	} /* if */

	switch (n->type) {
	case FLAG_ISDIR: {
		int res;
		FILE *ixf;
		AVL_ITERATOR it;

		/* 1.- mkdir */
		if (flags & FLAG_DEBUG_PROCESS2) {
			printf(PR("process2:   mkdir(\"%s\");\n"),
				n->full_name);
		} /* if */
		res = mkdir(n->full_name, 0777);
		if (res < 0) {
			fprintf(stderr,
				"process2:error:MKDIR:%s:%s(errno=%d)\n",
				n->full_name, strerror(errno), errno);
			return; /* cannot continue */
		} /* if */

		/* 2.- create html file and open it for writing */
		assert(n->html_file);
		if (flags & FLAG_DEBUG_PROCESS2) {
			printf(PR("process2:   "
				"html_create(\"%s\");\n"),
				n->html_file->full_name);
		} /* if */
		ixf = html_create(n);
		fprintf(ixf, "      <ul>\n");

		/* 3.- write on the parent html file a reference to us
		 * (of course if it has a parent) */
		if (n->parent) {
			fprintf(n->parent->html_file->index_f,
				"      <li><span class=\"dir\">"
				"<a href=\"%s\">%s</a> directory."
				"</span></li>\n",
				rel_path(n->parent->html_file, n->html_file), n->name);
		} /* if */
				
		/* 4.- recurse to subdirectories/files */
		for (	it = avl_tree_first(n->subnodes);
			it;
			it = avl_iterator_next(it))
		{
			process2(avl_iterator_data(it));
		} /* for */

		/* 4.- close files */
		fprintf(ixf, "      </ul>\n");
		html_close(n);
	} break;

	/* 4.- PROCESS FILE */
	case FLAG_ISFILE: {
		FILE *ex_fd;
		AVL_ITERATOR it;

		assert(n->parent);
		assert(n->parent->html_file);
		assert(n->parent->html_file->index_f);

		fprintf(n->parent->html_file->index_f,
			"      <li><span class=\"file\">"
			"<a href=\"%s\">%s</a> file.</span>\n",
			n->html_file->name, n->name);

		ex_fd = popen(EX_PATH, "w");
		send_ex(ex_fd, "set notagstack\n");
		{	const char *s;

			s = strchr(n->full_name, '/');
			if (!s) s = n->full_name;
			while (*s == '/') s++;
			send_ex(ex_fd, "e! %s\n", s); /* edit file */
			if (flags & FLAG_DEBUG_PROCESS2) {
				printf(PR("editing sesion on file %s -> %s: begin\n"),
					s, n->full_name);
			} /* if */
		} /* block */

		/* for every tag in this file */
		printf(PR("FILE name=[%s]\n"), n->full_name);
		if (avl_tree_size(n->subnodes)) {
			fprintf(n->parent->html_file->index_f, "        <ul>\n");
			for (	it = avl_tree_first(n->subnodes);
					it;
					it = avl_iterator_next(it))
			{
				const ctag *p;
				assert(p = avl_iterator_data(it));
				/* the first tag in the list contains the total number of tags in list */
				if (p->tag_no_in_file > 1) {
					for (; p; p = p->next_in_file) {
						printf(PR("TAG[%p] id=[%s], fi=[%s], ss=<%s>, tag_no_in_file=%d, next_in_file=[%p]\n"),
							p, p->id, p->fi, p->ss, p->tag_no_in_file, p->next_in_file);
						fprintf(n->parent->html_file->index_f,
							"          <li><span class=\"tag\">"
							"<a href=\"%s#%s-%d\">%s</a></span></li>\n",
							n->html_file->name, p->id, p->tag_no_in_file, p->id);
						/* tag select, se vim(1) help */
						send_ex(ex_fd,
							"ts %s\n"
							"%d\n",
							p->id,
							p->tag_no_in_file);
						send_ex(ex_fd,
							"s:^:(@a name=\"%s-%d\"@)(@/a@):\n",
							p->id, p->tag_no_in_file); /* change */
					} /* for */
				} else {
					fprintf(n->parent->html_file->index_f,
						"          <li><span class=\"tag\">"
						"<a href=\"%s#%s-%d\">%s</a></span></li>\n",
						n->html_file->name, p->id, p->tag_no_in_file, p->id);
					send_ex(ex_fd, "ta %s\n", p->id); /* goto tag, only one tag in this file */
					send_ex(ex_fd, "s:^:(@a name=\"%s-%d\"@)(@/a@):\n",
						p->id, p->tag_no_in_file); /* change */
				} /* if */
			} /* for */
			fprintf(n->parent->html_file->index_f, "        </ul>\n");
		} /* if */
		send_ex(ex_fd, "w! %s\n", n->full_name); /* write file */
		send_ex(ex_fd, "q!\n"); /* terminate */
		if (flags & FLAG_PROGRESS) {	/* print progress */
			static int i=0;
			static char *progress[] = { "\\", "|", "/", "-" };
			static long acum; 
			static int percent = 0;

			if (!i++) acum = n_files >> 1;

			acum += 100000;
			if (acum >= n_files) {
				percent += acum / n_files;
				acum %= n_files;
			} /* if */
			fprintf(stderr, "\r%s (%d/%d -- %3d.%03d%%) %s\033[K",
				progress[i&3], i, n_files, percent / 1000,
				percent % 1000, n->full_name);
			if (i >= n_files) fprintf(stderr, "\n");
		} /* block */
		pclose(ex_fd);
		fprintf(n->parent->html_file->index_f, "      </li>\n");
		if (flags & FLAG_DEBUG_PROCESS2) {
			printf(PR("scanning file %s -> %s\n"),
				n->full_name, n->html_file->full_name);
		} /* if */
		scanfile(n);
	} break;

	/* 5.- DATABASE INCONSISTENCY */
	default:
		fprintf(stderr,
			PROGNAME":"__FILE__"(%d): DATABASE "
			"INCONSISTENCY (n->type == %d)\n",
			__LINE__, n->type);
		abort();
	} /* switch */

#if DEBUG
	printf("process2: leaving \"%s\"\n", n->full_name);
#endif
} /* process2 */

/* print help message */
void do_usage (void)
{
	fprintf(stderr, 
"Usage: " PROGNAME " [ options ... ]\n"
PROGNAME " " VERSION ": Copyright (C) 1999 <Luis.Colorado@SLUG.HispaLinux.ES>\n"
"This program is under GNU PUBLIC LICENSE, version 2 or later\n"
"see the terms and conditions of use at http://www.gnu.org/\n"
"(you might receive a copy of it with this program)\n"
"\n"
"This program operates on a constructed tags file (see ctags(1)), and\n"
"constructs an HTML hierarchy of source files, parallel to their C\n"
"counterparts, with hyperlink cross references to all the C identifiers\n"
"located in the code.\n"
"\n"
"It uses the tags file to locate all the identifier definitions in the\n"
"C code, and then, it constructs a syntax marked HTML file, with each\n"
"definition found in the tags file marked in the code, and every reference\n"
"to it, surrounded by a <a href> tag, so clicking with the mouse leads us\n"
"quickly and efficiently to the definition.\n"
"Options:\n"
"  -h   Help.  This help screen.\n"
"  -t <tag_file>  The tag file to be used (default: " DEFAULT_TAG_FILE ")\n"
"  -b <base_dir>  Base directory for URL composition\n"
"       This causes to generate <BASE> tags. (default: " DEFAULT_BASE_DIR_STRING ")\n"
"  -d <debug_options>  Debug options can be several of:  \n"
"         1  process1 debug.\n"
"         2  process2 debug.\n"
"         d  database debug.\n"
"         l  lexical processing debug.\n"
"         x  ex(1) commands debug.\n"
"         m  create_menu debug.\n"
"  -n   Output linenumbers.\n"
"  -o   Output directory (default " DEFAULT_OUTPUT ")\n"
"  -p   Progress is shown on stderr.\n"
"  -s   Style file (default " DEFAULT_STYLE_FILE ")\n"
"  -j   Javascript file (default " DEFAULT_JS_FILE ")\n"
		);

} /* do_usage */

/* main program */
int main (int argc, char **argv)
{

	extern int optind;
	extern char *optarg;
	int opt;

	while ((opt = getopt(argc, argv, "t:hb:2d:rno:ps:j:")) != EOF) {
		switch(opt) {
		case 'h': do_usage(); exit(EXIT_SUCCESS);
		case 't': tag_file = optarg; break;
		case 'b': base_dir = optarg; break;
		case 'n': flags |= FLAG_LINENUMBERS; break;
		case 'o': output = optarg; break;
		case 'p': flags |= FLAG_PROGRESS; break;
		case 's': style_file = optarg; break;
		case 'j': js_file = optarg; break;
		case 'd': {	char *p; /* debug */
				for (p = optarg; *p; p++) {
					switch(*p) {
					case '1': flags |= FLAG_DEBUG_PROCESS1; break;
					case 'd': flags |= FLAG_DEBUG_DB; break;
					case 'l': flags |= FLAG_DEBUG_LEX; break;
					case 'x': flags |= FLAG_DEBUG_EX; break;
					case 'm': flags |= FLAG_DEBUG_CREATE_MENU; break;
					case '2': flags |= FLAG_DEBUG_PROCESS2; break;
					} /* switch */
				} /* for */
			} /* block */
			break;
		default:
			do_usage(); exit(EXIT_FAILURE);
		} /* switch */
	} /* while */

	db_init(output);
	style_node = new_node(style_file, db_root_node, FLAG_ISFILE);
	js_node = new_node(js_file, db_root_node, FLAG_ISFILE);

	/* Process files */

	process1(tag_file);

	{	AVL_ITERATOR i;
		for (	i = avl_tree_first(db_menus);
				i;
				i = avl_iterator_next(i))
		{
			tag_menu *men = avl_iterator_data(i);
			AVL_ITERATOR j;
			printf(PR("MENU[%s]: ntags=%d, nod=[%s], last_tag=<%s,%s,%p>\n"),
				men->name, men->ntags, men->nod->full_name,
				men->last_tag->id, men->last_tag->fi, men->last_tag->ss);	
			for (	j = avl_tree_first(men->group_by_file);
					j;
					j = avl_iterator_next(j))
			{
				char *fn = avl_iterator_key(j);
				ctag *tag;
				printf(PR("  FILE: %s\n"), fn);
				for (tag = avl_iterator_data(j); tag; tag = tag->next_in_file) {
					printf(PR("    TAG[%p]: id=[%s], fi=[%s], tag_no_in_file=%d, next_in_file=[%p]\n"),
						tag, tag->id, tag->fi, tag->tag_no_in_file, tag->next_in_file);
				} /* for */
			} /* for */
			
		} /* for */
	} /* block */

	process2(db_root_node);

} /* main */

/* $Id: c2html.c,v 0.24 2009/01/03 22:23:11 luis Exp $ */
