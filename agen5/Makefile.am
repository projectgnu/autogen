##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright (C) 1992-2002 Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
## Last Modified:   $Date: 2002/07/11 20:28:44 $
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.13 2002/07/11 20:28:44 bkorb Exp $
## ---------------------------------------------------------------------

SUBDIRS     = test
INCLUDES    = @INCLIST@

GETDEFS     = $(top_builddir)/getdefs/@GDEXE@
AGEXE       = $(top_builddir)/agen5/@AGEXE@

TEMPLATES   = directive.tpl fsm-macro.tpl fsm-trans.tpl fsm.tpl snarf.tpl
DEFINITIONS = cgi.def opts.def pseudo.def
GENDOCS     = autogen.texi autogen.menu autogen.1
CLEANFILES  = $(GENDOCS)
STAMPS      = stamp-directive stamp-exprini stamp-opts stamp-fsm

#  GEN_CSRC omits defParse.c and defParse.h because automake understands
#
GEN_CSRC = opts.c opts.h directive.h expr.ini expr.h proto.h \
	cgi-fsm.c cgi-fsm.h pseudo-fsm.h
GEN_SRC = $(GEN_CSRC) defParse.c defParse.h $(GENDOCS)

pkgdata_DATA = schemedef.scm fsm.tpl fsm-macro.tpl fsm-trans.tpl
man_MANS     = autogen.1

MAINTAINERCLEANFILES = Makefile.in $(STAMPS) $(GEN_SRC)

EXTRA_DIST = $(GEN_CSRC) $(DEFINITIONS) $(TEMPLATES) bootstrap.dir schemedef.scm

bin_PROGRAMS = autogen

autogen_SOURCES = $(GEN_CSRC) \
  agCgi.c       agShell.c     agUtils.c     agUtils.h     autogen.c  \
  autogen.h     defDirect.c   defFind.c     defLex.c      defLoad.c  \
  defParse.y    defReduce.c   expExtract.c  expFormat.c   expGperf.c \
  expGuile.c    expMake.c     expOutput.c   expPrint.c    expState.c \
  expString.c   funcCase.c    funcDef.c     funcEval.c    funcFor.c  \
  funcIf.c      functions.c   functions.h   loadPseudo.c  tpLoad.c   \
  tpParse.c     tpProcess.c

autogen_LDADD	= $(top_builddir)/autoopts/libopts.la
autogen_LDFLAGS = @DYNAMIC_AG@

FUNCLIST =	funcCase.c  funcDef.c   funcEval.c  funcFor.c   funcIf.c \
		functions.c
EXPRLIST =	expFormat.c expGuile.c  expOutput.c expPrint.c  expState.c \
		expString.c expGperf.c expExtract.c $(FUNCLIST)

YFLAGS = -d

all : @AGEXE@

## Funny build rules for extracting definitions.  "Makefile.in"
## should never depend on these, so they must *not* be listed in the
## "BUILT_SOURCES" list.  They are to be rebuilt on explicit request:
## make gen.  ALSO:  keep the "GEN - RULE" comments - used by bootstrap.dir
##
GEN_LIST = $(STAMPS) $(GENDOCS) defParse.c defParse.h
BOOTENV  = top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	srcdir=$(srcdir) builddir=$(builddir) \
	AGEXE=@AGEXE@ GDEXE=@GDEXE@ COLEXE=@COLEXE@

defLex.o : defParse.h

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
stamp-fsm : pseudo.def fsm.tpl
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## proto.h GEN-RULE
proto.h :
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## autogen.texi GEN-RULE
autogen.texi : $(STAMPS) @AGEXE@ ../getdefs/getdefs ../columns/columns
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## autogen.1 GEN-RULE
autogen.1 : opts.def
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## defParse.c GEN-RULE
defParse.c defParse.h : defParse.y
	@$(BOOTENV) YACC="$(YACC) $(YFLAGS)" \
	$(SHELL) $(srcdir)/bootstrap.dir $@

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
