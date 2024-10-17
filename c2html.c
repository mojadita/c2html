/* $Id: c2html.c,v 0.25 2014/09/09 20:22:05 luis Exp $
 * Author: Luis Colorado <Luis.Colorado.Urcola@gmail.com>
 * Date: Thu Jun  3 19:30:16 MEST 1999
 * Copyright: (C) 1999--2022 Luis Colorado.  All rights reserved.
 * License: BSD
 */

#define IN_C2HTML_C

/* Standard include files */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#include <avl.h>

#include "configure.h"
#include "node.h"
#include "ctag.h"
#include "menu.h"
#include "debug.h"
#include "intern.h"
#include "html_output.h"
#include "lexical.h"

#include "c2html.h"

/* constants */

#define MAXLINELENGTH   4096

/* variables */

int flags              = DEFAULT_FLAGS;
const char *tag_file   = DEFAULT_TAG_FILE;
const char *output     = DEFAULT_OUTPUT;
const char *base_dir   = DEFAULT_BASE_DIR;

const char *style_file = DEFAULT_STYLE_FILE;
node *style_node       = NULL;

const char *js_file    = DEFAULT_JS_FILE;
node *js_node          = NULL;
node *db_root_node     = NULL;

static AVL_TREE files_db = NULL;

/* process file of name tags_filename (tags file) */
void
process1(const char *tags_filename)
{
    FILE *tagsfile;
    char line[MAXLINELENGTH];
    unsigned long line_num = 0;

    files_db = new_avl_tree(
            (AVL_FCOMP) strcmp,
            NULL, NULL,
            (AVL_FPRNT) fputs);

    tagsfile = fopen (tags_filename, "r");
    if (!tagsfile) {
        ERR(EXIT_FAILURE,
            "Error opening %s: %s\n",
            tags_filename, strerror(errno));
        /* NOTREACHED */
    } /* if */

    DEB(FLAG_DEBUG_PROCESS1,
        "Processing tags file '%s'\n",
        tags_filename);

    /* file open, process lines */
    while (fgets(line, sizeof line, tagsfile)) {
        const char *tag_id, *filename, *ss;
        const ctag *tag;

        line_num++;

        tag_id = strtok (line, "\t\n");
        if (!tag_id) { /* tag name */
            WRN("%s:%ld:bad syntax, unrecognized tag_id.\n",
                tags_filename, line_num);
            continue;
        } /* if */

        filename = strtok (NULL, "\t\n");
        if (!filename) { /* tag file */
            WRN("%s:%ld:bad syntax, unrecognized file name.\n",
                tags_filename, line_num);
            continue;
        } /* if */

        ss = strtok (NULL, "\n");
        if (!ss) { /* tag search string */
            WRN("%s:%ld:bad syntax, unrecognized search string.\n",
                tags_filename, line_num);
            continue;
        } /* if */

        /* Ignore VIM ctags(1) private symbols starting in ! */
        if (tag_id[0] == '!') {
            DEB(FLAG_DEBUG_PROCESS1,
                "%s:%ld: ignoring '%s' vim private identifier\n",
                tags_filename, line_num, tag_id);
            continue;
        } /* if */

        /* if we got here, we do have a correct tag entry */

        DEB(FLAG_DEBUG_PROCESS1,
            "%s:%ld: tag_id='%s', file='%s', search='%s'\n",
            tags_filename, line_num,
            tag_id, filename, ss);

        /* first, find the ctag entry. */
        tag = lookup_ctag(
                tag_id   = intern(tag_id),
                filename = intern(filename),
                ss       = intern(ss),
                db_root_node);

        tag->nod->orig_name = filename;

        avl_tree_put(files_db, tag->nod->full_name, tag->nod);

    } /* while ... */

    DEB(FLAG_DEBUG_PROCESS1,
        "closing tagsfile '%s'\n",
        tags_filename);
    fclose(tagsfile);

} /* process1 */

static void
send_ex(FILE *ex, const char *fmt, ...)
{
    va_list p;

    char command[4096];
    va_start(p, fmt);
    vsnprintf(command, sizeof command, fmt, p);
    va_end(p);

    char *s;
    for (s = strtok(command, "\n"); s; s = strtok(NULL, "\n")) {
        DEB(FLAG_DEBUG_EX,
            "sending command: [%s]\n", s);
        fputs(s, ex); fputc('\n', ex);
    }
} /* send_ex */

