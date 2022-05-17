# $Id: Makefile.am,v 1.3 2014/09/09 20:22:03 luis Exp $
# Author: Luis.Colorado@SLUG.HispaLinux.ES
# Date: Sun Jun  6 15:43:44 MEST 1999
# C2HTML -- A program to convert a C source package into a cross reference HTML
#      hierarchy.
# Copyright (C) 1999 <Luis.Colorado@SLUG.HispaLinux.ES>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS= -DAUTHOR=\""@AUTHOR_NAME@"\" -DPROGNAME=\""@PACKAGE@"\" \
	-DVERSION=\""@VERSION@"\" -DEX_PATH=\""@EX_PATH@"\" \
	-DAUTHOR_EMAIL=\""@AUTHOR_EMAIL@"\" \
	-DAUTHOR_NAME=\""@AUTHOR_NAME@"\" \
	-DPACKAGE_URL=\""@PACKAGE_URL@"\" \
	-DPACKAGE_URL=\""@PACKAGE_URL@"\"

bin_PROGRAMS=c2html
check_PROGRAMS=test_node test_menu test_ctag

LIBS = -lavl

c2html_CFLAGS = -I/usr/local/include
c2html_SOURCES = c2html.c lexical.l html_output.c intern.c ctag.c menu.c node.c
c2html_LDFLAGS = -L/usr/local/lib

test_node_SOURCES = test_node.c node.c intern.c
test_menu_SOURCES = test_menu.c menu.c intern.c node.c
test_ctag_SOURCES = test_ctag.c ctag.c menu.c node.c intern.c
man_MANS = c2html.1

#
# $Id: Makefile.am,v 1.3 2014/09/09 20:22:03 luis Exp $
