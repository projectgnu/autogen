##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen copyright 1992-1999 Bruce Korb
## 
## Author:	   Bruce Korb <korbb@datadesign.com>
## Maintainer:	   Bruce Korb <korbb@datadesign.com>
## Created:	   Mon Jun 30 15:35:12 1997
## Last Modified:   Thu Jul  8 14:43:59 1999				      
##            by:   Bruce Korb <korb@datadesign.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.14 1999/07/08 21:44:04 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS = getdefs

getdefs_SOURCES = getdefs.c getdefs.h opts.c opts.h

GEN_SRC 	= opts.c opts.h getdefs.texi getdefs.menu getdefs.1
EXTRA_DIST	= $(GEN_SRC) opts.def getdefs.texi

getdefs_LDADD	= $(top_builddir)/autoopts/libopts.la \
		$(top_builddir)/snprintfv/snprintfv/libsnprintfv.la

getdefs_LDFLAGS = -static
INCLUDES 	= -I$(top_srcdir) -I$(top_srcdir)/autoopts

MAINTAINERCLEANFILES = Makefile.in

gen : $(GEN_SRC)

all : gen

opts.c opts.h: opts.def
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -L $(top_srcdir)/autoopts \
		$(top_srcdir)/getdefs/opts.def

getdefs.texi : opts.def getdefs
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -Taginfo -bgetdefs \
		-L $(top_srcdir)/autoopts -DLEVEL=section \
		$(top_srcdir)/getdefs/opts.def

getdefs.1 : opts.def getdefs
	${top_builddir}/src/autogen -T agman1 -L $(top_srcdir)/autoopts \
		-b getdefs opts.def

# Makefile.am ends here
