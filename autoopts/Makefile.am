##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##  Autoopts copyright 1992-1999 Bruce Korb
## 
## Author:          Bruce Korb <korbb@datadesign.com>
## Maintainer:      Bruce Korb <korbb@datadesign.com>
## Created:         Mon Jun 30 15:35:12 1997                                  
## Last Modified:   Tue Apr 13 08:10:01 1999
##            by:   Bruce Korb <korb@datadesign.com>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.12 1999/04/14 17:44:03 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

include $(top_srcdir)/MakeRules.in

lib_LTLIBRARIES = libopts.la
libopts_la_SOURCES = \
		autoopts.c pgusage.c putshell.c save.c \
		stack.c usage.c version.c streqvcmp.c \
		autoopts.h options.h streqv.h makeshell.c \
		snprintfv.h snprintfv.c

libopts_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@

pkgdata_DATA	= optcode.tpl opthead.tpl options.tpl libhead.tpl
EXTRA_DIST	= $(pkgdata_DATA) TODO autoopts.texi autoopts-config.in \
		genshell.def genshell.c genshell.h config-opts.def
INCLUDES	= -I$(top_srcdir) -I$(srcdir) -I$(top_builddir) -I.
bin_SCRIPTS	= autoopts-config
include_HEADERS = options.h
MAINTAINERCLEANFILES = Makefile.in

makeshell.o : genshell.c

genshell.c : genshell.def

# Makefile.am ends here
