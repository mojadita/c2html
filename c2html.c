/* $Id: c2html.c,v 0.21 2005/02/22 19:23:36 luis Exp $
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
#include <time.h>
#include <multifree.h>
#include <hashTable.h>
#include "c2html.h"

/* constants */
#define MAXLINELENGTH	2048

char *rcsId = "\n$Id: c2html.c,v 0.21 2005/02/22 19:23:36 luis Exp $\n";

/* types */

/* prototypes */

/* variables */
HashTable syms_table, files_table;
FileNode *files_first = NULL, *files_last = NULL;
int files_n = 0;
FileNode **files_array;
char *base_dir = NULL;
int flags = 0;

/* functions */
static int files_cmp (FileNode **f1, FileNode **f2)
{
	return strcmp(f1[0]->name, f2[0]->name);
} /* files_cmp */

/* process file of name fn */
void process(char *fn)
{
	FILE *tagfile;
	char line [MAXLINELENGTH];
	
	if (!fn) {
		fn = "<stdin>";
		tagfile = stdin;
	} else {
		tagfile = fopen (fn, "r");
		if (!tagfile) {
			fprintf (stderr,
				PROGNAME": Error opening %s: %s\n",
				fn, sys_errlist[errno]);
			exit (EXIT_FAILURE);
		}
	}

	/* file open, process lines */
	while (fgets(line, sizeof line, tagfile)) {
		char *id, *fi;
		HashEntry *che; CtagNode *ce;
		HashEntry *fhe; FileNode *fe;

		id = strtok (line, " \t\n"); if (!id) continue;
		fi = strtok (NULL, " \t\n"); if (!fi) continue;
		/* We don't use the third field. */

		/* Ignore VIM ctags(1) private symbols */
		if (!isalnum(id[0])) continue;

		/* first, find the ctag entry */
		che = hashTableLookup (&syms_table, id);
		if (!che) {
			/* hashTableLookup has not been able to get an entry for this
			 * symbol, so give up */
			fprintf (stderr,
				PROGNAME": "__FILE__"(%d): hashTableLookup: %s\n",
				__LINE__, sys_errlist[errno]);
			exit(EXIT_FAILURE);
		} /* if (!che) */

		/* symbol found, let's find if already in database */
		{	CtagNode *auxce;
			for (auxce = (CtagNode *)(che->data);
				auxce;
				auxce = auxce->next)
			{
				if (!strcmp(id,auxce->sym) && !strcmp(fi, auxce->file))
					break;
			} /* for */
			if (auxce) { /* found */
				fprintf(stderr,
					PROGNAME": "__FILE__"(%d): Symbol %s(%s) already "
					"in database\n", __LINE__, id, fi);
				continue;
			}
		} /* check for found */

		/* ...add it */
		ce = malloc (sizeof *ce);
		if (!ce) {
			fprintf (stderr,
				PROGNAME": "__FILE__"(%d): malloc: %s\n",
				__LINE__, sys_errlist[errno]);
			exit(EXIT_FAILURE);
		}
		ce->sym = che->key;
		ce->flags = 0;
		/* defer ce->file initialization until appropiate (see below) */
		ce->ctfile = fn;  /* not used, but don't hurts */
		ce->ctags_next = NULL;
		/* insert in the ctags list for this sym */
		ce->next = che->data;
		che->data = ce;

		/* next, find the file */
		fhe = hashTableLookup (&files_table, fi);
		if (!fhe) {
			fprintf (stderr,
				PROGNAME": "__FILE__"(%d): hashTablePermLookup: %s\n",
				__LINE__, sys_errlist[errno]);
			exit(EXIT_FAILURE);
		}
		ce->file = fhe->key; /* deferred: see above */
		fe = fhe->data;
		if (!fe) {  /* File is new */
			fhe->data = fe = malloc (sizeof *fe);
			if (!fe) {
				fprintf (stderr,
					PROGNAME": "__FILE__"(%d): malloc: %s\n",
					__LINE__, sys_errlist[errno]);
				exit(EXIT_FAILURE);
			}
			fe->name = fhe->key;
			fe->files_next = NULL;
			fe->ctags_first = fe->ctags_last = NULL;

			/* insert file in file list */
			if (!files_first)
				files_first = fe;
			if (files_last)
				files_last->files_next = fe;
			files_last = fe;
			files_n++;
		} /* new file */

		/* insert ctag in file ctag list */
		if (!fe->ctags_first)
			fe->ctags_first = ce;
		if (fe->ctags_last)
			fe->ctags_last->ctags_next = ce;
		fe->ctags_last = ce;
	} /* while ... */
} /* process */

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
	fprintf(stderr, "  -h   Help.  This help screen.\n");
	fprintf(stderr, "  -b base_dir.  Base directory for URL composition\n");
	fprintf(stderr, "       This causes to generate <BASE> tags.  Useful if more than one directory\n");
	fprintf(stderr, "       and no relative pathnames\n");
	fprintf(stderr, "  -2   Two level.  Generate Index in a two level structure, showing\n");
	fprintf(stderr, "       directories at the upper level, and files and symbols at the lower.\n");
	fprintf(stderr, "  -d   Debug.  Print file and file number order, instead of just the\n");
	fprintf(stderr, "       percentage of the default case\n");
	fprintf(stderr, "  -r   Relative pathnames.  Generate relative pathnames in the links between\n");
	fprintf(stderr, "       symbols and their definitions\n");
} /* do_usage */

