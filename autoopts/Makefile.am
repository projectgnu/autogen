##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##  Autoopts copyright 1992-2001 Bruce Korb
## 
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997                                  
## Last Modified:   $Date: 2001/12/09 19:43:58 $
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.0 2001/12/09 19:43:58 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

SUBDIRS		= test

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = @INCLIST@ $(GUILE_INC)

pkgdata_DATA	= \
        aginfo.tpl    agman1.tpl    optcode.tpl   opthead.tpl   options.tpl  \
        optlib.tpl    optmain.tpl   stdoptions.def

GENSRC		= genshell.c genshell.h
EXTRA_DIST	= $(pkgdata_DATA)  $(GENSRC) COPYING \
	TODO autoopts.m4 bootstrap.dir autoopts-config.in genshell.def

m4datadir	= $(datadir)/aclocal
m4data_DATA	= autoopts.m4

lib_LTLIBRARIES = libopts.la libguileopts.la

##  libopts_la_SOURCES does _NOT_ include $(GENSRC) !!
##  It is compiled as a part of the makeshell module.
##
libopts_la_SOURCES = \
	autoopts.c   autoopts.h   boolean.c   enumeration.c  \
	makeshell.c  numeric.c    options.h   pgusage.c      \
	putshell.c   save.c       stack.c     streqv.h       \
	streqvcmp.c  usage.c      version.c

libopts_la_LIBADD = ../snprintfv/snprintfv/libsnprintfvc.la @COMPATOBJ@
libopts_la_LDFLAGS = -version-info @AO_CURRENT@:@AO_REVISION@:@AO_AGE@

libguileopts_la_SOURCES = guileopt.c
libguileopts_la_LDFLAGS = -version-info @GO_CURRENT@:@GO_REVISION@:@GO_AGE@

bin_SCRIPTS	= autoopts-config

include_HEADERS = options.h

makeshell.lo : genshell.c @COMPATOBJ@
genshell.c : genshell.def
	@AGEXE=`cd $(top_builddir)/agen5; pwd`/autogen ; cd $(srcdir) ; \
	  AGEXE="$$AGEXE" $(SHELL) ./bootstrap.dir $@

strcspn.lo : $(top_srcdir)/compat/strcspn.c
	$(LTCOMPILE) -o $@ -c $(top_srcdir)/compat/strcspn.c

pathfind.lo : $(top_srcdir)/compat/pathfind.c
	$(LTCOMPILE) -o $@ -c $(top_srcdir)/compat/pathfind.c

../snprintfv/snprintfv.h :
	if [ ! -L ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

../snprintfv/snprintfv/libsnprintfvc.la :
	cd ../snprintfv/snprintfv ; $(MAKE) libsnprintfvc.la

# Makefile.am ends here
