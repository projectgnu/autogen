##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen copyright 1992-1999 Bruce Korb
## 
## Author:	    Bruce Korb <autogen@linuxbox.com>
## Maintainer:	    Bruce Korb <autogen@linuxbox.com>
## Created:	    Mon Jun 30 15:35:12 1997
## Last Modified:   Thu Jul 29 13:40:42 1999				      
##            by:   Bruce Korb <autogen@linuxbox.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.20 1999/10/29 02:35:29 bruce Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS = columns

columns_SOURCES = columns.c opts.c opts.h

GEN_SRC 	= opts.c opts.h columns.menu columns.texi columns.1
EXTRA_DIST	= $(GEN_SRC) opts.def bootstrap.dir

columns_LDADD	= $(top_builddir)/autoopts/libopts.la

INCLUDES 	= -I$(top_srcdir)/autoopts -I$(top_builddir)/autoopts

MAINTAINERCLEANFILES = Makefile.in columns.texi

gen : $(GEN_SRC)

all : gen

opts.c opts.h: opts.def
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/AutoGen -L $(top_srcdir)/autoopts \
		$(top_srcdir)/columns/opts.def

columns.menu columns.texi : opts.def columns
	top_builddir=$(top_builddir) \
	$(top_builddir)/src/AutoGen -Taginfo -bcolumns \
		-L $(top_srcdir)/autoopts -DLEVEL=section \
		$(top_srcdir)/columns/opts.def

columns.1 : opts.def columns
	${top_builddir}/src/AutoGen -T agman1 -L $(top_srcdir)/autoopts \
		-b columns $(srcdir)/opts.def

# Makefile.am ends here
