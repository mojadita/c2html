dnl Process this file with autoconf to produce a configure script.
dnl $Id: configure.in,v 0.20 2014/09/09 20:22:05 luis Exp $
dnl Author: Luis.Colorado@SLUG.HispaLinux.ES
dnl Copyright: GPL (C) 1999 Luis.Colorado@SLUG.HispaLinux.ES
dnl
dnl C2HTML -- A program to convert C source code to cross referenced HTML.
dnl Copyright (C) 1999  <Luis.Colorado@SLUG.HispaLinux.ES>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl

AC_INIT(c2html,2.14-20220517,luis.colorado.urcola@gmail.com,https://github.com/mojadita/c2html/)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(./c2html.c)

AUTHOR_NAME="Luis Colorado"
AUTHOR_EMAIL="luis.colorado.urcola@gmail.com"
OPERATING_SYSTEM=$(uname -s)
OPERATING_SYSTEM_VERSION=$(uname -r)

AM_PROG_LEX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

dnl Checks for programs.
AC_PATH_PROG(EX_PATH, [ex],,
	[$PATH:/bin:/usr/bin:/usr/local/bin:/sbin:/usr/sbin:/usr/local/sbin])

if test `basename ${LEX}` != 'flex'
then
	echo "$0: We ONLY support flex (LEX = ${LEX})" >&2
	exit 1
fi

dnl Checks for libraries.
AC_CHECK_HEADERS(avl.h)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/time.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_CHECK_FUNCS(strdup strtok strerror)

AC_SUBST(AUTHOR_NAME)
AC_SUBST(AUTHOR_EMAIL)
AC_SUBST(AUTHOR)
AC_SUBST(PACKAGE_URL)
AC_SUBST(EX_PATH)
AC_SUBST(OPERATING_SYSTEM)
AC_SUBST(OPERATING_SYSTEM_VERSION)

AC_OUTPUT(Makefile c2html.1)

dnl $Id: configure.in,v 0.20 2014/09/09 20:22:05 luis Exp $
