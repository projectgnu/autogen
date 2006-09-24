##  -*- Mode: Makefile -*-
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Time-stamp:      "2006-09-23 19:39:02 bkorb"
## Last Committed:  $Date: 2006/09/24 02:57:02 $
##            by: bkorb
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 4.3 2006/09/24 02:57:02 bkorb Exp $
## ---------------------------------------------------------------------

TESTS      = cfg.test index.test option.test subblock.test
EXTRA_DIST = defs $(TESTS)

MAINTAINERCLEANFILES	= Makefile.in

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
