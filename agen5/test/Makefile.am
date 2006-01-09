##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Time-stamp:      "2006-01-08 19:14:52 bkorb"
## Copyright (c):   1997-2005 by Bruce Korb
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18 1997
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.7 2006/01/09 03:35:12 bkorb Exp $
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

TESTS       = @AGEN5_TESTS@
EXTRA_DIST  = $(TESTS) daemon.test
FUNCLIST    = funcCase.c funcDef.c funcEval.c funcFor.c funcIf.c functions.c
EXPRLIST    = expFormat.c expGuile.c expOutput.c  expPrint.c  expState.c \
	      expString.c expGperf.c expExtract.c $(FUNCLIST)

TESTS_ENVIRONMENT = \
	COMPILE='$(COMPILE)' CFLAGS='$(CFLAGS)' LDFLAGS='$(LDFLAGS)' \
	EXPRLIST="$(EXPRLIST)" SHELL=$(SHELL)

distclean-local:
	-rm -rf testdir FAILURES

$(TESTS)    : defs

defs        : ${top_builddir}/autoopts/test/defs
	@set -x ; \
	sed -e '/^srcdir=/s@".*"@"$(srcdir)"@' \
	    -e '/^top_srcdir=/s@".*"@"$(top_srcdir)"@' \
	   ${top_builddir}/autoopts/test/defs > $@ ; \
	cd $(srcdir) ; chmod +x *.test || :

${top_builddir}/autoopts/test/defs :
	cd ${top_builddir}/autoopts/test ; $(MAKE) defs

verbose     : defs
	rm -rf FAILURES testdir ; \
	VERBOSE=true $(MAKE) check TESTS=$(TESTS)

# Makefile.am ends here
