##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2004-10-30 13:07:53 bkorb"
## Copyright (c):   1997-2004 by Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18 1997
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 3.14 2004/10/30 20:43:56 bkorb Exp $
## ---------------------------------------------------------------------

SHELL_TESTS = define.test directives.test error.test expr.test extract.test \
	include.test opts.test output.test snarf.test suffix.test shell.test

NOSHELL_TESTS = \
  alist.test   case.test                 defref.test  dynref.test  \
  endmac.test  for.test     forfrom.test forin.test   format.test  get.test  \
  gperf.test   heredef.test html.test    in.test      license.test line.test \
  loop.test    make.test    match.test   reorder.test stack.test   stress.test \
  string.test
UNREADY_TESTS = daemon.test
MAINTAINERCLEANFILES = Makefile.in

TESTS      = @AGEN5_TESTS@
testsubdir = ./testdir
EXTRA_DIST = defs $(TESTS) daemon.test
FUNCLIST   = funcCase.c  funcDef.c  funcEval.c   funcFor.c   funcIf.c \
		     functions.c
EXPRLIST   = expFormat.c expGuile.c expOutput.c  expPrint.c  expState.c \
		     expString.c expGperf.c expExtract.c $(FUNCLIST)

TESTS_ENVIRONMENT = \
	CC="$(CC)" CFLAGS='@CFLAGS@' \
	AGexe=$(AGexe) GDexe=$(GDexe) CLexe=$(CLexe) \
	INC="$(INCLIST) $(CPPFLAGS)" \
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
