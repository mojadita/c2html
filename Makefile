# Makefile -- script to build c2html.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Thu Oct 10 13:00:54 EEST 2024
# Copyright: (c) 2024 Luis Colorado.  All rights reserved.

targets       = c2html
manpages      = c2html.1.gz
toclean       = $(targets) $(manpages)

GZFLAGS      ?= -v

PACKAGE       = c2html
PACKAGE_URL  ?= https://github.com/mojadita/c2html.git
PROGNAME     ?= c2html
AUTHOR_NAME  ?= Luis Colorado
AUTHOR_EMAIL ?= luiscoloradourcola@gmail.com
DEFAULT_FLAGS ?= (FLAG_DEBUG_ALWAYS | FLAG_PROGRESS)

VERSION      ?= 2.14-2022.05.27
RM           ?= rm -f
EX_PATH      ?= /usr/bin/ex

prefix       ?= /usr/local
exec_prefix  ?= $(prefix)
bindir       ?= $(exec_prefix)/bin
libdir       ?= $(exec_prefix)/lib
includedir   ?= $(prefix)/include
datadir      ?= $(libdir)/$(PACKAGE)
datarootdir  ?= $(prefix)/share
mandir       ?= $(datarootdir)/man
man1dir      ?= $(mandir)/man1
man2dir      ?= $(mandir)/man2
man3dir      ?= $(mandir)/man3
man4dir      ?= $(mandir)/man4
man5dir      ?= $(mandir)/man5
man6dir      ?= $(mandir)/man6
man7dir      ?= $(mandir)/man7
man8dir      ?= $(mandir)/man8
man9dir      ?= $(mandir)/man9
infodir      ?= $(datarootdir)/info
docdir       ?= $(datarootdir)/doc/$(PACKAGE)
confdir      ?= $(exec_prefix)/etc

.SUFFIXES: .h.in .c.in

OWN          ?= root
GRP          ?= wheel
XMOD         ?= 0111
FMOD         ?= 0444


c2html_deps   =
c2html_objs   = c2html.o \
			    ctag.o \
			    html_output.o \
			    intern.o \
			    lexical.o \
			    menu.o \
			    node.o
c2html_ldfl   = 
c2html_libs   = -lavl
toclean      += $(c2html_objs) lexical.c

all: $(targets) $(manpages)
clean:
	$(RM) $(toclean)
install: $(all)

c2html: $(c2html_deps) $(c2html_objs)
	$(CC) $(LDFLAGS) $($@_ldfl) $($@_objs) $($@_libs) -o $@

%:%.in
	$(DO_SED)
configure.h:configure.h.in Makefile
	$(DO_SED)

define DO_SED
	sed \
		-e 's:@bindir@:$(bindir):g' \
		-e 's:@confdir@:$(confdir):g' \
		-e 's:@datadir@:$(datadir):g' \
		-e 's:@datarootdir@:$(datarootdir):g' \
		-e 's:@DEFAULT_FLAGS@:$(DEFAULT_FLAGS):g' \
		-e 's:@docdir@:$(docdir):g' \
		-e 's:@exec_prefix@:$(exec_prefix):g' \
		-e 's:@includedir@:$(includedir):g' \
		-e 's:@infodir@:$(infodir):g' \
		-e 's:@libdir@:$(libdir):g' \
		-e 's:@man1dir@:$(man1dir):g' \
		-e 's:@man2dir@:$(man2dir):g' \
		-e 's:@man3dir@:$(man3dir):g' \
		-e 's:@man4dir@:$(man4dir):g' \
		-e 's:@man5dir@:$(man5dir):g' \
		-e 's:@man6dir@:$(man6dir):g' \
		-e 's:@man7dir@:$(man7dir):g' \
		-e 's:@man8dir@:$(man8dir):g' \
		-e 's:@man9dir@:$(man9dir):g' \
		-e 's:@mandir@:$(mandir):g' \
		-e 's:@prefix@:$(prefix):g' \
		-e 's:@PACKAGE@:$(PACKAGE):g' \
		-e 's@PACKAGE_URL@$(PACKAGE_URL)g' \
		-e 's:@PROGNAME@:$(PROGNAME):g' \
		-e 's:@AUTHOR_NAME@:$(AUTHOR_NAME):g' \
		-e 's:@AUTHOR_EMAIL@:$(AUTHOR_EMAIL):g' \
		-e 's:@VERSION@:$(VERSION):g' \
		-e 's:@EX_PATH@:$(EX_PATH):g' \
		$< > $@
endef

include .depend

c2html.1.gz: c2html.1
	gzip $(GZFLAGS) < $? > $@
