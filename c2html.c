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

#include "debug.h"
#include "intern.h"
#include "node.h"
#include "menu.h"
#include "ctag.h"
#include "c2html.h"
#include "html_output.h"
#include "lexical.h"

/* constants */

#define MAXLINELENGTH   4096

/* variables */

int flags              = FLAG_DEBUG_ALWAYS;
const char *tag_file   = DEFAULT_TAG_FILE;
const char *output     = DEFAULT_OUTPUT;
const char *base_dir   = DEFAULT_BASE_DIR;
const char *style_file = DEFAULT_STYLE_FILE;
node *style_node       = NULL;
const char *js_file    = DEFAULT_JS_FILE;
node *js_node          = NULL;
node *db_root_node     = NULL;

static AVL_TREE files_db = NULL;

/* functions */

/* process file of name fn */
void process1(const char *fn)
{
    FILE *tagfile;
    char line [MAXLINELENGTH];
    unsigned long line_num = 0;

    files_db = new_avl_tree(
            (AVL_FCOMP) strcmp, NULL, NULL,
            (AVL_FPRNT) fputs);

    tagfile = fopen (fn, "r");
    if (!tagfile) {
        ERR(EXIT_FAILURE,
            "Error opening %s: %s\n",
            fn, strerror(errno));
        /* NOTREACHED */
    } /* if */

    DEB(FLAG_DEBUG_PROCESS1,
            "Processing tags file \"%s\":\n", fn);

    /* file open, process lines */
    while (fgets(line, sizeof line, tagfile)) {
        const char *id, *fi, *ss;
        const ctag *tag;

        line_num++;

        id = strtok (line, "\t\n");
        if (!id) { /* tag name */
            WRN("%s:%ld:bad syntax, unrecognized id.\n",
                fn, line_num);
            continue;
        } /* if */

        fi = strtok (NULL, "\t\n");
        if (!fi) { /* tag file */
            WRN("%s:%ld:bad syntax, unrecognized file name.\n",
                fn, line_num);
            continue;
        } /* if */

        ss = strtok (NULL, "\n");
        if (!ss) { /* tag search string */
            WRN("%s:%ld:bad syntax, unrecognized search string.\n",
                fn, line_num);
            continue;
        } /* if */

        /* Ignore VIM ctags(1) private symbols starting in ! */
        if (id[0] == '!') {
            DEB(FLAG_DEBUG_PROCESS1,
                "%s:%ld:ignoring \"%s\" vim private identifier\n",
                fn, line_num, id);
            continue;
        } /* if */

        DEB(FLAG_DEBUG_PROCESS1,
                "%s:%ld:id=[%s],fi=[%s],ss=[%s]\n",
                fn, line_num,
                id, fi, ss);

        /* first, find the ctag entry, this interns the three strings. */
        D(tag = lookup_ctag(id, fi, ss, db_root_node));

        avl_tree_put(files_db, tag->nod->full_name, tag->nod);

    } /* while ... */

    DEB(FLAG_DEBUG_PROCESS1,
        "closing tagfile [%s]\n",
        fn);
    fclose(tagfile);

} /* process1 */

int send_ex(FILE *ex, const char *fmt, ...)
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

int process_dir_pre(const node *d)
{
    FILE *h;

    DEB(FLAG_DEBUG_PROCESS_DIR,
            "Creating directory %s\n",
            d->full_name);

    int res = mkdir(d->full_name, 0777);
    if (res < 0 && (errno != EEXIST)) {
        ERR(EXIT_FAILURE,
            "error:MKDIR:%s:%s(errno=%d)\n",
            d->full_name, strerror(errno), errno);
    } /* if */

    DEB(FLAG_DEBUG_PROCESS_DIR,
            "creating html file [%s] for directory [%s]\n",
            d->html_file->full_name, d->full_name);
    h = html_create(d);
    fprintf(h, "      <ul>\n");
    if (d->parent) {
        char *rp = rel_path(
            d->parent->html_file,
            d->html_file);
        DEB(FLAG_DEBUG_PROCESS_DIR,
                "Writing an entry [%s] in parent "
                "html file [%s] for dir [%s]\n",
            rp, d->parent->html_file->full_name,
            d->html_file->full_name);
        fprintf(d->parent->html_file->index_f,
            "      <li><span class=\"dir\">"
            "<a href=\"%s\">%s</a> directory."
            "</span></li>\n",
            rp, d->name);
    } /* if */
    DEB(FLAG_DEBUG_PROCESS_DIR, "end\n");
    return 0;
} /* process_dir_pre */

