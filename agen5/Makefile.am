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
## $Id: Makefile.am,v 1.2 1999/10/29 02:43:57 bruce Exp $
## ---------------------------------------------------------------------

INCLUDES	= -I$(top_builddir)/autoopts -I$(top_srcdir)/autoopts \
		-I$(top_builddir)/snprintfv -I$(top_srcdir)/snprintfv \
		-I$(top_builddir) -I$(top_srcdir) -I. -I$(srcdir)

GETDEFS         = $(top_builddir)/getdefs/getdefs
COLUMNS         = $(top_builddir)/columns/columns
AGEXE		= $(top_builddir)/agen5/AutoGen

AGARGS  	= autogen=$(AGEXE)
OPTDBG		= `if ( echo @CPPFLAGS@|egrep DDEBUG) >/dev/null 2>&1 ; \
		  then echo -DDEBUG=1 ; fi`

TPLDIRS	= -L$(top_srcdir)/autoopts -L$(top_srcdir)/agen5 -L$(top_srcdir)/compat

MAINTAINERCLEANFILES = Makefile.in

STAMPS  = stamp-directive stamp-exprini stamp-functions stamp-opts
GEN_SRC = opts.c opts.h directive.h functions.h expr.def expr.ini \
	defParse.c defParse.h autogen.1 autogen.menu autogen.texi

EXTRADIST = directive.tpl expr.def functions.tpl opts.def snarf.tpl \
	$(GEN_SRC)

bin_PROGRAMS	= AutoGen
AutoGen_SOURCES =agShell.c  agUtils.c   agUtils.h   autogen.c   \
		autogen.h   defDirect.c defFind.c   defLex.c    defLoad.c \
		defParse.y  defReduce.c directive.h expFormat.c expGuile.c \
		expGuile.h  expOutput.c expPrint.c  expState.c  expString.c \
		expr.ini    funcCase.c  funcDef.c   funcEval.c  funcFor.c \
		funcIf.c    functions.c functions.h opts.c      opts.h \
		proto.h     tpLoad.c    tpParse.c   tpProcess.c

AutoGen_LDADD	= $(top_builddir)/autoopts/libopts.la \
		/usr/local/lib/libguile.a -ldl -lm -lc

CFLAGS  = $(OPTDBG) -g

FARGS = base-name=functions template=functions filelist agarg=$(OPTDBG)

FUNCLIST =	funcCase.c  funcDef.c   funcEval.c  funcFor.c   funcIf.c \
		functions.c
EXPRLIST =	expFormat.c expGuile.c  expOutput.c expPrint.c  expState.c \
		expString.c $(FUNCLIST)

all : AutoGen

clean :
	rm -f *.o AutoGen $(STAMPS) $(GEN_SRC) *~

##  Funny build rules for extracting definitions.
##  "Makefile.in" should never depend on these,
##  so they must *not* be listed in the "BUILT_SOURCES" list.
##  They are to be rebuilt on explicit request:  make gen
##
stamp-opts: opts.def $(top_srcdir)/VERSION
	top_srcdir=$(top_srcdir) top_builddir=$(top_builddir) \
	HOME=/dev/null $(AGEXE) $(OPTDBG) -L ../autoopts opts.def
	@date > $@

stamp-directive:
	HOME=/dev/null $(GETDEFS) $(AGARGS) \
		base-name=directive template=directive defDirect.c
	@date > $@

stamp-functions: $(FUNCLIST) functions.tpl
	HOME=/dev/null $(GETDEFS) $(AGARGS) $(FARGS) defs=macfunc func*.c
	@date > $@

expr.def : $(EXPRLIST)
	@echo $(GETDEFS) assign=group=ag defs=gfunc templ=snarf \
		out=expr.def srcfile $(EXPRLIST)
	@res=`$(GETDEFS) assign=group=ag defs=gfunc templ=snarf \
		out=expr.def srcfile $(EXPRLIST) 2>&1 ` ; \
	( echo $$res | egrep -v 'no copies of pattern' ) || :

stamp-exprini : expr.def snarf.tpl
	HOME=/dev/null SHELL=ksh $(AGEXE) expr.def
	@date > $@

autogen.texi : opts.def AutoGen ../getdefs/getdefs ../columns/columns
	top_builddir=$(top_builddir) \
	$(AGEXE) -L ${top_srcdir}/autoopts -Taginfo -DLEVEL=chapter \
		-bautogen -DDEBUG=1 ./opts.def
 
autogen.1 : opts.def AutoGen
	$(AGEXE) -L ${top_srcdir}/autoopts -Tagman1 -bautogen ./opts.def

gen : $(STAMPS) defParse.h autogen.texi autogen.1

../getdefs/getdefs :
	cd ../getdefs ; $(MAKE)

../columns/columns :
	cd ../columns ; $(MAKE)

all : autogen.texi autogen.1

../snprintfv/snprintfv.h :
	@if [ ! -L ../snprintfv/snprintfv.h ] ; then cd ../snprintfv ; \
	ln -s $(top_srcdir)/snprintfv/snprintfv/snprintfv.h . ; fi

# Makefile.am ends here
