##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen copyright 1992-1999 Bruce Korb
## 
## Author:	    Bruce Korb <korbb@datadesign.com>
## Maintainer:	    Bruce Korb <korbb@datadesign.com>
## Created:	    Mon Jun 30 15:35:12 1997
## Last Modified:   Wed Jul  7 14:32:37 1999				      
##            by:   Bruce Korb <korb@datadesign.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.10 1999/07/07 21:32:59 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS = columns

columns_SOURCES = columns.c opts.c opts.h

GEN_SRC 	= opts.c opts.h columns.menu columns.texi
EXTRA_DIST	= $(GEN_SRC) opts.def columns.tpl columns.texi columns.1

columns_LDADD	= $(top_builddir)/autoopts/libopts.la \
		$(top_builddir)/snprintfv/snprintfv/libsnprintfv.la

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

columns.1 : opts.def columns
	${top_builddir}/src/autogen -T agman1 -L $(top_srcdir)/autoopts \
		-b columns opts.def

# Makefile.am ends here