int process_dir_post(const node *d)
{
    FILE *h = d->html_file->index_f;

    DEB(FLAG_DEBUG_PROCESS_DIR,
            "Cleaning and closing html file [%s]\n",
            d->html_file->full_name);
    fprintf(h, "      </ul>\n");
    html_close(d);
    DEB(FLAG_DEBUG_PROCESS_DIR, "end\n");

    return 0;
} /* process_dir_post */

int process_file(const node *f)
{
    FILE *ex_fd;
    int ntags;

    DEB(FLAG_DEBUG_PROCESS_FILE,
            "START: file=[%s]\n", f->html_file->full_name);

    DEB(FLAG_DEBUG_PROCESS_FILE,
            "writing an entry for %s in parent html file %s\n",
        f->html_file->name, f->parent->html_file->name);

    DEB(FLAG_DEBUG_PROCESS_DIR,
        "Writing:[<li><span class=\"file\">"
        "<a href=\"%s\">%s</a> file.</span>] To file [%s]\n",
        f->html_file->name, f->full_name,
        f->parent->html_file->full_name);

    fprintf(f->parent->html_file->index_f,
        "      <li><span class=\"file\">"
        "<a href=\"%s\">%s</a> file.</span>\n",
        f->html_file->name, f->name);

    DEB(FLAG_DEBUG_PROCESS_FILE,
        "launching an instance of "EX_PATH"\n");

    ex_fd = popen(EX_PATH, "w");
    send_ex(ex_fd, "set notagstack");
    {   const char *s;
        s = strchr(f->full_name, '/');
        if (!s) s = f->full_name;
        while (*s == '/') s++;
        send_ex(ex_fd, "e! %s", s); /* edit original file */

        DEB(FLAG_DEBUG_PROCESS_FILE,
            "begin editing session on file %s -> %s\n",
            s, f->full_name);
    } /* block */

    /* for every tag in this file */
    DEB(FLAG_DEBUG_PROCESS_FILE,
            "for every tag in file [%s]\n", f->full_name);
    D(ntags = avl_tree_size(f->subnodes));
    DEB(FLAG_DEBUG_PROCESS_FILE,
            "it has %d tags:\n", ntags);
    if (ntags) {
        AVL_ITERATOR it1, it2;

        DEB(FLAG_DEBUG_PROCESS_FILE, "begin list of tags: ");

        fprintf(f->parent->html_file->index_f,
            "        <ul>\n");
        for (   it2 = it1 = avl_tree_first(f->subnodes);
                it1;
                it1 = avl_iterator_next(it1))
        {
            const ctag *tag1;

            tag1 = avl_iterator_data(it1);
            if (!tag1) {
                ERR(EXIT_FAILURE,
                    "cannot get a tag iterator, "
                    "this should not happen\n");
                /* NOTREACHED */
            }

            DEB_TAIL(FLAG_DEBUG_PROCESS_FILE,
                    "%s[%s]",
                    it1 == it2
                        ? ""
                        : "; ",
                    tag1->id);

            /* the first tag in the list contains the total number of tag in this list */
            if (tag1->tag_no_in_file > 1) { /* several tags for this id */
                const ctag *tag2;
                for (tag2 = tag1; tag2; tag2 = tag2->next_in_file) {
                    DEB_TAIL(FLAG_DEBUG_PROCESS_FILE,
                            "%s%d", tag2 == tag1 ? ": " : ", ", tag2->tag_no_in_file);
                    fprintf(f->parent->html_file->index_f,
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
                /* LCU: Mon 16 May 2022 12:44:57 PM EEST
                 * 00-Index entry has to be inserted */
            } else { /* only one tag for this id. */
                fprintf(f->parent->html_file->index_f,
                    "            <li><span class=\"tag\">"
                    "<a href=\"%s#%s-%d\">%s</a></span></li>\n",
                    f->html_file->name,
                    tag1->id,
                    tag1->tag_no_in_file,
                    tag1->id);
                send_ex(ex_fd, "ta %s", tag1->id); /* goto tag, only one tag in this file */
                send_ex(ex_fd, "s:^:(@a name=\"%s-%d\"@)(@/a@):",
                    tag1->id, tag1->tag_no_in_file); /* change */
                /* LCU: Mon 16 May 2022 12:44:57 PM EEST
                 * 00-Index entry has to be inserted */
            } /* if */
        } /* for */
        DEB_TAIL(FLAG_DEBUG_PROCESS_FILE, ".\n"); /* end list of tags */
        fprintf(f->parent->html_file->index_f,
                "        </ul>\n");
    } /* if */
    DEB(FLAG_DEBUG_PROCESS_FILE,
            "terminate "EX_PATH" editing session and write on parent [%s]\n",
            f->parent->html_file->full_name);
    fprintf(f->parent->html_file->index_f, "      </li>\n");
    send_ex(ex_fd, "w! %s", f->full_name); /* write file */
    send_ex(ex_fd, "q!"); /* and terminate */

    if (flags & FLAG_PROGRESS) {    /* print progress */
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
    DEB(FLAG_DEBUG_PROCESS_FILE,
            "scanning file %s -> %s\n",
            f->full_name, f->html_file->full_name);
    scanfile(f);
    DEB(FLAG_DEBUG_PROCESS_FILE,
            "end [%s]\n",
            f->full_name);

    return 0;
} /* process_file */

/* print help message */
void do_usage (void)
{
    fprintf(stderr,
            "Usage: " PROGNAME " [ options ... ]\n"
            PROGNAME " " VERSION ": Copyright (C) 1999-2022 <Luis.Colorado.Urcola@gmail.com>\n"
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
            "         D  database debug.\n"
            "         l  lexical processing debug.\n"
            "         x  ex(1) commands debug.\n"
            "         M  create_menu debug.\n"
            "         d  process dir debug.\n"
            "         f  process file debug.\n"
            "         i  process ident debug.\n"
            "         m  process menu debug.\n"
            "         s  process scanfile debug.\n"
            "         I  string intern debug.\n"
            "         c  ctags debug.\n"
            "         n  nodes debug.\n"
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
        case 'd': { char *p; /* debug */
                for (p = optarg; *p; p++) {
                    switch(*p) {
                    case '1': flags |= FLAG_DEBUG_PROCESS1; break;
                    case 'D': flags |= FLAG_DEBUG_DB; break;
                    case 'l': flags |= FLAG_DEBUG_LEX; break;
                    case 'x': flags |= FLAG_DEBUG_EX; break;
                    case 'M': flags |= FLAG_DEBUG_CREATE_MENU; break;
                    case '2': flags |= FLAG_DEBUG_PROCESS2; break;
                    case 'd': flags |= FLAG_DEBUG_PROCESS_DIR; break;
                    case 'f': flags |= FLAG_DEBUG_PROCESS_FILE; break;
                    case 'i': flags |= FLAG_DEBUG_PROCESS_IDENT; break;
                    case 'm': flags |= FLAG_DEBUG_PROCESS_MENU; break;
                    case 's': flags |= FLAG_DEBUG_SCANFILE; break;
                    case 'I': flags |= FLAG_DEBUG_INTERN; break;
                    case 'c': flags |= FLAG_DEBUG_CTAGS; break;
                    case 'n': flags |= FLAG_DEBUG_NODES; break;
                    case 'a': flags |= FLAG_DEBUG_ALL; break;
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

    db_root_node = new_node(output, NULL, TYPE_DIR);
    if (!db_root_node) {
        ERR(EXIT_FAILURE,
            "Cannot get a new node: %s\n",
            strerror(errno));
        /* NOTREACHED */
    }
    style_node = new_node(style_file, db_root_node, TYPE_HTML);
    js_node = new_node(js_file, db_root_node, TYPE_HTML);

    /* Process files */

    /* this process constructs the file node hierarchy of source file pages */
    process1(tag_file);

    D(fflush(stdout));

    D(do_recur(db_root_node,
        process_dir_pre,
        process_file,
        process_dir_post));

} /* main */

/* $Id: c2html.c,v 0.25 2014/09/09 20:22:05 luis Exp $ */
