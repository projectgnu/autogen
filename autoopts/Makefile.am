##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##  Autoopts copyright 1992-1997 Bruce Korb
## 
## Author:          Bruce Korb <korbb@datadesign.com>
## Maintainer:      Bruce Korb <korbb@datadesign.com>
## Created:         Mon Jun 30 15:35:12 1997                                  
## Last Modified:   $Date: 1998/07/18 05:34:47 $
##            by:   Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.6 1998/07/18 05:34:47 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

lib_LTLIBRARIES = libopts.la
libopts_la_SOURCES = \
		autoopts.c pgusage.c putshell.c save.c \
		stack.c usage.c version.c streqvcmp.c \
                autoopts.h options.h streqv.h

datadir		= $(prefix)/share
pkgdata_DATA	= optcode.tpl opthead.tpl options.tpl
EXTRA_DIST      = $(pkgdata_DATA) TODO
INCLUDES        = -I$(top_srcdir) -I$(srcdir) -I$(top_builddir) -I.
include_HEADERS = options.h
MAINTAINERCLEANFILES = Makefile.in

# Makefile.am ends here