/* main program */
int main (int argc, char **argv)
{

	extern int optind;
	extern char *optarg;
	int opt;

	while ((opt = getopt(argc, argv, "hb:2dr")) != EOF) {
		switch(opt) {
		case 'h': do_usage(); exit(EXIT_SUCCESS);
		case 'b': base_dir = optarg; break;
		case '2': flags |= FLAG_TWOLEVEL; break;
		case 'd': flags |= FLAG_VERBOSE; break;
		case 'r': flags |= FLAG_RELFILENAME; break;
		default:
			do_usage(); exit(EXIT_FAILURE);
		}
	}
	argc -= optind; argv += optind; /* shift all the options */

	/* Initialize hash tables */
	hashTableInit(&syms_table);
	hashTableInit(&files_table);

	/* Process files */
	if (argc) while (argc) {
		process(argv[0]);
		argc--; argv++; /* shift */
	} else {
		process(DEFAULT_TAGS); /* process tags by default */
	}

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

				acum += 100;
				if (acum >= files_n) {
					percent += acum / files_n;
					acum %= files_n;
				}
				if (flags & FLAG_VERBOSE)
					fprintf(stderr, "%s (%d of %d -- %d%%)\n",
						f->name, i+1, files_n, percent);
				else
					fprintf(stderr, "%s %02d%%\r", progress[i&3], percent);
				fflush(stderr);
			}

			/* edit the file */
			sprintf (buffer, EX_PATH" %s", f->name);
			ex = popen (buffer, "w");
			if (!ex) {
				fprintf (stderr,
					PROGNAME": "__FILE__"(%d): popen(\"%s\"): %s\n",
					__LINE__, buffer, sys_errlist[errno]);
				exit(EXIT_FAILURE);
			}

			/* Now, process the list of symbols related to this file */
			fprintf (idx, "    <UL>\n");
			for (c = f->ctags_first; c; c = c->ctags_next) {
				fprintf (idx,
					"      <LI><A HREF=\"%s"EXT2"#%s\">%s</a>\n",
					c->file, c->sym, c->sym);
				/* switch to the tag, using ex(1) */
				fprintf (ex,
					"ta %s\n",
					c->sym);
				/* insert a mark at the beggining of the line.
				 * The mark format is
				 * (@A NAME="<symbol name>"@)(@/a@)
				 * The mark format must not be changed, or the procedure
				 * to analyse it again to generate the tags won't work */
				fprintf (ex,
					"s:^:(@A NAME=\"%s\"@)(@/a@):\n",
					c->sym);
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
} /* main */

/* $Id: c2html.c,v 0.21 2005/02/22 19:23:36 luis Exp $ */
