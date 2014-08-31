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
#include "db.h"

/* constants */

#ifndef DEBUG
#define DEBUG 1
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
			PROGNAME": Error opening %s: %s\n",
			fn, strerror(errno));
		exit (EXIT_FAILURE);
	} /* if */

#if DEBUG
	printf("Processing %s...\n", fn);
#endif

	/* file open, process lines */
	while (fgets(line, sizeof line, tagfile)) {
		const char *id, *fi, *st;
		const ctag *tag;
		
		line_num++;

		id = strtok (line, "\t\n"); if (!id) {
			fprintf(stderr,
				"%s:%lld:warning: bad syntax, unrecognized id.\n",
				fn, line_num);
			continue;
		} /* if */
		fi = strtok (NULL, "\t\n"); if (!fi) {
			fprintf(stderr,
				"%s:%lld:warning: bad syntax, unrecognized file name.\n",
				fn, line_num);
			continue;
		} /* if */
		st = strtok (NULL, "\n"); if (!st) {
			fprintf(stderr,
				"%s:%lld:warning: bad syntax, unrecognized search string.\n",
				fn, line_num);
			continue;
		} /* if */

		/* Ignore VIM ctags(1) private symbols */
		if (id[0] == '!') {
			fprintf(stderr,
				"%s:%lld:warning: ignoring \"%s\" vim private identifier\n",
				fn, line_num, id);
			continue;
		} /* if */

		/* first, find the ctag entry */
		tag = ctag_lookup(id, fi, st);
#if DEBUG
		printf(
			"TAG:\n"
			"  tag->id: %s\n"
			"  tag->tag_no: %d\n"
			"  tag->fi: %s\n"
			"  tag->ss: %s\n"
			"  tag->nod: %s\n",
			tag->id,
			tag->tag_no,
			tag->fi,
			tag->ss,
			tag->nod->full_name);
#endif

	} /* while ... */
	fclose(tagfile);
} /* process1 */

int send_ex(FILE *ex, const char *fmt, ...)
{
	va_list p;

#if DEBUG
	fprintf(stdout, ":");
	va_start(p, fmt);
	vfprintf(stdout, fmt, p);
	va_end(p);
#endif
	va_start(p, fmt);
	vfprintf(ex, fmt, p);
	va_end(p);
} /* send_ex */

