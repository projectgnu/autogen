##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <bkorb@gnu.org>
## Maintainer:      Bruce Korb <bkorb@gnu.org>
## Created:         Fri July 18
## Last Modified:   Thu Jul 29 13:42:18 1999				      
##            by:   Bruce Korb <bkorb@gnu.org>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.7 2000/04/04 13:23:32 bkorb Exp $
## ---------------------------------------------------------------------

TESTS      = subblock.test
EXTRA_DIST = defs $(TESTS)

MAINTAINERCLEANFILES	= Makefile.in

TESTS_ENVIRONMENT = GETDEFS=`cd $(top_builddir)/getdefs; pwd`/getdefs \
	testsubdir=$(testsubdir) srcdir=$(srcdir) \
	top_srcdir=$(top_srcdir)

testsubdir = ./testdir

distclean-local:
	-rm -rf $(testsubdir) FAILURES

check : perm-stamp

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test

# Makefile.am ends here
