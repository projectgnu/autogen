##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen copyright 1992-1999 Bruce Korb
## 
## Author:	    Bruce Korb <bkorb@gnu.org>
## Maintainer:	    Bruce Korb <bkorb@gnu.org>
## Created:	    Mon Jun 30 15:35:12 1997
## Last Modified:   Thu Jul 29 13:40:42 1999				      
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.1 2001/12/29 11:24:56 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS    = columns

columns_SOURCES = columns.c opts.c opts.h

BUILT_SOURCES   = columns.menu columns.texi columns.1
EXTRA_DIST      = opts.def
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

gen-stamp : opts.def columns
	top_builddir=$(top_builddir) \
	$(AGEXE) -Taginfo -bcolumns -DLEVEL=section \
		$(top_srcdir)/columns/opts.def
	$(AGEXE) -T agman1 -b columns $(srcdir)/opts.def
	@touch $@

clean : columns-clean

columns-clean :
	rm -f *-stamp

# Makefile.am ends here