void process2(node *n)
{
	assert(n);
#if DEBUG
	printf("process2: entering \"%s\"\n", n->full_name);
#endif
	switch (n->type) {
	case FLAG_ISDIR: {
		/* 1.- mkdir */
		{	int res;
#if DEBUG
			printf("process2:   mkdir(\"%s\");\n", n->full_name);
#endif
			res = mkdir(n->full_name, 0777);
			if (res < 0) {
				fprintf(stderr,
					"process2:error:MKDIR:%s:%s(errno=%d)\n",
					n->full_name, strerror(errno), errno);
				return; /* cannot continue */
			} /* if */
		} /* block */

		/* 2.- make index.html */
		
		n->html_file = new_node("index.html", n, FLAG_ISFILE);
#if DEBUG
		printf("process2:   "
			"html_create(\"%s\");\n",
			n->full_name);
#endif
		n->index_f = html_create(n);

		/* write on the parent html file a reference to us */
		if (n->parent) {
			fprintf(n->parent->html_file->index_f,
				"      <li><span class=\"dir\"><a href=\"%s\">%s</a> directory.</span></li>\n",
				rel_path(n->parent->html_file, n->html_file), n->name);
		} /* if */
				
		/* 3.- recurse to subdirectories */
		{	AVL_ITERATOR it;
			for (	it = avl_tree_first(n->subnodes);
				it;
				it = avl_iterator_next(it))
			{
				process2(avl_iterator_data(it));
			} /* for */
		} /* block */

		/* 4.- close files */
		html_close(n);
		n->index_f = NULL;

		/* 5.- and finish */
	} break;

	/* 4.- PROCESS FILE */
	case FLAG_ISFILE: {
		FILE *ex_fd;
		AVL_ITERATOR it;

		assert(n->parent);
		assert(n->parent->html_file);
		assert(n->parent->html_file->index_f);

		{	char buffer[128];
			snprintf(buffer, sizeof buffer, "%s.html", n->name);
			n->html_file = avl_tree_get(n->parent->subnodes, buffer);
			if (n->html_file) {
				fprintf(stderr, PROGNAME":error:name clash with file %s\n",
					n->html_file->full_name);
				exit(EXIT_FAILURE);
			} /* if */
			n->html_file = new_node(buffer, n->parent, FLAG_ISFILE);
			/* hidden, as it's not being put in his parent's subnodes */
		} /* block */

		fprintf(n->parent->html_file->index_f,
			"      <li><span class=\"file\">"
			"<a href=\"%s\">%s</a> file.</span>\n",
			n->html_file->name, n->name);

		ex_fd = popen(EX_PATH, "w");
		send_ex(ex_fd, "set notagstack\n");
		{	const char *s;
			s = strchr(n->full_name, '/');
			while (*s == '/') s++;
			send_ex(ex_fd, "e! %s\n", s); /* edit file */
		} /* block */

		/* for every tag in this file */
		if (avl_tree_size(n->subnodes)) {
			fprintf(n->parent->html_file->index_f, "        <ul>\n");
			for (	it = avl_tree_first(n->subnodes);
					it;
					it = avl_iterator_next(it))
			{
				const ctag *p;
				p = avl_iterator_data(it);
				if (p->tag_no > 1) {
					for (; p; p = p->next) {
						fprintf(n->parent->html_file->index_f,
							"          <li><span class=\"tag\"><a href=\"%s#%s-%d\">%s</a></span></li>\n",
							n->html_file->name, p->id, p->tag_no, p->id);
						send_ex(ex_fd, "ts %s\n%d\n", p->id, p->tag_no); /* tag select, se vim(1) help */
						send_ex(ex_fd, "s:^:(@a name=\"%s-%d\"/@):\n", p->id, p->tag_no); /* change */
					} /* for */
				} else {
					fprintf(n->parent->html_file->index_f,
						"          <li><span class=\"tag\"><a href=\"%s#%s-%d\">%s</a></span></li>\n",
						n->html_file->name, p->id, p->tag_no, p->id);
					send_ex(ex_fd, "ta %s\n", p->id); /* goto tag, only one tag in this file */
					send_ex(ex_fd, "s:^:(@a name=\"%s-%d\"/@):\n", p->id, p->tag_no); /* change */
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
"  -t tag_file. The tag file to be used (default: " DEFAULT_TAG_FILE ")\n"
"  -b base_dir.  Base directory for URL composition\n"
"       This causes to generate <BASE> tags. (default: " DEFAULT_BASE_DIR_STRING ")\n"
"  -d   Debug.\n"
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

	while ((opt = getopt(argc, argv, "t:hb:2drno:ps:j:")) != EOF) {
		switch(opt) {
		case 'h': do_usage(); exit(EXIT_SUCCESS);
		case 't': tag_file = optarg; break;
		case 'b': base_dir = optarg; break;
		case 'd': flags |= FLAG_VERBOSE; break;
		case 'n': flags |= FLAG_LINENUMBERS; break;
		case 'o': output = optarg; break;
		case 'p': flags |= FLAG_PROGRESS; break;
		case 's': style_file = optarg; break;
		case 'j': js_file = optarg; break;
		default:
			do_usage(); exit(EXIT_FAILURE);
		} /* switch */
	} /* while */

	db_init(output);
	style_node = new_node(style_file, db_root_node, FLAG_ISFILE);
	js_node = new_node(js_file, db_root_node, FLAG_ISFILE);

	/* Process files */

	process1(tag_file);
	process2(db_root_node);

} /* main */

/* $Id: c2html.c,v 0.24 2009/01/03 22:23:11 luis Exp $ */
