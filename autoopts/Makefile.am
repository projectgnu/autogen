##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##  Autoopts copyright 1992-2000 Bruce Korb
## 
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997                                  
## Last Modified:   $Date: 2000/10/02 04:20:04 $
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 2.30 2000/10/02 04:20:04 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

SUBDIRS		= test

MAINTAINERCLEANFILES = Makefile.in

INCLUDES	= -I$(top_srcdir)/snprintfv -I$(top_srcdir) \
		-I$(top_builddir)/snprintfv -I$(top_builddir) -I.

pkgdata_DATA	= \
	aginfo.tpl   agman1.tpl   doc.tpl      libhead.tpl  optcode.tpl \
	opthead.tpl  options.tpl  optlib.tpl   optmain.tpl

GENSRC		= genshell.c genshell.h
EXTRA_DIST	= $(pkgdata_DATA)  $(GENSRC) \
	TODO autoopts.m4 doc.tpl bootstrap.dir autoopts-config.in \
	config-opts.def genshell.def

m4datadir	= $(datadir)/aclocal
m4data_DATA	= autoopts.m4

lib_LTLIBRARIES = libopts.la libguileopts.la

##  libopts_la_SOURCES does _NOT_ include $(GENSRC) !!
##  It is compiled as a part of the makeshell module.
##
libopts_la_SOURCES = \
	autoopts.c   autoopts.h   makeshell.c  options.h \
	pgusage.c    putshell.c   save.c       stack.c      streqv.h \
	streqvcmp.c  usage.c      version.c

libopts_la_LDFLAGS = -no-undefined \
		-version-info @AO_CURRENT@:@AO_REVISION@:@AO_AGE@
libopts_la_LIBADD = ../snprintfv/snprintfv/libsnprintfvc.la

libguileopts_la_SOURCES = guileopt.c
libguileopts_la_LDFLAGS = -version-info @GO_CURRENT@:@GO_REVISION@:@GO_AGE@
libguileopts_la_LIBADD  = ./libopts.la

bin_SCRIPTS	= autoopts-config

include_HEADERS = options.h

makeshell.o : genshell.c
genshell.c : genshell.def
	@AGEXE=`cd $(top_builddir)/agen5; pwd`/autogen ; cd $(srcdir) ; \
	  AGEXE="$$AGEXE" $(SHELL) ./bootstrap.dir $@

../snprintfv/snprintfv.h :
	if [ ! -L ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

../snprintfv/snprintfv/libsnprintfvc.la :
	cd ../snprintfv/snprintfv ; $(MAKE) libsnprintfvc.la

# Makefile.am ends here
