##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2004-07-30 20:51:38 bkorb"
## Copyright (c):   1992-2004 by Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.35 2004/07/31 18:51:31 bkorb Exp $
## ---------------------------------------------------------------------

SUBDIRS         = test
INCLUDES        = @INCLIST@

GETDEFS         = $(top_builddir)/getdefs/@GDEXE@
AGEXE           = $(top_builddir)/agen5/@AGEXE@
COLEXE          = $(top_builddir)/columns/@COLEXE@

TEMPLATES       = directive.tpl fsm-macro.tpl fsm-trans.tpl fsm.tpl snarf.tpl
DEFINITIONS     = cgi.def opts.def pseudo.def defParse.def
GENDOCS         = autogen.texi autogen.menu autogen.1
CLEANFILES      = $(GENDOCS)
STAMPS          = stamp-opts stamp-directive stamp-exprini stamp-fsm stamp-parse
GEN_CSRC        = opts.h directive.h expr.ini expr.h proto.h pseudo-fsm.h
GEN_SRC         = $(GEN_CSRC) $(GENDOCS)

pkgdata_DATA    = fsm.tpl fsm-macro.tpl fsm-trans.tpl
man_MANS        = autogen.1

DISTCLEANFILES  = $(STAMPS) ag.c
MAINTAINERCLEANFILES = Makefile.in $(GEN_SRC)

bin_PROGRAMS    = autogen

csrc = \
    autogen.c   agCgi.c     agInit.c       agShell.c  cgi-fsm.c    \
    defDirect.c agUtils.c   defParse-fsm.c defLex.c   defLoad.c    \
    defFind.c   expFormat.c expGperf.c     expGuile.c expExtract.c \
    expOutput.c expPrint.c  expState.c     expMake.c  fmemopen.c   \
    funcCase.c  funcDef.c   expString.c    funcFor.c  funcIf.c     \
    functions.c funcEval.c  loadPseudo.c   opts.c     tpLoad.c     \
    tpProcess.c tpParse.c

hsrc = autogen.h defParse-fsm.h functions.h cgi-fsm.h

EXTRA_DIST = $(csrc) $(hsrc) $(DEFINITIONS) $(TEMPLATES) \
             bootstrap.dir schemedef.scm

nodist_autogen_SOURCES = ag.c
autogen_SOURCES = $(GEN_CSRC)
autogen_LDADD   = $(top_builddir)/autoopts/libopts.la $(LIBGUILE_LIBS)
autogen_LDFLAGS = $(DYNAMIC_AG)
autogen_CFLAGS  = $(LIBGUILE_CFLAGS)
FUNCLIST        = \
	funcCase.c funcDef.c funcEval.c funcFor.c funcIf.c functions.c
EXPRLIST        = \
	expFormat.c expGuile.c expOutput.c  expPrint.c expState.c \
	expString.c expGperf.c expExtract.c $(FUNCLIST)

AM_YFLAGS = -d

all : @AGEXE@

ag.c : $(csrc) $(hsrc)
	( echo '#define DEFINING' ; echo '#include "autogen.h"' ; \
	for f in $(csrc) ; do echo "#include \"$$f\"" ; done ) > $@

SUFFIXES = .i
.c.i :
	$(COMPILE) -E $< > $@

## Funny build rules for extracting definitions.  "Makefile.in"
## should never depend on these, so they must *not* be listed in the
## "BUILT_SOURCES" list.  They are to be rebuilt on explicit request:
## make gen.  ALSO:  keep the "GEN - RULE" comments - used by bootstrap.dir
##
GEN_LIST = $(STAMPS) $(GENDOCS)
BOOTENV  = top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	srcdir=$(srcdir) builddir=$(builddir) \
	PATH=`cd ../columns >/dev/null && pwd`:$$PATH \
	AGEXE=$(AGEXE) GDEXE=$(GDEXE) COLEXE=$(COLEXE)

## stamp-opts GEN-RULE
stamp-opts: opts.def $(top_srcdir)/VERSION
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## stamp-directive GEN-RULE
stamp-directive: defDirect.c
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## stamp-exprini GEN-RULE
stamp-exprini : $(EXPRLIST) snarf.tpl
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## stamp-fsm GEN-RULE
stamp-fsm : pseudo.def fsm.tpl fsm-macro.tpl fsm-trans.tpl
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## stamp-parse GEN-RULE
stamp-parse : defParse.def fsm.tpl fsm-macro.tpl fsm-trans.tpl
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## proto.h GEN-RULE
proto.h :
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## autogen.texi GEN-RULE
autogen.texi : $(STAMPS) @AGEXE@ ../getdefs/getdefs ../columns/columns
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## autogen.1 GEN-RULE
autogen.1 : opts.def @AGEXE@ ../getdefs/getdefs ../columns/columns
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

gen : $(GEN_LIST)

../getdefs/getdefs :
	cd ../getdefs ; $(MAKE) getdefs

../columns/columns :
	cd ../columns ; $(MAKE) columns

dist : autogen.texi autogen.1

../snprintfv/snprintfv.h :
	@if [ ! -f ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

# Makefile.am ends here
