##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2004-02-01 13:25:08 bkorb"
## Copyright (c):   1992-2004 by Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>                    
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.6 2004/02/01 21:26:46 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS    = columns
csrc            = opts.h columns.c opts.c
nodist_columns_SOURCES = cols.c

BUILT_SOURCES   = columns.menu columns.texi columns.1
EXTRA_DIST      = opts.def $(csrc)
AGEXE           = $(top_builddir)/agen5/@AGEXE@ -L $(top_srcdir)/autoopts
columns_LDADD   = $(top_builddir)/autoopts/libopts.la
man_MANS        = columns.1

INCLUDES        = @INCLIST@

CONFIG_CLEAN_FILES = columns.texi columns.menu columns.1
MAINTAINERCLEANFILES = Makefile.in $(CONFIG_CLEAN_FILES)

gen : $(BUILT_SOURCES)

all : gen

## opts.h cannot be built until columns is built, so no rules for it.

columns.1 columns.menu columns.texi : gen-stamp

cols.c : $(csrc)
	for f in $(csrc) ; do echo "#include \"$$f\"" ; done > $@

gen-stamp : opts.def columns$(EXEEXT)
	top_builddir=$(top_builddir) \
	$(AGEXE) -Taginfo -bcolumns -DLEVEL=section \
		$(top_srcdir)/columns/opts.def
	$(AGEXE) -T agman1 -b columns $(srcdir)/opts.def
	@touch $@

clean : columns-clean
distclean : columns-clean

columns-clean :
	rm -f *-stamp

# Makefile.am ends here
