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
#include <time.h>

#include <avl.h>

#include "db.h"
#include "c2html.h"
#include "intern.h"
#include "db.h"

/* constants */
#define MAXLINELENGTH	4096

char *rcsId = "\n$Id: c2html.c,v 0.24 2009/01/03 22:23:11 luis Exp $\n";

/* types */

/* prototypes */

/* variables */

const char *base_dir = NULL;
int flags = 0;
const char *tag_file = "tags";

static FILE *ex_process = NULL;

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

	printf("Processing %s...\n", fn);

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
#if 0
		printf(
			"TAG:\n"
			"  tag->id: %s\n"
			"  tag->tag_no: %d\n"
			"  tag->fi: %s\n"
			"  tag->ss: %s\n  ",
			tag->id,
			tag->tag_no,
			tag->fi,
			tag->ss);
		{	int i;
			for (i = 0; tag->path[i]; i++)
				printf("[%s|%p]", tag->path[i], tag->path[i]);
			printf("\n");
		}
#endif

	} /* while ... */
	fclose(tagfile);
} /* process1 */

void process2(node *n)
{
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
		{	char *p;
			static char buffer[4096];
			snprintf(buffer, sizeof buffer, "%s/index.html", n->full_name);
#if DEBUG
			printf("process2:   "
				"html_create(\"%s\", \"Directory %%s\", \"%s\");\n",
				buffer, n->full_name);
#endif
			n->index_f = html_create(buffer, n);
		} /* block */
		if (n->parent) {
			fprintf(n->parent->index_f,
				"      <li><div class=\"dir\"><a href=\"%s/index.html\">%s</a> directory.</div></li>\n",
				n->name, n->name);
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
		{	html_close(n->index_f);
			n->index_f = NULL;
		} /* block */

		/* 5.- and finish */
		} break;

	/* 4.- PROCESS FILE */
	case FLAG_ISFILE: {
		if (n->parent) {
			const ctag *p;
			AVL_ITERATOR it;
			fprintf(n->parent->index_f,
				"      <li><div class=\"file\"><a href=\"%s.html\">%s</a> file.</div>\n",
				n->name, n->name);
			if (avl_tree_size(n->subnodes)) {
				fprintf(n->parent->index_f, "        <ul>\n");
				for (	it = avl_tree_first(n->subnodes);
						it;
						it = avl_iterator_next(it))
				{
					for (	p = avl_iterator_data(it);
							p; 
							p = p->next_in_nod)
					{
						fprintf(n->parent->index_f,
							"          <li><div class=\"tag\"><a href=\"%s.html#%s-%d\">%s</a></div></li>\n",
							n->name,p->id,p->tag_no,p->id);
					} /* for */
				} /* for */
				fprintf(n->parent->index_f, "        </ul>\n");
			} /* if */
			fprintf(n->parent->index_f, "       </li>\n");
		} /* if */
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
	fprintf(stderr, "Usage: "PROGNAME" [ options ... ] tagfile1 ...\n");
	fprintf(stderr, PROGNAME" "VERSION": Copyright (C) 1999 <Luis.Colorado@SLUG.HispaLinux.ES>\n");
	fprintf(stderr, "This program is under GNU PUBLIC LICENSE, version 2 or later\n");
	fprintf(stderr, "see the terms and conditions of use at http://www.gnu.org/\n");
	fprintf(stderr, "(you might receive a copy of it with this program)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "This program operates on a constructed tags file (see ctags(1)), and\n");
	fprintf(stderr, "constructs an HTML hierarchy of source files, parallel to their C\n");
	fprintf(stderr, "counterparts, with hyperlink cross references to all the C identifiers\n");
	fprintf(stderr, "located in the code.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "It uses the tags file to locate all the identifier definitions in the\n");
	fprintf(stderr, "C code, and then, it constructs a syntax marked HTML file, with each\n");
	fprintf(stderr, "definition found in the tags file marked in the code, and every reference\n");
	fprintf(stderr, "to it, surrounded by a <a href> tag, so clicking with the mouse leads us\n");
	fprintf(stderr, "quickly and efficiently to the definition.\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -t tag_file. The tag file to be used\n");
	fprintf(stderr, "  -h   Help.  This help screen.\n");
	fprintf(stderr, "  -b base_dir.  Base directory for URL composition\n");
	fprintf(stderr, "       This causes to generate <BASE> tags.  Useful if more than one directory\n");
	fprintf(stderr, "       and no relative pathnames\n");
	fprintf(stderr, "  -d   Debug.  Print file and file number order, instead of just the\n");
	fprintf(stderr, "       percentage of the default case\n");
} /* do_usage */

/* main program */
int main (int argc, char **argv)
{

	extern int optind;
	extern char *optarg;
	int opt;

	while ((opt = getopt(argc, argv, "t:hb:2drn")) != EOF) {
		switch(opt) {
		case 't': tag_file = optarg; break;
		case 'h': do_usage(); exit(EXIT_SUCCESS);
		case 'b': base_dir = optarg; break;
		case 'd': flags |= FLAG_VERBOSE; break;
		case 'n': flags |= FLAG_LINENUMBERS; break;
		default:
			do_usage(); exit(EXIT_FAILURE);
		} /* switch */
	} /* while */

	/* Process files */
	db_init();
	ex_process = popen(EX_PATH, "w");
	if (!ex_process) {
		fprintf(stderr,
			PROGNAME":ex_process:%s(errno=%d)\n",
			strerror(errno), errno);
		exit(EXIT_FAILURE);
	} /* if */

	process1(tag_file);
	process2(db_root_node);
	pclose(ex_process);

#if 0
	{	AVL_ITERATOR i;
		const ctag *old_tag = NULL;
		const node *old_file = NULL;
		char buffer[MAXLINELENGTH];
		

		for (i = avl_tree_first(db_ctag); i; i = avl_iterator_next(i)) {
			const ctag *new_tag = avl_iterator_data(i);
			const node *new_file = new_tag->nod;
			if (old_file != new_file) { /* change in file */
				char *p = buffer;
				size_t sz = sizeof buffer;
				if (old_file) {
					p = get_name(old_file, &p, &sz);
					printf("closing [%s].\n", p);
					printf("      EX>>> w %s.temp\n", p);
				} /* if */
				p = buffer; sz = sizeof buffer;
				p = get_name(new_file, &p, &sz);
				printf("opening [%s]:\n", p);
				printf("      EX>>> e %s\n", p);
			} /* if */
			/* TODO: Here goes the stuff */
			printf("    tag [%s/%d]\n", new_tag->id, new_tag->tag_no);
			printf("      EX>>> %dta %s\n", new_tag->tag_no, new_tag->id);
			printf("      EX>>> s:^:(@a name=\"%s-%d\"/@):\n", new_tag->id, new_tag->tag_no);
			old_tag = new_tag; old_file = new_file;
		} /* for */
		if (old_tag) {
			char *p = buffer;
			size_t sz = sizeof buffer;
			p = get_name(old_tag->nod, &p, &sz);
			printf("closing [%s].\n", p);
			printf("      EX>>> w %s.temp\n", p);
		} /* if */
	} /* block */
#endif

#if 0
	/* print the results ---do the heavy work--- */
	{	FileNode *f; CtagNode *c;
		FILE *toc, *idx;
		int i,acum, percent, llen;
		char *dir, *ldir;

		/* sort the filelist */
		files_array = calloc(files_n, sizeof files_array[0]);
		for (f = files_first, i=0; f; f = f->files_next, i++)
			files_array[i] = f;
		qsort(files_array, files_n, sizeof files_array[0],
			(int (*)(const void *, const void *))files_cmp);

		/* open the directory files */
		if (flags & FLAG_TWOLEVEL) {
			toc = html_create("index.html", "Directories");
			idx = NULL;
			fprintf(toc, "    <UL>\n");
		} else {
			toc = NULL;
			idx = html_create("index.html", "Files");
			fprintf(idx, "    <UL>\n");
		}

		acum = files_n >> 1; percent = 0;
		/* process each file in the database, in order */
		for (i = 0; i < files_n; i++) {
			char buffer [EXCMD_BUFSIZE];
			FILE *ex;

			f = files_array[i];

			if (flags & FLAG_TWOLEVEL) {
				if (!idx || strncmp(f->name, ldir, llen)) {
					char *p;
					if (idx) {
						fprintf(idx, "    </ul>\n");
						html_close (idx);
					}
					sprintf (buffer, PFX2"%05d"EXT2, i);
					ldir = f->name; p = strrchr(ldir, '/');
					llen = p ? p - ldir : 0;
					idx = html_create(buffer, "Directory %0.*s", 
						llen ? llen : 1, llen ? ldir : ".");
					fprintf(toc,
						"    <LI><A HREF=\"%s\">%0.*s</a>\n",
						buffer, llen ? llen : 1, llen ? ldir : ".");
				}
			}

			fprintf(idx,
				"    <H2>File <A HREF=\"%s"EXT2"\">%s</a>:</h2>\n",
				f->name, f->name);

			/* Print the progress, as this is a lengthy process */
			{	static char *progress[] = { "\\", "|", "/", "-" };

				acum += 100000;
				if (acum >= files_n) {
					percent += acum / files_n;
					acum %= files_n;
				}
				fprintf(stderr, "%s (%d/%d -- %2d.%03d%%) %s\033[K\r",
					progress[i&3], i+1, files_n, percent / 1000, percent % 1000, f->name);
				fflush(stderr);
			}

			/* edit the file */
			ex = popen (EX_PATH, "w");
			if (!ex) {
				fprintf (stderr,
					PROGNAME": "__FILE__"(%d): popen(\"%s\"): %s\n",
					__LINE__, EX_PATH, strerror(errno));
				exit(EXIT_FAILURE);
			}

			/* Now, process the list of symbols related to this file */
			fprintf (idx, "    <UL>\n");
			for (c = f->ctags_first; c; c = c->ctags_next) {
				assert(c->file == f->name);
				fprintf (idx,
					"      <LI><A HREF=\"%s"EXT2"#%s_%d\">%s(%s/%d)</a>\n",
					c->file, c->sym, c->tag_num, c->sym, c->file, c->tag_num);
				/* switch to the tag, using ex(1) */
				fprintf (ex,
					"%dta %s\n",
					c->tag_num, c->sym);
				/* insert a mark at the beggining of the line.
				 * The mark format is
				 * (@A NAME="<symbol name>"@)(@/a@)
				 * The mark format must not be changed, or the procedure
				 * to analyse it again to generate the tags won't work */
				fprintf (ex,
					"s:^:(@A NAME=\"%s_%d\"@)(@/a@):\n",
					c->sym, c->tag_num);
			} /* foreach tag in this file */
			fprintf(idx, "    </ul>\n");
			/* write and exit ex(1) */
			fprintf(ex,
				"w %s"EXT1"\n"
				"q\n", f->name);
			pclose (ex);

			/* convert the temporary file into the html one */
			{	char *p = buffer;  /* we use buffer again */

				p += sprintf (p, "%s"EXT1, f->name); p++;
				sprintf (p, "%s"EXT2, f->name);
				scanfile (f->name, buffer, p);
				/* erase temp file after work is done */
				unlink (buffer);
			} /* conversion */
		}
		/* finish the index. */
		if (idx) html_close(idx);
		if (flags & FLAG_TWOLEVEL) {
			fprintf(toc, "    </ul>\n");
			html_close(toc);
		}
	} /* output phase */
#endif
} /* main */

/* $Id: c2html.c,v 0.24 2009/01/03 22:23:11 luis Exp $ */
