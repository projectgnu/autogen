##  -*- Mode: Makefile -*-
##
## Author:          Bruce Korb <bkorb@gnu.org>
##
##  This file is part of AutoOpts, a companion to AutoGen.
##  AutoOpts is free software.
##  AutoOpts is Copyright (C) 1992-2014 by Bruce Korb - all rights reserved
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
##  These files have the following sha256 sums:
##
##  8584710e9b04216a394078dc156b781d0b47e1729104d666658aecef8ee32e95  COPYING.gplv3
##  4379e7444a0e2ce2b12dd6f5a52a27a4d02d39d247901d3285c88cf0d37f477b  COPYING.lgplv3
##  13aa749a5b0a454917a944ed8fffc530b784f5ead522b1aacaf4ec8aa55a6239  COPYING.mbsd

BUILT_SOURCES  	    =
MOSTLYCLEANFILES    =

SUBDIRS         = @OPTS_TESTDIR@
AM_CPPFLAGS     = @INCLIST@
libsrc          = libopts-@AO_CURRENT@.@AO_REVISION@.@AO_AGE@.tar.gz
LIBOPTS_VER     = @AO_CURRENT@:@AO_REVISION@:@AO_AGE@

if  NEED_PATHFIND
PATHFIND_MAN    = pathfind.3
else
PATHFIND_MAN    =
endif

MAN_STAMP       = man3-stamp
GENTEXI         = libopts.texi libopts.menu
TEXI_STAMP      = texi-stamp
EXTRA_DIST      =
GENMAN          = $(PATHFIND_MAN) \

GENHDRS         = autoopts/options.h autoopts/usage-txt.h genshell.h \
		option-xat-attribute.h option-value-type.h ao-strs.h \
		ag-char-map.h
HDRS            = $(GENHDRS) autoopts.h project.h proto.h
GEN_SRC         = ao-strs.c option-value-type.c option-xat-attribute.c

## The primary source (autoopts.c) must be by itself on the first line after
## "CSRC".  'sed' does some magic here to get the list of source files for the
## documentation.  Files without documentation are on the CSRC = line.
##
CSRC            = parse-duration.c $(GEN_SRC) \
    autoopts.c \
    alias.c    boolean.c   check.c     configfile.c cook.c \
    enum.c     env.c       file.c      find.c       genshell.c \
    load.c     makeshell.c nested.c    numeric.c    pgusage.c \
    putshell.c reset.c     restore.c   save.c       sort.c \
    stack.c    streqvcmp.c text_mmap.c time.c       tokenize.c \
    usage.c    version.c   init.c

SRC             = $(HDRS) $(CSRC) $(GENSRC)
DEF_FILES       = genshell.def $(srcdir)/funcs.def ao-strs.def
pkgconfigdir    =$(libdir)/pkgconfig

## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
##
##   A U T O M A K E   V A R S
##
## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

CLEANFILES                = tmp-* libopts.c
DISTCLEANFILES            = $(GENMAN) $(GENTEXI) *-stamp
MAINTAINERCLEANFILES      = $(GENHDRS) $(GENSRC) $(GENSCRIPTS)
m4datadir                 = $(datadir)/aclocal

nodist_libopts_la_SOURCES = libopts.c
libopts_la_SOURCES        = $(HDRS)
libopts_la_CFLAGS         = -DPKGDATADIR='"$(pkgdatadir)"'
libopts_la_LDFLAGS        = -version-info $(LIBOPTS_VER)
libopts_la_LIBADD         = $(top_builddir)/snprintfv/libsnprintfv.la

EXTRA_DIST += $(SRC) $(EXTRA_DATA) $(man_MANS) $(DEF_FILES)

INST_MANS                 = autoopts-config.1 $(GENMAN)
INST_PKGCFG               = pkgconfig/autoopts.pc
INST_PKGDATA              = autoopts.m4 $(TPL_FILES)
INST_LIBS                 = libopts.la
INST_HDRS                 = autoopts/options.h autoopts/usage-txt.h
INST_SH                   = autoopts-config

