##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##  Autoopts copyright 1992-1999 Bruce Korb
## 
## Author:          Bruce Korb <korbb@datadesign.com>
## Maintainer:      Bruce Korb <korbb@datadesign.com>
## Created:         Mon Jun 30 15:35:12 1997                                  
## Last Modified:   Tue Jul  6 14:32:59 1999				      
##            by:   Bruce Korb <korb@datadesign.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.15 1999/07/06 21:34:06 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

include $(top_srcdir)/MakeRules.in
MAINTAINERCLEANFILES = Makefile.in

INCLUDES	= -I$(top_srcdir)/snprintfv -I$(top_srcdir) \
		-I$(top_builddir)/snprintfv -I$(top_builddir) -I.

EXTRA_DIST	= $(pkgdata_DATA) TODO autoopts.m4 autoopts.texi bootstrap \
		autoopts-config.in config-opts.def \
		genshell.def genshell.c genshell.h agman.tpl

m4datadir	= $(datadir)/aclocal
m4data_DATA	= autoopts.m4

lib_LTLIBRARIES = libopts.la
libopts_la_SOURCES = \
		autoopts.c pgusage.c putshell.c save.c \
		stack.c usage.c version.c streqvcmp.c \
		autoopts.h options.h streqv.h makeshell.c
libopts_la_LDFLAGS = -version-info @AO_CURRENT@:@AO_REVISION@:@AO_AGE@

pkgdata_DATA	= optcode.tpl opthead.tpl options.tpl libhead.tpl agman.tpl

bin_SCRIPTS	= autoopts-config

include_HEADERS = options.h

makeshell.o : genshell.c
genshell.c : genshell.def

# Makefile.am ends here
