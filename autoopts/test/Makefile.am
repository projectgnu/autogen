##  -*- Mode: Makefile -*- 
##
## Makefile.am
##
## Time-stamp:      "2005-12-05 11:02:51 bkorb"
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Last Modified:   $Date: 2005/12/05 20:46:44 $
##
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.9 2005/12/05 20:46:44 bkorb Exp $
## ---------------------------------------------------------------------

EXTRA_DIST = defs stdopts.def $(TESTS)
MAINTAINERCLEANFILES	= Makefile.in
TESTS      = nested.test \
    argument.test cond.test   config.test enums.test   equiv.test \
    errors.test   getopt.test guile.test  handler.test immediate.test \
    keyword.test  main.test   nls.test    rc.test      shell.test \
    stdopts.test  usage.test  vers.test

testsubdir = ./testdir

TESTS_ENVIRONMENT = set -a ; \
	CFLAGS='$(DEFS) $(CFLAGS)' CPPFLAGS=$(CPPFLAGS) \
	testsubdir=$(testsubdir)
#`
distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

verbose :
	rm -rf FAILURES testdir ; \
	VERBOSE=true $(MAKE) check TESTS=$(TESTS)

# Makefile.am ends here
