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

#include "debug.h"
#include "intern.h"
#include "node.h"
#include "menu.h"
#include "ctag.h"
#include "c2html.h"
#include "html_output.h"

/* constants */


#define MAXLINELENGTH	4096

char *rcsId = "\n$Id: c2html.c,v 0.24 2009/01/03 22:23:11 luis Exp $\n";

/* variables */

int flags = 0;
const char *tag_file = DEFAULT_TAG_FILE;
const char *output = DEFAULT_OUTPUT;
const char *base_dir = DEFAULT_BASE_DIR;
const char *style_file = DEFAULT_STYLE_FILE;
node *style_node = NULL;
const char *js_file = DEFAULT_JS_FILE;
node *js_node = NULL;
node *db_root_node = NULL;

static AVL_TREE files_db = NULL;

/* functions */

/* process file of name fn */
void process1(const char *fn)
{
	FILE *tagfile;
	char line [MAXLINELENGTH];
	unsigned long long line_num = 0;

	files_db = new_avl_tree(
		(AVL_FCOMP) strcmp,
		NULL, NULL,
		(AVL_FPRNT) fputs);
	
	tagfile = fopen (fn, "r");
	if (!tagfile) {
		fprintf (stderr,
			PR("Error opening %s: %s\n"),
			fn, strerror(errno));
		exit (EXIT_FAILURE);
	} /* if */

	DEB((PR("Processing %s...\n"), fn));

	/* file open, process lines */
	while (fgets(line, sizeof line, tagfile)) {
		const char *id, *fi, *st;
		const ctag *tag;
		
		line_num++;

		DEB((PR("line[%lld]: %s\n"), line_num, line));

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
		D(tag = lookup_ctag(id, fi, st, db_root_node));

		avl_tree_put(files_db, tag->nod->full_name, tag->nod);

	} /* while ... */

	DEB((PR("closing tagfile [%s]\n"), fn));
	fclose(tagfile);
} /* process1 */

int send_ex(FILE *ex, const char *fmt, ...)
{
	va_list p;

#if DEBUG
	DEB((PR("EX:")));
	va_start(p, fmt);
	vprintf(fmt, p);
	va_end(p);
#endif

	va_start(p, fmt);
	vfprintf(ex, fmt, p);
	va_end(p);
} /* send_ex */

int process_dir_pre(const node *d, void *arg_not_used)
{
	FILE *h;

	DEB((PR("Creating directory %s\n"), d->full_name));
	int res = mkdir(d->full_name, 0777);
	if (res < 0) {
		fprintf(stderr,
			PR("error:MKDIR:%s:%s(errno=%d)\n"),
			d->full_name, strerror(errno), errno);
		return -1;
	} /* if */
	DEB((PR("creating html file [%s] for directory [%s]\n"),
		d->html_file->full_name, d->full_name));
	h = html_create(d);
	fprintf(h, "      <ul>\n");
	if (d->parent) {
		char *rp = rel_path(
			d->parent->html_file,
			d->html_file);
		DEB((PR("Writing an entry [%s] in parent html file [%s] for [%s]\n"),
			rp, d->parent->html_file->full_name, d->html_file->full_name));
		fprintf(d->parent->html_file->index_f,
			"      <li><span class=\"dir\">"
			"<a href=\"%s\">%s</a> directory."
			"</span></li>\n",
			rp, d->name);
	} /* if */
	DEB((PR("end\n")));
	return 0;
} /* process_dir_pre */

int process_dir_post(const node *d, void *arg_not_used)
{
	FILE *h = d->html_file->index_f;

	DEB((PR("Cleaning and closing %s\n"),
		d->html_file->full_name));
	fprintf(h, "      </ul>\n");
	html_close(d);
	DEB((PR("end\n")));

	return 0;
} /* process_dir_post */

