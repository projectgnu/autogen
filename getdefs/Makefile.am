##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    Autogen copyright 1992-1998 Bruce Korb
## 
## Author:	   Bruce Korb <korbb@datadesign.com>
## Maintainer:	   Bruce Korb <korbb@datadesign.com>
## Created:	   Mon Jun 30 15:35:12 1997
## Last Modified:   Mon Sep 14 06:59:06 1998
##            by:   Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.2 1998/09/22 20:53:44 bkorb Exp $
## ---------------------------------------------------------------------

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

bin_PROGRAMS = getdefs

getdefs_SOURCES = getdefs.c getdefs.h opts.c opts.h opts.def

GEN_SRC 	= opts.c opts.h
EXTRA_DIST	= $(GEN_SRC) opts.def getdefs.texi

getdefs_LDADD	= $(top_builddir)/autoopts/libopts.la

INCLUDES 	= -I$(top_srcdir)/autoopts -I$(top_builddir)/autoopts

MAINTAINERCLEANFILES = Makefile.in

# Makefile.am ends here
