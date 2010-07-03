##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2010-06-26 10:11:05 bkorb"
## Copyright (c):   1992-2010 by Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
##
## This file is part of AutoGen.
##
## AutoGen is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## AutoGen is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  If not, see <http://www.gnu.org/licenses/>.
##
## ---------------------------------------------------------------------
## $Id: a5e1f445e668e0f9f052a46099ea76eba5c7704c $
## ---------------------------------------------------------------------

bin_PROGRAMS    = columns
csrc            = opts.h columns.c opts.c
nodist_columns_SOURCES = cols.c

LO_LIB          = $(top_builddir)/autoopts/libopts.la
columns_LDADD   = $(LO_LIB) $(LIBGUILE_LIBS)

BUILT_SOURCES   = columns.menu columns.texi columns.1
EXTRA_DIST      = opts.def $(csrc)

RUNAG           = $(AGexe) -L$(top_srcdir)/autoopts -bcolumns -MF$@
man_MANS        = columns.1

INCLUDES        = @INCLIST@

CONFIG_CLEAN_FILES = $(BUILT_SOURCES) stamp-*
MAINTAINERCLEANFILES = $(CONFIG_CLEAN_FILES)
DISTCLEANFILES  = cols.c

gen : $(BUILT_SOURCES)

all : gen

## opts.h cannot be built until columns is built, so no rules for it.

columns.menu columns.texi : stamp-aginfo
columns.1                 : stamp-agman

cols.c : Makefile
	for f in config.h $(csrc) ; do echo "#include \"$$f\"" ; done > $@

stamp-aginfo : opts.def columns$(EXEEXT)
	top_builddir=$(top_builddir) \
	$(RUNAG) -Taginfo -DLEVEL=section $(srcdir)/opts.def

stamp-agman  : opts.def columns$(EXEEXT)
	top_builddir=$(top_builddir) \
	$(RUNAG) -Tagman1 $(srcdir)/opts.def

.NOTPARALLEL:

# Makefile.am ends here