/* ARGSUSED */
int process_file(const node *f, void *not_used)
{
	FILE *ex_fd;
	int ntags;

	DEB((PR("begin [%s]\n"), f->html_file->full_name));
	DEB((PR("writing an entry for %s in parent html file %s\n"),
		f->html_file->name, f->parent->html_file->name));

	fprintf(f->parent->html_file->index_f,
		"      <li><span class=\"file\">"
		"<a href=\"%s\">%s</a> file.</span>\n",
		f->html_file->name, f->name);

	DEB((PR("launching an instance of "EX_PATH"\n")));

	ex_fd = popen(EX_PATH, "w");
	send_ex(ex_fd, "set notagstack\n");
	{	const char *s;
		s = strchr(f->full_name, '/');
		if (!s) s = f->full_name;
		while (*s == '/') s++;
		send_ex(ex_fd, "e! %s\n", s); /* edit original file */

		DEB((PR("begin editing session on file %s -> %s\n"),
			s, f->full_name));
	} /* block */

	/* for every tag in this file */
	DEB((PR("for every tag in this file:\n")));
	DEB((PR("FILE name=[%s]\n"), f->full_name));
	D(ntags = avl_tree_size(f->subnodes));
	DEB((PR("it has %d tags:\n"), ntags));
	if (ntags) {
		AVL_ITERATOR it1, it2;

#if DEBUG
		printf(PR("begin list of tags: "));
#endif

		fprintf(f->parent->html_file->index_f,
			"        <ul>\n");
		for (	it2 = it1 = avl_tree_first(f->subnodes);
				it1;
				it1 = avl_iterator_next(it1))
		{
			const ctag *tag1;

			assert(tag1 = avl_iterator_data(it1));

#if DEBUG
			printf("%s[%s]", it1 == it2 ? "" : "; ", tag1->id);
#endif
			/* the first tag in the list contains the total number of tag in this list */
			if (tag1->tag_no_in_file > 1) { /* several tags for this id */
				const ctag *tag2;
				for (tag2 = tag1; tag2; tag2 = tag2->next_in_file) {
#if DEBUG
					printf("%s%d", tag2 == tag1 ? ": " : ", ", tag2->tag_no_in_file);
#endif
					fprintf(f->parent->html_file->index_f,
						"          <li><span class=\"tag\">"
						"<a href=\"%s#%s-%d\">%s</a></span></li>\n",
						f->html_file->name,
						tag2->id, tag2->tag_no_in_file,
						tag2->id);
					/* tag select, see vim(1) help for details */
					send_ex(ex_fd,
						"ts %s\n"
						"%d\n",
						tag2->id,
						tag2->tag_no_in_file);
					send_ex(ex_fd,
						"s:^:(@a name=\"%s-%d\"@)(@/a@):\n",
						tag2->id, tag2->tag_no_in_file); /* change */
				} /* for */
			} else {
				fprintf(f->parent->html_file->index_f,
					"            <li><span class=\"tag\">"
					"<a href=\"%s#%s-%d\">%s</a></span></li>\n",
					f->html_file->name, tag1->id, tag1->tag_no_in_file, tag1->id);
				send_ex(ex_fd, "ta %s\n", tag1->id); /* goto tag, only one tag in this file */
				send_ex(ex_fd, "s:^:(@a name=\"%s-%d\"@)(@/a@):\n",
					tag1->id, tag1->tag_no_in_file); /* change */
			} /* if */
		} /* for */
#if DEBUG
		printf(".\n"); /* end list of tags */
#endif
		fprintf(f->parent->html_file->index_f,
			"        </ul>\n");
	} /* if */
	DEB((PR("terminate "EX_PATH" editing session and write on parent [%s]\n"),
		f->parent->html_file->full_name));
	fprintf(f->parent->html_file->index_f, "      </li>\n");
	send_ex(ex_fd, "w! %s\n", f->full_name); /* write file */
	send_ex(ex_fd, "q!\n"); /* and terminate */

	if (flags & FLAG_PROGRESS) {	/* print progress */
		static int i=0;
		static char *progress[] = { "\\", "|", "/", "-" };
		static long acum; 
		static int percent = 0;
		static int n_files = 0;

		if (!n_files)
			n_files = avl_tree_size(files_db);

		if (!i++) acum = n_files >> 1;

		acum += 100000;
		if (acum >= n_files) {
			percent += acum / n_files;
			acum %= n_files;
		} /* if */
		fprintf(stderr, "\r%s (%d/%d -- %3d.%03d%%) %s\033[K\r",
			progress[i&3], i, n_files, percent / 1000,
			percent % 1000, f->full_name);
		if (i >= n_files) fprintf(stderr, "\n");
	} /* block */

	pclose(ex_fd);
	DEB((PR("scanning file %s -> %s\n"),
		f->full_name, f->html_file->full_name));
	scanfile(f);
	DEB((PR("end [%s]\n"),
		f->full_name));

	return 0;
} /* process_file */

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

	while ((opt = getopt(argc, argv, "b:d:hj:m:no:prs:t:")) != EOF) {
		switch(opt) {
		case 'b': base_dir = optarg; break;
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
		case 'h': do_usage(); exit(EXIT_SUCCESS);
		case 'j': js_file = optarg; break;
		case 'm': default_menu_name = optarg; break;
		case 'n': flags |= FLAG_LINENUMBERS; break;
		case 'o': output = optarg; break;
		case 'p': flags |= FLAG_PROGRESS; break;
		case 's': style_file = optarg; break;
		case 't': tag_file = optarg; break;
		default:
			do_usage(); exit(EXIT_FAILURE);
		} /* switch */
	} /* while */

	assert(db_root_node = new_node(output, NULL, TYPE_DIR));
	style_node = new_node(style_file, db_root_node, TYPE_HTML);
	js_node = new_node(js_file, db_root_node, TYPE_HTML);

	/* Process files */

	process1(tag_file);

	D(fflush(stdout));

	D(do_recur(db_root_node,
		process_dir_pre, NULL,
		process_file, NULL,
		process_dir_post, NULL));
		
	//process2(db_root_node);

} /* main */

/* $Id: c2html.c,v 0.24 2009/01/03 22:23:11 luis Exp $ */
