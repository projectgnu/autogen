##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18
## Last Modified:   $Date: 2001/12/26 20:07:57 $
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.2 2001/12/26 20:07:57 bkorb Exp $
## ---------------------------------------------------------------------

SHELL_TESTS = define.test directives.test error.test expr.test extract.test \
	include.test opts.test output.test snarf.test suffix.test shell.test

NOSHELL_TESTS = alist.test case.test defref.test dynref.test endmac.test \
	for.test forfrom.test get.test gperf.test heredef.test html.test in.test \
	license.test match.test reorder.test stack.test string.test

TESTS = @AGEN5_TESTS@

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
