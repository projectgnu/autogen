##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18
## Last Modified:   $Date: 2001/07/22 20:03:56 $
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.25 2001/07/22 20:03:56 bkorb Exp $
## ---------------------------------------------------------------------

TESTS      = \
	alist.test      case.test    define.test  defref.test \
	directives.test dynref.test  endmac.test  error.test \
	expr.test       extract.test for.test     forfrom.test \
	get.test        gperf.test   heredef.test include.test \
	license.test    match.test   nestmac.test opts.test \
	output.test     reorder.test snarf.test   stack.test \
	string.test     suffix.test  shell.test   in.test

EXTRA_DIST = defs $(TESTS)

MAINTAINERCLEANFILES = Makefile.in

FUNCLIST = funcCase.c  funcDef.c   funcEval.c  funcFor.c   funcIf.c \
		functions.c
EXPRLIST = expFormat.c expGuile.c  expOutput.c expPrint.c  expState.c \
		expString.c expGperf.c expExtract.c $(FUNCLIST)

testsubdir = ./testdir

TESTS_ENVIRONMENT = \
	CC="$(CC)" CFLAGS='@CFLAGS@' \
	GDEXE=`cd $(top_builddir)/getdefs; pwd`/@GDEXE@ \
	AGEXE=`cd $(top_builddir)/agen5  ; pwd`/@AGEXE@ \
	COLUMNS_EXE="`cd $(top_builddir)/columns ; pwd`/@COLEXE@" \
	INC="@INCLIST@ $(GUILE_INC)" \
	testsubdir=$(testsubdir) \
	top_builddir=`cd $(top_builddir) ; pwd` \
	builddir=`cd $(builddir) ; pwd` \
	srcdir=`pwd` \
	top_srcdir=`cd $(top_srcdir) ; pwd` \
	EXPRLIST="$(EXPRLIST)" SHELL=$(SHELL)

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
