##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##  Autoopts copyright 1992-1997 Bruce Korb
## 
## Author:          Bruce Korb <korbb@datadesign.com>
## Maintainer:      Bruce Korb <korbb@datadesign.com>
## Created:         Mon Jun 30 15:35:12 1997                                  
## Last Modified:   1998/04/01 at 08:57:58
##            by:   Bruce Korb <korbb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.1.1.1 1998/04/29 23:14:31 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

lib_LTLIBRARIES = libopts.la
libopts_la_SOURCES = autoopts.c pgusage.c putshell.c save.c \
		  stack.c usage.c version.c streqvcmp.c \
                  autoopts.h options.h streqv.h

datadir		= $(prefix)/share/$(PACKAGE)
pkgdata_DATA	= optcode.tpl opthead.tpl options.tpl
EXTRA_DIST      = $(pkgdata_DATA)
INCLUDES        = -I$(top_srcdir) -I$(srcdir) -I$(top_builddir) -I.
include_HEADERS = options.h

# Makefile.am ends here