man_MANS                  = $(INST_MANS)
m4data_DATA               = autoopts.m4
nobase_data_DATA          = $(INST_PKGCFG)
lib_LTLIBRARIES           = $(INST_LIBS)
nobase_include_HEADERS    = $(INST_HDRS)
bin_SCRIPTS               = $(INST_SH)

BOOTENV         = AGexe="$(AGexe)" GDexe="$(GDexe)" CLexe="$(CLexe)" \
	srcdir="$(srcdir)"     top_srcdir="$(top_srcdir)" \
	builddir="$(builddir)" top_builddir="$(top_builddir)" \
	POSIX_SHELL="$(POSIX_SHELL)"

## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
##
##   M A K E F I L E   R U L E S
##
## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

libopts.c       : tpl-config-stamp
$(GENSCRIPTS)   : tpl-config-stamp

tpl-config-stamp: $(HDRS) $(CSRC) \
	    $(top_builddir)/config.h $(srcdir)/mk-tpl-config.sh
	$(BOOTENV) \
	    $(POSIX_SHELL) $(srcdir)/mk-tpl-config.sh $@ $(HDRS) $(CSRC)

makeshell.lo    : genshell.c
genshell.c      : $(srcdir)/genshell.def
	$(BOOTENV) \
	    $(POSIX_SHELL) $(srcdir)/bootstrap.dir $@

strcspn.lo      : $(top_srcdir)/compat/strcspn.c
	$(LTCOMPILE) -o $@ -c $(top_srcdir)/compat/strcspn.c

install-data-local : install-man3

$(GENMAN)       : $(MAN_STAMP)
$(MAN_STAMP)    : $(srcdir)/funcs.def
	@test -x ../agen5/autogen || exit 0 ; \
	touch tmp-$@ ; \
	opts='-L$(srcdir)/tpl -L$(builddir)/tpl -Tagman3.tpl' ; \
	echo ! $(AGexe) $${opts} $(srcdir)/funcs.def ; \
	$(AGexe) $${opts} $(srcdir)/funcs.def ; \
	mv -f tmp-$@ $@

$(GENTEXI)      : $(TEXI_STAMP)
$(TEXI_STAMP)   : ../agen5/autogen $(srcdir)/funcs.def
	@touch tmp-$@ ; \
	opts='-L$(srcdir)/tpl -L$(builddir)/tpl -Taginfo3.tpl' ; \
	cmd="$(AGexe) $${opts} -DLEVEL=subsection -blibopts" ; \
	cmd="$${cmd} -L$(srcdir) $(srcdir)/funcs.def" ; \
	echo ! $$cmd ; $$cmd ; mv -f tmp-$@ $@

libsrc          : $(libsrc)
$(libsrc)       :
	@$(BOOTENV) \
	  AO_AGE=@AO_AGE@ AO_CURRENT=@AO_CURRENT@ AO_REVISION=@AO_REVISION@ \
	  $(POSIX_SHELL) $(top_srcdir)/pkg/libopts/mklibsrc.sh

pkgconfig/autoopts.pc : mk-autoopts-pc
	$(POSIX_SHELL) mk-autoopts-pc $@

install-data-hook:
	@DESTdestdir='$(DESTDIR)$(includedir)/autoopts' \
	DESTpkgdatadir='$(DESTDIR)$(pkgdatadir)' \
	top_builddir='$(top_builddir)' \
	LIBOPTS_VER='$(LIBOPTS_VER)' \
	POSIX_SHELL='$(POSIX_SHELL)'  \
	bindir='$(bindir)' \
	$(POSIX_SHELL) $(srcdir)/install-hook.sh

.NOTPARALLEL:

if GL_GENERATE_STDNORETURN_H
$(STDNORETURN_H) : libguile/__scm.h
libguile/__scm.h :
	$(SHELL) $(srcdir)/fix-guile.sh $(LIBGUILE_CFLAGS)
endif

# Makefile.am ends here
