##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am
##
##  Autoopts copyright 1992-2003 Bruce Korb
##
## Time-stamp:      "2003-07-12 10:57:13 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.20 2003/08/21 02:06:09 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

SUBDIRS  = @OPTS_TESTDIR@

DISTCLEANFILES = option*.3 libopts.????
MAINTAINERCLEANFILES = Makefile.in

INCLUDES = @INCLIST@
GENMAN   = optionFree.3 optionLoadLine.3 optionProcess.3 optionRestore.3 \
optionSaveFile.3 optionSaveState.3 optionVersion.3
man_MANS = autoopts-config.1 $(GENMAN)
libsrc   = libopts-@AO_CURRENT@.@AO_REVISION@.@AO_AGE@.tar.gz

nodist_pkgdata_DATA	= $(libsrc)
pkgdata_DATA	= stdoptions.def autoopts.m4 \
    aginfo.tpl agman1.tpl  agman3.tpl aginfo3.tpl \
    options.tpl optcode.tpl opthead.tpl optlib.tpl optmain.tpl

HDRS        = autoopts.h options.h  streqv.h
CSRC        = \
	autoopts.c boolean.c enumeration.c makeshell.c \
	numeric.c  pgusage.c putshell.c    restore.c   \
	save.c     stack.c   streqvcmp.c   usage.c     version.c
ALLSRC		= genshell.c genshell.h $(CSRC)

EXTRA_DIST	= $(pkgdata_DATA) $(ALLSRC) COPYING $(man_MANS) options_h.tpl \
	TODO autoopts.m4 bootstrap.dir autoopts-config.in genshell.def funcs.def

m4datadir	= $(datadir)/aclocal
m4data_DATA	= autoopts.m4
lib_LTLIBRARIES = libopts.la libguileopts.la

##  libopts_la_SOURCES does _NOT_ include $(GENSRC) !!
##  It is compiled as a part of the makeshell module.
##
nodist_libopts_la_SOURCES = libopts.c
libopts_la_SOURCES = $(HDRS)

libopts_la_LIBADD  = ../snprintfv/libsnprintfv.la
libopts_la_LDFLAGS = -version-info @AO_CURRENT@:@AO_REVISION@:@AO_AGE@ -lm

libguileopts_la_SOURCES  = guileopt.c
libguileopts_la_CFLAGS   = $(LIBGUILE_CFLAGS)
libguileopts_la_LDFLAGS  = -version-info @GO_CURRENT@:@GO_REVISION@:@GO_AGE@

bin_SCRIPTS	= autoopts-config

include_HEADERS = options.h

libopts.c : autoopts.h $(CSRC)
	( for f in autoopts.h $(CSRC) ; do echo "#include \"$$f\"" ; done ) > $@

makeshell.lo : genshell.c
genshell.c : genshell.def
	@AGEXE=`cd $(top_builddir)/agen5; pwd`/autogen ; cd $(srcdir) ; \
	  AGEXE="$$AGEXE" $(SHELL) ./bootstrap.dir $@

strcspn.lo : $(top_srcdir)/compat/strcspn.c
	$(LTCOMPILE) -o $@ -c $(top_srcdir)/compat/strcspn.c

../snprintfv/snprintfv.h :
	if [ ! -f ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

../snprintfv/libsnprintfv.la :
	cd ../snprintfv ; $(MAKE) libsnprintfv.la

install-data-local : install-man3

$(GENMAN) : funcs.def
	if [ -x ../agen5/autogen ] ; then \
	echo ../agen5/autogen -Tagman3.tpl funcs.def ; \
	../agen5/autogen -Tagman3.tpl funcs.def ; else true ; fi

libopts.texi : ../agen5/autogen funcs.def
	../agen5/autogen -DLEVEL=subsection -Taginfo3.tpl -b libopts funcs.def

##  "AutoMake" insists on building this before it is ready to be built.
##  Therefore, no-op this (successfully) until the proper time.
##
libsrc : $(libsrc)
$(libsrc) :
	if [ ! -x  $(top_builddir)/columns/columns ] ; then exit 0 ; fi ; \
	top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) ; \
	export top_srcdir top_builddir ; \
	AO_AGE=@AO_AGE@ AO_CURRENT=@AO_CURRENT@ AO_REVISION=@AO_REVISION@ \
	$(SHELL) $(top_srcdir)/pkg/libopts/mklibsrc.sh

# Makefile.am ends here
