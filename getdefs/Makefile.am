##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen copyright 1992-1999 Bruce Korb
## 
## Author:	   Bruce Korb <bkorb@gnu.org>
## Maintainer:	   Bruce Korb <bkorb@gnu.org>
## Created:	   Mon Jun 30 15:35:12 1997
## Last Modified:   Thu Jul 29 13:41:31 1999				      
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.1 2001/12/29 11:24:56 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS    = getdefs
SUBDIRS         = test
getdefs_SOURCES = getdefs.c gdemit.c gdinit.c getdefs.h proto.h
nodist_getdefs_SOURCES = opts.c opts.h

BUILT_SOURCES   = opts.c opts.h getdefs.texi getdefs.menu getdefs.1
EXTRA_DIST      = opts.def
AGEXE           = $(top_builddir)/agen5/@AGEXE@ -L $(top_srcdir)/autoopts
getdefs_LDADD   = $(top_builddir)/autoopts/libopts.la
man_MANS        = getdefs.1

getdefs_LDFLAGS = -static
INCLUDES        = @INCLIST@

MAINTAINERCLEANFILES = Makefile.in $(CONFIG_CLEAN_FILES)

gen : $(BUILT_SOURCES)

all : gen

getdefs.o : opts.h

opts.c opts.h : opts-stamp

opts-stamp : opts.def
	top_builddir=$(top_builddir) COLUMNS_EXE=`cd ../columns;pwd`/columns \
	$(AGEXE) -L $(top_srcdir)/autoopts $(srcdir)/opts.def
	@touch $@

getdefs.1 getdefs.menu getdefs.texi : gen-stamp

gen-stamp : opts.def getdefs
	top_builddir=$(top_builddir) \
	$(AGEXE) -Taginfo -bgetdefs -DLEVEL=section \
		$(top_srcdir)/getdefs/opts.def
	$(AGEXE) -T agman1 -b getdefs $(srcdir)/opts.def
	@touch $@

clean : getdef-clean

getdef-clean :
	rm -f *-stamp

# Makefile.am ends here
