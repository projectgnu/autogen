##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2005-02-21 11:48:49 bkorb"
## Copyright (c):   1992-2005 by Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>                    
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.5 2005/02/21 23:01:08 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS    = columns
csrc            = opts.h columns.c opts.c
nodist_columns_SOURCES = cols.c

BUILT_SOURCES   = columns.menu columns.texi columns.1
EXTRA_DIST      = opts.def $(csrc)
RUNAG           = $(AGexe) -L$(top_srcdir)/autoopts -bcolumns
columns_LDADD   = $(top_builddir)/autoopts/libopts.la
man_MANS        = columns.1

INCLUDES        = @INCLIST@

CONFIG_CLEAN_FILES = columns.texi columns.menu columns.1 gen-stamp
MAINTAINERCLEANFILES = Makefile.in $(CONFIG_CLEAN_FILES)
DISTCLEANFILES  = cols.c

gen : $(BUILT_SOURCES)

all : gen

## opts.h cannot be built until columns is built, so no rules for it.

columns.1 columns.menu columns.texi : gen-stamp

cols.c : Makefile
	for f in config.h $(csrc) ; do echo "#include \"$$f\"" ; done > $@

gen-stamp : opts.def columns$(EXEEXT)
	top_builddir=$(top_builddir) \
	$(RUNAG) -Taginfo -DLEVEL=section $(srcdir)/opts.def
	$(RUNAG) -Tagman1 $(srcdir)/opts.def
	@touch $@

# Makefile.am ends here
