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
## $Id: Makefile.am,v 1.13 2000/10/17 19:46:15 bkorb Exp $
## ---------------------------------------------------------------------

TESTS   = argument.test errors.test guile.test keyword.test \
	rc.test shell.test vers.test

EXTRA_DIST = defs stdopts.def $(TESTS)

MAINTAINERCLEANFILES	= Makefile.in

testsubdir = ./testdir

TESTS_ENVIRONMENT = CC="$(CC)" \
	AGEN="`cd $(top_builddir)/agen5 ; pwd`/@AGEXE@" \
	LIB="`cd $(top_builddir)/autoopts/.libs/; \
	      echo \`pwd\`/libopts.a \`pwd\`/libguileopts.a `" \
	INC="-I`cd $(top_builddir)/autoopts; pwd`" \
	testsubdir=$(testsubdir) srcdir=`cd $(srcdir) ; pwd` \
	top_srcdir=`cd $(top_srcdir) ; pwd`

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
