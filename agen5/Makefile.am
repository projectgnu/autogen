##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
##    AutoGen copyright (C) 1992-2003 Bruce Korb
##
## Time-stamp:      "2003-05-16 12:14:54 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.24 2003/05/18 17:13:43 bkorb Exp $
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

pkgdata_DATA = fsm.tpl fsm-macro.tpl fsm-trans.tpl
man_MANS     = autogen.1

MAINTAINERCLEANFILES = Makefile.in $(STAMPS) $(GEN_SRC)

bin_PROGRAMS = autogen

csrc = \
  autogen.c     agCgi.c       agShell.c     agUtils.c     agUtils.h  \
  autogen.h     defDirect.c   defFind.c     defLex.c      defLoad.c  \
  defParse.y    defReduce.c   expExtract.c  expFormat.c   expGperf.c \
  expGuile.c    expMake.c     expOutput.c   expPrint.c    expState.c \
  expString.c   funcCase.c    funcDef.c     funcEval.c    funcFor.c  \
  funcIf.c      functions.c   functions.h   loadPseudo.c  tpLoad.c   \
  tpParse.c     tpProcess.c

EXTRA_DIST = $(csrc) $(DEFINITIONS) $(TEMPLATES) bootstrap.dir schemedef.scm

nodist_autogen_SOURCES = ag.c
autogen_SOURCES = defParse.y $(GEN_CSRC)
autogen_LDADD   = $(top_builddir)/autoopts/libopts.la $(LIBGUILE_LIBS)
autogen_LDFLAGS = $(DYNAMIC_AG)
autogen_CFLAGS  = $(LIBGUILE_CFLAGS)
FUNCLIST =	funcCase.c  funcDef.c   funcEval.c  funcFor.c   funcIf.c \
		functions.c
EXPRLIST =	expFormat.c expGuile.c  expOutput.c expPrint.c  expState.c \
		expString.c expGperf.c expExtract.c $(FUNCLIST)

AM_YFLAGS = -d

all : @AGEXE@

ag.c : $(csrc)
	( echo '#define DEFINING' ; echo '#include "autogen.h"' ; \
	for f in $(csrc) ; do case "$$f" in \
	  *.c ) echo "#include \"$$f\"" ;; \
	  esac ; done ) > $@

##  This is a LIE:  only defLex.o depends upon defParse.h.  However, automake
##  is unable to cope with that and they don't provide another way to do it.
##
$(autogen_OBJECTS) : defParse.h defParse.c

SUFFIXES = .i
.c.i :
	$(COMPILE) -E $< > $@

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
