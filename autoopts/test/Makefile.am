##  -*- Mode: Makefile -*-
##
## Makefile.am
##
## Time-stamp:      "2007-04-29 12:42:33 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Last Modified:   $Date: 2007/06/23 20:19:39 $
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.14 2007/06/23 20:19:39 bkorb Exp $
## ---------------------------------------------------------------------

EXTRA_DIST = defs.in stdopts.def $(TESTS)
TESTS      = nested.test \
    argument.test cond.test    config.test enums.test   equiv.test \
    errors.test   getopt.test  guile.test  handler.test immediate.test \
    keyword.test  library.test main.test   nls.test     rc.test \
    shell.test    stdopts.test usage.test  vers.test

testsubdir = ./testdir

TESTS_ENVIRONMENT = set -a ; \
	CFLAGS='$(DEFS) $(CFLAGS)' CPPFLAGS='$(CPPFLAGS)' \
	testsubdir=$(testsubdir)
#`
distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

perm-stamp :
	@-cd $(srcdir) ; chmod +x *.test 2>/dev/null

verbose :
	rm -rf FAILURES testdir ; \
	VERBOSE=true $(MAKE) check TESTS="$(TESTS)"

# Makefile.am ends here
