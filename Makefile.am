# $Id: Makefile.am,v 1.1 2005/02/22 19:23:36 luis Exp $
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

INCLUDES=-DVERSION=\""@VERSION@"\" -DEX_PATH=\""@EX_PATH@ >/dev/null 2>&1"\"

bin_PROGRAMS=c2html

c2html_SOURCES = c2html.c calcCRC.c crc12.c hashCRC12.c hashTable.c multifree.c \
	lexical.l html_output.c
#
# $Id: Makefile.am,v 1.1 2005/02/22 19:23:36 luis Exp $
