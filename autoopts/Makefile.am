##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##  Autoopts copyright 1992-1997 Bruce Korb
## 
## Author:          Bruce Korb <korbb@datadesign.com>
## Maintainer:      Bruce Korb <korbb@datadesign.com>
## Created:         Mon Jun 30 15:35:12 1997                                  
## Last Modified:   Tue Oct 13 07:41:57 1998
##            by:   Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.4 1998/10/13 15:30:42 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

@MAKEINCLUDE@ @MAKEQUOTE@$(top_builddir)/MakeRules@MAKEQUOTE@

lib_LTLIBRARIES = libopts.la
libopts_la_SOURCES = \
		autoopts.c pgusage.c putshell.c save.c \
		stack.c usage.c version.c streqvcmp.c \
                autoopts.h options.h streqv.h makeshell.c

libopts_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@

datadir		= $(prefix)/share
pkgdata_DATA	= optcode.tpl opthead.tpl options.tpl libhead.tpl
EXTRA_DIST      = $(pkgdata_DATA) TODO autoopts.texi
INCLUDES        = -I$(top_srcdir) -I$(srcdir) -I$(top_builddir) -I.
include_HEADERS = options.h
MAINTAINERCLEANFILES = Makefile.in

# Makefile.am ends here
