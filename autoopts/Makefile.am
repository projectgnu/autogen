##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##  Autoopts copyright 1992-1999 Bruce Korb
## 
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997                                  
## Last Modified:   Thu Jul 29 11:57:04 1999				      
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.26 2000/08/11 15:05:47 gvv Exp $
## ---------------------------------------------------------------------
## Code:

SUBDIRS		= test

MAINTAINERCLEANFILES = Makefile.in

INCLUDES	= -I$(top_srcdir)/snprintfv -I$(top_srcdir) \
		-I$(top_builddir)/snprintfv -I$(top_builddir) -I.

pkgdata_DATA	= optcode.tpl opthead.tpl options.tpl libhead.tpl \
		aginfo.tpl agman1.tpl optlib.tpl

EXTRA_DIST	= $(pkgdata_DATA) TODO autoopts.m4 doc.tpl \
		bootstrap.dir autoopts-config.in config-opts.def \
		genshell.def genshell.c genshell.h

m4datadir	= $(datadir)/aclocal
m4data_DATA	= autoopts.m4

lib_LTLIBRARIES = libopts.la
libopts_la_SOURCES = \
		autoopts.c pgusage.c putshell.c save.c \
		stack.c usage.c version.c streqvcmp.c \
		autoopts.h options.h streqv.h makeshell.c

libopts_la_LDFLAGS = -no-undefined \
		-version-info @AO_CURRENT@:@AO_REVISION@:@AO_AGE@
libopts_la_LIBADD = ../snprintfv/snprintfv/libsnprintfvc.la

bin_SCRIPTS	= autoopts-config

include_HEADERS = options.h

makeshell.o : genshell.c
genshell.c : genshell.def
	@AGEXE=`cd $(top_builddir)/agen5; pwd`/@AGEXE@ ; cd $(srcdir) ; \
	  AGEXE="$$AGEXE" $(SHELL) ./bootstrap.dir $@

../snprintfv/snprintfv.h :
	if [ ! -L ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

../snprintfv/snprintfv/libsnprintfvc.la :
	cd ../snprintfv/snprintfv ; $(MAKE) libsnprintfvc.la

# Makefile.am ends here
