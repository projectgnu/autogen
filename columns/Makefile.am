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
## $Id: Makefile.am,v 1.24 2000/03/19 22:11:47 bruce Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS = columns

columns_SOURCES = columns.c opts.c opts.h

GEN_SRC 	= opts.c opts.h columns.menu columns.texi columns.1
EXTRA_DIST	= $(GEN_SRC) opts.def
AGEXE		= $(top_builddir)/agen5/AutoGen
columns_LDADD	= $(top_builddir)/autoopts/libopts.la
man_MANS        = columns.1

INCLUDES 	= -I$(top_srcdir)/autoopts -I$(top_builddir)/autoopts

MAINTAINERCLEANFILES = Makefile.in columns.texi

gen : $(GEN_SRC)

all : gen

opts.c opts.h: opts.def
	top_builddir=$(top_builddir) \
	$(AGEXE) -L $(top_srcdir)/autoopts \
		$(top_srcdir)/columns/opts.def

columns.menu columns.texi : opts.def columns
	top_builddir=$(top_builddir) \
	$(AGEXE) -Taginfo -bcolumns \
		-L $(top_srcdir)/autoopts -DLEVEL=section \
		$(top_srcdir)/columns/opts.def

columns.1 : opts.def columns
	$(AGEXE) -T agman1 -L $(top_srcdir)/autoopts \
		-b columns $(srcdir)/opts.def

# Makefile.am ends here
