# Makefile -- script to build c2html.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Thu Oct 10 13:00:54 EEST 2024
# Copyright: (c) 2024 Luis Colorado.  All rights reserved.

targets            = c2html
manpages           = c2html.1.gz
toclean            = $(targets) $(manpages)

GZFLAGS           ?= -v

PACKAGE            = c2html
PACKAGE_URL       ?= https://github.com/mojadita/c2html.git
PROGNAME          ?= c2html
AUTHOR_NAME       ?= Luis Colorado
AUTHOR_EMAIL      ?= luiscoloradourcola@gmail.com
DEFAULT_FLAGS     ?= (FLAG_DEBUG_ALWAYS | FLAG_PROGRESS)
DEFAULT_MENU_BASE ?= 00-Index

VERSION           ?= 3.2.21-2024.10.19 # PREV: 3.2.20-2024.10.19
RM                ?= rm -f
INSTALL           ?= install
EX_PATH           ?= /usr/bin/vim -e

prefix            ?= /usr/local
exec_prefix       ?= $(prefix)
bindir            ?= $(exec_prefix)/bin
libdir            ?= $(exec_prefix)/lib
includedir        ?= $(prefix)/include
datadir           ?= $(datarootdir)/$(PACKAGE)

style_css         ?= style.css
holes_png         ?= holes.png
bgtile_png        ?= bgtile.png
background_png    ?= background.png

datarootdir       ?= $(prefix)/share
mandir            ?= $(datarootdir)/man
man1dir           ?= $(mandir)/man1
man2dir           ?= $(mandir)/man2
man3dir           ?= $(mandir)/man3
man4dir           ?= $(mandir)/man4
man5dir           ?= $(mandir)/man5
man6dir           ?= $(mandir)/man6
man7dir           ?= $(mandir)/man7
man8dir           ?= $(mandir)/man8
man9dir           ?= $(mandir)/man9
infodir           ?= $(datarootdir)/info
docdir            ?= $(datarootdir)/doc/$(PACKAGE)
confdir           ?= $(exec_prefix)/etc

.SUFFIXES: .1.in .1 .h.in .h

OWN               ?= root
GRP               ?= bin
XMOD              ?= 0711
FMOD              ?= 0644
DMOD              ?= 0755


c2html_deps        =
c2html_objs        = c2html.o \
     			     ctag.o \
     			     html_output.o \
     			     intern.o \
     			     lexical.o \
     			     menu.o \
     			     node.o
c2html_ldfl        = 
c2html_libs        = -lavl
toclean           += $(c2html_objs) lexical.c

.PHONY: all clean distclean install version

all: $(targets) $(manpages)
clean:
	$(RM) $(toclean)
distclean: clean
	$(RM) configure.h
install: $(all)
	$(INSTALL) -o $(OWN) -g $(GRP) -m $(DMOD) -d $(bindir)
	$(INSTALL) -o $(OWN) -g $(GRP) -m $(DMOD) -d $(datadir)
	$(INSTALL) -o $(OWN) -g $(GRP) -m $(FMOD) \
		style.css holes.png bgtile.png background.png $(datadir)
	$(INSTALL) -o $(OWN) -g $(GRP) -m $(XMOD) c2html $(bindir)
	$(INSTALL) -o $(OWN) -g $(GRP) -m $(FMOD) c2html.1.gz $(man1dir)

version:
	@echo $(VERSION)

c2html: $(c2html_deps) $(c2html_objs)
	$(CC) $(LDFLAGS) $($@_ldfl) $($@_objs) $($@_libs) -o $@

c2html.1: c2html.1.in Makefile
configure.h: configure.h.in Makefile

DO_SED=sed \
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
		-e 's:@DEFAULT_MENU_BASE@:$(DEFAULT_MENU_BASE):g' \
		-e 's:@style_css@:$(style_css):g' \
		-e 's:@holes_png@:$(holes_png):g' \
		-e 's:@bgtile_png@:$(bgtile_png):g' \
		-e 's:@background_png@:$(background_png):g' \
		$< > $@

.h.in.h:
	$(DO_SED)
.1.in.1:
	$(DO_SED)

c2html.1.gz: c2html.1
	gzip $(GZFLAGS) < $? > $@
