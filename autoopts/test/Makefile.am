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
## $Id: Makefile.am,v 1.8 2000/04/04 13:22:19 bkorb Exp $
## ---------------------------------------------------------------------

TESTS   = argument.test errors.test shell.test vers.test

EXTRA_DIST = defs stdopts.def $(TESTS)

MAINTAINERCLEANFILES	= Makefile.in

testsubdir = ./testdir

TESTS_ENVIRONMENT = CC="$(CC)" \
	AGEN="`cd $(top_builddir)/agen5 ; pwd`/@AGEXE@" \
	LIB="`cd $(top_builddir)/autoopts/.libs/; pwd`/libopts.a" \
	testsubdir=$(testsubdir) srcdir=`cd $(srcdir) ; pwd` \
	top_srcdir=`cd $(top_srcdir) ; pwd`

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