static void
write_entry_on_parent(node *entry,
        const char *label,
        const char *cl)
{
    node *parent = entry->parent;
    if (parent) {
        char *rp = rel_path(
            parent->html_file,
            entry->html_file);
        DEB(FLAG_DEBUG_PROCESS_DIR,
            "Writing an entry '%s' in parent "
            "html file '%s' for %s '%s'\n",
            rp, parent->html_file->full_name,
            label, entry->name);
        fprintf(parent->index_f,
            "      <li><span class=\"%s\">"
            "<a href=\"%s\">%s</a> %s."
            "</span></li>\n",
            cl,
            rp, entry->name, label);
    } /* if */
} /* write_entry_on_parent */

/* process a TYPE_DIR node (before processing files) */
static int
process_dir_pre(
        node *dir,
        void *clsr)
{

    DEB(FLAG_DEBUG_PROCESS_DIR,
        "Creating directory %s\n",
        dir->full_name);

    int res = mkdir(dir->full_name, 0777);

    if (res < 0 && (errno != EEXIST)) {
        ERR(EXIT_FAILURE, "error: MKDIR: %s: %s(errno=%d)\n",
            dir->full_name, strerror(errno), errno);
    } /* if */

    write_entry_on_parent(dir, "directory", "dir");

    DEB(FLAG_DEBUG_PROCESS_DIR,
        "creating html file [%s] "
        "for directory [%s]\n",
        dir->html_file->full_name,
        dir->full_name);

    FILE *h = html_create(dir);

    fprintf(h, "      <ul>\n");

    return 0;
} /* process_dir_pre */

/* process a TYPE_DIR node (after processing files) */
static int
process_dir_post(
        node *dir,
        void *clsr)
{
    FILE *h = dir->index_f;

    DEB(FLAG_DEBUG_PROCESS_DIR,
        "Cleaning and closing html file '%s'\n",
        dir->html_file->full_name);

    fprintf(h, "      </ul>\n");

    html_close(dir);

    return 0;
} /* process_dir_post */

/* process a TYPE_SOURCE file.
 * this does:
 * 1) running ex on source file to generate <a> tags in (@a@) form.
 * 2) scan the file generated to generate the HTML files to be produced.
 */
void process_source(node *f, void *clsr)
{
    DEB(FLAG_DEBUG_PROCESS_FILE,
        "launching an instance of "EX_PATH"\n");

    write_entry_on_parent(f, "file", "file");

    FILE *ex_fd = popen(EX_PATH, "w");
    /* send_ex(ex_fd, "set notagstack"); */
    send_ex(ex_fd, "e! %s", f->full_name); /* edit original file */

    DEB(FLAG_DEBUG_PROCESS_FILE,
        "begin "EX_PATH" editing session on file %s\n",
        f->full_name);

    /* for every tag in this file */
    DEB(FLAG_DEBUG_PROCESS_FILE,
        "for every tag in file [%s]\n",
        f->full_name);
    int ntags = avl_tree_size(f->subnodes);
    DEB(FLAG_DEBUG_PROCESS_FILE,
        "it has %d tags:\n", ntags);

    if (ntags) {
        AVL_ITERATOR it;

        DEB(FLAG_DEBUG_PROCESS_FILE,
            "begin list of tags: ");

        fprintf(f->parent->index_f,
                "        <ul>\n");

        const char *sep = "";
        for (   it = avl_tree_first(f->subnodes);
                it;
                it = avl_iterator_next(it))
        {
            const ctag *tag1;

            tag1 = avl_iterator_data(it);
            if (!tag1) {
                ERR(EXIT_FAILURE,
                    "cannot get a tag iterator data, "
                    "this shouldn't happen\n");
                /* NOTREACHED */
            }

            DEB_TAIL(FLAG_DEBUG_PROCESS_FILE,
                "%s[%s]", sep, tag1->id);
            sep = ", ";

            /* LCU: Mon Oct 14 15:12:43 EEST 2024
             * TODO menus have to be generated even if list has only
             * one tag. (for now this will be ok) */
            /* the first tag in the list contains the total number
             * of tag in this list */
            if (tag1->tag_no_in_file > 1) { /* several tags for
                                               this id */
                const ctag *tag2;
                const char *sep2 = ": ";
                for (tag2 = tag1; tag2; tag2 = tag2->next_in_file) {
                    DEB_TAIL(FLAG_DEBUG_PROCESS_FILE,
                        "%s%d",
                        sep2,
                        tag2->tag_no_in_file);
                    sep2 = ", ";
                    fprintf(f->parent->index_f,
                        "          <li><span class=\"tag\">"
                        "<a href=\"%s#%s-%d\">%s</a></span></li>\n",
                        f->html_file->name,
                        tag2->id,
                        tag2->tag_no_in_file,
                        tag2->id);
                    /* tag select, see vim(1) help for details */
                    send_ex(ex_fd,
                        "ts %s\n"
                        "%d",
                        tag2->id,
                        tag2->tag_no_in_file);
                    send_ex(ex_fd,
                        "s:^:(@a name=\"%s-%d\"@)(@/a@):",
                        tag2->id, tag2->tag_no_in_file); /* change */
                } /* for */
            } else { /* only one tag for this id. */
                fprintf(f->parent->index_f,
                    "            <li><span class=\"tag\">"
                    "<a href=\"%s#%s-%d\">%s</a></span></li>\n",
                    f->html_file->name,
                    tag1->id,
                    tag1->tag_no_in_file,
                    tag1->id);
                send_ex(ex_fd, "ta %s", tag1->id); /* goto tag, only one
                                                    * tag in this file */
                send_ex(ex_fd, "s:^:(@a name=\"%s-%d\"@)(@/a@):",
                    tag1->id, tag1->tag_no_in_file); /* change */
            } /* if */
        } /* for */
        DEB_TAIL(FLAG_DEBUG_PROCESS_FILE,
                ".\n"); /* end list of tags */
        fprintf(f->parent->index_f,
                "        </ul>\n");
    } /* if */
    DEB(FLAG_DEBUG_PROCESS_FILE,
        "terminate "EX_PATH" editing session and "
        "write on parent [%s]\n",
        f->parent->html_file->full_name);
    fprintf(f->parent->index_f, "      </li>\n");
    send_ex(ex_fd, "w! %s", f->full_name); /* write file */
    send_ex(ex_fd, "q!"); /* and terminate */
    pclose(ex_fd);

    DEB(FLAG_DEBUG_PROCESS_FILE,
        "scanning file %s -> %s\n",
        f->full_name, f->html_file->full_name);
    scanfile(f);

} /* process_source */

