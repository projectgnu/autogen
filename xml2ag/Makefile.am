##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:  ""
## Author:	    Bruce Korb <bkorb@gnu.org>
## Created:	    Jun 10 2002
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.1 2002/06/08 17:07:26 bkorb Exp $
## ---------------------------------------------------------------------

bin_PROGRAMS    = xml2ag

xml2ag_SOURCES  = xml2ag.c opts.c fork.c
nodist_xml2ag_SOURCES = opts.c opts.h fork.c

BUILT_SOURCES   = opts.c opts.h fork.c
EXTRA_DIST      = opts.def fork.tpl
AGEXE           = $(top_builddir)/agen5/@AGEXE@ -L $(top_srcdir)/autoopts
xml2ag_LDADD    = $(top_builddir)/autoopts/libopts.la
man_MANS        = xml2ag.1

opts.c $(OBJ) : opts.h

opts.h : opts.def
	autogen -L$(top_srcdir) opts.def

fork.c : fork.tpl opts.def
	autogen -T fork.tpl -L$(top_srcdir) opts.def
