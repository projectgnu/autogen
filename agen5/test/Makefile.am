##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18
## Last Modified:   Feb 27, 2000
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.17 2000/04/04 13:21:58 bkorb Exp $
## ---------------------------------------------------------------------

TESTS      = \
	alist.test       case.test        define.test      defref.test   \
	directives.test  endmac.test      error.test       expr.test     \
	for.test         forfrom.test     include.test     match.test    \
	nestmac.test     opts.test        reorder.test

EXTRA_DIST = defs $(TESTS)

MAINTAINERCLEANFILES	= Makefile.in

FUNCLIST =	funcCase.c  funcDef.c   funcEval.c  funcFor.c   funcIf.c \
		functions.c
EXPRLIST =	expFormat.c expGuile.c  expOutput.c expPrint.c  expState.c \
		expString.c $(FUNCLIST)

testsubdir = ./testdir

TESTS_ENVIRONMENT = \
	GDEXE=`cd $(top_builddir)/getdefs; pwd`/@GDEXE@ \
	AGEXE=`cd $(top_builddir)/agen5  ; pwd`/@AGEXE@ \
	testsubdir=$(testsubdir) \
	top_srcdir=`cd $(top_srcdir) ; pwd` srcdir=`cd $(srcdir) ; pwd` \
	top_builddir=`cd $(top_builddir) ; pwd` builddir=`cd $(builddir) ; pwd` \
	EXPRLIST="$(EXPRLIST)" SHELL=$(SHELL)

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
