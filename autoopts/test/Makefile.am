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
## $Id: Makefile.am,v 1.19 2001/10/13 22:16:33 bkorb Exp $
## ---------------------------------------------------------------------

TESTS   = argument.test cond.test errors.test guile.test handler.test \
	keyword.test rc.test shell.test stdopts.test vers.test

EXTRA_DIST = defs stdopts.def $(TESTS)

MAINTAINERCLEANFILES	= Makefile.in

testsubdir = ./testdir

TESTS_ENVIRONMENT = \
	CC="$(CC)" \
	CFLAGS='@CFLAGS@' \
	PATH="`cd $(top_builddir)/columns ; pwd`:$$PATH" \
	AGEN="`cd $(top_builddir)/agen5 ; pwd`/@AGEXE@" \
	COLUMNS_EXE="`cd $(top_builddir)/columns ; pwd`/@COLEXE@" \
	LIB="`cd $(top_builddir)/autoopts/.libs/; \
	echo \`pwd\`/libopts.a \`pwd\`/libguileopts.a ` $(LIBS)" \
	INC="@INCLIST@ $(GUILE_INC)" \
	testsubdir=$(testsubdir) \
	srcdir=`pwd` \
	top_srcdir=`cd $(top_srcdir) ; pwd`

distclean-local:
	-rm -rf $(testsubdir) FAILURES

$(TESTS) : perm-stamp

perm-stamp :
	cd $(srcdir) ; chmod +x *.test || :

# Makefile.am ends here
