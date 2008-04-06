##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am
##
## Time-stamp:      "2008-04-06 12:45:12 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
##
##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is copyright (c) 1992-2007 by Bruce Korb - all rights reserved
##
##  AutoOpts is available under any one of two licenses.  The license
##  in use must be one of these two and the choice is under the control
##  of the user of the license.
##
##   The GNU Lesser General Public License, version 3 or later
##      See the files "COPYING.lgplv3" and "COPYING.gplv3"
##
##   The Modified Berkeley Software Distribution License
##      See the file "COPYING.mbsd"
##
##  These files have the following md5sums:
##
##  239588c55c22c60ffe159946a760a33e pkg/libopts/COPYING.gplv3
##  fa82ca978890795162346e661b47161a pkg/libopts/COPYING.lgplv3
##  66a5cedaf62c4b2637025f049f9b826f pkg/libopts/COPYING.mbsd
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.40 2008/04/06 22:46:05 bkorb Exp $
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
    ao_string_tokenize.3  configFileLoad.3      optionFileLoad.3 \
    optionFindNextValue.3 optionFindValue.3     optionFree.3 \
    optionGetValue.3      optionLoadLine.3      optionNextValue.3 \
    optionOnlyUsage.3     optionProcess.3       optionRestore.3 \
    optionSaveFile.3      optionSaveState.3     optionUnloadNested.3 \
    optionVersion.3       strequate.3           streqvcmp.3 \
    streqvmap.3           strneqvcmp.3          strtransform.3
MAN_STAMP   = man3-stamp

GENTEXI     = libopts.texi libopts.menu
TEXI_STAMP  = texi-stamp

libsrc      = libopts-@AO_CURRENT@.@AO_REVISION@.@AO_AGE@.tar.gz

GENHDRS     = autoopts/options.h autoopts/usage-txt.h genshell.h \
	xat-attribute.h value-type.h ag-char-map.h
HDRS        = $(GENHDRS) autoopts.h proto.h
GEN_SRC     = value-type.c xat-attribute.c

## The primary source (autoopts.c) must be listed first and on a separate
## line.  'sed' does some magic here to get the list of source files for the
## documentation.
##
CSRC        = $(GEN_SRC) \
    autoopts.c  \
    boolean.c configfile.c cook.c      enumeration.c environment.c \
    file.c    genshell.c   load.c      makeshell.c   nested.c numeric.c \
    pgusage.c putshell.c   restore.c   save.c        sort.c        \
    stack.c   streqvcmp.c  text_mmap.c tokenize.c    usage.c version.c

SRC         = $(HDRS) $(CSRC) $(GENSRC)

STAMP_FILES = $(MAN_STAMP) $(TEXI_STAMP)
LIBOPTS_VER = @AO_CURRENT@:@AO_REVISION@:@AO_AGE@
LGUILE_VER  = @GO_CURRENT@:@GO_REVISION@:@GO_AGE@

OPTCODE_TPL = options.tpl optcode.tpl opthead.tpl optlib.tpl optmain.tpl
CLIENT_TPLS = stdoptions.def rc-sample.tpl agman-lib.tpl \
    aginfo.tpl  aginfo3.tpl agman1.tpl  agman3.tpl getopt.tpl usage.tpl

TPL_FILES   = $(CLIENT_TPLS) $(OPTCODE_TPL)
DEF_FILES   = genshell.def stdoptions.def $(srcdir)/funcs.def
MISC_FILES  = autoopts.m4 bootstrap.dir autoopts-config.in
pkgconfigdir=$(libdir)/pkgconfig

## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
##
##   A U T O M A K E   V A R S
##
## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

CLEANFILES                = tmp-* libopts.c
DISTCLEANFILES            = $(GENMAN) $(GENTEXI) $(STAMP_FILES)
MAINTAINERCLEANFILES      = $(GENHDRS) $(GENSRC) $(srcdir)/funcs.def
m4datadir                 = $(datadir)/aclocal

nodist_libopts_la_SOURCES = libopts.c
libopts_la_SOURCES        = $(HDRS)
libopts_la_CFLAGS         = -DPKGDATADIR='"$(pkgdatadir)"'
libopts_la_LDFLAGS        = -version-info $(LIBOPTS_VER)
libopts_la_LIBADD         = $(top_builddir)/snprintfv/libsnprintfv.la