/* process a FILE_MENU node, this should generate all the tag lists
 * to allow browsing of tags. */
void
process_menu(node *f, void *clsr)
{
    write_entry_on_parent(f, "symbol", "sym");
    create_menu(f->menu);
} /* process_menu */

/* outputs progress to stderr. */
void
output_progress(node *f, void *clsr)
{
    static int   i          = 0;
    static char *progress[] = { "\\", "|", "/", "-" };
    static int   percent    = 0;
    static int   n_files    = 0;
    static long  acum;

    if (!n_files) {
        n_files = avl_tree_size(files_db) +
            avl_tree_size(db_menus);
    }

    if (!i++) acum = n_files >> 1;

    acum += 100000;
    if (acum >= n_files) {
        percent += acum / n_files;
        acum %= n_files;
    } /* if */

    fprintf(stderr, "\r%s (%d/%d -- %3d.%03d%%) %s \033[K",
        progress[i % NELEM(progress)], i, n_files, percent / 1000,
        percent % 1000, f->full_name);
    if (i >= n_files)
        fprintf(stderr, "\n");
} /* output_progress */

/* file processing routine.  Just calls the proper routine, outputs progress
 * and logs */
static int
process_file(
        node *f,
        void *clsr)
{

    DEB(FLAG_DEBUG_PROCESS_FILE,
        "START: file=%s\n",
        f->full_name);

    switch(f->type) {
    case TYPE_SOURCE: process_source(f, clsr); break;
    case TYPE_MENU:   process_menu(f, clsr);   break;
    case TYPE_DIR:    /* nothing */            break;
    case TYPE_HTML:   /* nothihg */            break;
    } /* switch (f->type) */

    if (flags & FLAG_PROGRESS) { /* print progress */
        output_progress(f, clsr);
    }

    DEB(FLAG_DEBUG_PROCESS_FILE,
        "end [%s]\n",
        f->full_name);

    return 0;
} /* process_file */

