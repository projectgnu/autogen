##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen copyright 1992-1999 Bruce Korb
## 
## Author:	   Bruce Korb <korbb@datadesign.com>
## Maintainer:	   Bruce Korb <korbb@datadesign.com>
## Created:	   Mon Jun 30 15:35:12 1997
## Last Modified:   Wed Jul  7 14:32:03 1999				      
##            by:   Bruce Korb <korb@datadesign.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.13 1999/07/07 21:32:59 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS = getdefs

getdefs_SOURCES = getdefs.c getdefs.h opts.c opts.h

GEN_SRC 	= opts.c opts.h getdefs.texi getdefs.menu getdefs.1
EXTRA_DIST	= $(GEN_SRC) opts.def getdefs.texi getdefs.tpl

getdefs_LDADD	= $(top_builddir)/autoopts/libopts.la \
		$(top_builddir)/snprintfv/snprintfv/libsnprintfv.la

getdefs_LDFLAGS = -static
INCLUDES 	= -I$(top_srcdir) -I$(top_srcdir)/autoopts

MAINTAINERCLEANFILES = Makefile.in

gen : $(GEN_SRC)

all : gen

getdefs.texi : getdefs.tpl opts.def getdefs
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -T getdefs.tpl \
		-L $(top_srcdir)/getdefs \
		$(top_srcdir)/getdefs/opts.def

getdefs.menu : opts.def
	@( echo "[=autogen template menu=getdefs.menu =]" ; \
	   echo "* [=prog_name #:: + #%-32s _printf=] [=prog_title=]" \
	 ) > .getdefs.tpl
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -T .getdefs.tpl \
		$(top_srcdir)/getdefs/opts.def && rm -f .getdefs.tpl

opts.c opts.h: opts.def
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -L $(top_srcdir)/autoopts \
		$(top_srcdir)/getdefs/opts.def

getdefs.1 : opts.def getdefs
	${top_builddir}/src/autogen -T agman1 -L $(top_srcdir)/autoopts \
		-b getdefs opts.def

# Makefile.am ends here
