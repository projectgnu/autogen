##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    Autogen copyright 1992-1998 Bruce Korb
## 
## Author:	    Bruce Korb <korbb@datadesign.com>
## Maintainer:	    Bruce Korb <korbb@datadesign.com>
## Created:	    Mon Jun 30 15:35:12 1997
## Last Modified:   Wed Feb 24 11:03:51 1999
##            by:   Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.7 1999/03/19 20:58:50 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS = columns

columns_SOURCES = columns.c opts.c opts.h

GEN_SRC 	= opts.c opts.h columns.menu columns.texi
EXTRA_DIST	= $(GEN_SRC) opts.def columns.tpl columns.texi

columns_LDADD	= $(top_builddir)/autoopts/libopts.la

INCLUDES 	= -I$(top_srcdir)/autoopts -I$(top_builddir)/autoopts

MAINTAINERCLEANFILES = Makefile.in columns.texi

gen : $(GEN_SRC)

all : gen

columns.texi : columns.tpl opts.def columns
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -T columns.tpl \
		-L $(top_srcdir)/columns \
		$(top_srcdir)/columns/opts.def

columns.menu : opts.def
	@( echo "[=autogen template menu=columns.menu =]" ; \
	   echo "* [=prog_name #:: + #%-32s _printf=] [=prog_title=]" \
	 ) > .columns.tpl
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -T .columns.tpl \
		$(top_srcdir)/columns/opts.def && rm -f .columns.tpl

opts.c opts.h: opts.def
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/autogen -L $(top_srcdir)/autoopts \
		$(top_srcdir)/columns/opts.def

# Makefile.am ends here
