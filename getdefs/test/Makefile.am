##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Time-stamp:      "2007-03-24 14:22:49 bkorb"
## Last Committed:  $Date: 2007/06/23 20:19:39 $
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.4 2007/06/23 20:19:39 bkorb Exp $
## ---------------------------------------------------------------------

TESTS      = cfg.test index.test option.test subblock.test
EXTRA_DIST = defs $(TESTS)
testsubdir = ./testdir

TESTS_ENVIRONMENT = top_srcdir=$(top_srcdir) \
	testsubdir=$(testsubdir) srcdir=$(srcdir) \
	GDexe=$(GDexe) AGexe=$(AGexe) CLexe=$(CLexe)

distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test

# Makefile.am ends here
