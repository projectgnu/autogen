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
## $Id: Makefile.am,v 2.29 2001/12/01 20:26:20 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS	= getdefs
SUBDIRS		= test
getdefs_SOURCES = getdefs.c gdemit.c gdinit.c getdefs.h opts.c opts.h

GEN_SRC 	= opts.c opts.h getdefs.texi getdefs.menu getdefs.1
EXTRA_DIST	= $(GEN_SRC) opts.def
AGEXE		= $(top_builddir)/agen5/@AGEXE@ -L $(top_srcdir)/autoopts
getdefs_LDADD	= $(top_builddir)/autoopts/libopts.la
man_MANS        = getdefs.1

getdefs_LDFLAGS = -static
INCLUDES = @INCLIST@

CONFIG_CLEAN_FILES = getdefs.texi getdefs.menu getdefs.1
MAINTAINERCLEANFILES = Makefile.in $(CONFIG_CLEAN_FILES)

gen : $(GEN_SRC)

all : gen

opts.c opts.h : opts.def
	top_builddir=$(top_builddir) \
	$(AGEXE) $(top_srcdir)/getdefs/opts.def

getdefs.o : opts.h

getdefs.menu getdefs.texi : opts.def getdefs
	top_builddir=$(top_builddir) \
	$(AGEXE) -Taginfo -bgetdefs -DLEVEL=section \
		$(top_srcdir)/getdefs/opts.def

getdefs.1 : opts.def getdefs
	$(AGEXE) -T agman1 -b getdefs $(srcdir)/opts.def && touch $@

# Makefile.am ends here
