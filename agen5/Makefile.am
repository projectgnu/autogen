##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
## 
##    AutoGen Copyright (C) 1992-1999 Bruce Korb
## 
## Author:          Bruce Korb <autogen@linuxbox.com>
## Maintainer:      Bruce Korb <autogen@linuxbox.com>
## Created:         Mon Jun 30 15:35:12 1997
## Last Modified:   Thu Aug  5 10:18:42 1999				      
##            by:   Bruce Korb <autogen@linuxbox.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.20 2000/03/06 02:03:26 bruce Exp $
## ---------------------------------------------------------------------

SUBDIRS		= test
INCLUDES	= -I$(top_builddir)/autoopts -I$(top_srcdir)/autoopts \
		-I$(top_builddir)/snprintfv -I$(top_srcdir)/snprintfv \
		-I$(top_builddir) -I$(top_srcdir) -I. -I$(srcdir)

BLOCKOPT	= subblock=exparg=arg_name,arg_desc,arg_optional,arg_list
GETDEFS         = $(top_builddir)/getdefs/getdefs
AGEXE		= $(top_builddir)/agen5/AutoGen

AGARGS  	= autogen=$(AGEXE)
OPTDBG		= `if ( echo @CPPFLAGS@|egrep DDEBUG) >/dev/null 2>&1 ; \
		  then echo -DDEBUG=1 ; else echo -UDEBUG ; fi`

MAINTAINERCLEANFILES = Makefile.in

GEN_SRC = opts.c opts.h directive.h functions.h expr.ini expr.h \
	defParse.c defParse.h autogen.1 autogen.menu autogen.texi

pkgdata_DATA	= schemedef.scm

EXTRA_DIST	= directive.tpl functions.tpl opts.def snarf.tpl \
		$(GEN_SRC) bootstrap.dir schemedef.scm

bin_PROGRAMS	= AutoGen
AutoGen_SOURCES =agShell.c  agUtils.c   agUtils.h   autogen.c   \
		autogen.h   defDirect.c defFind.c   defLex.c    defLoad.c \
		defParse.y  defReduce.c directive.h expFormat.c expGuile.c \
		expGuile.h  expOutput.c expPrint.c  expState.c  expString.c \
		expr.ini    funcCase.c  funcDef.c   funcEval.c  funcFor.c \
		funcIf.c    functions.c functions.h opts.c      opts.h \
		proto.h     tpLoad.c    tpParse.c   tpProcess.c

AutoGen_LDADD	= $(top_builddir)/autoopts/libopts.la \
		@GUILE_LINK@

FARGS = base-name=functions template=functions filelist agarg=$(OPTDBG)

FUNCLIST =	funcCase.c  funcDef.c   funcEval.c  funcFor.c   funcIf.c \
		functions.c
EXPRLIST =	expFormat.c expGuile.c  expOutput.c expPrint.c  expState.c \
		expString.c $(FUNCLIST)

YFLAGS = -d

all : AutoGen

clean :
	rm -f *.o AutoGen $(STAMPS) $(GEN_SRC) *~

##  Funny build rules for extracting definitions.  "Makefile.in"
## should never depend on these, so they must *not* be listed in the
## "BUILT_SOURCES" list.  They are to be rebuilt on explicit request:
## make gen.  ALSO:  keep the "GEN - RULE" comments - used by bootstrap.dir
##
STAMPS  = stamp-directive stamp-exprini stamp-functions stamp-opts
GEN_LIST = $(STAMPS) autogen.texi autogen.1 defParse.c

## stamp-opts GEN-RULE
stamp-opts: opts.def $(top_srcdir)/VERSION
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	DEBUG_OPTS=$(OPTDBG) sh $(srcdir)/bootstrap.dir $@

## stamp-directive GEN-RULE
stamp-directive: defDirect.c
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	AUTOGEN_TEMPL_DIRS=$(srcdir) \
	DEBUG_OPTS=$(OPTDBG) sh $(srcdir)/bootstrap.dir $@

## stamp-functions GEN-RULE
stamp-functions: $(FUNCLIST) functions.tpl
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	AUTOGEN_TEMPL_DIRS=$(srcdir) \
	DEBUG_OPTS=$(OPTDBG) sh $(srcdir)/bootstrap.dir $@

## stamp-exprini GEN-RULE
stamp-exprini : $(EXPRLIST) snarf.tpl
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	AUTOGEN_TEMPL_DIRS=$(srcdir) \
	DEBUG_OPTS=$(OPTDBG) sh $(srcdir)/bootstrap.dir $@

## proto.h GEN-RULE
proto.h : defParse.y
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	DEBUG_OPTS=$(OPTDBG) sh $(srcdir)/bootstrap.dir $@

## autogen.texi GEN-RULE
autogen.texi : ./AutoGen ../getdefs/getdefs ../columns/columns
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	DEBUG_OPTS=$(OPTDBG) sh $(srcdir)/bootstrap.dir $@
 
## autogen.1 GEN-RULE
autogen.1 : opts.def
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	DEBUG_OPTS=$(OPTDBG) sh $(srcdir)/bootstrap.dir $@

## defParse.c GEN-RULE
defParse.c defParse.h : defParse.y
	@top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	YACC="$(YACC) $(YFLAGS)" sh $(srcdir)/bootstrap.dir $@

gen : $(GEN_LIST)

../getdefs/getdefs :
	cd ../getdefs ; $(MAKE)

../columns/columns :
	cd ../columns ; $(MAKE)

dist : autogen.texi autogen.1

../snprintfv/snprintfv.h :
	@if [ ! -L ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

# Makefile.am ends here