libguileopts_la_SOURCES   = guileopt.c
libguileopts_la_CFLAGS    = $(LIBGUILE_CFLAGS)
libguileopts_la_LIBADD    = libopts.la
libguileopts_la_LDFLAGS   = -version-info $(LGUILE_VER)

EXTRA_DIST = $(SRC) $(pkgdata_DATA) $(man_MANS) \
	$(MISC_FILES) $(DEF_FILES) install-options-h mk-autoopts-pc.in po

INST_MANS                 = autoopts-config.1 $(GENMAN)
INST_M4DATA               = autoopts.m4
INST_ND_PKGDATA           = $(libsrc)
INST_PKGCFG               = autoopts.pc
INST_PKGDATA              = autoopts.m4 $(TPL_FILES)
INST_LIBS                 = libopts.la libguileopts.la
INST_HDRS                 = autoopts/options.h autoopts/usage-txt.h
INST_SH                   = autoopts-config

man_MANS                  = $(INST_MANS)
m4data_DATA               = $(INST_M4DATA)
nodist_pkgdata_DATA       = $(INST_ND_PKGDATA)
pkgconfig_DATA            = $(INST_PKGCFG)
pkgdata_DATA              = $(INST_PKGDATA)
lib_LTLIBRARIES           = $(INST_LIBS)
nobase_include_HEADERS    = $(INST_HDRS)
bin_SCRIPTS               = $(INST_SH)


## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
##
##   M A K E F I L E   R U L E S
##
## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

libopts.c : $(HDRS) $(CSRC)
	( echo '#define AUTOOPTS_INTERNAL' ; \
	echo '#include "compat/compat.h"' ; \
	echo '#define LOCAL static' ; \
	for f in $(HDRS) $(CSRC) ; do echo "#include \"$$f\"" ; done ) > $@

makeshell.lo : genshell.c
genshell.c : genshell.def
	@cd $(srcdir) ; AGexe=$(AGexe) $(SHELL) ./bootstrap.dir $@

strcspn.lo : $(top_srcdir)/compat/strcspn.c
	$(LTCOMPILE) -o $@ -c $(top_srcdir)/compat/strcspn.c

install-data-local : install-man3

$(GENMAN) : $(MAN_STAMP)
$(MAN_STAMP) : $(srcdir)/funcs.def
	@if [ -x ../agen5/autogen ] ; then \
	touch tmp-$@ ; \
	echo ! $(AGexe) -Tagman3.tpl $(srcdir)/funcs.def ; \
	$(AGexe) -Tagman3.tpl $(srcdir)/funcs.def ; \
	mv -f tmp-$@ $@ ; else true ; fi

$(GENTEXI) : $(TEXI_STAMP)
$(TEXI_STAMP) : ../agen5/autogen $(srcdir)/funcs.def
	@touch tmp-$@ ; \
	cmd="$(AGexe) -DLEVEL=subsection -Taginfo3.tpl -blibopts " ; \
	cmd="$${cmd} -L$(srcdir) $(srcdir)/funcs.def" ; \
	echo ! $$cmd ; $$cmd ; mv -f tmp-$@ $@

libsrc : $(libsrc)
$(libsrc) :
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) ; \
	export top_srcdir top_builddir ; \
	AGexe="$(AGexe)" CLexe="$(CLexe)" \
	AO_AGE=@AO_AGE@ AO_CURRENT=@AO_CURRENT@ AO_REVISION=@AO_REVISION@ \
	$(SHELL) $(top_srcdir)/pkg/libopts/mklibsrc.sh

autoopts.pc : mk-autoopts-pc
	$(SHELL) mk-autoopts-pc $@

install-data-hook:
	@destdir='$(DESTDIR)$(includedir)/autoopts' \
	pkgdatadir='$(DESTDIR)$(pkgdatadir)' \
	top_builddir='$(top_builddir)' \
	LIBOPTS_VER='$(LIBOPTS_VER)' \
	conf_shell='$(conf_shell)'  \
	$(SHELL) $(srcdir)/install-options-h $(TPL_FILES)

libopts.i : libopts.o
	$(COMPILE) -E libopts.c > libopts-lines.i
	egrep -v $$'^[ \t]*(#|$$)' libopts-lines.i > $@

.NOTPARALLEL:

# Makefile.am ends here
