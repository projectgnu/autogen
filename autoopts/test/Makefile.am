##  -*- Mode: Makefile -*- 
## ---------------------------------------------------------------------
## Makefile.am -- process this file with automake to produce Makefile.in
##
## Author:          Bruce Korb <autogen@linuxbox.com>
## Maintainer:      Bruce Korb <autogen@linuxbox.com>
## Created:         Fri July 18
## Last Modified:   Feb 27, 2000
##            by:   Bruce Korb <autogen@linuxbox.com>			      
## ---------------------------------------------------------------------
## $Id: Makefile.am,v 1.7 2000/03/16 02:29:59 bruce Exp $
## ---------------------------------------------------------------------

TESTS   = argument.test errors.test shell.test vers.test

EXTRA_DIST = defs stdopts.def $(TESTS)

MAINTAINERCLEANFILES	= Makefile.in

testsubdir = ./testdir

TESTS_ENVIRONMENT = CC="$(CC)" \
	AGEN="`cd $(top_builddir)/agen5 ; pwd`/AutoGen" \
	LIB="`cd $(top_builddir)/autoopts/.libs/; pwd`/libopts.a" \
	testsubdir=$(testsubdir) srcdir=`cd $(srcdir) ; pwd` \
	top_srcdir=`cd $(top_srcdir) ; pwd`

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
