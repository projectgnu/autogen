##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    Autogen copyright 1992-1998 Bruce Korb
## 
## Author:	   Bruce Korb <korbb@datadesign.com>
## Maintainer:	   Bruce Korb <korbb@datadesign.com>
## Created:	   Mon Jun 30 15:35:12 1997
## Last Modified:   Wed Oct 21 07:13:30 1998
##            by:   Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.7 1998/10/21 14:28:17 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS = getdefs

getdefs_SOURCES = getdefs.c getdefs.h opts.c opts.h opts.def

GEN_SRC 	= opts.c opts.h getdefs.texi getdefs.menu
EXTRA_DIST	= $(GEN_SRC) opts.def getdefs.texi getdefs.tpl

getdefs_LDADD	= $(top_builddir)/autoopts/libopts.la
getdefs_LDFLAGS = -static
INCLUDES 	= -I$(top_srcdir)/autoopts -I$(top_builddir)/autoopts

MAINTAINERCLEANFILES = Makefile.in

gen : $(GEN_SRC)

all : gen

getdefs.texi : getdefs.tpl opts.def
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -T getdefs.tpl \
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

# Makefile.am ends here
