##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen copyright 1992-1999 Bruce Korb
## 
## Author:	   Bruce Korb <autogen@linuxbox.com>
## Maintainer:	   Bruce Korb <autogen@linuxbox.com>
## Created:	   Mon Jun 30 15:35:12 1997
## Last Modified:   Thu Jul 29 13:41:31 1999				      
##            by:   Bruce Korb <autogen@linuxbox.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.24 2000/03/19 22:09:33 bruce Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS	= getdefs
SUBDIRS		= test
getdefs_SOURCES = getdefs.c getdefs.h opts.c opts.h

GEN_SRC 	= opts.c opts.h getdefs.texi getdefs.menu getdefs.1
EXTRA_DIST	= $(GEN_SRC) opts.def
AGEXE		= $(top_builddir)/agen5/AutoGen
getdefs_LDADD	= $(top_builddir)/autoopts/libopts.la
man_MANS        = getdefs.1

getdefs_LDFLAGS = -static
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/autoopts -I$(top_srcdir)/snprintfv

MAINTAINERCLEANFILES = Makefile.in

gen : $(GEN_SRC)

all : gen

opts.c opts.h : opts.def
	top_builddir=$(top_builddir) \
	$(AGEXE) -L $(top_srcdir)/autoopts \
		$(top_srcdir)/getdefs/opts.def

getdefs.menu getdefs.texi : opts.def getdefs
	top_builddir=$(top_builddir) \
	$(AGEXE) -Taginfo -bgetdefs \
		-L $(top_srcdir)/autoopts -DLEVEL=section \
		$(top_srcdir)/getdefs/opts.def

getdefs.1 : opts.def getdefs
	$(AGEXE) -T agman1 -L $(top_srcdir)/autoopts \
		-b getdefs $(srcdir)/opts.def

# Makefile.am ends here