/* print help message */
void do_usage (int code)
{
    fprintf(stderr,
"Usage: " PROGNAME " [ options ... ]\n"
"\n"
"                     " PROGNAME " " VERSION "\n"
"     Copyright (C) 1999-2024 " "<luiscoloradourcola@gmail.com>\n"
"\n"
"This program is under GNU PUBLIC  LICENSE, version 2 or later see\n"
"the terms and conditions of use at http://www.gnu.org/ (you might\n"
"receive a copy of it with this program)\n"
"\n"
"This program operates on a  constructed tags file (see ctags(1)),\n"
"and constructs  an HTML  hierarchy of  source files,  parallel to\n"
"their C counterparts, with hyperlink  cross references to all the\n"
"C identifiers located in the code.\n"
"\n"
"It uses the tags file to locate all the identifier definitions in\n"
"the C  code, and then, it  constructs a syntax marked  HTML file,\n"
"with each definition  found in the tags file marked  in the code,\n"
"and  every reference  to it,  surrounded by  a <a  href> tag,  so\n"
"clicking with the  mouse leads us quickly and  efficiently to the\n"
"definition.\n"
"\n"
"Options:\n"
"  -b <base_dir>  Base directory for URL composition\n"
"       This causes to generate <BASE> tags. (default: "
"       " DEFAULT_BASE_DIR_STRING ")\n"
"  -d <debug_options>  Debug options can be several of:\n"
"         1  process1 debug.\n"
"         c  ctags debug.\n"
"         D  database debug.\n"
"         d  process dir debug.\n"
"         f  process file debug.\n"
"         i  process ident debug.\n"
"         I  string intern debug.\n"
"         l  lexical processing debug.\n"
"         M  create_menu debug.\n"
"         m  process menu debug.\n"
"         n  nodes debug.\n"
"         s  process scanfile debug.\n"
"         x  ex(1) commands debug.\n"
"  -h   Help.  This help screen.\n"
"  -j   Javascript file (default " DEFAULT_JS_FILE ")\n"
"  -n   Output linenumbers.\n"
"  -o   Output directory (default " DEFAULT_OUTPUT ")\n"
"  -p   Progress is shown on stderr.\n"
"  -s   Style file (default " DEFAULT_STYLE_FILE ")\n"
"  -t <tag_file>  The tag file to be used (default: '" DEFAULT_TAG_FILE "')\n"
        );
    exit(code);
} /* do_usage */

/* main program */
int main (int argc, char **argv)
{

    extern int optind;
    extern char *optarg;
    int opt;

    while ((opt = getopt(argc, argv, "b:d:hj:m:no:prs:Tt:")) != EOF) {
        switch(opt) {
        case 'b': base_dir = optarg;                     break;
        case 'd': { char *p; /* debug */
                for (p = optarg; *p; p++) {
                    switch(*p) {
                    case '1': flags |= FLAG_DEBUG_PROCESS1;      break;
                    case '2': flags |= FLAG_DEBUG_PROCESS2;      break;
                    case 'a': flags |= FLAG_DEBUG_ALL;           break;
                    case 'c': flags |= FLAG_DEBUG_CTAGS;         break;
                    case 'D': flags |= FLAG_DEBUG_DB;            break;
                    case 'd': flags |= FLAG_DEBUG_PROCESS_DIR;   break;
                    case 'f': flags |= FLAG_DEBUG_PROCESS_FILE;  break;
                    case 'I': flags |= FLAG_DEBUG_INTERN;        break;
                    case 'i': flags |= FLAG_DEBUG_PROCESS_IDENT; break;
                    case 'l': flags |= FLAG_DEBUG_LEX;           break;
                    case 'M': flags |= FLAG_DEBUG_CREATE_MENU;   break;
                    case 'm': flags |= FLAG_DEBUG_PROCESS_MENU;  break;
                    case 'n': flags |= FLAG_DEBUG_NODES;         break;
                    case 's': flags |= FLAG_DEBUG_SCANFILE;      break;
                    case 'x': flags |= FLAG_DEBUG_EX;            break;
                    } /* switch */
                } /* for */
            } /* block */
            break;
        case 'h': do_usage(EXIT_SUCCESS);                break;
        case 'j': js_file = optarg;                      break;
        case 'm': default_menu_name = optarg;            break;
        case 'n': flags |= FLAG_LINENUMBERS;             break;
        case 'o': output = optarg;                       break;
        case 'p': flags |= FLAG_PROGRESS;                break;
        case 's': style_file = optarg;                   break;
        case 't': tag_file = optarg;                     break;
        case 'T': flags |= FLAG_DONT_DELETE_TEMPORARIES; break;
        default: do_usage(EXIT_FAILURE);                 break;
        } /* switch */
    } /* while */

    db_root_node = new_node(output, NULL, TYPE_DIR);
    if (!db_root_node) {
        ERR(EXIT_FAILURE,
            "Cannot get a new node: %s\n",
            strerror(errno));
        /* NOTREACHED */
    }
    style_node = new_node(style_css, db_root_node, TYPE_HTML);
    js_node    = new_node(js_file,   db_root_node, TYPE_HTML);

    /* this process constructs the file node hierarchy of source file pages */
    process1(tag_file);

    //print_menus();

    D(do_recur(db_root_node,
        process_dir_pre,
        process_file,
        process_dir_post,
        NULL));

} /* main */

/* $Id: c2html.c,v 0.25 2014/09/09 20:22:05 luis Exp $ */
