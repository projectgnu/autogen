##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen Copyright (C) 1992-1999 Bruce Korb
## 
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Mon Jun 30 15:35:12 1997
## Last Modified:   $Date: 2001/06/24 00:32:52 $
##            by:   Bruce Korb <bkorb@gnu.org>
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.32 2001/06/24 00:32:52 bkorb Exp $
## ---------------------------------------------------------------------

SUBDIRS		= test
INCLUDES = -I. -I$(srcdir) \
	-I$(top_builddir)/autoopts -I$(top_srcdir)/autoopts \
	-I$(top_builddir)/snprintfv -I$(top_srcdir)/snprintfv \
	-I$(top_builddir) -I$(top_srcdir) $(GUILE_INC)

BLOCKOPT	= subblock=exparg=arg_name,arg_desc,arg_optional,arg_list
GETDEFS         = $(top_builddir)/getdefs/getdefs
AGEXE		= $(top_builddir)/agen5/@AGEXE@

AGARGS  	= autogen=$(AGEXE)
OPTDBG		= `if ( echo @CPPFLAGS@|egrep DDEBUG) >/dev/null 2>&1 ; \
		  then echo -DDEBUG=1 ; else echo -UDEBUG ; fi`

MAINTAINERCLEANFILES = Makefile.in $(STAMPS) $(GEN_SRC)

#  GEN_CSRC omits defParse.c and defParse.h because automake understands
#
GEN_CSRC = opts.c opts.h directive.h functions.h expr.ini expr.h proto.h
GEN_SRC = $(GEN_CSRC) defParse.c defParse.h autogen.1 autogen.menu autogen.texi

pkgdata_DATA = schemedef.scm fsm.tpl fsm-macro.tpl fsm-trans.tpl
man_MANS     = autogen.1

EXTRA_DIST	 = directive.tpl functions.tpl opts.def snarf.tpl \
	pseudo.def pseudo-fsm.h $(GEN_SRC) bootstrap.dir $(pkgdata_DATA)

bin_PROGRAMS = autogen

autogen_SOURCES = $(GEN_CSRC) \
        agShell.c   agUtils.c    agUtils.h   autogen.c  autogen.h \
        defDirect.c defFind.c    defLex.c    defLoad.c  defParse.y \
        defReduce.c expExtract.c expFormat.c expGperf.c expGuile.c \
        expOutput.c expPrint.c   expState.c  expString.c \
        funcCase.c  funcDef.c    funcEval.c  funcFor.c  funcIf.c \
        functions.c loadPseudo.c tpLoad.c    tpParse.c  tpProcess.c

autogen_LDADD	= $(top_builddir)/autoopts/libopts.la @GUILE_LINK@

autogen_LDFLAGS = @DYNAMIC_AG@

FARGS = base-name=functions template=functions filelist agarg=$(OPTDBG)

FUNCLIST =	funcCase.c  funcDef.c   funcEval.c  funcFor.c   funcIf.c \
		functions.c
EXPRLIST =	expFormat.c expGuile.c  expOutput.c expPrint.c  expState.c \
		expString.c expGperf.c expExtract.c $(FUNCLIST)

YFLAGS = -d

all : @AGEXE@

##  Funny build rules for extracting definitions.  "Makefile.in"
## should never depend on these, so they must *not* be listed in the
## "BUILT_SOURCES" list.  They are to be rebuilt on explicit request:
## make gen.  ALSO:  keep the "GEN - RULE" comments - used by bootstrap.dir
##
STAMPS   = stamp-directive stamp-exprini stamp-functions stamp-opts stamp-fsm
GEN_LIST = $(STAMPS) autogen.texi autogen.1 defParse.c
BOOTENV  = top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	srcdir=$(srcdir) builddir=$(builddir) DEBUG_OPTS=$(OPTDBG) \
	AGEXE=@AGEXE@ GDEXE=@GDEXE@ COLEXE=@COLEXE@

defLex.o : defParse.h

## stamp-opts GEN-RULE
stamp-opts: opts.def $(top_srcdir)/VERSION
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## stamp-directive GEN-RULE
stamp-directive: defDirect.c
	@$(BOOTENV) $(SHELL) $(srcdir)/bootstrap.dir $@

## stamp-functions GEN-RULE
stamp-functions: $(FUNCLIST) functions.tpl
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

check : $(GEN_LIST)
gen : $(GEN_LIST)

../getdefs/getdefs :
	cd ../getdefs ; $(MAKE) getdefs

../columns/columns :
	cd ../columns ; $(MAKE) columns

dist : autogen.texi autogen.1

../snprintfv/snprintfv.h :
	@if [ ! -L ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

# Makefile.am ends here
