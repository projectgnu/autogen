##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am
##
## Time-stamp:      "2005-02-10 18:41:01 bkorb"
## Copyright (c):   1992-2005 by Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.11 2005/02/13 01:47:59 bkorb Exp $
## ---------------------------------------------------------------------
## Code:

SUBDIRS     = @OPTS_TESTDIR@
INCLUDES    = @INCLIST@

if  NEED_PATHFIND
PATHFIND_MAN= pathfind.3
else
PATHFIND_MAN=
endif

GENMAN      = $(PATHFIND_MAN) \
    configFileLoad.3  optionFree.3      optionLoadLine.3  optionProcess.3 \
    optionRestore.3   optionSaveFile.3  optionSaveState.3 optionVersion.3 \
    strequate.3       streqvcmp.3       streqvmap.3       string_tokenize.3 \
    strneqvcmp.3      strtransform.3    text_mmap.3       text_munmap.3
MAN_STAMP   = man3-stamp

GENTEXI     = libopts.texi libopts.menu
TEXI_STAMP  = texi-stamp

libsrc      = libopts-@AO_CURRENT@.@AO_REVISION@.@AO_AGE@.tar.gz

GENHDRS     = autoopts/options.h autoopts/usage-txt.h genshell.h
HDRS        = $(GENHDRS) autoopts.h proto.h

## The primary source (autoopts.c) must be listed first and on a separate
## line.  'sed' does some magic here to get the list of source files for the
## documentation.
##
CSRC        = \
    autoopts.c  \
    boolean.c   configfile.c enumeration.c environment.c genshell.c  \
    load.c      makeshell.c  numeric.c     pgusage.c     putshell.c  \
    restore.c   save.c       sort.c        stack.c       streqvcmp.c \
    text_mmap.c tokenize.c   usage.c       version.c

STAMP_FILES = $(MAN_STAMP) $(TEXI_STAMP)
LIBOPTS_VER = @AO_CURRENT@:@AO_REVISION@:@AO_AGE@
LGUILE_VER  = @GO_CURRENT@:@GO_REVISION@:@GO_AGE@

CLIENT_TPLS = stdoptions.def rc-sample.tpl \
    aginfo.tpl  aginfo3.tpl agman1.tpl  agman3.tpl getopt.tpl  \
    optcode.tpl opthead.tpl options.tpl optlib.tpl optmain.tpl
PRIVATE_TPL = options_h.tpl usage-txt.tpl
TPL_FILES   = $(CLIENT_TPLS) $(PRIVATE_TPL)
DEF_FILES   = genshell.def stdoptions.def usage-txt.def funcs.def
MISC_FILES  = autoopts.m4 bootstrap.dir autoopts-config.in COPYING
pkgconfigdir=$(libdir)/pkgconfig

## # # # # # # # # # # # # # #
##
##   A U T O M A K E   V A R S
##
## # # # # # # # # # # # # # #

CLEANFILES                = tmp-* libopts.c
DISTCLEANFILES            = $(GENMAN) $(GENTEXI) $(STAMP_FILES)
MAINTAINERCLEANFILES      = Makefile.in $(GENHDRS) funcs.def

man_MANS                  = autoopts-config.1 $(GENMAN)
m4datadir                 = $(datadir)/aclocal
m4data_DATA               = autoopts.m4

nodist_pkgdata_DATA       = $(libsrc)
pkgconfig_SCRIPTS         = autoopts.pc
pkgdata_DATA              = autoopts.m4 $(CLIENT_TPLS)

lib_LTLIBRARIES           = libopts.la libguileopts.la

nodist_libopts_la_SOURCES = libopts.c
libopts_la_SOURCES        = $(HDRS)
libopts_la_LIBADD         = ../snprintfv/libsnprintfv.la
libopts_la_LDFLAGS        = -version-info $(LIBOPTS_VER)

libguileopts_la_SOURCES   = guileopt.c
libguileopts_la_CFLAGS    = $(LIBGUILE_CFLAGS)
libguileopts_la_LIBADD    = libopts.la
libguileopts_la_LDFLAGS   = -version-info $(LGUILE_VER)

bin_SCRIPTS               = autoopts-config

nobase_include_HEADERS    = autoopts/options.h autoopts/usage-txt.h

EXTRA_DIST = $(HDRS) $(CSRC) $(pkgdata_DATA) $(man_MANS) $(TPL_FILES) \
	$(MISC_FILES) $(DEF_FILES) mk-autoopts-pc.in

## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
##
##   M A K E F I L E   R U L E S
##
## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

libopts.c : $(HDRS) $(CSRC)
	( echo '#define AUTOOPTS_INTERNAL' ; \
	for f in $(HDRS) $(CSRC) ; do echo "#include \"$$f\"" ; done ) > $@

makeshell.lo : genshell.c
genshell.c : genshell.def
	@cd $(srcdir) ; AGexe=$(AGexe) $(SHELL) ./bootstrap.dir $@

strcspn.lo : $(top_srcdir)/compat/strcspn.c
	$(LTCOMPILE) -o $@ -c $(top_srcdir)/compat/strcspn.c

../snprintfv/snprintfv.h :
	if [ ! -f ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

../snprintfv/libsnprintfv.la :
	cd ../snprintfv ; $(MAKE) libsnprintfv.la

install-data-local : install-man3

$(GENMAN) : $(MAN_STAMP)
$(MAN_STAMP) : funcs.def
	@if [ -x ../agen5/autogen ] ; then \
	touch tmp-$@ ; \
	echo ! $(AGexe) -Tagman3.tpl funcs.def ; \
	$(AGexe) -Tagman3.tpl funcs.def ; \
	mv -f tmp-$@ $@ ; else true ; fi

$(GENTEXI) : $(TEXI_STAMP)
$(TEXI_STAMP) : ../agen5/autogen funcs.def
	@touch tmp-$@ ; \
	cmd="$(AGexe) -DLEVEL=subsection -Taginfo3.tpl -blibopts funcs.def" ; \
	echo ! $$cmd ; $$cmd ; mv -f tmp-$@ $@

gettext po      : usage-txt.po
usage-txt.po    : autoopts/usage-txt.h
	gt=`which xgettext | sed 's,.* ,,'` ; \
	test -x $${gt} || exit 0 ; \
	opts='--omit-header -a -s ' ; \
	$${gt} -o $@ $${opts} autoopts/usage-txt.h

libsrc : $(libsrc)
$(libsrc) :
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) ; \
	export top_srcdir top_builddir ; \
	AO_AGE=@AO_AGE@ AO_CURRENT=@AO_CURRENT@ AO_REVISION=@AO_REVISION@ \
	$(SHELL) $(top_srcdir)/pkg/libopts/mklibsrc.sh

autoopts.pc : mk-autoopts-pc
	$(SHELL) mk-autoopts-pc $@

# Makefile.am ends here
