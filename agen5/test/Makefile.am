##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <autogen@linuxbox.com>
## Maintainer:      Bruce Korb <autogen@linuxbox.com>
## Created:         Fri July 18
## Last Modified:   Feb 27, 2000
##            by:   Bruce Korb <autogen@linuxbox.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.16 2000/03/31 05:03:54 bruce Exp $
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

TESTS_ENVIRONMENT = GETDEFS=`cd $(top_builddir)/getdefs; pwd`/getdefs \
	AGEN=`cd $(top_builddir)/agen5 ; pwd`/AutoGen \
	testsubdir=$(testsubdir) srcdir=`cd $(srcdir) ; pwd` \
	top_srcdir=`cd $(top_srcdir) ; pwd` \
	EXPRLIST="$(EXPRLIST)"

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
