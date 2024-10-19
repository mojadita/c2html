# Makefile -- script to build c2html.
# Author: Luis Colorado <luiscoloradourcola@gmail.com>
# Date: Thu Oct 10 13:00:54 EEST 2024
# Copyright: (c) 2024 Luis Colorado.  All rights reserved.

targets               = c2html
manpages              = c2html.1.gz
toclean               = $(targets) $(manpages)
build_date           != date +%Y.%m.%d
year                 != echo '$(build_date)' | sed -e 's/\(....\).*/\1/'
SEDFLAGS             != sed-options.sh <configure.mk
locate_ex            != which vim

GZFLAGS              ?= -v


RM                   ?= rm -f
INSTALL              ?= install

# configuration options
include configure.mk

.SUFFIXES: .1

c2html_deps        =
c2html_objs        = c2html.o \
     			     ctag.o \
     			     html_output.o \
     			     intern.o \
     			     lexical.o \
     			     menu.o \
     			     node.o
c2html_ldfl        = 
c2html_libs        = -lavl_c
toclean           += $(c2html_objs) lexical.c configure.h

.PHONY: all clean distclean install version

all: $(targets) $(manpages)

clean:
	$(RM) $(toclean)

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

configure.h: configure.mk mk-configure.h.in.sh Makefile
	mk-configure.h.in.sh <configure.mk | sed $(SEDFLAGS) > $@

c2html.1: c2html.1.in
	sed $(SEDFLAGS) < $@.in > $@

c2html.1.gz: c2html.1
	gzip $(GZFLAGS) < $? > $@

c2html.1.pdf: c2html.1
	groff -mandoc -Tpdf c2html.1 > $@
