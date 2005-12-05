##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2005-12-05 10:01:35 bkorb"
## Copyright (c):   1997-2005 by Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18 1997
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.5 2005/12/05 20:46:43 bkorb Exp $
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
testsubdir = testdir
EXTRA_DIST = defs $(TESTS) daemon.test
FUNCLIST   = funcCase.c  funcDef.c  funcEval.c   funcFor.c   funcIf.c \
		     functions.c
EXPRLIST   = expFormat.c expGuile.c expOutput.c  expPrint.c  expState.c \
		     expString.c expGperf.c expExtract.c $(FUNCLIST)

TESTS_ENVIRONMENT = \
	COMPILE='$(COMPILE)' CFLAGS='$(CFLAGS)' LDFLAGS='$(LDFLAGS)' \
	testsubdir=$(testsubdir) \
	EXPRLIST="$(EXPRLIST)" SHELL=$(SHELL)

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

tests-current :
	if test -f .sdir ; then for f in $(TESTS) ; do \
	cp `cat .sdir`/$$f $$f ; done ; fi

verbose : tests-current
	rm -rf FAILURES $(testsubdir) ; \
	VERBOSE=true $(MAKE) check TESTS=$(TESTS)

# Makefile.am ends here
