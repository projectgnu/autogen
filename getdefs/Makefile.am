##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    Autogen copyright 1992-1998 Bruce Korb
## 
## Author:	   Bruce Korb <korbb@datadesign.com>
## Maintainer:	   Bruce Korb <korbb@datadesign.com>
## Created:	   Mon Jun 30 15:35:12 1997
## Last Modified:   Tue Aug 18 09:16:51 1998
##            by:   Caldera Openlinux User <bkorb@ellen>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.0 1998/09/10 20:29:21 bkorb Exp $
## ---------------------------------------------------------------------

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

SUBDIRS = tests

bin_PROGRAMS = getdefs

getdefs_SOURCES = getdefs.c getdefs.h opts.c opts.h

BUILT_SOURCES	= opts.c opts.h

getdefs_LDADD	= $(top_builddir)/autoopts/libopts.la

INCLUDES 	= -I$(top_srcdir)/autoopts -I$(top_builddir)/autoopts

MAINTAINERCLEANFILES = Makefile.in

# Makefile.am ends here
